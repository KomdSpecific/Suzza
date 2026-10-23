cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)

project (Suzza_Shared_Moudle)

add_library("${PROJECT_NAME}" INTERFACE)

target_include_directories("${PROJECT_NAME}" INTERFACE
	"${CMAKE_CURRENT_SOURCE_DIR}"
)