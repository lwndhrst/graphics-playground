find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)
find_program(SLANG_COMPILER slangc)

function(compile_shaders TARGET_NAME SHADER_PATH)

    ########
    # GLSL #
    ########

    file(GLOB_RECURSE GLSL_SOURCES
        "${SHADER_PATH}/*.frag"
        "${SHADER_PATH}/*.vert"
        "${SHADER_PATH}/*.comp"
    )

    foreach(GLSL ${GLSL_SOURCES})
        message(STATUS "Found GLSL shader: ${GLSL}")

        set(SPIRV "${GLSL}.spv")

        add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
            DEPENDS ${GLSL}
        )

        list(APPEND SPIRV_BINARY_FILES ${SPIRV})
    endforeach(GLSL)

    #########
    # SLANG #
    #########

    file(GLOB_RECURSE SLANG_SOURCES
        "${SHADER_PATH}/*.slang"
    )

    foreach(SLANG ${SLANG_SOURCES})
        message(STATUS "Found Slang shader: ${SLANG}")

        set(SPIRV "${SLANG}.spv")

        add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${SLANG_COMPILER} ${SLANG} -o ${SPIRV}
            DEPENDS ${SLANG}
        )

        list(APPEND SPIRV_BINARY_FILES ${SPIRV})
    endforeach(SLANG)

    add_custom_target(
        ${TARGET_NAME}-shaders
        DEPENDS ${SPIRV_BINARY_FILES}
    )

    add_dependencies(${TARGET_NAME} ${TARGET_NAME}-shaders)

endfunction()
