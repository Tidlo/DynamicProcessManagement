cmake_minimum_required(VERSION 3.14)
project(dynamic_process_manage)

set(CMAKE_CXX_STANDARD 14)

find_package(MPI REQUIRED)
INCLUDE_DIRECTORIES(SYSTEM ${MPI_INCLUDE_PATH})

add_executable(Master master.cpp main.h)
add_executable(Slave slave.cpp main.h)
target_link_libraries(Master ${MPI_CXX_LIBRARIES})
target_link_libraries(Slave ${MPI_CXX_LIBRARIES})