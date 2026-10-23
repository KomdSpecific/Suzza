
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)

project (imgui_docking)

file(GLOB_RECURSE IMGUI_SOURCES CONFIGURE_DEPENDS
	"${CMAKE_CURRENT_SOURCE_DIR}/imgui.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/imgui_demo.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/imgui_draw.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/imgui_widgets.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/imgui_tables.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/backends/imgui_impl_glfw.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp"
)

add_library("${PROJECT_NAME}")
target_sources("${PROJECT_NAME}" PRIVATE "${IMGUI_SOURCES}")

target_include_directories("${PROJECT_NAME}" PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/backends"
)

target_link_libraries("${PROJECT_NAME}" PRIVATE
    glfw
    Vulkan::Vulkan
)
