#ifndef MAIN_HEADER
#define MAIN_HEADER

    #include "./types/types.h"

    extern int server_endpoint_key;
    extern int servers_count;
    extern int * servers;
    extern int key;
    extern int key_response;

    extern User users[MAX_USERS];
    extern Room rooms[MAX_ROOMS];

#endif