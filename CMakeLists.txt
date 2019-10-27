cmake_minimum_required(VERSION 3.14)
project(lab2 C)

set(CMAKE_C_STANDARD 99)
SET(GCC_COVERAGE_COMPILE_FLAGS "-Wall -Wextra —std=gnu99 -pedantic -Werror -Wno-unused-variable")
SET(CMAKE_C_FLAGS  "-Wall -Wextra -pedantic -Werror -D_FILE_OFFSET_BITS=64 -fsanitize=address -g -fPIE -O1 -fno-omit-frame-pointer")
add_executable(mytee mytee.c)