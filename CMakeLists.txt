cmake_minimum_required(VERSION 3.28)
project(MeinFischCandidate)

set(CMAKE_CXX_STANDARD 20)
# Nur für Windows + MinGW
if (MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -static-libgcc -static-libstdc++")
endif()

add_compile_options(-g -O3)

add_subdirectory(src)
add_subdirectory(test)
