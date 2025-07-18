cmake_minimum_required(VERSION 3.16)
project(Engine VERSION 1.0.0 LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose Debug or Release" FORCE)
endif()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(Engine ${SOURCES})

target_compile_features(Engine PRIVATE cxx_std_23)
target_include_directories(Engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_definitions(Engine PRIVATE ENGINE_VERSION="${PROJECT_VERSION}")

# Enable LTO (Link Time Optimization) if supported
include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT lto_output)
if(lto_supported)
    set_property(TARGET Engine PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
endif()

set_target_properties(Engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

target_compile_options(Engine PRIVATE
    $<$<CONFIG:Debug>:-g -march=native>
    $<$<CONFIG:Release>:-O3 -march=native -DNDEBUG>
)

# Can comment out to make the engine play a variaty of book moves.
target_compile_definitions(Engine PRIVATE TOPBOOK) 

# Copy book.bin to output folder after build
add_custom_command(TARGET Engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/book.bin
            ${CMAKE_BINARY_DIR}/bin/book.bin
)

# Usage:
# cmake -S . -B build
# cmake --build build --config Release