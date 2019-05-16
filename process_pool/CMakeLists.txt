cmake_minimum_required(VERSION 3.10)
project(process_pool)
aux_source_directory(. SRC)
add_executable(${PROJECT_NAME} ${SRC} process_pool.cc process_pool.h)