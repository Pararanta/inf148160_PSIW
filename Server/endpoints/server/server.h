/*
* Server endpoints.
* Endpoints put out for handling request from other servers.
* We assume that all functions implemented here are called through servers queue by othre Server application.
* It awaits requests from servers queue and handles them by calling service, then responding if nescessary.
*/

#ifndef SERVER_ENDPOINT_HEADER
#define SERVER_ENDPOINT_HEADER

	#include "../../types/types.h"
	#include "sys/types.h"
	#include "sys/ipc.h"

	#define SERVER_CREATE_USER 1001
	#define SERVER_CREATE_ROOM 1002
	#define SERVER_REMOVE_USER 1101

	#define SERVER_SEND_TO_USER 2001
	#define SERVER_SEND_TO_ROOM 2002

    #define SERVER_JOIN_USER_TO_ROOM 2101
    #define SERVER_LEAVE_USER_FROM_ROOM 2102

	#define SERVER_ADD_MESSAGE_TO_ROOM 2201

	#define SERVER_RESPONSE_OK 11001
	#define SERVER_RESPONSE_ERROR 11002

	#define SERVER_TYPE_REQUEST_MAX 3000
	#define SERVER_TYPE_RESPONSE_MAX 12000

	int start_server(int key, int service_key, int service_key_response);

#endif