cmake_minimum_required(VERSION 3.20.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(wfc_sudoku VERSION 1.0.0 DESCRIPTION "Sudoku creator using wave function collapse")
add_executable(wfc_sudoku src/sudoku.cpp)
project(wfc_image VERSION 1.0.0 DESCRIPTION "Image generator using wave function collapse")
add_executable(wfc_image src/image.cpp)