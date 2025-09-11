cmake_minimum_required (VERSION 3.8)

project("graphics-playground")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)



################
# DEPENDENCIES #
################

find_package(Vulkan REQUIRED)

find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)



########
# UTIL #
########

function(compile_shaders TARGET_NAME SHADER_DIR)
    file(GLOB_RECURSE GLSL_SOURCE_FILES
        "${SHADER_DIR}/*.frag"
        "${SHADER_DIR}/*.vert"
        "${SHADER_DIR}/*.comp"
    )

    foreach(GLSL ${GLSL_SOURCE_FILES})
        get_filename_component(FILE_NAME ${GLSL} NAME)
        set(SPIRV "${SHADER_DIR}/${FILE_NAME}.spv")
        message(STATUS ${GLSL})
        add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
            DEPENDS ${GLSL})
        list(APPEND SPIRV_BINARY_FILES ${SPIRV})
    endforeach(GLSL)

    add_custom_target(
        ${TARGET_NAME}-shaders
        DEPENDS ${SPIRV_BINARY_FILES}
    )

    add_dependencies(${TARGET_NAME} ${TARGET_NAME}-shaders)
endfunction()



###########
# TARGETS #
###########

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

add_subdirectory(lib)
add_subdirectory(apps/sandbox)
