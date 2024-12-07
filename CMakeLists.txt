cmake_minimum_required(VERSION 3.28)
project(custom_error_code)

set(CMAKE_CXX_STANDARD 23)

find_package(magic_enum REQUIRED)
find_package(fmt REQUIRED)

add_executable(custom_error_code main.cpp)
target_link_libraries(custom_error_code magic_enum::magic_enum fmt::fmt)
