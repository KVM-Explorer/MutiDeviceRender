# CMakeList.txt: CMakeProject1 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.10)

# 添加预编译宏
add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)

option(AUTO_LOCATE_VULKAN "AUTO_LOCATE_VULKAN" ON)
set(CMAKE_CXX_STANDARD 17)
set(TARGET_NAME "SFR")
find_package(glm REQUIRED)

if(AUTO_LOCATE_VULKAN)
	message(STATUS "Attempting auto locate Vulkan using Cmake ......")
	find_package(Vulkan)
	#include_directories(${Vulkan_INCLUDE_DIRS}/include)
	#message(STATUS "vulkan=${Vulkan_INCLUDE_DIRS}")
	if(NOT Vulkan_FOUND)
		message(STATUS "Failed find vulkan SDK")
	endif()
endif()


# TODO: 如有需要，请添加测试并安装目标。
function(add_shader TARGET SHADER)
    find_program(GLSLC glslc)

    get_filename_component(CURRENT_FOLDER ${CMAKE_CURRENT_SOURCE_DIR} NAME)

    set(current-shader-path ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${SHADER})
    set(current-output-path ${CMAKE_BINARY_DIR}/${CURRENT_FOLDER}/shaders/${SHADER}.spv)

    # Add a custom command to compile GLSL to SPIR-V.
    get_filename_component(current-output-dir ${current-output-path} DIRECTORY)
    file(MAKE_DIRECTORY ${current-output-dir})

    add_custom_command(
           OUTPUT ${current-output-path}
           COMMAND ${GLSLC} -o ${current-output-path} ${current-shader-path}
           DEPENDS ${current-shader-path}
           IMPLICIT_DEPENDS CXX ${current-shader-path}
           VERBATIM)

    # Make sure our build depends on this output.
    set_source_files_properties(${current-output-path} PROPERTIES GENERATED TRUE)
    target_sources(${TARGET} PRIVATE ${current-output-path})
endfunction(add_shader)

find_package(SDL2 CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb.h")
    
    

file(GLOB_RECURSE TARGET_HEADER *.hpp)
file(GLOB_RECURSE TARGET_SOURCE *.cpp)

# 将源代码添加到此项目的可执行文件。
add_executable (${TARGET_NAME} ${TARGET_HEADER} ${TARGET_SOURCE})

add_shader(${TARGET_NAME} "RayTrace.comp")
add_shader(${TARGET_NAME} "Texture.vert")
add_shader(${TARGET_NAME} "Texture.frag")
add_shader(${TARGET_NAME} "Common.vert")
add_shader(${TARGET_NAME} "Common.frag")

message(STATUS ${TARGET_SOURCE})
message(STATUS ${TARGET_HEADER})

target_link_libraries(${TARGET_NAME} PUBLIC	glm::glm 
											Vulkan::Vulkan
											SDL2::SDL2
											glfw) 


target_include_directories(${TARGET_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS} ${STB_INCLUDE_DIRS})


