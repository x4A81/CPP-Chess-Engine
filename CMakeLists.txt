cmake_minimum_required(VERSION 3.16)

project(Engine CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type (Debug, Release)" FORCE)
endif()

file (GLOB_RECURSE SOURCES src/*.cpp)

add_executable(Engine ${SOURCES})

target_compile_features(Engine PRIVATE cxx_std_23)
target_include_directories(Engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_options(Engine PRIVATE
    $<$<CONFIG:Debug>:>
    $<$<CONFIG:Release>: -march=native>
)

# cmake -S "path to CMakeLists.txt" -B build -DCMAKE_BUILD_TYPE=Debug
# or
# cmake -S "path to CMakeLists.txt" -B build -DCMAKE_BUILD_TYPE=Release
# then
# cmake --build build