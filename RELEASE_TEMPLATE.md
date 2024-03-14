Change Logs from v0.7.1

- Fixed a bug where certain daemon rpc method would fail due to missing transactions.
- Removed seed nodes that are not working and add new ones.
- Fixed upnp port forwarding issues.
- Update RocksDB provider settings.
- Changed RocksDB default read/write buffer to 2048 MB and 1024 MB respectively.
- Changed p2p block downloading to dynamic block rate based on system load.
- Added `--db-optimize` option to rebuild your current RocksDB database. This would also allow you to load outdated RocksDB database if they are not compatible.
- Added `--export-checkpoints` option to export checkpoints.csv for bootstrapping.

Update classification: optional / ~~mandatory~~

For Linux and MacOS user: You can use `tar -xf <FileName>.tar.gz` to unzip, which would also set the permission to execute correctly.