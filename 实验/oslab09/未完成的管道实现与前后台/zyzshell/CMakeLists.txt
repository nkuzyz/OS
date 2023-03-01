cmake_minimum_required(VERSION 3.23)
project(minishell)

set(CMAKE_CXX_STANDARD 17)

add_executable(minishell main.c parse.c init.c builtin.c)
