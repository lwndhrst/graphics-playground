set(TARGET_NAME engine)

add_library(${TARGET_NAME} STATIC
    engine.cpp
)

target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 20)

target_compile_definitions(engine PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)

target_precompile_headers(engine PUBLIC
    <vector>
    <glm/mat4x4.hpp>
    <glm/vec4.hpp>
    <vulkan/vulkan.h>
)

target_link_libraries(${TARGET_NAME} PUBLIC
    fmt::fmt
    glm
    imgui
    SDL3::SDL3
    stb_image
    vma
)
