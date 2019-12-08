cmake_minimum_required(VERSION 3.14)    # 3.14 - UseSWIG
cmake_policy(SET CMP0078 NEW) # ``UseSWIG`` generates standard target names.

project(libperspective)

add_executable(tests tests.cpp)

find_package (Python2 COMPONENTS Interpreter Development)
find_package(SWIG REQUIRED)
include(UseSWIG)
INCLUDE(${SWIG_USE_FILE})
set(CMAKE_SWIG_FLAGS "")
set_source_files_properties(libperspective.i PROPERTIES CPLUSPLUS ON)
# set_source_files_properties(libperspective.i PROPERTIES SWIG_FLAGS "-includeall")
swig_add_library(libperspective TYPE SHARED LANGUAGE python SOURCES libperspective.i Projection.cpp Graph.cpp)
target_include_directories(libperspective PRIVATE "." ${Python2_INCLUDE_DIRS})
target_link_libraries(libperspective PRIVATE ${Python2_LIBRARIES})
target_compile_options(libperspective PRIVATE -flto -O3 -ggdb3 -std=c++17)
target_link_options(libperspective PRIVATE -flto -O3)

install(TARGETS libperspective RUNTIME DESTINATION bin)
