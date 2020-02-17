cmake_minimum_required(VERSION 3.15)
project(Assignment1)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(KernelHeaders REQUIRED)
include_directories(BEFORE SYSTEM ${KERNELHEADERS_INCLUDE_DIRS})
add_compile_options(-Wall -Wextra -pedantic -Werror -Wshadow
        -Wstrict-overflow -fno-strict-aliasing -Wno-missing-field-initializers)

set(DRIVER_FILE problem4-partIb.ko)
set(KBUILD_CMD $(MAKE) -C ${KERNELHEADERS_DIR} modules M=${CMAKE_CURRENT_BINARY_DIR} src=${CMAKE_CURRENT_SOURCE_DIR})
FILE(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/Kbuild "obj-m := problem4-partIb.o")

add_custom_command(OUTPUT ${DRIVER_FILE}
        COMMAND ${KBUILD_CMD}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS problem4-partIb.c VERBATIM)

add_custom_target(${PROJECT_NAME} ALL DEPENDS ${DRIVER_FILE})