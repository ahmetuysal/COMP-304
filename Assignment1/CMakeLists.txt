# cmake_minimum_required(VERSION <specify CMake version here>)
project(Assignment1)

set(CMAKE_CXX_STANDARD 14)

# add_executable(Assignment1 Problem1/problem1-partI.c)
# add_executable(Assignment1 Problem1/problem1-partII.c)
# add_executable(Assignment1 Problem2/problem2-partI.c)
# add_executable(Assignment1 Problem2/problem2-partII.c)
# add_executable(Assignment1 Problem3/problem3-partI.c)
add_executable(Assignment1 Problem3/problem3-partII.c)

target_link_libraries(Assignment1 rt)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Problem3/input.txt
        ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Problem3/words.txt
        ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)