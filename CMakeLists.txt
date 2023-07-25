project("ezUnpacker")

cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_FILES_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build/")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/")
set(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin/")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin/") 

set(PROJECT_DIR "${CMAKE_SOURCE_DIR}/")
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")

#dependency paths
set(FORMATPE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/formatPE")
set(ASMJIT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/asmjit")
set(JSON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/json")

message(STATUS "Adding files from ${SRC_DIR}")

file(GLOB_RECURSE PROJECT_DIR
    "${SRC_DIR}/**.hpp"
    "${SRC_DIR}/**.h"
    "${SRC_DIR}/**.cpp"
    "${SRC_DIR}/**.txt"
    "${SRC_DIR}/**.cc"
    "${SRC_DIR}/**.cxx"
    "${SRC_DIR}/**.asm"
)

add_executable("ezUnpacker" "${PROJECT_DIR}")
target_include_directories("ezUnpacker" PRIVATE "${SRC_DIR}")

#set_property(TARGET "ArxanStripper" PROPERTY COMPILE_WARNING_AS_ERROR ON)

target_precompile_headers("ezUnpacker" PRIVATE "${SRC_DIR}/pch.hpp")
add_compile_definitions("ezUnpacker" 
    "NOMINMAX"
    "WIN32_LEAN_AND_MEAN"
)

message(STATUS "Adding required packages")


message("Adding formatPE from ${FORMATPE_DIR}")
add_subdirectory("${FORMATPE_DIR}")  # As subfolder

message("Adding asmjit from ${ASMJIT_DIR}")
add_subdirectory("${ASMJIT_DIR}")

target_link_libraries("ezUnpacker" PRIVATE 
    formatPE::Pe
    asmjit
)

message("Adding nolhmann/json from ${JSON_DIR}")

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(${JSON_DIR})
target_link_libraries("ezUnpacker" PRIVATE nlohmann_json::nlohmann_json)