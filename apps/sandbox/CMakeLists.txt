set(TARGET_NAME sandbox)

add_executable(${TARGET_NAME}
    src/main.cpp
)

set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 20)

target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${TARGET_NAME} PUBLIC engine)



###########
# SHADERS #
###########

compile_shaders(${TARGET_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
