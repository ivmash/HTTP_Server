cmake_minimum_required(VERSION 2.8)

project(final)
set (LIBS "-pthread -std=c++11")
add_compile_options(-pthread -std=c++11)
add_executable(final server.cpp)
target_link_libraries(final ${LIBS})


