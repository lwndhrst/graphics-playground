#######
# fmt #
#######

add_subdirectory(fmt EXCLUDE_FROM_ALL)



#######
# glm #
#######

add_library(glm INTERFACE)

# header-only library
target_include_directories(glm INTERFACE glm)



#######
# vma #
#######

add_library(vma INTERFACE)

# header-only library
target_include_directories(vma INTERFACE VulkanMemoryAllocator/include)



#######
# sdl #
#######

add_subdirectory(SDL EXCLUDE_FROM_ALL)



#########
# imgui #
#########

add_library(imgui STATIC)

target_include_directories(imgui PUBLIC imgui)

target_sources(imgui PRIVATE 
    imgui/imgui.h
    imgui/imgui.cpp

    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp

    imgui/backends/imgui_impl_sdl3.cpp
    imgui/backends/imgui_impl_vulkan.cpp
)

target_link_libraries(imgui PUBLIC Vulkan::Vulkan SDL3::SDL3)
