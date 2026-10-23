cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)

project (Suzza_Core_Moudle)

file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library("${PROJECT_NAME}")
target_sources("${PROJECT_NAME}" PRIVATE ${CORE_SOURCES})

target_include_directories("${PROJECT_NAME}" PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries("${PROJECT_NAME}" PRIVATE
	Suzza_Options
	glm
)