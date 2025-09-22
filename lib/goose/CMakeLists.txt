set(TARGET_NAME goose)

file(GLOB_RECURSE SOURCES *.cpp)
file(GLOB_RECURSE HEADERS *.hpp)

add_library(${TARGET_NAME} STATIC)

set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 20)

target_sources(${TARGET_NAME}
    PRIVATE
        ${SOURCES}

    PUBLIC FILE_SET HEADERS
    BASE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/..
    FILES
        ${HEADERS}
)

target_link_libraries(${TARGET_NAME}
    PRIVATE
        stb_image
        SDL3::SDL3

    PUBLIC
        Vulkan::Vulkan
        vma
        fmt::fmt
        glm
        imgui
)



###########
# DEFINES #
###########

target_compile_definitions(${TARGET_NAME}
    PUBLIC
        $<$<CONFIG:Debug>:GOOSE_DEBUG>
        $<$<CONFIG:RelWithDebInfo>:GOOSE_DEBUG>

        GLM_FORCE_DEPTH_ZERO_TO_ONE
)
