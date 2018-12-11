cmake_minimum_required(VERSION 3.2)
project(function2containtest VERSION 2.2.0 LANGUAGES CXX)

find_package(function2 2)

add_executable(function2-dist-package main.cpp)

target_link_libraries(function2-dist-package function2::function2)
