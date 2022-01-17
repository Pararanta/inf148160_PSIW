cmake_minimum_required(VERSION 3.16)
project ("Server" LANGUAGES C VERSION 0.1.0)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wpedantic -Wextra -Werror -g")
add_executable (Server "main.c" "main.h" "types/types.h" "utils/utils.c" "utils/utils.h" "service/service.c" "service/service.h" "endpoints/server/server.c" "endpoints/server/server.h" "endpoints/client/client.c" "endpoints/client/client.h" "endpoints/public/public.c" "endpoints/public/public.h")