cmake_minimum_required(VERSION 3.17)
project(lab03)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS -pthread)

add_executable(server server/server.cpp server/server.h)
add_executable(client client/client.cpp port.h)