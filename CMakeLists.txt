cmake_minimum_required(VERSION 3.16)
project(bones)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add_library(ufbx STATIC external/ufbx/ufbx.c)
# target_include_directories(ufbx PUBLIC external/ufbx)

# add_subdirectory(external/ufbx)
# target_link_libraries(bones PRIVATE ufbx)

# add_executable(bones main.cpp)
# target_link_libraries(bones PRIVATE ufbx)

include_directories(external/ufbx)

add_executable(bones
    main.cpp
    external/ufbx/ufbx.c
)
