cmake_minimum_required(VERSION 3.21)

project(test_project)
set(CMAKE_C_STANDARD 17)

add_executable(test_project
        main.c
        copy_input.h
        copy_input.c
        sanitize_cookie_path.h
        sanitize_cookie_path.c
        )