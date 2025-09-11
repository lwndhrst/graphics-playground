cmake_minimum_required (VERSION 3.8)

project("graphics-playground")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)



################
# DEPENDENCIES #
################

find_package(Vulkan REQUIRED)



###########
# TARGETS #
###########

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

add_subdirectory(lib)
add_subdirectory(apps/sandbox)
