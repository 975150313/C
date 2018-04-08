cmake_minimum_required(VERSION 3.8)
project(HttpClient)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp mongoose.c mongoose.h main.cpp)
link_libraries(ws2_32)
add_executable(HttpClient ${SOURCE_FILES})