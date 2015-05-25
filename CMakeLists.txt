cmake_minimum_required(VERSION 2.8)
project(big_wall)

find_package(GLEW REQUIRED)

option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
option(GLFW_INSTALL "Generate installation target" OFF)

add_subdirectory(glfw)
include_directories(glfw/include)
include_directories(include)
include_directories(glm)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
file(GLOB_RECURSE SOURCE_FILES "src/*")
#set(SOURCE_FILES src/main.cpp)


add_definitions(-Dwallpng="${PROJECT_SOURCE_DIR}/data/wall.png")

add_executable(big_wall ${SOURCE_FILES})
target_link_libraries(big_wall glfw ${GLFW_LIBRARIES})
target_link_libraries(big_wall ${GLEW_LIBRARIES})
