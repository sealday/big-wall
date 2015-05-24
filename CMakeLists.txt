cmake_minimum_required(VERSION 3.2)
project(big_wall)


find_package(GLEW REQUIRED)

add_subdirectory(glfw)
include_directories(glfw/include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(SOURCE_FILES main.cpp)

add_executable(big_wall ${SOURCE_FILES})
target_link_libraries(big_wall glfw ${GLFW_LIBRARIES})
