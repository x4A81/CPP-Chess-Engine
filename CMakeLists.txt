cmake_minimum_required(VERSION 3.16)

project(Engine VERSION 1.0.0 LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type (Debug, Release)" FORCE)
endif()

file (GLOB_RECURSE SOURCES src/*.cpp)

add_executable(Engine ${SOURCES})

target_compile_features(Engine PRIVATE cxx_std_23)
target_include_directories(Engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_definitions(Engine PRIVATE
    ENGINE_VERSION="${PROJECT_VERSION}"
)

include(CheckIPOSupported)
check_ipo_supported(RESULT result OUTPUT output)
if(result)
    set_property(TARGET Engine PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
endif()

set_target_properties(Engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

target_link_libraries(Engine PRIVATE
    $<$<CONFIG:Debug>:>
)

target_compile_options(Engine PRIVATE
    $<$<CONFIG:Debug>: -g -march=native>
    $<$<CONFIG:Release>: -O3 -march=native -DNDEBUG>
)

# cmake -S "path to CMakeLists.txt" -B build -DCMAKE_BUILD_TYPE=Debug
# or
# cmake -S "path to CMakeLists.txt" -B build -DCMAKE_BUILD_TYPE=Release
# then
# cmake --build build