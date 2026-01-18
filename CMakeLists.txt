cmake_minimum_required(VERSION 4.1)
project(nesquik C)

set(CMAKE_C_STANDARD 11)

add_executable(nesquik main.c
        src/list/list.c
        src/hash/hash.c
        src/hash/hashtable.c
        src/hash/hashset.c
        src/hash/pointer_hashtable.c
        src/hash/pointer_hashset.c)

target_include_directories(nesquik PUBLIC include)
