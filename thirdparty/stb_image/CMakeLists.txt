# CMakeList.txt: GameEngine 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

project (stb_image)

add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE ${PROJECT_SOURCE_DIR})