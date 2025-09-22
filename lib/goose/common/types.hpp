#pragma once

#include "vulkan/vulkan.h"
#include "vulkan/vulkan_core.h"

#include "vk_mem_alloc.h"

#include <algorithm>
#include <limits>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <cstdint>

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

struct Window;

} // namespace goose

namespace goose::render {

struct RenderContext;

struct Instance;
struct Device;
struct Swapchain;
struct Frame;

} // namespace goose::render
