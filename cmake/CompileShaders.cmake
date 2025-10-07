function(compile_shaders TARGET_NAME SHADER_PATH)
    file(GLOB_RECURSE GLSL_SOURCE_FILES
        "${SHADER_PATH}/*.frag"
        "${SHADER_PATH}/*.vert"
        "${SHADER_PATH}/*.comp"
    )

    foreach(GLSL ${GLSL_SOURCE_FILES})
        get_filename_component(FILE_NAME ${GLSL} NAME)
        set(SPIRV "${SHADER_PATH}/${FILE_NAME}.spv")
        message(STATUS "Found shader: ${GLSL}")
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
