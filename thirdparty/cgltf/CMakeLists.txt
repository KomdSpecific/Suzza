# CMakeList.txt: GameEngine 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

project (cgltf)

add_library(cgltf INTERFACE)
target_include_directories(cgltf INTERFACE ${PROJECT_SOURCE_DIR})