# genocon2021-docker

A docker image where you can run a judge program (`eval.c`) and multiple sequence alignment converter (`decode_cigar.py`).

## Requirements

- For Windows
  - [Windows Subsystem for Linux 2 (WSL2)](https://docs.microsoft.com/ja-jp/windows/wsl/install-win10)
  - [Docker Desktop for Windows WSL 2 バックエンド](https://docs.docker.jp/docker-for-windows/wsl.html)
- For MacOS
  - [Docker Desktop for Mac](https://docs.docker.jp/docker-for-mac/install.html)
- For Linux
  - [Docker](https://docs.docker.jp/linux/index.html)

Note: If you use Windows, this document expects you run all the commands on terminal of Ubuntu on WSL2.

## How to Evaluate Your Output on Docker

### For Very Small Sample Data

1. Prepare very small sample data

   ```
   cd <your working directory for genocon2021>

   mkdir -p data/very-small-sample
   echo "ACACAGCGGCGACC\nACACAGCGG-GACC" > data/very-small-sample/answer.txt
   echo "ACACAGCGGCGACC\nACACAGCGGGACC" > data/very-small-sample/output.txt
   echo "ACACAGCGGCGACC\n> 0 3=1XT10=\n< 0 14=\n< 0 9=1D4=\n< 0 14=\n< 0 14=\n< 0 9=1D4=\n=\n" > data/very-small-sample/testcase.txt
   ```

2. Confirm the judge program works well with the sample output data

   ```
   docker run -it --rm -v $(pwd)/data:/app/data exkazuu/genocon2021 ./eval data/very-small-sample/answer.txt data/very-small-sample/output.txt
   ```

3. Run YOUR program with `data/very-small-sample/testcase.txt` to generate `data/very-small-sample/your-output.txt`

4. Run the judge program with YOUR output data

   ```
   docker run -it --rm -v $(pwd)/data:/app/data exkazuu/genocon2021 ./eval data/very-small-sample/answer.txt data/very-small-sample/your-output.txt
   ```

### For Small Sample Data

1. Create `data` directory

   ```
   cd <your working directory for genocon2021>

   mkdir -p data
   ```

2. Download `src_genocon2021.tar.gz` (Please see the topic on 2021/9/11 in https://atcoder.jp/contests/genocon2021)

3. Extract `src_genocon2021.tar.gz` (e.g. `tar -xvf src_genocon2021.tar.gz`)

4. Move 'src_genocon2021/dat' directory into `data` directory (e.g. `mv src_genocon2021/dat <your working directory for genocon2021>/data/`)

5. Confirm the judge program works well with the sample output data

   ```
   docker run -it --rm -v $(pwd)/data:/app/data exkazuu/genocon2021 ./eval src_genocon2021/dat/gen1_small_10.answer.txt src_genocon2021/dat/your- output.txt
   ```

6. Run YOUR program with `data/dat/gen1_small_10.testcase.txt` to generate `data/dat/your-output.txt`

7. Run the judge program with YOUR output data

   ```
   docker run -it --rm -v $(pwd)/data:/app/data exkazuu/genocon2021 ./eval data/dat/gen1_small_10.answer.txt data/dat/your-output.txt
   ```

## How to Convert `testcase.txt` into Multiple Sequence Alignment (MSA)

1. `docker run -it --rm -v $(pwd)/data:/app/data exkazuu/genocon2021 python3 decode_cigar.py data/very-small-sample/testcase.txt > msa.txt`

2. Open `msa.txt` (e.g. `less msa.txt`)

## LICENSE

BSD 3-Clause License
