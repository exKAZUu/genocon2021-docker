#!/usr/bin/python3
# NOTE: Please use Python 3.x
#       Not compatible with Python 2.x
#
# Copyright 2021 Kasahara Lab. at the University of Tokyo
# License: BSD 3-Clause License

import os
import sys
import re


def error(message):
    print(message, file=sys.stderr)
    sys.exit(2)


def process_one_read_for_checking_insertions(reference_sequence, offset, cigar, mispb):
    i = 0
    rpos = offset
    cigar_len = len(cigar)
    digits_regexp = re.compile(r"(\d+)")
    while i < cigar_len:
        # parse number
        r = digits_regexp.match(cigar[i:])
        if r is None:
            error("Parse error at col %d\nCIGAR = %s\nCS = [%s]" % (i + 1, cigar, cigar[i:]))
        clen = int(r.group(1))
        i += len(r.group(1))
        ctype = cigar[i]
        i += 1
        if ctype == '=':
            rpos += clen
        elif ctype == 'X':
            i += clen
            rpos += clen
        elif ctype == 'I':
            mispb[rpos] = max(mispb[rpos], clen)
            i += clen
        elif ctype == 'D':
            rpos += clen
        else:
            error("Logic error. CIGAR=%s\n" % cigar)


def get_padded_string(reference_sequence, offset, cigar, mispb):
    padded_sequence = [" " * (offset + sum(mispb[0:offset]))]
    i = 0
    rpos = offset
    cigar_len = len(cigar)
    digits_regexp = re.compile(r"(\d+)")
    already_padded = False
    while i < cigar_len:
        # parse number
        r = digits_regexp.match(cigar[i:])
        if r is None:
            error("Parse error at col %d\nCIGAR = %s\nCS = [%s]" % (i + 1, cigar, cigar[i:]))
        clen = int(r.group(1))
        i += len(r.group(1))
        ctype = cigar[i]
        i += 1
        if ctype == '=':
            for j in range(clen):
                if mispb[rpos] > 0 and not (already_padded and j == 0):
                    padded_sequence.append('-' * mispb[rpos])
                padded_sequence.append(reference_sequence[rpos])
                rpos += 1
            already_padded = False
        elif ctype == 'X':
            for j in range(clen):
                if mispb[rpos] > 0 and not (already_padded and j == 0):
                    padded_sequence.append('-' * mispb[rpos])
                padded_sequence.append(cigar[i])
                rpos += 1
                i += 1
            already_padded = False
        elif ctype == 'I':
            padded_sequence.append(cigar[i: i + clen])
            pad_len = mispb[rpos] - clen
            if 0 < pad_len:
                padded_sequence.append('=' * pad_len)
            i += clen
            already_padded = True
        elif ctype == 'D':
            for j in range(clen):
                if mispb[rpos] > 0 and not (already_padded and j == 0):
                    padded_sequence.append('-' * mispb[rpos])
                rpos += 1
            padded_sequence.append('-' * clen)
        else:
            error("Logic error. CIGAR=%s\n" % cigar)
    return "".join(padded_sequence)


def solve(inf):
    """
        Parse one case.
        inf is an input stream
    """
    reference_sequence = inf.readline().strip()
    lines = inf.readlines()
    max_insertion_size_per_base = [0] * len(reference_sequence)
    for line in lines:
        columns = line.rstrip().split(' ')
        if columns[0] == '=': break
        start_offset = int(columns[1])
        cigar_string = columns[2]
        process_one_read_for_checking_insertions(reference_sequence, start_offset, cigar_string, max_insertion_size_per_base)
    # print(max_insertion_size_per_base)
    padded_reference_sequence = get_padded_string(reference_sequence, 0, "%d=" % len(reference_sequence), max_insertion_size_per_base)
    print("REF  : %s" % padded_reference_sequence)
    for i, line in enumerate(lines):
        columns = line.rstrip().split(' ')
        if columns[0] == '=': break
        orientation = columns[0]  # unused
        start_offset = int(columns[1])
        cigar_string = columns[2]
        padded_string = get_padded_string(reference_sequence, start_offset, cigar_string, max_insertion_size_per_base)
        print("%3d %c: %s" % (i, orientation, padded_string))


def main():
    if len(sys.argv) < 2:
        error("Usage: decode_cigar <sample_input_file>")
    input_file_name = sys.argv[1]
    if not os.path.exists(input_file_name):
        error("ERROR: Cannot find input file '%s'" % input_file_name)
    with open(input_file_name, "r") as f:
        solve(f)


if __name__ == "__main__":
    main()
