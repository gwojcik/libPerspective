cmake_minimum_required(VERSION 3.14)    # 3.14 - UseSWIG
cmake_policy(SET CMP0078 NEW) # ``UseSWIG`` generates standard target names.
cmake_policy(SET CMP0054 NEW) # Only interpret ``if()`` arguments as variables or keywords when unquoted.

project(libperspective)

# set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(use_python_3 TRUE)
if (use_python_3)
    find_package (Python3 COMPONENTS Interpreter Development)
    set(python_libraries ${Python3_LIBRARIES})
    set(python_inlude_dirs ${Python3_INCLUDE_DIRS})
else()
    find_package (Python2 COMPONENTS Interpreter Development)
    set(python_libraries ${Python2_LIBRARIES})
    set(python_inlude_dirs ${Python2_INCLUDE_DIRS})
endif()

find_package(Catch2 QUIET)
if (Catch2_FOUND)
    include(CTest)
    include(Catch)
    add_executable(tests tests/quaternion.cpp tests/main.cpp tests/graph.cpp Graph.cpp Projection.cpp tests/graph_python.cpp PythonGraph.cpp)
    target_link_libraries(tests PRIVATE Catch2::Catch2 ${python_libraries})
    target_include_directories(tests PRIVATE ${Catch2_INCLUDE_DIRS} ${python_inlude_dirs})
    target_compile_options(tests PRIVATE -O0 -ggdb3 -std=c++14 -Wall -Wextra)
    set_property(TARGET tests PROPERTY CXX_STANDARD 14)
    catch_discover_tests(tests)
endif()


find_package(SWIG REQUIRED)
include(UseSWIG)
INCLUDE(${SWIG_USE_FILE})
set(CMAKE_SWIG_FLAGS "")
set_source_files_properties(libperspective.i PROPERTIES CPLUSPLUS ON)
set_source_files_properties(libperspective.i PROPERTIES GENERATED_COMPILE_OPTIONS "-std=c++14")
# set_source_files_properties(libperspective.i PROPERTIES SWIG_FLAGS "-includeall")
swig_add_library(libperspective TYPE SHARED LANGUAGE python SOURCES libperspective.i Projection.cpp Graph.cpp RawData.cpp PythonGraph.cpp)
target_include_directories(libperspective PRIVATE "." ${python_inlude_dirs})
target_link_libraries(libperspective PRIVATE ${python_libraries})
target_compile_options(libperspective PRIVATE -ggdb3 -std=c++14 -Wall -Wextra)
target_link_options(libperspective PRIVATE)

install(TARGETS libperspective RUNTIME DESTINATION bin)
