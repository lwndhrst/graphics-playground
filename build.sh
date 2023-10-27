#!/bin/sh

mkdir -p build

clang++ src/*.cpp -o build/vk-renderer \
    -std=c++20 \
    -fexperimental-library \
    -fuse-ld=gold \
    -DLOG_LEVEL=DEBUG \
    -DENABLE_VALIDATION_LAYERS=true \
    -lSDL2 \
    -lvulkan
