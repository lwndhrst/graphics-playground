#!/bin/sh

mkdir -p build

clang++ src/*.cpp -o build/vk-renderer \
    -std=c++20 \
    -DLOG_LEVEL=DEBUG \
    -DENABLE_VALIDATION_LAYERS=true \
    -fuse-ld=gold \
    -lSDL2 \
    -lvulkan
