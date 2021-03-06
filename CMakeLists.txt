cmake_minimum_required(VERSION 2.8)
project(big_wall)

if (WIN32)
    option(GLFW_USE_OPTIMUS_HPG "Force use of high-performance GPU on Optimus systems" ON)
endif()

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

add_definitions(-DWALL="${PROJECT_SOURCE_DIR}/data/wall.jpg")
add_definitions(-DFLOOR="${PROJECT_SOURCE_DIR}/data/floor.png")
add_definitions(-DDATA="${PROJECT_SOURCE_DIR}/data/")

add_executable(big_wall ${SOURCE_FILES})
target_link_libraries(big_wall glfw ${GLFW_LIBRARIES})

if(WIN32)
    message("no need for big_wall")
else()
    target_link_libraries(big_wall dl)
endif()
