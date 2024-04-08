Change Logs from v0.7.1

Daemon
- Fixed syncing issues due to dead seed nodes.
- Fixed upnp port forwarding issues.
- Fixed daemon rpc method would fail due to missing transactions.
- Fixed `--db-*` options such that it will no longer get overridden by default values.
- Added `--db-optimize` option to rebuild your current RocksDB database. This would also allow you to load outdated RocksDB database if they are not compatible.
- Added `--export-checkpoints` option to export checkpoints.csv for bootstrapping.
- Added `--enable-trtl-api` option to enable turtlecoin rpc api.
- Changed `--db-enable-compression` to true by default.

P2P
- Changed p2p block downloading to dynamic block rate based on system load.

RocksDB
- Update RocksDB provider to v9.0.0.
- Changed RocksDB default read/write buffer to 2048 MB and 1024 MB respectively.

Update classification: optional / ~~mandatory~~

For Linux and MacOS user: You can use `tar -xf <FileName>.tar.gz` to unzip, which would also set the permission to execute correctly.

WARNING:
- Upgrading to this version will update the current rocksDB database. Compatibility between v0.7.1 and v0.7.2.1 is not guaranteed. 