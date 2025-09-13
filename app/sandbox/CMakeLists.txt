set(TARGET_NAME sandbox)

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
        goose
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
    copy_runtime_dlls(${TARGET_NAME})
endif()
