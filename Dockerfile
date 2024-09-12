# syntax=docker/dockerfile:1

ARG UBUNTU_VERSION=20.04

ARG CMAKE_VERSION=3.30.3
ARG COMPILER_TYPE=gcc

ARG NODE_VERSION=20
ARG NVM_DIR=/root/.nvm

ARG VCPKG_BINARY_SOURCES=clear;default,readwrite
ARG ACTIONS_CACHE_URL

ARG CCACHE_MAXSIZE=250M

ARG GITHUB_REF

##################################################
# Default Build Environment
##################################################

FROM --platform=${BUILDPLATFORM} ubuntu:${UBUNTU_VERSION} AS dev_env_default
ARG DEBIAN_FRONTEND=noninteractive

ARG BUILDPLATFORM
ARG TARGETPLATFORM
ARG TARGETARCH

ARG CMAKE_VERSION
ARG COMPILER_TYPE

ARG NODE_VERSION
ARG NVM_DIR

ARG VCPKG_BINARY_SOURCES
ARG ACTIONS_CACHE_URL

ARG CCACHE_MAXSIZE

ARG GITHUB_REF

##################################################
# Build Environment
##################################################

FROM dev_env_default AS env_install
RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt,sharing=locked \
    if [ "${BUILDPLATFORM}" = "linux/amd64" ]; then \
        apt-get update && apt-get install -y binutils-aarch64-linux-gnu build-essential ccache clang cmake crossbuild-essential-arm64 curl git libssl-dev ninja-build pkg-config tar unzip zip zstd; \
    elif [ "${BUILDPLATFORM}" = "linux/arm64" ]; then \
        apt-get update && apt-get install -y binutils-x86_64-linux-gnu build-essential ccache clang cmake crossbuild-essential-amd64 curl git libssl-dev ninja-build pkg-config tar unzip zip zstd; \
    fi

RUN curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.40.0/install.sh | bash \
    && [ -s "${NVM_DIR}/nvm.sh" ] && . "${NVM_DIR}/nvm.sh" \
    && nvm install ${NODE_VERSION}

RUN --mount=type=bind,target=/usr/local/src/docker,source=docker \
    mkdir /usr/local/sysroot \
    && if [ "${BUILDPLATFORM}" = "linux/amd64" ]; then \
        tar -xzf /usr/local/src/docker/sysroot/ubuntu-${UBUNTU_VERSION}-aarch64-linux-gnu-sysroot.tar.gz -C /usr/local/sysroot; \
    elif [ "${BUILDPLATFORM}" = "linux/arm64" ]; then \
        tar -xzf /usr/local/src/docker/sysroot/ubuntu-${UBUNTU_VERSION}-x86_64-linux-gnu-sysroot.tar.gz -C /usr/local/sysroot; \
    fi

FROM env_install AS restore_ccache
RUN --mount=type=cache,target=/root/.ccache \
    --mount=type=bind,target=/usr/local/src/docker,source=docker,rw \
    --mount=type=secret,id=ACTIONS_RUNTIME_TOKEN \
    [ -s "${NVM_DIR}/nvm.sh" ] && . "${NVM_DIR}/nvm.sh" \
    && if [ -s /run/secrets/ACTIONS_RUNTIME_TOKEN ]; then \
        cd /usr/local/src/docker/github-actions-proxy && \
        npm install && npm run build && \
        ACTIONS_RUNTIME_TOKEN=$(cat /run/secrets/ACTIONS_RUNTIME_TOKEN) node dist/index.js -a actions/cache/restore@v4.0.2 -i path=/root/.ccache -i "key=ccache_\${{ /root/.ccache/**/* }}" -i restore-keys=ccache_; \
    fi

FROM restore_ccache AS build_cmake
ADD https://github.com/Kitware/CMake.git#v${CMAKE_VERSION} /usr/local/src/CMake
WORKDIR /usr/local/src/CMake

RUN --mount=type=cache,target=/root/.ccache \
    if [ "${COMPILER_TYPE}" = "gcc" ]; then \
        CC=gcc CXX=g++ cmake -D CMAKE_C_COMPILER_LAUNCHER=ccache -D CMAKE_CXX_COMPILER_LAUNCHER=ccache -D CMAKE_BUILD_TYPE=Release -S . -B build && cmake --build build -t install -j $(nproc); \
    elif [ "${COMPILER_TYPE}" = "clang" ]; then \
        CC=clang CXX=clang++ cmake -D CMAKE_C_COMPILER_LAUNCHER=ccache -D CMAKE_CXX_COMPILER_LAUNCHER=ccache -D CMAKE_BUILD_TYPE=Release -S . -B build && cmake --build build -t install -j $(nproc); \
    fi

##################################################
# Build Step
##################################################

FROM build_cmake AS build_gcc_clang

ADD . /usr/local/src/DeroGold
WORKDIR /usr/local/src/DeroGold

RUN --mount=type=cache,target=/root/.ccache \
    --mount=type=cache,target=/root/.cache/vcpkg/archives \
    --mount=type=secret,id=ACTIONS_RUNTIME_TOKEN \
    if [ -e /run/secrets/ACTIONS_RUNTIME_TOKEN ]; then \
        export ACTIONS_RUNTIME_TOKEN=$(cat /run/secrets/ACTIONS_RUNTIME_TOKEN); \
    fi \
    && if [ "${BUILDPLATFORM}" != "${TARGETPLATFORM}" ]; then \
        cmake --preset linux-${TARGETARCH}-${COMPILER_TYPE}-cross-install -D CMAKE_C_COMPILER_LAUNCHER=ccache -D CMAKE_CXX_COMPILER_LAUNCHER=ccache -D CMAKE_INSTALL_PREFIX=/usr/local && cmake --build --preset linux-${TARGETARCH}-${COMPILER_TYPE}-cross-install -j $(nproc); \
    else \
        cmake --preset linux-${TARGETARCH}-${COMPILER_TYPE}-install -D CMAKE_C_COMPILER_LAUNCHER=ccache -D CMAKE_CXX_COMPILER_LAUNCHER=ccache && cmake --build --preset linux-${TARGETARCH}-${COMPILER_TYPE}-install -j $(nproc); \
    fi

FROM build_gcc_clang AS save_ccache
RUN --mount=type=cache,target=/root/.ccache \
    --mount=type=bind,target=/usr/local/src/docker,source=docker,rw \
    --mount=type=secret,id=ACTIONS_RUNTIME_TOKEN \
    [ -s "${NVM_DIR}/nvm.sh" ] && . "${NVM_DIR}/nvm.sh" \
    && if [ -s /run/secrets/ACTIONS_RUNTIME_TOKEN ]; then \
        cd /usr/local/src/docker/github-actions-proxy && \
        npm install && npm run build && \
        ACTIONS_RUNTIME_TOKEN=$(cat /run/secrets/ACTIONS_RUNTIME_TOKEN) node dist/index.js -a actions/cache/save@v4.0.2 -i path=/root/.ccache/** -i "key=ccache_\${{ /root/.ccache/** }}"; \
    fi

##################################################
# Default runtime environment
##################################################
FROM ubuntu:${UBUNTU_VERSION} AS release_default
LABEL org.opencontainers.image.source="https://github.com/jianmingyong/derogold"
LABEL org.opencontainers.image.description="DeroGold is a digital assets project focused on preserving our life environment here on Earth."
LABEL org.opencontainers.image.licenses="GPL-3.0-or-later"

COPY --from=save_ccache /usr/local/bin/DeroGoldd /usr/local/bin/
