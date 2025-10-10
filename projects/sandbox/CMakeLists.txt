##########
# TARGET #
##########

set(TARGET_NAME sandbox)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/${TARGET_NAME})

file(GLOB_RECURSE SOURCES *.cpp)
file(GLOB_RECURSE HEADERS *.hpp)

add_executable(${TARGET_NAME})

set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 20)

target_sources(${TARGET_NAME}
    PRIVATE
        ${SOURCES}

    PUBLIC FILE_SET HEADERS
    BASE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    FILES
        ${HEADERS}
)

target_link_libraries(${TARGET_NAME}
    PRIVATE
        SDL3::SDL3
        vk-bootstrap::vk-bootstrap
        volk::volk
        GPUOpen::VulkanMemoryAllocator
        glm
        imgui
        fmt::fmt
        Tracy::TracyClient
)



###########
# SHADERS #
###########

find_program(SLANG_COMPILER slangc REQUIRED)

set(SHADER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders)

set(ENTRY_POINTS -fvk-use-entrypoint-name)
list(APPEND ENTRY_POINTS -entry taskMain)
list(APPEND ENTRY_POINTS -entry meshMain)
list(APPEND ENTRY_POINTS -entry fragMain)

add_custom_command(
    OUTPUT ${SHADER_PATH}/triangle.spv
    COMMAND ${SLANG_COMPILER} ${SHADER_PATH}/triangle.slang -target spirv ${ENTRY_POINTS} -o ${SHADER_PATH}/triangle.spv
)

add_custom_command(
    OUTPUT ${SHADER_PATH}/triangle.spv-asm
    COMMAND ${SLANG_COMPILER} ${SHADER_PATH}/triangle.slang -target spirv-asm ${ENTRY_POINTS} -o ${SHADER_PATH}/triangle.spv-asm
)

add_custom_target(
    ${TARGET_NAME}-shaders
    DEPENDS
        ${SHADER_PATH}/triangle.spv
        ${SHADER_PATH}/triangle.spv-asm
)

add_dependencies(${TARGET_NAME} ${TARGET_NAME}-shaders)



###########
# DEFINES #
###########

target_compile_definitions(${TARGET_NAME}
    PRIVATE
        SHADER_PATH="${SHADER_PATH}"
        GLM_FORCE_DEPTH_ZERO_TO_ONE

        # For volk
        VK_NO_PROTOTYPES
)



###########
# WINDOWS #
###########

if(WIN32)
    copy_runtime_dlls(${TARGET_NAME})
endif()
