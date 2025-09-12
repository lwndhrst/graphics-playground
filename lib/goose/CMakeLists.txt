set(TARGET_NAME goose)

file(GLOB_RECURSE SOURCES *.cpp)
file(GLOB_RECURSE HEADERS *.h)

add_library(${TARGET_NAME} STATIC
    ${SOURCES}
)

target_include_directories(${TARGET_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")

set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 20)

target_link_libraries(${TARGET_NAME} PRIVATE
    fmt::fmt
    glm
    imgui
    SDL3::SDL3
    stb_image
    vma
)



###########
# DEFINES #
###########

target_compile_definitions(${TARGET_NAME} PRIVATE GLM_FORCE_DEPTH_ZERO_TO_ONE)
