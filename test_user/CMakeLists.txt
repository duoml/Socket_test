cmake_minimum_required(VERSION 3.10)
project(test_user)

include_directories(${PROJECT_SOURCE_DIR}/../)

add_executable(${PROJECT_NAME} test_user.cc)
