cmake_minimum_required(VERSION 2.8)

project(final)
set (LIBS "-pthread")

add_executable(final server.cpp)
target_link_libraries(final ${LIBS})


