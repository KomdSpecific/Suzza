# CMakeList.txt: VulkanEngine 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)

project (Brocast_Delegate_Moudle)


add_library("${PROJECT_NAME}" INTERFACE)

target_include_directories("${PROJECT_NAME}" INTERFACE
	"${CMAKE_CURRENT_SOURCE_DIR}/src"
)