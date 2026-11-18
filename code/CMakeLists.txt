cmake_minimum_required(VERSION 3.16)
project(lexer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(LEXER_STATIC "静态链接，便于分发" ON)

add_executable(lexer
    src/token.cpp
    src/keyword_table.cpp
    src/diagnostic.cpp
    src/lexer.cpp
    src/main.cpp)

target_include_directories(lexer PRIVATE inc)

if (MSVC)
    target_compile_options(lexer PRIVATE /W4 /utf-8)
    if (LEXER_STATIC)
        set_property(TARGET lexer PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
    endif()
else()
    target_compile_options(lexer PRIVATE -Wall -Wextra)
    if (LEXER_STATIC)
        target_link_options(lexer PRIVATE -static)
    endif()
endif()
