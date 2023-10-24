#!/bin/sh

mkdir -p build

clang++ src/*.cpp -o build/vk-renderer \
    -DLOG_LEVEL=DEBUG \
    -fuse-ld=gold \
    -lSDL2 \
    -lvulkan \

