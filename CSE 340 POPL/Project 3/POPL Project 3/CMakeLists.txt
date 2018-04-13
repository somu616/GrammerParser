cmake_minimum_required(VERSION 3.8)
project(POPL_Project_3)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        "inputbuf.cc"
        "lexer.cc"
        "parser.cc")

add_executable(POPL_Project_3 ${SOURCE_FILES})