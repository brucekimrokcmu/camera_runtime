# docker/Dockerfile
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# Install essential build tools, C++ dependencies, and OpenCV dev libraries
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    gdb \
    libopencv-dev \
    pkg-config \
    git \
    libv4l-dev \
    v4l-utils \ 
    libpthread-stubs0-dev \
    && rm -rf /var/lib/apt-get/lists/*

# Set working directory inside the container
WORKDIR /workspace

CMD ["/bin/bash"]
