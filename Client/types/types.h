/*
* Type declaration.
* Declaration of types used by whole application.
*/

#ifndef MODEL_HEADER
#define MODEL_HEADER

	#define PUBLIC_REQUEST_NEW_CLIENT 1001
	#define PUBLIC_RESPONSE_NEW_CLIENT_OK 11001
	#define PUBLIC_RESPONSE_NEW_CLIENT_ERROR 11001

	#define CLIENT_SEND_TO_USER 1001
	#define CLIENT_SEND_TO_ROOM 1002
	#define CLIENT_JOIN_ROOM 1101
	#define CLIENT_LEAVE_ROOM 1102
	#define CLIENT_REQUEST_USER_LIST 1201
	#define CLIENT_REQUEST_ROOM_LIST 1202
	#define CLIENT_REQUEST_USERS_IN_ROOM_LIST 1301
	#define CLIENT_REQUEST_PAST 1302

	#define CLIENT_LOGOUT 2001

	/*#define CLIENT_RESPONSE_USER_LIST 11201
	#define CLIENT_RESPONSE_ROOM_LIST 11202
	#define CLIENT_RESPONSE_USERS_IN_ROOM_LIST 11301
	#define CLIENT_RESPONSE_PAST 11302*/

	#define CLIENT_RESPONSE_OK 15001
	#define CLIENT_RESPONSE_ERROR 15002

	#define CLIENT_REQUEST_MAX 10000
	#define CLIENT_RESPONSE_MAX 20000

	#define CLIENT_HEARTBEAT_REQUEST 100001
	#define CLIENT_HEARTBEAT_RESPONSE 100002

	#define CLIENTAPP_MESSAGE_RECEIVE 5001
	#define CLIENTAPP_MESSAGE_RECEIVED 5002

	#define ANSI_COLOR_RED     "\x1b[31m"
	#define ANSI_COLOR_GREEN   "\x1b[32m"
	#define ANSI_COLOR_YELLOW  "\x1b[33m"
	#define ANSI_COLOR_BLUE    "\x1b[34m"
	#define ANSI_COLOR_MAGENTA "\x1b[35m"
	#define ANSI_COLOR_CYAN    "\x1b[36m"
	#define ANSI_COLOR_RESET   "\x1b[0m"

	#include "sys/ipc.h"
	#include "sys/types.h"
	#include <sys/msg.h>

	#define MAX_MESSAGE_SIZE 4095
	#define MAX_NAME_SIZE 63
	#define MAX_PAYLOAD 2

	typedef struct {
		char initialized;
		long payload[MAX_PAYLOAD];
		char source[MAX_NAME_SIZE];
		char target[MAX_NAME_SIZE];
		char content[MAX_MESSAGE_SIZE];
		time_t timestamp;
	} Message;

	typedef struct {
		long type;
		Message message;
	} MessageBuffer;

#endif