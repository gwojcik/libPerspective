cmake_minimum_required(VERSION 3.0)

project(libperspective)

add_executable(libperspective main.cpp)

install(TARGETS libperspective RUNTIME DESTINATION bin)
