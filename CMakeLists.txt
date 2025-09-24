cmake_minimum_required (VERSION 3.8)

project("graphics-playground")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)



################
# DEPENDENCIES #
################

find_package(Vulkan REQUIRED)

find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)

add_subdirectory(dependencies)



########
# UTIL #
########

function(compile_shaders TARGET_NAME SHADER_PATH)
    file(GLOB_RECURSE GLSL_SOURCE_FILES
        "${SHADER_PATH}/*.frag"
        "${SHADER_PATH}/*.vert"
        "${SHADER_PATH}/*.comp"
    )

    foreach(GLSL ${GLSL_SOURCE_FILES})
        message(STATUS "")
        message(STATUS "FOUND SHADER")
        get_filename_component(FILE_NAME ${GLSL} NAME)
        set(SPIRV "${SHADER_PATH}/${FILE_NAME}.spv")
        message(STATUS ${GLSL})
        message(STATUS "")
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

function(copy_runtime_dlls TARGET_NAME)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${TARGET_NAME}> $<TARGET_FILE_DIR:${TARGET_NAME}>
        COMMAND_EXPAND_LISTS
    )
endfunction()



###########
# TARGETS #
###########

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

add_subdirectory(lib/goose)
add_subdirectory(app/sandbox)
