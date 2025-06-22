cmake_minimum_required(VERSION 3.16)

project(ChateauChess CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type (Debug, Release)" FORCE)
endif()

include_directories(include)

file (GLOB_RECURSE SOURCES src/*.cpp)

add_executable(Engine ${SOURCES})

target_compile_features(Engine PRIVATE cxx_std_23)

# target_compile_options(Engine PRIVATE -O2 -O3 -march=native)

target_compile_options(Engine PRIVATE
    $<$<CONFIG:Debug>:-g -march=native>
    $<$<CONFIG:Release>:-O2 -O3 -march=native>
)