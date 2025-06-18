cmake_minimum_required(VERSION 3.22)
project(testing_stuff)

set(CMAKE_CXX_STANDARD 23)

add_executable(testing_stuff main.cpp test_lazy_allocation.cpp)