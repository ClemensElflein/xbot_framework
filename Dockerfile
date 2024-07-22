# Use Ubuntu 22.04 as the base image
FROM ubuntu:22.04 AS base
ENV DEBIAN_FRONTEND=noninteractive
# Install required packages
RUN apt-get update && \
    apt-get install -y \
    build-essential \
    cmake \
    git \
    libasio-dev iproute2 \
    python3 python3-venv python3-pip && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

FROM base AS build

COPY --link . /workspace
WORKDIR /workspace
RUN cmake --preset=CI . && cd build/CI && cmake --build . -j$(nproc) && cpack

FROM scratch AS export
COPY --from=build /workspace/build/CI/xbot_framework*.tar.gz /
