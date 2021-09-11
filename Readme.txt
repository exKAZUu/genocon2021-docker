1. For judging your output

Compile:
gcc -O3 -o eval eval.c

Run:
eval <correct_answer_file> <user_answer_file>

Example:
./eval ./dat_safe/art.answer.txt ./dat_safe/art.output.txt

Expected output of the example:
raw_score: 27
final_score: 0

About output:
raw_score and final_score are shown
final_score is equal to the judge point and it is calculated by MAX(raw_score, 0)/100.

2. For decoding input format to multiple sequence alignment (MSA)

Format of MSA:
Sequence name1: Sequence1
Sequence name2: Sequence2
...

Run:
decode_cigar.py <input_file>

Example:
decode_cigar.py ./dat_safe/art.testcase.txt 

An simple idea to see MSA:
decode_cigar.py ./dat_safe/art.testcase.txt > ./dat_safe/art.msa
less -S ./dat_safe/art.msa
