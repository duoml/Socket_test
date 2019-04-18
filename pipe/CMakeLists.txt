cmake_minimum_required(VERSION 3.10)
project(pipe_test)

include_directories(${PROJECT_SOURCE_DIR}/../)

add_executable(${PROJECT_NAME} pipe.cc)