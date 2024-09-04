# syntax=docker/dockerfile:1

ARG UBUNTU_VERSION=latest

##################################################
# Create Dev Environment
##################################################

FROM ubuntu:${UBUNTU_VERSION} AS dev_env_default
ARG DEBIAN_FRONTEND=noninteractive

ADD . /usr/local/src/DeroGold
WORKDIR /usr/local/src/DeroGold

FROM dev_env_default AS dev_env_gcc

RUN apt-get update && apt-get install -y git cmake ninja-build build-essential curl zip unzip tar pkg-config \
    && rm -rf /var/lib/apt/lists/*

FROM dev_env_default AS dev_env_clang

RUN apt-get update && apt-get install -y git cmake ninja-build clang curl zip unzip tar pkg-config \
    && rm -rf /var/lib/apt/lists/*

##################################################
# Build Step
##################################################

FROM dev_env_gcc AS build_gcc
ARG TARGETARCH
RUN cmake --preset linux-${TARGETARCH}-gcc-publish && cmake --build --preset linux-${TARGETARCH}-gcc-publish

FROM dev_env_clang AS build_clang
ARG TARGETARCH
RUN cmake --preset linux-${TARGETARCH}-clang-publish && cmake --build --preset linux-${TARGETARCH}-clang-publish

##################################################
# Release Step
##################################################

FROM ubuntu:${UBUNTU_VERSION} AS release_gcc

LABEL org.opencontainers.image.source="https://github.com/jianmingyong/derogold"
LABEL org.opencontainers.image.description="DeroGold is a digital assets project focused on preserving our life environment here on Earth."
LABEL org.opencontainers.image.licenses="GPL-3.0-or-later"

COPY --from=build_gcc /usr/local/bin /usr/local/bin

FROM ubuntu:${UBUNTU_VERSION} AS release_clang

LABEL org.opencontainers.image.source="https://github.com/jianmingyong/derogold"
LABEL org.opencontainers.image.description="DeroGold is a digital assets project focused on preserving our life environment here on Earth."
LABEL org.opencontainers.image.licenses="GPL-3.0-or-later"

COPY --from=build_clang /usr/local/bin /usr/local/bin
