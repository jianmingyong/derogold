# Change Logs from v0.7.1

## Daemon
- Changed seed node address.
- Allow upnp to port forward regardless if it is behind a NAT devices.
- Fixed daemon rpc method would fail due to missing transactions.
- Fixed `--db-*` options such that it will no longer get overridden by default values.
- Added `--db-optimize` option to optimize your database for reading.
- Added `--export-checkpoints` option to export checkpoints.csv for bootstrapping.
- Added `--enable-trtl-rpc` option to enable turtlecoin rpc api.
- Changed `--db-enable-compression` to true by default.
- Changed `status` command behaviour so that it doesn't rely on rpc anymore.

## P2P
- Changed p2p block downloading to dynamic block rate based on system load.

## RocksDB
- Update RocksDB provider to v9.2.1.
- Changed RocksDB default read/write buffer to 2048 MB and 1024 MB respectively.
- Changed RocksDB logger output and reduced the file history to 1.

## External Dependencies Version
- boost 1.85.0
- cpp-httplib 0.16.2
- cryptopp 8.9.0
- cxxopts 3.2.1
- leveldb 1.23
- miniupnpc 2.3.7
- openssl 3.3.1
- rapidjson 2023-07-17
- rocksdb 9.2.1

# Install Notes

## For Windows user: (x64)
- Download `DeroGold-0.7.2.3-windows-x64-msvc.zip` and unzip.

## For Linux user: (x64)
- (**Debian package installer**) Download `DeroGold-0.7.2.3-linux-x64-gcc.deb` to install via `sudo apt install ./DeroGold-0.7.2.3-linux-x64-gcc.deb`.
- Download `DeroGold-0.7.2.3-linux-x64-gcc.tar.gz` and use `tar -xf DeroGold-0.7.2.3-linux-x64-gcc.tar.gz` to unzip.

## For MacOS user: (x64)
- Download `DeroGold-0.7.2.3-osx-x64-clang.tar.gz` and use `tar -xf DeroGold-0.7.2.3-osx-x64-clang.tar.gz` to unzip.

# **USAGE WARNING:**
- Pre-released versions are not guaranteed to be stable. Use with caution.
