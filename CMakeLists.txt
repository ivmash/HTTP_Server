cmake_minimum_required(VERSION 2.8)

project(final)
set (LIBS "-lstdc++fs -std=c++17 -pthread")

add_executable(final server.cpp)
target_link_libraries(final ${LIBS})


