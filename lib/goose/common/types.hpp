#pragma once

#include "volk.h"

#include "vk_mem_alloc.h"

#include "glm/mat4x4.hpp"
#include "glm/vec4.hpp"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <set>
#include <span>
#include <string>
#include <utility>
#include <vector>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef uintptr_t usize;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef intptr_t isize;

typedef float f32;
typedef double f64;

// Forward declarations

namespace goose {

struct WindowInfo;

} // namespace goose

namespace goose::render {

struct Instance;
struct Device;

struct RenderContext;

struct SwapchainImageInfo;
struct SwapchainInfo;

struct FrameDataCreateInfo;
struct FrameData;
struct Frame;

struct ImmediateData;

struct ImageInfo;
struct ImageBuilder;

struct BufferInfo;
struct BufferBuilder;

} // namespace goose::render
