set(TARGET_NAME sandbox)

add_executable(${TARGET_NAME}
    src/main.cpp
)

target_include_directories(${TARGET_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 20)

target_link_libraries(${TARGET_NAME} PUBLIC goose)



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
