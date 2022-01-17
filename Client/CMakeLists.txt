cmake_minimum_required(VERSION 3.16)
project ("Client" LANGUAGES C VERSION 0.1.0)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wpedantic -Wextra -Werror -g")
add_executable (Client "main.c" "types/types.h" "utils/utils.c" "utils/utils.h")