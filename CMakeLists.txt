cmake_minimum_required(VERSION 3.2)
project(big_wall)

find_package(GLEW REQUIRED)

add_subdirectory(glfw)
include_directories(glfw/include)
include_directories(include)
include_directories(glm)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(SOURCE_FILES src/main.cpp)

add_executable(big_wall ${SOURCE_FILES})
target_link_libraries(big_wall glfw ${GLFW_LIBRARIES})
target_link_libraries(big_wall ${GLEW_LIBRARIES})
