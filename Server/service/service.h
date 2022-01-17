/*
* Internal application services.
* Services put out in order to handle this application internal needs.
* We assume that all functions implemented here are called through private server queque by this application.
* It awaits requests from endpoints and handles them by manipulating data and calling Client application's or other Server's endpoints, then responding if nescessary.
*/

#ifndef SERVICE_HEADER
#define SERVICE_HEADER

	#include <sys/types.h>
	#include <sys/ipc.h>
	#include <sys/msg.h>

	#include "../types/types.h"
	#include "../utils/utils.h"

	#define CREATE_LOCAL_USER 1001
	#define CREATE_LOCAL_ROOM 1002
	#define REMOVE_LOCAL_USER 1101

	#define CREATE_REMOTE_USER 2001
	#define CREATE_REMOTE_ROOM 2002
	#define REMOVE_REMOTE_USER 2101

	#define JOIN_USER_TO_ROOM_LOCAL 3001
	#define LEAVE_USER_FROM_ROOM_LOCAL 3002

	#define JOIN_USER_TO_ROOM_REMOTE 4001
	#define LEAVE_USER_FROM_ROOM_REMOTE 4002

	#define FORWARD_TO_CLIENT 5001
	#define FORWARD_TO_ROOM 5002
	#define FORWARD_TO_All_CLIENTS 5003
	#define ADD_MESSAGE_TO_ROOM 5004

	#define SEND_USER_LIST_TO_CLIENT 6001
	#define SEND_ROOM_LIST_TO_CLIENT 6002
	#define SEND_USERS_IN_ROOM_LIST_TO_CLIENT 6101
	#define SEND_PAST_TO_CLIENT 6102

	/*#define RESPONSE_USER_LIST 14001
	#define RESPONSE_ROOM_LIST 14002
	#define RESPONSE_USERS_IN_ROOM_LIST 14101
	#define RESPONSE_PAST 14102*/

	#define RESPONSE_OK 15001
	#define RESPONSE_ERROR 15002

	#define TYPE_REQUEST_MAX 10000
	#define TYPE_RESPONSE_MAX 16000

	#define CLIENTAPP_MESSAGE_RECEIVE 5001
	#define CLIENTAPP_TYPE_RESPONSE_MAX 20000

	int start_service();

	int createUser(Message message, int local, int respond);
	int createRoom(Message message, int local, int respond);
	int removeUser(Message message, int local, int respond);
	int joinUserToRoom(Message message, int local, int respond);
	int leaveUserFromRoom(Message message, int local, int respond);
	int forwardToClient(Message message, int respond, int override_index);
	int forwardToRoom(Message message, int respond);
	int forwardToAll(Message message, int respond);
	int sendUserList(Message message);
	int sendRoomList(Message message);
	int sendUsersInRoomList(Message message);
	int addMessageToRoom(Message message, int respond);
	int sendPastMessages(Message message, int respond);

#endif