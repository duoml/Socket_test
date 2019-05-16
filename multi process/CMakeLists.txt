cmake_minimum_required(VERSION 3.10)
project(mulit_process)

aux_source_directory(. SRC)
add_executable(${PROJECT_NAME} ${SRC} include.h)
target_link_libraries(${PROJECT_NAME} rt)
