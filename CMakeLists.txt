cmake_minimum_required(VERSION 3.16)

project(Phase5HFTOrderBook LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(hft_order_book
    src/main.cpp
)

add_executable(order_book_tests
    tests/test_order_book.cpp
)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(hft_order_book PRIVATE -O3 -Wall -Wextra -pedantic)
    target_compile_options(order_book_tests PRIVATE -O3 -Wall -Wextra -pedantic)
endif()