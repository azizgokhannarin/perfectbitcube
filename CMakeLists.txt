cmake_minimum_required(VERSION 3.10)
project(PerfectBitCube)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4 /O2)
else()
    add_compile_options(-O3 -Wall -Wextra)
endif()

find_package(Threads REQUIRED)

set(SOURCES
    main.cpp
    BalancedSet.cpp
    CubeSearcherV2.cpp
)

add_executable(perfect_bit_cube ${SOURCES})

target_link_libraries(perfect_bit_cube PRIVATE Threads::Threads)
