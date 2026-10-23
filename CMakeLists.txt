# CMakeList.txt: SuzzaEngine 的 CMake 项目
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)

# 在正式发布时将此设置为ON而不是OFF
# 设置是否为生产构建
# Make Sure PRODUCTION_BUILD WHEN SEND TO INTERNET
set(PRODUCTION_BUILD OFF CACHE BOOL "Make this a production build" FORCE)
# 资产输出目录
set(ASSETS_DIR "${CMAKE_BINARY_DIR}/assets")

if (MSVC)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
	add_compile_options(-mavx2 -mfma)
endif()

project (SuzzaEngine)

find_package(Vulkan REQUIRED)

add_subdirectory(thirdparty/glfw)
add_subdirectory(thirdparty/glm)
add_subdirectory(thirdparty/stb_image)


add_subdirectory(tool/broadcast)
add_subdirectory(tool/imageloader)


set(SHADERS_DIR "${CMAKE_SOURCE_DIR}/shaders")
# 输出spv存放目录，构建目录下生成，不污染源码
set(SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/shaders")

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
	${PROJECT_BINARY_DIR}/config.h
)

# 着色器源文件列表
set(SHADER_SOURCES
    "shaders/simple_vertexshader.vert"
    "shaders/simple_fragmentshader.frag"
)

file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})

set(SHADER_SPV_FILES "")

foreach(shader ${SHADER_SOURCES})
    # 获取文件名
    get_filename_component(shader_name ${shader} NAME)
    set(spv_file ${SHADER_OUTPUT_DIR}/${shader_name}.spv)

    add_custom_command(
        OUTPUT ${spv_file}
        MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${shader}
        COMMAND Vulkan::glslc
            ${CMAKE_CURRENT_SOURCE_DIR}/${shader}
            -o ${spv_file}
        COMMENT "Compiling shader ${shader_name}"
        VERBATIM
    )
    list(APPEND SHADER_SPV_FILES ${spv_file})
endforeach()

# 自定义目标，触发着色器编译
add_custom_target(shaders ALL
    DEPENDS ${SHADER_SPV_FILES}
)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# 将源代码添加到此项目的可执行文件。
add_executable("${CMAKE_PROJECT_NAME}")
set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY CXX_STANDARD 20)
# 让可执行程序依赖 shaders 目标：编译app前自动编译着色器
add_dependencies("${CMAKE_PROJECT_NAME}" shaders)

if(PRODUCTION_BUILD)

	# 设置生产环境变量, 处于生产环境时为1
	# PRODUCTION_BUILD value is 1 WHEN IN PRODUCTION_BUILD
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=1)

else()
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=0)

endif()

target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${MY_SOURCES})

if(MSVC)
	
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC _CRT_SECURE_NO_WARNINGS)

	if(PRODUCTION_BUILD)
		set_target_properties("${CMAKE_PROJECT_NAME}"
		PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")		# Remove black console
	endif()

elseif(MINGW)	# Windows / MinGW GCC / Clang
	if(PRODUCTION_BUILD)
		target_link_options("${CMAKE_PROJECT_NAME}" PRIVATE
		-mwindows)
	endif()

elseif(APPLE OR UNIX)	# MacOS / Linux
	if(PRODUCTION_BUILD)
		target_link_options("${CMAKE_PROJECT_NAME}" PRIVATE
		-s)
	endif()

endif()

target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/src"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/io"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/engine"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/scene"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/common"
)
# Include Binary Files
target_include_directories("${PROJECT_NAME}" PRIVATE "${PROJECT_BINARY_DIR}")

target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE
	glfw
	glm
	Vulkan::Vulkan
	Brocast_Delegate_Moudle
	Image_Loader_Moudle
	stb_image
)

# 拷贝资产文件目录
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		   "${CMAKE_SOURCE_DIR}/assets"
		   $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/assets	
)