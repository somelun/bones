cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME bones)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(ufbx STATIC external/ufbx/ufbx.c)
target_include_directories(ufbx PUBLIC external/ufbx)

find_package(GLEW REQUIRED)
find_package(SDL3 REQUIRED)

file(GLOB SOURCES "src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE ufbx)
target_link_libraries(${PROJECT_NAME} PRIVATE GLEW::GLEW)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)
