
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)

project (Model_Loader_Moudle)

file(GLOB_RECURSE MODEL_LOADER_SOURCES CONFIGURE_DEPENDS
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library("${PROJECT_NAME}")
target_sources("${PROJECT_NAME}" PRIVATE "${MODEL_LOADER_SOURCES}")

target_include_directories("${PROJECT_NAME}" PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries("${PROJECT_NAME}" PRIVATE
	Suzza_Shared_Moudle
	cgltf
	glm
)