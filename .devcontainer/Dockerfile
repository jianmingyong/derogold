# syntax=docker/dockerfile:1

ARG UBUNTU_VERSION=20.04
ARG CCACHE_VERSION=4.10.2

ARG COMPILER_TYPE=gcc

##################################################
# Default Build Environment
##################################################

FROM --platform=${BUILDPLATFORM} ubuntu:${UBUNTU_VERSION} AS dev_env_default
ARG DEBIAN_FRONTEND=noninteractive

ARG BUILDPLATFORM

ARG UBUNTU_VERSION
ARG CCACHE_VERSION

ARG COMPILER_TYPE

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

RUN curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.40.0/install.sh | bash && \
    export NVM_DIR="$HOME/.nvm" && [ -s "$NVM_DIR/nvm.sh" ] && . "$NVM_DIR/nvm.sh" && \
    nvm install 22
