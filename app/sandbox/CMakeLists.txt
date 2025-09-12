set(TARGET_NAME sandbox)

file(GLOB_RECURSE SOURCES *.cpp)
file(GLOB_RECURSE HEADERS *.h)

add_executable(${TARGET_NAME}
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 20)

target_link_libraries(${TARGET_NAME} PRIVATE
    goose
    SDL3::SDL3
    imgui
)



###########
# SHADERS #
###########

set(SHADER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
# set(SHADER_PATH "./shaders") # expect relative path for release

compile_shaders(${TARGET_NAME} ${SHADER_PATH})



###########
# DEFINES #
###########

target_compile_definitions(${TARGET_NAME} PRIVATE SHADER_PATH="${SHADER_PATH}")



###########
# WINDOWS #
###########

if (WIN32)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${TARGET_NAME}> $<TARGET_FILE_DIR:${TARGET_NAME}>
        COMMAND_EXPAND_LISTS
    )
endif()
