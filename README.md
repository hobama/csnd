# csnd

Citizen Seismology Network Daemon

## Prerequisites

* build-essential
* cmake (> 2.8.12)
* following dependencies

なお、USB接続のセンサーから読み取るため、csndを動かすユーザーは dialout グループに属している必要がある。

## Dependencies

### common

|  name   | debian (raspbian) |    centos     |          note          |
|:-------:|:-----------------:|:-------------:|:----------------------:|
| spdlog  |  N/A (embedded*)  | spdlog (epel) | *required CMake >= 3.1 |


### Avro-C++

|  name   | debian (raspbian) |    centos     |          note          |
|:-------:|:-----------------:|:-------------:|:----------------------:|
|  boost  | libboost-all-dev  |  boost-devel  | version >= 1.38        |
|  zlib   |     liblz-dev     |  zlib-devel   ||

### Avro-C

|  name   | debian (raspbian) |    centos     |          note          |
|:-------:|:-----------------:|:-------------:|:----------------------:|
|  lzma   |    liblzma-dev    |    xz-libs    ||
|  zlib   |     liblz-dev     |  zlib-devel   ||
| jansson |  libjansson-dev   | jansson-devel |    version >= 2.3      |
| snappy  |  libsnappy-dev    | snappy-devel  ||

### azure-iot-sdk-c

|  name   |   debian (raspbian)  |    centos     |          note          |
|:-------:|:--------------------:|:-------------:|:----------------------:|
| OpenSSL |      libssl-dev      | openssl-devel ||
|  cURL   | libcurl4-openssl-dev | libcurl-devel ||
|  uuid   |       uuid-dev       | libuuid-devel ||

## Build

* git clone https://github.com/sgr/csnd.git
* mkdir <BUILD_DIRECTORY>
* cd <BUILD_DIRECTORY>
* cmake <SOURCE_DIRECTORY>
* make

## Install

1. csnd を配置する場所を決める。
   - csnd を置くディレクトリ
   - 設定ファイルを置くディレクトリ
   - 加速度データおよびイベントデータ出力先 (out_dir)
   - ログファイル出力先 (logging.file_settings.filename)
   - pidファイル出力先 (pid_file)
2. src/csnd を上で決めたディレクトリに配置する。
3. csnd.yml をひな形ファイル doc/csnd.yml.example をもとに上で決めた内容に合わせて作成する。
   - iothub.connection_string は CSN 管理者から発行された接続文字列を設定する。接続文字列がない場合は offline_mode を true に設定し、オフラインモードで使用する。
   - logging.logger を console にする場合は、 -d オプションを用いずフォアグラウンドモードで実行する。
4. csnd.service をひな形ファイル doc/csnd.service.example をもとに上で決めた内容に合わせて作成する。
5. csnd.service を /etc/systemd/system/ にコピーし、 systemctl で有効化する。
   ```(sh)
   # systemctl enable csnd.service
   ```

## Command line options

| option | description|
|------|-------------|
| -c /path/of/csnd.yml | set config file path |
| -d | run on daemon mode |

## Known Issues

* Avro-C シリアライザー (libserializer_avroc.a) は正しく動作しない。 Avro-C++ 版を用いること。
* CentOS では azure-iot-sdk-c のビルドはサブモジュールのクローンが出来ずに失敗する。失敗後手動で `git submodule update --init --recursive` などとして make をやり直す必要がある。
* azure-iot-sdk-c は libcurl を必要とするが、CentOS 7 標準パッケージは OpenSSL を使っていないため、HTTP接続の場合初期化で Segmentation Fault を起こす。curl のソースコードから --with-ssl つきでビルドし、ライブラリの場所を LD_LIBRARY_PATH に与えて起動する必要がある。
