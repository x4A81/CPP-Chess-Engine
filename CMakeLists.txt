cmake_minimum_required(VERSION 3.16)

project(ChateauChess CXX)

include_directories(include)

file (GLOB_RECURSE SOURCES src/*.cpp)

add_executable(Engine ${SOURCES})

target_compile_features(Engine PRIVATE cxx_std_23)

target_compile_options(Engine PRIVATE -O2 -O3 -march=native)