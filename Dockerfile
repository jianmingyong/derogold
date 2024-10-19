# syntax=docker/dockerfile:1

ARG UBUNTU_VERSION=20.04

ARG CMAKE_VERSION=3.30.3
ARG COMPILER_TYPE=gcc

ARG VCPKG_BINARY_SOURCES=clear;default,readwrite
ARG ACTIONS_CACHE_URL

ARG CCACHE_VERSION=4.10.2
ARG CCACHE_MAXSIZE=250M

##################################################
# Default Build Environment
##################################################

FROM --platform=${BUILDPLATFORM} ubuntu:${UBUNTU_VERSION} AS dev_env_default
ARG DEBIAN_FRONTEND=noninteractive

ARG BUILDPLATFORM
ARG TARGETPLATFORM
ARG TARGETOS
ARG TARGETARCH

ARG UBUNTU_VERSION

ARG CMAKE_VERSION
ARG COMPILER_TYPE

ARG VCPKG_BINARY_SOURCES
ARG ACTIONS_CACHE_URL

ARG CCACHE_VERSION
ARG CCACHE_MAXSIZE

##################################################
# Build Environment
##################################################

FROM dev_env_default AS env_install

RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt,sharing=locked \
    if [ "${BUILDPLATFORM}" = "linux/amd64" ]; then \
        apt-get update && apt-get install -y binutils-aarch64-linux-gnu build-essential clang cmake crossbuild-essential-arm64 curl git libssl-dev ninja-build pkg-config tar unzip zip; \
    elif [ "${BUILDPLATFORM}" = "linux/arm64" ]; then \
        apt-get update && apt-get install -y binutils-x86_64-linux-gnu build-essential clang cmake crossbuild-essential-amd64 curl git libssl-dev ninja-build pkg-config tar unzip zip; \
    fi

RUN --mount=type=bind,target=/usr/local/src/docker,source=docker \
    mkdir /usr/local/sysroot && \
    if [ "${BUILDPLATFORM}" = "linux/amd64" ] && [ -e /usr/local/src/docker/sysroot/ubuntu-${UBUNTU_VERSION}-aarch64-linux-gnu-sysroot.tar.gz ]; then \
        tar -xzf /usr/local/src/docker/sysroot/ubuntu-${UBUNTU_VERSION}-aarch64-linux-gnu-sysroot.tar.gz -C /usr/local/sysroot; \
    elif [ "${BUILDPLATFORM}" = "linux/arm64" ] && [ -e /usr/local/src/docker/sysroot/ubuntu-${UBUNTU_VERSION}-x86_64-linux-gnu-sysroot.tar.gz ]; then \
        tar -xzf /usr/local/src/docker/sysroot/ubuntu-${UBUNTU_VERSION}-x86_64-linux-gnu-sysroot.tar.gz -C /usr/local/sysroot; \
    fi

RUN git clone --branch v${CCACHE_VERSION} --depth 1 --recursive https://github.com/ccache/ccache.git /usr/local/src/ccache && \
    cd /usr/local/src/ccache && \
    if [ "${COMPILER_TYPE}" = "gcc" ]; then \
        CC=gcc CXX=g++ cmake -D CMAKE_BUILD_TYPE=Release -S . -B build && cmake --build build -t install -j $(nproc); \
    elif [ "${COMPILER_TYPE}" = "clang" ]; then \
        CC=clang CXX=clang++ cmake -D CMAKE_BUILD_TYPE=Release -S . -B build && cmake --build build -t install -j $(nproc); \
    fi && \
    rm -r /usr/local/src/ccache

RUN --mount=type=cache,id=ccache_${TARGETOS}_${TARGETARCH}_${COMPILER_TYPE},target=/root/.ccache \
    git clone --branch v${CMAKE_VERSION} --depth 1 --recursive https://github.com/Kitware/CMake.git /usr/local/src/CMake && \
    cd /usr/local/src/CMake && \
    if [ "${COMPILER_TYPE}" = "gcc" ]; then \
        CC=gcc CXX=g++ cmake -D CMAKE_C_COMPILER_LAUNCHER=ccache -D CMAKE_CXX_COMPILER_LAUNCHER=ccache -D CMAKE_BUILD_TYPE=Release -S . -B build && cmake --build build -t install -j $(nproc); \
    elif [ "${COMPILER_TYPE}" = "clang" ]; then \
        CC=clang CXX=clang++ cmake -D CMAKE_C_COMPILER_LAUNCHER=ccache -D CMAKE_CXX_COMPILER_LAUNCHER=ccache -D CMAKE_BUILD_TYPE=Release -S . -B build && cmake --build build -t install -j $(nproc); \
    fi && \
    rm -r /usr/local/src/CMake

##################################################
# Build Step
##################################################

FROM env_install AS build_gcc_clang

RUN --mount=type=bind,target=/usr/local/src/DeroGold,rw \
    --mount=type=cache,id=ccache_${TARGETOS}_${TARGETARCH}_${COMPILER_TYPE},target=/root/.ccache \
    --mount=type=cache,id=vcpkg_${TARGETOS}_${TARGETARCH}_${COMPILER_TYPE},target=/root/.cache/vcpkg/archives \
    --mount=type=secret,id=ACTIONS_RUNTIME_TOKEN \
    cd /usr/local/src/DeroGold && \
    if [ -s /run/secrets/ACTIONS_RUNTIME_TOKEN ]; then \
        export ACTIONS_RUNTIME_TOKEN=$(cat /run/secrets/ACTIONS_RUNTIME_TOKEN); \
    fi && \
    if [ "${BUILDPLATFORM}" != "${TARGETPLATFORM}" ]; then \
        cmake --preset linux-${TARGETARCH}-${COMPILER_TYPE}-cross-install -D CMAKE_INSTALL_PREFIX=/usr/local && cmake --build --preset linux-${TARGETARCH}-${COMPILER_TYPE}-cross-install -j $(nproc); \
    else \
        cmake --preset linux-${TARGETARCH}-${COMPILER_TYPE}-install && cmake --build --preset linux-${TARGETARCH}-${COMPILER_TYPE}-install -j $(nproc); \
    fi

##################################################
# Default runtime environment
##################################################
FROM ubuntu:${UBUNTU_VERSION} AS release_default
LABEL org.opencontainers.image.source="https://github.com/jianmingyong/derogold"
LABEL org.opencontainers.image.description="DeroGold is a digital assets project focused on preserving our life environment here on Earth."
LABEL org.opencontainers.image.licenses="GPL-3.0-or-later"

FROM release_default AS release_derogoldd
COPY --from=build_gcc_clang /usr/local/bin/DeroGoldd /usr/local/bin/
EXPOSE 42069/tcp
EXPOSE 6969/tcp
ENTRYPOINT [ "DeroGoldd" ]
