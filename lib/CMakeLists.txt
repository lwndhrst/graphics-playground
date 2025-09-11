set(TARGET_NAME engine)

add_library(${TARGET_NAME} STATIC
    engine.cpp
)

set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 20)

target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
