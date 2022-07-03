# CMakeList.txt: CMakeProject1 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.10)

# 添加预编译宏
add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)

option(AUTO_LOCATE_VULKAN "AUTO_LOCATE_VULKAN" ON)
set(CMAKE_CXX_STANDARD 17)
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


find_program(GLSLC_PROGRAM glslc REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
    

file(GLOB_RECURSE TARGET_HEADER *.hpp)
file(GLOB_RECURSE TARGET_SOURCE *.cpp)

# 将源代码添加到此项目的可执行文件。
add_executable (CMakeProject1 ${TARGET_HEADER} ${TARGET_SOURCE})

message(STATUS ${TARGET_SOURCE})
message(STATUS ${TARGET_HEADER})

target_link_libraries(CMakeProject1 PUBLIC	glm::glm 
											Vulkan::Vulkan
											SDL2::SDL2
											glfw) 

target_include_directories(CMakeProject1 PUBLIC ${Vulkan_INCLUDE_DIRS})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CMakeProject1 PROPERTY CXX_STANDARD 20)
endif()

# TODO: 如有需要，请添加测试并安装目标。
