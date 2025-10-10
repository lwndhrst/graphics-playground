cmake_minimum_required (VERSION 3.8)

project("graphics-playground")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)



###########
# OPTIONS #
###########

message(STATUS)
message(STATUS "Graphics Playground")
message(STATUS)
message(STATUS "Enabled options:")

if(ENABLE_ASAN)
    message(STATUS "  ASan")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -fno-omit-frame-pointer -fsanitize=address")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -fno-omit-frame-pointer -fsanitize=address")
endif()

if(ENABLE_TRACY)
    message(STATUS "  Tracy")
    set(TRACY_ENABLE ON CACHE BOOL "" FORCE)
else()
    set(TRACY_ENABLE OFF CACHE BOOL "" FORCE)
endif()

message(STATUS)



################
# DEPENDENCIES #
################

include(FetchDependencies)



########
# UTIL #
########

include(CopyRuntimeDlls)



###########
# TARGETS #
###########

add_subdirectory(projects/sandbox)
add_subdirectory(projects/hello-triangle)
