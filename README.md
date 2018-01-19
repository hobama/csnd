# csnd

Citizen Seismology Network Daemon

## Prerequisites

* build-essential
* cmake (>= 2.8.12)
* following dependencies

なお、USB接続のセンサーから読み取るため、csndを動かすユーザーは dialout グループに属している必要がある。

## Dependencies

### common

|  name   | debian (raspbian) |    centos     |          note          |
|:-------:|:-----------------:|:-------------:|:----------------------:|
| spdlog  |  N/A (embedded*)  | spdlog (epel) | *required CMake >= 3.1 |


### [Avro C++](https://github.com/apache/avro)

|  name   | debian (raspbian) |    centos     |          note          |
|:-------:|:-----------------:|:-------------:|:----------------------:|
|  boost  | libboost-filesystem-dev <br/> libboost-system-dev <br/> libboost-program-options-dev <br/> libboost-iostreams-dev |  <br/> boost-devel  | version >= 1.38        |
|  zlib   |     liblz-dev     |  zlib-devel   ||

### [Avro C](https://github.com/apache/avro)

|  name   | debian (raspbian) |    centos     |          note          |
|:-------:|:-----------------:|:-------------:|:----------------------:|
|  lzma   |    liblzma-dev    |    xz-devel   ||
|  zlib   |     liblz-dev     |  zlib-devel   ||
| jansson |  libjansson-dev   | jansson-devel |    version >= 2.3      |
| snappy  |  libsnappy-dev    | snappy-devel  ||

### [azure-iothub](https://github.com/sgr/azure-iothub) (contains [azure-iot-sdk-c](https://github.com/Azure/azure-iot-sdk-c))

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

0. (CentOSのみ) [curl](https://curl.haxx.se/) をソースコードからビルドし、インストールする。
   - csnd が OS の libcurl より先に見つけられるディレクトリにインストールする。例えば以下の csnd のインストールディレクトリと同一にする。
1. csnd を配置する場所を決める。
   - インストールディレクトリ (例えば /opt/csnd)
   - 設定ファイルを置くディレクトリ (例えば /opt/csnd/etc)
   - 加速度データおよびイベントデータ出力ディレクトリ (例えば /opt/csnd/out 設定ファイルでは out_dir で指定)
   - ログファイル (例えば /opt/csnd/log/csnd.log 設定ファイルでは logging.file_settings.filename で指定)
   - pidファイル (例えば /opt/csnd/run/csnd.pid 設定ファイルでは pid_file で指定)
2. cmake -DCMAKE_INSTALL_PREFIX=<インストールディレクトリ> <csndのソースディレクトリ>
3. make install
4. <インストールディレクトリ>/csnd.yml を作成
   - ひな形ファイル <インストールディレクトリ>/etc/csnd.yml.example をもとに作ると良い。
   - iothub.connection_string は CSN 管理者から発行された接続文字列を設定する。接続文字列がない場合は offline_mode を true に設定し、オフラインモードで使用する。
   - logging.logger を console にする場合は、 -d オプションを用いずフォアグラウンドモードで実行する。
5. /etc/systemd/system/csnd.service を作成
   - ひな形ファイル <インストールディレクトリ>/etc/csnd.service.example をもとに上で決めた内容に合わせて作成する。
6. csnd.service を systemctl で有効化する。
   ```(sh)
   # systemctl enable csnd.service
   ```

## Command line options

| option | description|
|------|-------------|
| -c /path/of/csnd.yml | set config file path |
| -d | run on daemon mode |

## Known Issues

* 今のところ Avro-C シリアライザー (libserializer_avroc.a) は正しく動作しない。 Avro-C++ 版を用いること。
* azure-iot-sdk-c は libcurl を必要とするが、CentOS 7 標準パッケージは OpenSSL を使っていないため、HTTP接続の場合初期化で Segmentation Fault を起こす。curl のソースコードから --with-ssl つきでビルドし、ライブラリの場所を LD_LIBRARY_PATH に与えて起動する必要がある。
