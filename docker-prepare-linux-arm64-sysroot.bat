@echo off
docker run --rm --platform linux/arm64 -v %~dp0docker\sysroot:/usr/local/src/derogold ubuntu:20.04 sh -c "apt-get update && apt-get install -y libc6-dev libgcc-9-dev libstdc++-9-dev && tar -chvzf /usr/local/src/derogold/sysroot.tar.gz /lib/aarch64-linux-gnu/ /lib/gcc/ /usr/lib/aarch64-linux-gnu/ /usr/lib/gcc/ /usr/include/"
