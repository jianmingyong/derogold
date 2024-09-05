@echo off

SET OUTPUT_VERSION=0.7.2.3

docker build -f Dockerfile --platform linux/amd64,linux/arm64 --target release_gcc -t jianmingyong/derogold:latest -t jianmingyong/derogold:latest-gcc -t jianmingyong/derogold:%OUTPUT_VERSION% -t jianmingyong/derogold:%OUTPUT_VERSION%-gcc .
docker build -f Dockerfile --platform linux/amd64,linux/arm64 --target release_clang -t jianmingyong/derogold:latest-clang -t jianmingyong/derogold:%OUTPUT_VERSION%-clang .
