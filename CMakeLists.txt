cmake_minimum_required(VERSION 3.16)

option(BUILD_ECS_TEST "build ecs test" OFF)

set(TARGET ecs)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# default release
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
    message(STATUS "No build type selected, default to Release")
endif()

project(
    ${TARGET}
    VERSION 0.1.0
    DESCRIPTION "A simple ECS(Entity Component System) framework written in C++23"
    HOMEPAGE_URL "https://github.com/m1dsolo/ecs"
    LANGUAGES C CXX
)

# build ecs
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "src/*.cpp")
add_library(${TARGET} OBJECT ${SRC})
target_include_directories(${TARGET}
    PUBLIC include
)

# build test
if (BUILD_ECS_TEST)
    add_subdirectory(test)
endif()
