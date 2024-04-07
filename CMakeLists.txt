cmake_minimum_required(VERSION 3.28)
project(hy486_project C)

set(CMAKE_C_STANDARD 11)

add_executable(hy486_project main.c
        stack/stack.c
        stack/stack.h
        queue/queue.c
        queue/queue.h
        common/reservations.h)
