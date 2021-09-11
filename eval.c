/*
 * eval.c
 * Copyright 2021 Shimizu Lab. at Waseda University
 * License: BSD 3-Clause License
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define error(fmt, ...)                                                        \
  fprintf(stderr, "%s(%d): " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define SAFE_FREE(buf)                                                         \
  if (buf) {                                                                   \
    free(buf);                                                                 \
    buf = NULL;                                                                \
  }

#define BAND_FOR_PENALTY 300
#define PENALTY -99
#define SWITCH_PENALTY -750
#define MATCH(x, y) ((x == y) ? 1 : -99)
#define GMATCH(x, y) ((x == y) ? 0 : -99)

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MAX3(a, b, c) MAX(MAX(a, b), c)
#define MIN(a, b) ((a) > (b) ? (b) : (a))

#define MATBUF_IDX(i, j, bandwidth) ((j) * (bandwidth - 1) + (i) + 1)
#define M(buf, i, j, bandwidth) (buf)[MATBUF_IDX(i, j, bandwidth)]
#define M_REV(buf, i, j, bandwidth, slen, tlen)                                \
  (buf)[MATBUF_IDX((slen)-1 - (i), (tlen)-1 - (j), bandwidth)]

int read_file(char *fname, char **buf, char **s1, char **s2) {
  FILE *fp = NULL;
  struct stat file_stat;
  int rslt;
  int retval = 0;
  char *p;
  size_t sz;
  int i, j;

  *buf = NULL;
  fp = fopen(fname, "r");
  if (fp == NULL) {
    error("cannot open %s", fname);
    retval = -1;
    goto END;
  }
  rslt = fstat(fileno(fp), &file_stat);
  if (rslt < 0) {
    error("fstat failed");
    retval = -1;
    goto END;
  }
  if (file_stat.st_size == 0) {
    error("%s is empty file", fname);
    retval = -1;
    goto END;
  }

  *buf = (char *)malloc(file_stat.st_size);
  sz = fread(*buf, 1, file_stat.st_size, fp);
  if (sz != (size_t)file_stat.st_size) {
    error("fread failed");
    retval = -1;
    goto END;
  }
  for (i = 0, j = 0; i < file_stat.st_size; i++) {
    if ((*buf)[i] != '\r')
      (*buf)[j++] = (*buf)[i];
  }
  sz = j;

  *s1 = *buf;
  p = (char *)memchr(*buf, '\n', sz);
  if (p == NULL) {
    error("invalid format");
    retval = -1;
    goto END;
  }

  *p++ = '\0';

  *s2 = p;
  p = (char *)memchr(p, '\n', sz - (size_t)(p - *buf));
  if (p == NULL) {
    error("invalid format");
    retval = -1;
    goto END;
  }
  *p = '\0';

END:
  if (retval < 0) {
    SAFE_FREE(*buf);
  }
  if (fp) {
    fclose(fp);
  }
  return retval;
}

int generate_score_matrix(char *s, char *t, int **buf, int bw, int reverse) {
  size_t slen, tlen;
  size_t i, j;
  int start_of_i, end_of_i;
  int topleft_score, top_score, left_score;

  *buf = NULL;
  slen = strlen(s);
  tlen = strlen(t);

  if (MAX(slen, tlen) - MIN(slen, tlen) > (size_t)bw / 2) {
    return INT_MIN;
  }

  *buf = (int *)malloc(tlen * bw * sizeof(int));
  // printf("s: %s(%ld)\n", s, slen);
  // printf("t: %s(%ld)\n", t, tlen);
  for (j = 0; j < tlen; j++) {
    start_of_i = j - bw / 2;
    if (start_of_i < 0) {
      start_of_i = 0;
    }
    end_of_i = j + bw / 2;
    if ((size_t)end_of_i >= slen) {
      end_of_i = slen - 1;
    }
    for (i = start_of_i; i <= (size_t)end_of_i; i++) {
      if (i == 0 && j == 0) {
        topleft_score = 0;
        left_score = PENALTY;
        top_score = PENALTY;
      } else {
        if (i == 0) {
          topleft_score = j * PENALTY;
          left_score = (j + 1) * PENALTY;
          top_score = M(*buf, i, j - 1, bw);
        } else {
          if (j == 0) {
            topleft_score = i * PENALTY;
            left_score = M(*buf, i - 1, j, bw);
            top_score = (i + 1) * PENALTY;
          } else {
            topleft_score = M(*buf, i - 1, j - 1, bw);
            left_score = M(*buf, i - 1, j, bw);
            top_score = M(*buf, i, j - 1, bw);
          }
        }
      }
#define S(i) (reverse ? s[slen - (i)-1] : s[i])
#define T(j) (reverse ? t[tlen - (j)-1] : t[j])
      topleft_score += MAX(MATCH(S(i), T(j)), GMATCH(S(i), 'X'));
      left_score += MAX(GMATCH(S(i), '-'), GMATCH(S(i), 'X'));
      top_score += PENALTY;
      if (i != 0 && i == j + bw / 2) {
        // upper cell is invalid
        M(*buf, i, j, bw) = MAX(topleft_score, left_score);
      } else if (i == (size_t)start_of_i && i == j - bw / 2) {
        // left cell is invalid
        M(*buf, i, j, bw) = MAX(topleft_score, top_score);
      } else {
        M(*buf, i, j, bw) = MAX3(topleft_score, left_score, top_score);
      }
      // printf("M(%ld,%ld)[%ld]: %d\n", i, j, MATBUF_IDX(i,j,bw), M(*buf, i, j,
      // bw));
    }
  }

  if (reverse) {
    return M(*buf, 0, 0, bw);
  }
  return M(*buf, slen - 1, tlen - 1, bw);
}

int *merge_score_matrix(int *buf1, int *buf2, int slen, int tlen, int bw) {
  int i, j;
  int buf1_start_of_j, buf1_end_of_j;
  int buf2_start_of_j, buf2_end_of_j;
  int sum;
  int max_score;
  int *score_vector;

  score_vector = (int *)malloc(sizeof(int) * (slen + 1));
  score_vector[0] = M_REV(buf2, 0, 0, bw, slen, tlen);
  for (i = 1; i < slen; i++) {
    buf1_start_of_j = i - bw / 2;
    if (buf1_start_of_j < 0) {
      buf1_start_of_j = 0;
    }
    buf1_end_of_j = i + bw / 2;
    if (buf1_end_of_j > tlen - 1) {
      buf1_end_of_j = tlen - 1;
    }
    buf2_start_of_j = tlen - slen + i - bw / 2;
    if (buf2_start_of_j < 0) {
      buf2_start_of_j = 0;
    }
    buf2_end_of_j = tlen - slen + i + bw / 2;
    if (buf2_end_of_j > tlen - 1) {
      buf2_end_of_j = tlen - 1;
    }

    max_score = INT_MIN;
    for (j = MAX(buf1_start_of_j, buf2_start_of_j - 1);
         j <= buf1_end_of_j && j + 1 <= buf2_end_of_j; j++) {
      sum = M(buf1, i - 1, j, bw) + M_REV(buf2, i, j + 1, bw, slen, tlen) +
            SWITCH_PENALTY;
      // printf("i:%d, j:%d, M:%d, M_REV:%d, ",i,j, M(buf1, i-1, j, bw),
      // M_REV(buf2, i, j+1, bw, slen, tlen)); printf("MATBUF_IDX: %d\n",
      // MATBUF_IDX(slen-1-i,tlen-1-j,bw));
      if (max_score < sum) {
        max_score = sum;
      }
    }
    score_vector[i] = max_score;
  }
  score_vector[slen] = M(buf1, slen - 1, tlen - 1, bw);

  return score_vector;
}

int calculate_score(char *s1, char *s2, int slen, char *t1, char *t2, int bw) {
  int rslt;
  int i, sum, final_score;
  int t1len = strlen(t1);
  int t2len = strlen(t2);
  int *matbuf_s1_t1 = NULL;
  int *matbuf_s2r_t1r = NULL;
  int *matbuf_s2_t2 = NULL;
  int *matbuf_s1r_t2r = NULL;
  int *matbuf_s1_t2 = NULL;
  int *matbuf_s2r_t2r = NULL;
  int *matbuf_s2_t1 = NULL;
  int *matbuf_s1r_t1r = NULL;
  int *score_vector1 = NULL;
  int *score_vector2 = NULL;
  int score1, score2;

  rslt = generate_score_matrix(s1, t1, &matbuf_s1_t1, bw, 0);
  if (rslt == INT_MIN) {
    final_score = INT_MIN;
    goto END;
  }
  rslt = generate_score_matrix(s2, t1, &matbuf_s2r_t1r, bw, 1);
  if (rslt == INT_MIN) {
    final_score = INT_MIN;
    goto END;
  }
  score_vector1 =
      merge_score_matrix(matbuf_s1_t1, matbuf_s2r_t1r, slen, t1len, bw);
  SAFE_FREE(matbuf_s1_t1);
  SAFE_FREE(matbuf_s2r_t1r);

  rslt = generate_score_matrix(s2, t2, &matbuf_s2_t2, bw, 0);
  if (rslt == INT_MIN) {
    final_score = INT_MIN;
    goto END;
  }
  rslt = generate_score_matrix(s1, t2, &matbuf_s1r_t2r, bw, 1);
  if (rslt == INT_MIN) {
    final_score = INT_MIN;
    goto END;
  }
  score_vector2 =
      merge_score_matrix(matbuf_s2_t2, matbuf_s1r_t2r, slen, t2len, bw);
  SAFE_FREE(matbuf_s2_t2);
  SAFE_FREE(matbuf_s1r_t2r);

  score1 = INT_MIN;
  for (i = 0; i <= slen; i++) {
    if (score_vector1[i] == INT_MIN || score_vector2[i] == INT_MIN) {
      // printf("score[%d]: INT_MIN\n", i);
      continue;
    }
    sum = score_vector1[i] + score_vector2[i];
    // printf("score[%d]: %d(%d + %d)\n", i, sum, score_vector1[i],
    // score_vector2[i]);
    if (sum > score1) {
      score1 = sum;
    }
  }
  SAFE_FREE(score_vector1);
  SAFE_FREE(score_vector2);

  rslt = generate_score_matrix(s1, t2, &matbuf_s1_t2, bw, 0);
  if (rslt == INT_MIN) {
    final_score = INT_MIN;
    goto END;
  }
  rslt = generate_score_matrix(s2, t2, &matbuf_s2r_t2r, bw, 1);
  if (rslt == INT_MIN) {
    final_score = INT_MIN;
    goto END;
  }
  score_vector1 =
      merge_score_matrix(matbuf_s1_t2, matbuf_s2r_t2r, slen, t2len, bw);
  SAFE_FREE(matbuf_s1_t2);
  SAFE_FREE(matbuf_s2r_t2r);

  rslt = generate_score_matrix(s2, t1, &matbuf_s2_t1, bw, 0);
  if (rslt == INT_MIN) {
    final_score = INT_MIN;
    goto END;
  }
  rslt = generate_score_matrix(s1, t1, &matbuf_s1r_t1r, bw, 1);
  if (rslt == INT_MIN) {
    final_score = INT_MIN;
    goto END;
  }
  score_vector2 =
      merge_score_matrix(matbuf_s2_t1, matbuf_s1r_t1r, slen, t1len, bw);
  SAFE_FREE(matbuf_s2_t1);
  SAFE_FREE(matbuf_s1r_t1r);

  score2 = INT_MIN;
  for (i = 0; i <= slen; i++) {
    if (score_vector1[i] == INT_MIN || score_vector2[i] == INT_MIN) {
      // printf("score[%d]: INT_MIN\n", i);
      continue;
    }
    sum = score_vector1[i] + score_vector2[i];
    // printf("score[%d]: %d(%d + %d)\n", i, sum, score_vector1[i],
    // score_vector2[i]);
    if (sum > score2) {
      score2 = sum;
    }
  }
  SAFE_FREE(score_vector1);
  SAFE_FREE(score_vector2);

  final_score = MAX(score1, score2);

END:
  SAFE_FREE(matbuf_s1_t1);
  SAFE_FREE(matbuf_s2r_t1r);
  SAFE_FREE(matbuf_s2_t2);
  SAFE_FREE(matbuf_s1r_t2r);
  SAFE_FREE(matbuf_s1_t2);
  SAFE_FREE(matbuf_s2r_t2r);
  SAFE_FREE(matbuf_s2_t1);
  SAFE_FREE(matbuf_s1r_t1r);
  SAFE_FREE(score_vector1);
  SAFE_FREE(score_vector2);

  return final_score;
}

int check_input(char *s1, char *s2, char *t1, char *t2) {
  int cnt_s1 = 0, cnt_s2 = 0;
  while (*t1 != '\0') {
    if (*t1 != 'A' && *t1 != 'C' && *t1 != 'G' && *t1 != 'T') {
      error("invalid input.");
      return -1;
    }
    t1++;
  }

  while (*t2 != '\0') {
    if (*t2 != 'A' && *t2 != 'C' && *t2 != 'G' && *t2 != 'T') {
      error("invalid input.");
      return -1;
    }
    t2++;
  }

  while (*s1 != '\0') {
    if (*s1 == '-' || *s1 == 'X') {
      cnt_s1++;
    }
    s1++;
  }

  while (*s2 != '\0') {
    if (*s2 == '-' || *s1 == 'X') {
      cnt_s2++;
    }
    s2++;
  }

  return MAX(cnt_s1, cnt_s2);
}

int main(int argc, char **argv) {
  int rslt;
  char *sbuf = NULL, *s1, *s2;
  size_t slen;
  char *tbuf = NULL, *t1, *t2;
  int *matbuf = NULL;
  int bandwidth = 3;
  int mode = 1;
  int final_score, score1, score2;

  if (argc < 3) {
    // error("USAGE: %s <correct_answer_file> <user_answer_file> <bandwidth>",
    // argv[0]);
    error("USAGE: %s <correct_answer_file> <user_answer_file>", argv[0]);
    goto END;
  }

  rslt = read_file(argv[1], &sbuf, &s1, &s2);
  if (rslt < 0) {
    error("read_file failed");
    goto END;
  }

  rslt = read_file(argv[2], &tbuf, &t1, &t2);
  if (rslt < 0) {
    error("read_file failed");
    goto END;
  }
  /**
  if(argc >= 4){
      bandwidth = atoi(argv[3]);
  }
  if(bandwidth % 2 != 1){
      error("bandwidth must be odd");
      goto END;
  }
  if(argc >= 5){
      mode = atoi(argv[4]);
  }
  **/
  slen = strlen(s1);
  if (slen != strlen(s2)) {
    error("strlen(s1) != strlen(s2)");
    goto END;
  }

  rslt = check_input(s1, s2, t1, t2);
  if (rslt >= 0) {
    bandwidth = (rslt + BAND_FOR_PENALTY) * 2 + 1;
    // printf("band: %d\n", bandwidth);
  } else {
    goto END;
  }

  if (mode == 0) {
    // no switch
    score1 = generate_score_matrix(s1, t1, &matbuf, bandwidth, 0);
    SAFE_FREE(matbuf);

    rslt = generate_score_matrix(s2, t2, &matbuf, bandwidth, 0);
    if (score1 == INT_MIN || rslt == INT_MIN) {
      score1 = INT_MIN;
    } else {
      score1 += rslt;
    }
    printf("score1 = %d\n", score1);
    SAFE_FREE(matbuf);

    score2 = generate_score_matrix(s1, t2, &matbuf, bandwidth, 0);
    SAFE_FREE(matbuf);

    rslt = generate_score_matrix(s2, t1, &matbuf, bandwidth, 0);
    if (score2 == INT_MIN || rslt == INT_MIN) {
      score2 = INT_MIN;
    } else {
      score2 += rslt;
    }
    printf("score2 = %d\n", score2);
    SAFE_FREE(matbuf);

    final_score = MAX(score1, score2);
  } else {
    // allow for one switch
    final_score = calculate_score(s1, s2, slen, t1, t2, bandwidth);
  }
  // printf("final_score: %d\n", final_score);
  printf("raw_score: %d\n", final_score);
  printf("final_score (raw_score / 100): %d\n", (MAX(final_score, 0) / 100));

END:
  SAFE_FREE(sbuf);
  SAFE_FREE(tbuf);

  return 0;
}
