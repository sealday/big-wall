cmake_minimum_required(VERSION 2.8)
project(big_wall)

#find_package(GLEW REQUIRED)

option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
option(GLFW_INSTALL "Generate installation target" OFF)

add_subdirectory(glfw)
add_subdirectory(glew-1.12.0)

set(GLEW_SOURCE
	glew-1.12.0/src/glew.c
)

set(GLEW_HEADERS
)

add_library(GLEW_190 STATIC
	${GLEW_SOURCE}
	${GLEW_INCLUDE}
)

target_link_libraries(GLEW_190
	${OPENGL_LIBRARY}
	${EXTRA_LIBS}
)

include_directories(glfw/include)
include_directories(include)
include_directories(glm)
include_directories(glew-1.12.0/include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
file(GLOB_RECURSE SOURCE_FILES "src/*")
#set(SOURCE_FILES src/main.cpp)


add_definitions(-DWALL="${PROJECT_SOURCE_DIR}/data/wall.jpg")
add_definitions(-DFLOOR="${PROJECT_SOURCE_DIR}/data/floor.png")

add_executable(big_wall ${SOURCE_FILES})
target_link_libraries(big_wall glfw ${GLFW_LIBRARIES})
target_link_libraries(big_wall ${GLEW_LIBRARIES})
target_link_libraries(big_wall GLEW_190)
#target_link_libraries(big_wall glew)
