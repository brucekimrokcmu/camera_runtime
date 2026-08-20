# docker/Dockerfile
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    ninja-build \
    gdb \
    clangd \
    clang-format \
    neovim \
    libopencv-dev \
    pkg-config \
    git \
    libv4l-dev \
    v4l-utils \
    libpthread-stubs0-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

CMD ["/bin/bash"]
