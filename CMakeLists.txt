cmake_minimum_required(VERSION 2.8.7 FATAL_ERROR)
project(lightconf_test)

set(GTEST_DIR $ENV{GTEST_DIR})

add_executable(lightconf_sample
    sample/lightconf_sample.cpp
)

if(WIN32)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
else(WIN32)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
endif(WIN32)

add_definitions("-std=gnu++11 -g -Wall")

find_package(GTest)
find_package(Threads)

if (GTEST_FOUND)
    add_executable(lightconf_test
        test/path.cpp
        test/group.cpp
        test/scanner.cpp
        test/readwrite.cpp
    )
    message("Found GTest...compiling test project.")
    message(${GTEST_INCLUDE_DIRS})
    include_directories(${GTEST_INCLUDE_DIRS})

    target_link_libraries(lightconf_test ${GTEST_BOTH_LIBRARIES})
else (GTEST_FOUND)
    message("Could not find GTest...skipping test project.")
endif (GTEST_FOUND)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})