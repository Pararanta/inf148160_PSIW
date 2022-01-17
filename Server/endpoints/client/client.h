/*
* Client endpoints.
* Endpoints put out for handling request from established clients.
* We assume that all functions implemented here are called through clients queue by Client application.
* It awaits requests from client's queue and handles them by calling service, then responding if nescessary.
*/

#ifndef CLIENT_ENDPOINT_HEADER
#define CLIENT_ENDPOINT_HEADER

	#include "../../types/types.h"
	#include "sys/types.h"
	#include "sys/ipc.h"

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

	int client_start(User * user, int key, int key_response);

	int handleSendToUser(Message Message, User user);
	int handleSendToRoom(Message Message, User user);
	int handleJoinRoom(Message Message, User user);
	int handleLeaveRoom(Message Message, User user);
	int handleRequestUserList(Message Message, User user);
	int handleRequestRoomList(Message Message, User user);
	int handleRequestUsersInRoomList(Message Message, User user);
	int handleRequestPast(Message Message, User user);
	int handleLogout(Message Message, User user);

#endif