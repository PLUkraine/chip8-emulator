cmake_minimum_required(VERSION 3.1)

# Fix behavior of CMAKE_CXX_STANDARD when targeting macOS.
if (POLICY CMP0025)
  cmake_policy(SET CMP0025 NEW)
endif ()

project(chip8-emulator)

# Find SDL2 library
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

# main app
add_executable(${PROJECT_NAME} "main.cpp" "Chip8.cpp" "Chip8.h")
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY})

# set c++11 standard
set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++")
# set wall and wextra
if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
endif(MSVC)
