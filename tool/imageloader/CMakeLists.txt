# CMakeList.txt: VulkanEngine 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)

project (Image_Loader_Moudle)

file(GLOB_RECURSE IMAGE_LOADER_SOURCES CONFIGURE_DEPENDS
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library("${PROJECT_NAME}")
target_sources("${PROJECT_NAME}" PRIVATE "${IMAGE_LOADER_SOURCES}")

target_include_directories("${PROJECT_NAME}" PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries("${PROJECT_NAME}" PRIVATE
	stb_image
)