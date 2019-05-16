cmake_minimum_required(VERSION 3.10)
project(multi_thread)

aux_source_directory(. SRC)
add_executable(${PROJECT_NAME} ${SRC} locker.h test.cc)

target_link_libraries(${PROJECT_NAME} pthread)