cmake_minimum_required(VERSION 3.20)
message(STATUS "CMake version: ${CMAKE_VERSION}")

project(ThreadPool)

add_executable(test_thread_pool test_thread_pool.cpp)