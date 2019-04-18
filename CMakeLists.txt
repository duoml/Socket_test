cmake_minimum_required(VERSION 3.13)
project(Socket_test)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(pipe)
add_subdirectory(test_user)
add_subdirectory(multiplexing)
add_subdirectory(client)
add_subdirectory(Chat)
add_subdirectory(timer)

add_executable(Socket_test main.cpp )