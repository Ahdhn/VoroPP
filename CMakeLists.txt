cmake_minimum_required(VERSION 3.4)

project(VoroPPApp) 

# List of headers locations
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/voro")

# VoroPP library 
file( GLOB_RECURSE lib_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(VoroPP STATIC ${lib_sources})

# App 
file(GLOB_RECURSE app_sources ${CMAKE_CURRENT_SOURCE_DIR}/app/*.*)

add_executable( VoroPPApp ${app_sources})

target_link_libraries( VoroPPApp VoroPP )