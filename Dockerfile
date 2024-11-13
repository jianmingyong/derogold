# syntax=docker/dockerfile:1

ARG UBUNTU_VERSION=20.04
ARG CCACHE_VERSION=4.10.2

ARG COMPILER_TYPE=gcc

ARG VCPKG_BINARY_SOURCES=clear;default,readwrite
ARG ACTIONS_CACHE_URL

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
ARG CCACHE_VERSION

ARG COMPILER_TYPE

ARG VCPKG_BINARY_SOURCES
ARG ACTIONS_CACHE_URL

ARG CMAKE_APT_PACKAGE="ca-certificates curl gpg"
ARG VCS_PACKAGE="git gpg"
ARG DEV_PACKAGE="cmake ninja-build"
ARG VCPKG_PACKAGE="curl zip unzip tar pkg-config"

ARG AMD64_GCC_PACKAGE="build-essential crossbuild-essential-arm64"
ARG AMD64_CLANG_PACKAGE="clang binutils-aarch64-linux-gnu"

ARG ARM64_GCC_PACKAGE="build-essential crossbuild-essential-amd64"
ARG ARM64_CLANG_PACKAGE="clang binutils-x86_64-linux-gnu"

RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt,sharing=locked \
    apt-get update && apt-get install --no-install-recommends --no-install-suggests -y ${CMAKE_APT_PACKAGE} && \
    curl -o- https://apt.kitware.com/keys/kitware-archive-latest.asc | gpg --dearmor - > /usr/share/keyrings/kitware-archive-keyring.gpg && \
    [ -s /etc/os-release ] && . /etc/os-release && \
    echo "deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ ${UBUNTU_CODENAME} main" > /etc/apt/sources.list.d/kitware.list && \
    if [ "${BUILDPLATFORM}" = "linux/amd64" ]; then \
        apt-get update && apt-get install --no-install-recommends --no-install-suggests -y ${VCS_PACKAGE} ${DEV_PACKAGE} ${VCPKG_PACKAGE} ${AMD64_GCC_PACKAGE} ${AMD64_CLANG_PACKAGE}; \
    elif [ "${BUILDPLATFORM}" = "linux/arm64" ]; then \
        apt-get update && apt-get install --no-install-recommends --no-install-suggests -y ${VCS_PACKAGE} ${DEV_PACKAGE} ${VCPKG_PACKAGE} ${ARM64_GCC_PACKAGE} ${ARM64_CLANG_PACKAGE}; \
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

##################################################
# Build Step
##################################################

FROM dev_env_default AS build_gcc_clang

RUN --mount=type=bind,target=/usr/local/src/DeroGold,rw \
    --mount=type=cache,id=ccache_${TARGETOS}_${TARGETARCH}_${COMPILER_TYPE},target=/root/.ccache \
    --mount=type=cache,id=vcpkg_${TARGETOS}_${TARGETARCH}_${COMPILER_TYPE},target=/root/.cache/vcpkg/archives \
    --mount=type=secret,id=ACTIONS_RUNTIME_TOKEN \
    cd /usr/local/src/DeroGold && \
    if [ -s /run/secrets/ACTIONS_RUNTIME_TOKEN ]; then \
        export ACTIONS_RUNTIME_TOKEN=$(cat /run/secrets/ACTIONS_RUNTIME_TOKEN); \
    fi && \
    if [ "${BUILDPLATFORM}" != "${TARGETPLATFORM}" ]; then \
        cmake --preset linux-${TARGETARCH}-${COMPILER_TYPE}-cross-all -D CMAKE_INSTALL_PREFIX=/usr/local && cmake --build --preset linux-${TARGETARCH}-${COMPILER_TYPE}-cross-all -t install -j $(nproc); \
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
