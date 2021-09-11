# genocon2021-docker

本リポジトリでは、ジャッジプログラム(`eval.c`)と Multiple Sequence Alignment (MSA) 変換プログラム(`decode_cigar.py`)を同梱した Docker イメージを提供しています。
また、サンプル解答プログラム(`sample_solver.py`)もあわせて提供していますので、ご活用ください。

[![dockeri.co](https://dockeri.co/image/exkazuu/genocon2021)](https://hub.docker.com/r/exkazuu/genocon2021)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat-square)](http://makeapullrequest.com)

[English version](https://github.com/exKAZUu/genocon2021-docker/blob/main/README-en.md) is also available.

## 必要なソフトウェア

- Windows をご利用の方
  - [Windows Subsystem for Linux 2 (WSL2)](https://docs.microsoft.com/ja-jp/windows/wsl/install-win10)
  - [Docker Desktop for Windows WSL 2 バックエンド](https://docs.docker.jp/docker-for-windows/wsl.html)
  - **注意：本ドキュメントは、WSL2 の Ubuntu 上でコマンドを実行していただくことを想定して作成しております。**
- MacOS をご利用の方
  - [Docker Desktop for Mac](https://docs.docker.jp/docker-for-mac/install.html)
- Linux をご利用の方
  - [Docker](https://docs.docker.jp/linux/index.html)

## あなたのプログラムが出力した結果を評価する方法

### 非常に小さなサンプルデータで評価する方法

1. サンプルデータを用意します。

   ```
   cd <genocon2021用のあなたのワーキングディレクトリ>

   mkdir -p data/very-small-sample
   echo "ACACAGCGGCGACC\nACACAGCGG-GACC" > data/very-small-sample/answer.txt
   echo "ACACAGCGGCGACC\nACACAGCGGGACC" > data/very-small-sample/output.txt
   echo "ACACAGCGGCGACC\n> 0 3=1XT10=\n< 0 14=\n< 0 9=1D4=\n< 0 14=\n< 0 14=\n< 0 9=1D4=\n=\n" > data/very-small-sample/testcase.txt
   ```

2. ジャッジプログラムが正常に動作することを確認します。

   ```
   docker run -it --rm -v $(pwd)/data:/app/data exkazuu/genocon2021 ./eval data/very-small-sample/answer.txt data/very-small-sample/output.txt
   ```

   上記コマンドを実行すると、以下の採点結果が表示されます。

   ```
   raw_score: 27
   final_score (raw_score / 100): 0
   ```

3. `data/very-small-sample/testcase.txt` に対してサンプル解答プログラムを実行して、 `data/very-small-sample/sample-output.txt` を生成します。

   - あなたの PC 上で Python 3 を実行する場合

     ```
     python3 sample_solver.py data/very-small-sample/testcase.txt > data/very-small-sample/sample-output.txt
     ```

   - Docker コンテナ上で Python 3 を実行する場合

     ```
     docker run -it --rm -v $(pwd)/data:/app/data exkazuu/genocon2021 python3 sample_solver.py data/very-small-sample/testcase.txt > data/very-small-sample/sample-output.txt
     ```

4. ジャッジプログラムを実行して得られた出力データを採点します。

   ```
   docker run -it --rm -v $(pwd)/data:/app/data exkazuu/genocon2021 ./eval data/very-small-sample/answer.txt data/very-small-sample/sample-output.txt
   ```

   上記コマンドを実行すると、以下の採点結果が表示されます。

   ```
   raw_score: -72
   final_score (raw_score / 100): 0
   ```

5. `data/very-small-sample/testcase.txt` に対してあなたが作成した解答プログラムを実行して、 `data/very-small-sample/your-output.txt` を生成します。

6. ジャッジプログラムを実行してあなたのプログラムの出力データを採点します。

   ```
   docker run -it --rm -v $(pwd)/data:/app/data exkazuu/genocon2021 ./eval data/very-small-sample/answer.txt data/very-small-sample/your-output.txt
   ```

### 公開中のサンプルデータ (`src_genocon2021.tar.gz`) で評価する方法

1. `data` ディレクトリを作成します。

   ```
   cd <genocon2021用のあなたのワーキングディレクトリ>

   mkdir -p data
   ```

2. `src_genocon2021.tar.gz` をダウンロードします。詳細は https://atcoder.jp/contests/genocon2021 に記載されている 2021/9/11 のトピックを御覧ください。

3. `src_genocon2021.tar.gz` を解凍します。 (例: `tar -xvf src_genocon2021.tar.gz`)

4. `src_genocon2021/dat` ディレクトリを手順 1 で作成した `data` ディレクトリの中に移動します。 (例: `mv src_genocon2021/dat <genocon2021用のあなたのワーキングディレクトリ>/data/`)

5. ジャッジプログラムが正常に動作することを確認します。

   ```
   docker run -it --rm -v $(pwd)/data:/app/data exkazuu/genocon2021 ./eval data/dat/gen1_small_10.answer.txt data/dat/gen1_small_10.output.txt
   ```

   上記コマンドを実行すると、以下の採点結果が表示されます。

   ```
   raw_score: 59948
   final_score (raw_score / 100): 599
   ```

6. `data/dat/gen1_small_10.testcase.txt` に対して **あなたが作成した解答プログラム** を実行して、 `data/dat/gen1_small_10.output.txt` を生成します。

   - あなたの PC 上で Python 3 を実行する場合

     ```
     python3 sample_solver.py data/dat/gen1_small_10.testcase.txt > data/dat/sample-output.txt
     ```

   - Docker コンテナ上で Python 3 を実行する場合

     ```
     docker run -it --rm -v $(pwd)/data:/app/data exkazuu/genocon2021 python3 sample_solver.py data/dat/gen1_small_10.testcase.txt > data/dat/sample-output.txt
     ```

7. ジャッジプログラムを実行して得られた出力データを採点します。

   ```
   docker run -it --rm -v $(pwd)/data:/app/data exkazuu/genocon2021 ./eval data/dat/gen1_small_10.answer.txt data/dat/sample-output.txt
   ```

   上記コマンドを実行すると、以下の採点結果が表示されます。

   ```
   raw_score: 44560
   final_score (raw_score / 100): 445
   ```

8. `data/dat/gen1_small_10.testcase.txt` に対して **あなたが作成した解答プログラム** を実行して、 `data/dat/gen1_small_10.output.txt` を生成します。

9. ジャッジプログラムを実行してあなたのプログラムの出力データを採点します。

   ```
   docker run -it --rm -v $(pwd)/data:/app/data exkazuu/genocon2021 ./eval data/dat/gen1_small_10.answer.txt data/dat/your-output.txt
   ```

## `testcase.txt` を Multiple Sequence Alignment (MSA) に変換する方法

1. `docker run -it --rm -v $(pwd)/data:/app/data exkazuu/genocon2021 python3 decode_cigar.py data/very-small-sample/testcase.txt > msa.txt`

2. `msa.txt` を開きます。 (例: `less msa.txt`)

## 本リポジトリのメンテナの方向けの情報

### コードのフォーマット

```
clang-format -i eval.c && npx prettier -w README*.md
```

### Docker イメージのデプロイ

```
docker build -t exkazuu/genocon2021 . && docker push exkazuu/genocon2021
```

## ライセンス

BSD 3-Clause License
