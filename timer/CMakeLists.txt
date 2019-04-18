cmake_minimum_required(VERSION 3.13)
project(timer)

include_directories(..)
aux_source_directory(. SRC)
add_executable(${PROJECT_NAME} ${SRC} time_wheel_timer.h min_heap_timer.h)
