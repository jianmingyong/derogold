FROM ubuntu:latest AS build

ARG DEBIAN_FRONTEND=noninteractive

ADD . /usr/local/src/DeroGold
WORKDIR /usr/local/src/DeroGold

RUN apt-get update && apt-get install -y git cmake ninja-build build-essential curl zip unzip tar pkg-config \
    && rm -rf /var/lib/apt/lists/* \
    && cmake --preset linux-x64-gcc-publish \
    && cmake --build --preset linux-x64-gcc-publish

FROM ubuntu:latest
COPY --from=build /usr/local/bin /usr/local/bin
