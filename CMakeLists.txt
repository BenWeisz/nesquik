cmake_minimum_required(VERSION 4.1)
project(nesquik C)

set(CMAKE_C_STANDARD 11)

add_library(nesquik
    src/hash/hash.c
    src/state_machine/state_machine.c)

target_include_directories(nesquik PUBLIC include)