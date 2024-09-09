# syntax=docker/dockerfile:1

# Select target ubuntu version to build.
ARG UBUNTU_VERSION=20.04

# Select target cmake version to build.
ARG CMAKE_VERSION=3.30.3

# vcpkg binary cache.
ARG VCPKG_BINARY_SOURCES=clear;default,readwrite
ARG ACTIONS_CACHE_URL

##################################################
# Default build environment
##################################################

FROM ubuntu:${UBUNTU_VERSION} AS dev_env_default
ARG DEBIAN_FRONTEND=noninteractive

FROM --platform=${BUILDPLATFORM} ubuntu:${UBUNTU_VERSION} AS dev_env_default_cross
ARG DEBIAN_FRONTEND=noninteractive

##################################################
# Default build environment (gcc/clang)
##################################################

FROM dev_env_default AS dev_env_clang_sysroot
ARG TARGETPLATFORM

RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt,sharing=locked \
    if [ "${TARGETPLATFORM}" = "linux/arm64" ]; then \
        apt-get update && apt-get install -y libc6-dev libgcc-9-dev libstdc++-9-dev \
        && tar -chvzf /root/sysroot.tar.gz /lib/aarch64-linux-gnu/ /lib/gcc/ /usr/lib/aarch64-linux-gnu/ /usr/lib/gcc/ /usr/include/; \
    fi

FROM dev_env_default_cross AS dev_env_gcc
ARG BUILDPLATFORM
ARG TARGETPLATFORM
ARG ACTIONS_CACHE_URL
ARG CMAKE_VERSION

RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt,sharing=locked \
    if [ "${BUILDPLATFORM}" != "${TARGETPLATFORM}" ] && [ "${TARGETPLATFORM}" = "linux/arm64" ]; then \
        apt-get update && apt-get install -y build-essential ccache cmake crossbuild-essential-arm64 curl git libssl-dev ninja-build pkg-config tar unzip zip; \
    else \
        apt-get update && apt-get install -y build-essential ccache cmake curl git libssl-dev ninja-build pkg-config tar unzip zip; \
    fi \
    && curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.40.0/install.sh | bash \
    && export NVM_DIR="$HOME/.nvm" && [ -s "$NVM_DIR/nvm.sh" ] && \. "$NVM_DIR/nvm.sh" \
    && nvm install 20

ADD docker/javascript-actions/cache /usr/local/src/cache
WORKDIR /usr/local/src/cache

RUN --mount=type=cache,target=/root/.ccache \
    --mount=type=secret,id=ACTIONS_RUNTIME_TOKEN \
    if [ -e /run/secrets/ACTIONS_RUNTIME_TOKEN ]; then \
        export ACTIONS_RUNTIME_TOKEN=$(cat /run/secrets/ACTIONS_RUNTIME_TOKEN) \
        && export INPUT_ACTION=restore \
		&& export NVM_DIR="$HOME/.nvm" && [ -s "$NVM_DIR/nvm.sh" ] && \. "$NVM_DIR/nvm.sh" \
        && node dist/index.js; \
    fi

ADD https://github.com/Kitware/CMake.git#v${CMAKE_VERSION} /usr/local/src/CMake
WORKDIR /usr/local/src/CMake

RUN --mount=type=cache,target=/root/.ccache \
    CC=gcc CXX=g++ cmake -D CMAKE_C_COMPILER_LAUNCHER=ccache -D CMAKE_CXX_COMPILER_LAUNCHER=ccache -D CMAKE_BUILD_TYPE=Release -S . -B build && cmake --build build -t install -j $(nproc)

FROM dev_env_default_cross AS dev_env_clang
ARG BUILDPLATFORM
ARG TARGETPLATFORM
ARG ACTIONS_CACHE_URL
ARG CMAKE_VERSION

RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt,sharing=locked \
    if [ "${BUILDPLATFORM}" != "${TARGETPLATFORM}" ] && [ "${TARGETPLATFORM}" = "linux/arm64" ]; then \
        apt-get update && apt-get install -y binutils-aarch64-linux-gnu ccache clang cmake curl git libssl-dev ninja-build nodejs npm pkg-config tar unzip zip; \
    else \
        apt-get update && apt-get install -y ccache clang cmake curl git libssl-dev ninja-build nodejs npm pkg-config tar unzip zip; \
    fi \
    && curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.40.0/install.sh | bash \
    && export NVM_DIR="$HOME/.nvm" && [ -s "$NVM_DIR/nvm.sh" ] && \. "$NVM_DIR/nvm.sh" \
    && nvm install 20

COPY --from=dev_env_clang_sysroot /root /usr/local/sysroot

RUN if [ "${BUILDPLATFORM}" != "${TARGETPLATFORM}" ] && [ "${TARGETPLATFORM}" = "linux/arm64" ]; then \
        tar -xzvf /usr/local/sysroot/sysroot.tar.gz -C /usr/local/sysroot; \
    fi

ADD docker/javascript-actions/cache /usr/local/src/cache
WORKDIR /usr/local/src/cache

RUN --mount=type=cache,target=/root/.ccache \
    --mount=type=secret,id=ACTIONS_RUNTIME_TOKEN \
    if [ -e /run/secrets/ACTIONS_RUNTIME_TOKEN ]; then \
        export ACTIONS_RUNTIME_TOKEN=$(cat /run/secrets/ACTIONS_RUNTIME_TOKEN) \
        && export INPUT_ACTION=restore \
		&& export NVM_DIR="$HOME/.nvm" && [ -s "$NVM_DIR/nvm.sh" ] && \. "$NVM_DIR/nvm.sh" \
        && node dist/index.js; \
    fi

ADD https://github.com/Kitware/CMake.git#v${CMAKE_VERSION} /usr/local/src/CMake
WORKDIR /usr/local/src/CMake

RUN --mount=type=cache,target=/root/.ccache \
    CC=clang CXX=clang++ cmake -D CMAKE_C_COMPILER_LAUNCHER=ccache -D CMAKE_CXX_COMPILER_LAUNCHER=ccache -D CMAKE_BUILD_TYPE=Release -S . -B build && cmake --build build -t install -j $(nproc)

##################################################
# Build Step
##################################################

FROM dev_env_gcc AS build_gcc
ARG BUILDPLATFORM
ARG TARGETPLATFORM
ARG TARGETARCH
ARG VCPKG_BINARY_SOURCES
ARG ACTIONS_CACHE_URL

ADD . /usr/local/src/DeroGold
WORKDIR /usr/local/src/DeroGold

RUN --mount=type=cache,target=/root/.ccache \
    --mount=type=cache,target=/root/.cache/vcpkg/archives \
    --mount=type=secret,id=ACTIONS_RUNTIME_TOKEN \
    if [ -e /run/secrets/ACTIONS_RUNTIME_TOKEN ]; then \
        export ACTIONS_RUNTIME_TOKEN=$(cat /run/secrets/ACTIONS_RUNTIME_TOKEN); \
    fi \
    && \
    if [ "${BUILDPLATFORM}" != "${TARGETPLATFORM}" ]; then \
        cmake --preset linux-${TARGETARCH}-gcc-cross-install -D CMAKE_C_COMPILER_LAUNCHER=ccache -D CMAKE_CXX_COMPILER_LAUNCHER=ccache -D CMAKE_INSTALL_PREFIX=/usr/local && cmake --build --preset linux-${TARGETARCH}-gcc-cross-install -j $(nproc); \
    else \
        cmake --preset linux-${TARGETARCH}-gcc-install -D CMAKE_C_COMPILER_LAUNCHER=ccache -D CMAKE_CXX_COMPILER_LAUNCHER=ccache && cmake --build --preset linux-${TARGETARCH}-gcc-install -j $(nproc); \
    fi \
    && \
    if [ -e /run/secrets/ACTIONS_RUNTIME_TOKEN ]; then \
        export ACTIONS_RUNTIME_TOKEN=$(cat /run/secrets/ACTIONS_RUNTIME_TOKEN) \
        && export INPUT_ACTION=save \
		&& export NVM_DIR="$HOME/.nvm" && [ -s "$NVM_DIR/nvm.sh" ] && \. "$NVM_DIR/nvm.sh" \
        && node dist/index.js; \
    fi

FROM dev_env_clang AS build_clang
ARG BUILDPLATFORM
ARG TARGETPLATFORM
ARG TARGETARCH
ARG VCPKG_BINARY_SOURCES
ARG ACTIONS_CACHE_URL

ADD . /usr/local/src/DeroGold
WORKDIR /usr/local/src/DeroGold

RUN --mount=type=cache,target=/root/.ccache \
    --mount=type=cache,target=/root/.cache/vcpkg/archives \
    --mount=type=secret,id=ACTIONS_RUNTIME_TOKEN \
    if [ -e /run/secrets/ACTIONS_RUNTIME_TOKEN ]; then \
        export ACTIONS_RUNTIME_TOKEN=$(cat /run/secrets/ACTIONS_RUNTIME_TOKEN); \
    fi \
    && \
    if [ "${BUILDPLATFORM}" != "${TARGETPLATFORM}" ]; then \
        cmake --preset linux-${TARGETARCH}-clang-cross-install -D CMAKE_C_COMPILER_LAUNCHER=ccache -D CMAKE_CXX_COMPILER_LAUNCHER=ccache -D CMAKE_INSTALL_PREFIX=/usr/local && cmake --build --preset linux-${TARGETARCH}-clang-cross-install -j $(nproc); \
    else \
        cmake --preset linux-${TARGETARCH}-clang-install -D CMAKE_C_COMPILER_LAUNCHER=ccache -D CMAKE_CXX_COMPILER_LAUNCHER=ccache && cmake --build --preset linux-${TARGETARCH}-clang-install -j $(nproc); \
    fi \
    && \
    if [ -e /run/secrets/ACTIONS_RUNTIME_TOKEN ]; then \
        export ACTIONS_RUNTIME_TOKEN=$(cat /run/secrets/ACTIONS_RUNTIME_TOKEN) \
        && export INPUT_ACTION=save \
		&& export NVM_DIR="$HOME/.nvm" && [ -s "$NVM_DIR/nvm.sh" ] && \. "$NVM_DIR/nvm.sh" \
        && node dist/index.js; \
    fi

##################################################
# Default runtime environment
##################################################
FROM ubuntu:${UBUNTU_VERSION} AS release_default
LABEL org.opencontainers.image.source="https://github.com/jianmingyong/derogold"
LABEL org.opencontainers.image.description="DeroGold is a digital assets project focused on preserving our life environment here on Earth."
LABEL org.opencontainers.image.licenses="GPL-3.0-or-later"

FROM release_default AS release_gcc
COPY --from=build_gcc /usr/local/bin/DeroGoldd /usr/local/bin

FROM release_default AS release_clang
COPY --from=build_clang /usr/local/bin/DeroGoldd /usr/local/bin
