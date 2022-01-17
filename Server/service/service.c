#include "service.h"
#include "../utils/utils.h"
#include "../endpoints/server/server.h"
#include "../main.h"
#include "string.h"
#include "../endpoints/client/client.h"
#include <stdio.h>

static MessageBuffer buffer;

int start_service(int key)
{
	printf(ANSI_COLOR_BLUE"info"ANSI_COLOR_RESET" - service: listening on key "ANSI_COLOR_BLUE"%d"ANSI_COLOR_RESET" and responding on "ANSI_COLOR_BLUE"%d"ANSI_COLOR_RESET"\n", key, key_response);
	// Listen to request to service
    while(msgrcv(key, &buffer, sizeof(buffer.message), -TYPE_REQUEST_MAX, 0))
    {
		printf("Request received %ld\n", buffer.type);
        if(buffer.type == CREATE_LOCAL_USER)
            createUser(buffer.message, 1, 1);
		else if(buffer.type == CREATE_LOCAL_ROOM)
            createRoom(buffer.message, 1, 1);
		else if(buffer.type == REMOVE_LOCAL_USER)
        	removeUser(buffer.message, 1, 1);

		else if(buffer.type == CREATE_REMOTE_USER)
            createUser(buffer.message, 0, 1);
		else if(buffer.type == CREATE_REMOTE_ROOM)
            createRoom(buffer.message, 0, 1);
		else if(buffer.type == REMOVE_REMOTE_USER)
        	removeUser(buffer.message, 0, 1);

		else if(buffer.type == JOIN_USER_TO_ROOM_LOCAL)
            joinUserToRoom(buffer.message, 1, 1);
		else if(buffer.type == LEAVE_USER_FROM_ROOM_LOCAL)
            leaveUserFromRoom(buffer.message, 1, 1);

		else if(buffer.type == JOIN_USER_TO_ROOM_REMOTE)
            joinUserToRoom(buffer.message, 0, 1);
		else if(buffer.type == LEAVE_USER_FROM_ROOM_REMOTE)
            leaveUserFromRoom(buffer.message, 0, 1);
		
		else if(buffer.type == FORWARD_TO_CLIENT)
            forwardToClient(buffer.message, 1, -1);
		else if(buffer.type == FORWARD_TO_ROOM)
        	forwardToRoom(buffer.message, 1);
		else if(buffer.type == FORWARD_TO_All_CLIENTS)
        	forwardToAll(buffer.message, 1);

		else if(buffer.type == SEND_USER_LIST_TO_CLIENT)
            sendUserList(buffer.message);
		else if(buffer.type == SEND_ROOM_LIST_TO_CLIENT)
        	sendRoomList(buffer.message);
		else if(buffer.type == SEND_USERS_IN_ROOM_LIST_TO_CLIENT)
        	sendUsersInRoomList(buffer.message);
		
		else if(buffer.type == ADD_MESSAGE_TO_ROOM)
        	addMessageToRoom(buffer.message, 1);
		else if(buffer.type == SEND_PAST_TO_CLIENT)
        	sendPastMessages(buffer.message, 1);

		printf("Request processed %ld\n", buffer.type);
    }

	return 0;
}

int addMessageToRoom(Message message, int respond)
{
	buffer.type = RESPONSE_OK;
	if(respond) msgsnd(key_response, &buffer, sizeof(buffer.message), 0);
	addPast(rooms, message);
	return 0;
}

int sendPastMessages(Message message, int respond)
{
	printf("c\n");
	buffer.type = RESPONSE_OK;
	if(respond) msgsnd(key_response, &buffer, sizeof(buffer.message), 0);

	int user_index = findUserIndex(users, message.source);
    int room_index = findRoomIndex(rooms, message.target);

    if(user_index < 0 || room_index < 0) return 1;

	for(int i = 0; i < MAX_PAST_MESSAGES; i++)
	{
		if(!rooms[room_index].past[i].initialized) continue;
		forwardToClient(rooms[room_index].past[i], 0, user_index);
	}

	return 0;
}

int createUser(Message message, int local, int respond)
{
	// Attempt creating user, and handle failure
	int result = 0;
    if((result = addUser(users, rooms, message, !local)) < 0)
	{
		// Sending response to endpoint that request was failed
		buffer.type = RESPONSE_ERROR;
		strcpy(buffer.message.content, "Cannot create user.");
		if(respond) msgsnd(key_response, &buffer, sizeof(buffer.message), 0);
		return result;
	}

	// Create child processes for handling users communication
	if(local) client_start(&users[result], key, key_response);

	// Send response status for public endpoint through service queue
	buffer.type = RESPONSE_OK;
	if(respond) msgsnd(key_response, &buffer, sizeof(buffer.message), 0);

	// Inform other servers about users creation
	buffer.type = SERVER_CREATE_USER;
	buffer.message.payload[0] = server_endpoint_key;
	if(local)
		for(int i = 0; i < servers_count; i++)
		{
				msgsnd(servers[i], &buffer, sizeof(buffer.message), 0);
				//msgrcv(servers[i], &buffer, sizeof(buffer.message), -SERVER_TYPE_RESPONSE_MAX, 0);
		}
	return 0;
}

int createRoom(Message message, int local, int respond)
{
	// Attempt creating room, and handle failure
	int result = 0;
    if((result = addRoom(users, rooms, message, !local)) < 0)
	{
		// Sending response to endpoint that request was failed
		buffer.type = RESPONSE_ERROR;
		strcpy(buffer.message.content, "Cannot create room.");
		if(respond) msgsnd(key_response, &buffer, sizeof(buffer.message), 0);
		return result;
	}

	// Send response status for endpoint through service queue
	buffer.type = RESPONSE_OK;
	if(respond) msgsnd(key_response, &buffer, sizeof(buffer.message), 0);

	// Inform other servers about rooms creation
	buffer.type = SERVER_CREATE_ROOM;
	buffer.message.payload[0] = server_endpoint_key;
	if(local)
		for(int i = 0; i < servers_count; i++)
		{
			msgsnd(servers[i], &buffer, sizeof(buffer.message), 0);
			//msgrcv(servers[i], &buffer, sizeof(buffer.message), -SERVER_TYPE_RESPONSE_MAX, 0);
		}
	return 0;
}

int removeUser(Message message, int local, int respond)
{
	printf("Removing user %s\n", message.source);
	// Attempt removing user, and handle failure
	int result = 0;
    if((result = clearUser(users, rooms, message.source)) < 0)
	{
		// Sending response to endpoint that request failed
		buffer.type = RESPONSE_ERROR;
		strcpy(buffer.message.content, "Cannot remove user.");
		if(respond) msgsnd(key_response, &buffer, sizeof(buffer.message), 0);
		return result;
	}

	// Send response status for endpoint through service queue
	buffer.type = RESPONSE_OK;
	if(respond) msgsnd(key_response, &buffer, sizeof(buffer.message), 0);

	// Inform other servers about users removal
	buffer.type = SERVER_REMOVE_USER;
	if(local)
		for(int i = 0; i < servers_count; i++)
		{
			msgsnd(servers[i], &buffer, sizeof(buffer.message), 0);
			//msgrcv(servers[i], &buffer, sizeof(buffer.message), -SERVER_TYPE_RESPONSE_MAX, 0);
		}
	sprintf(message.content, "User %s left network of servers", message.source);
	strcpy(message.target, message.source);
	strcpy(message.source, "Server");

	forwardToAll(message, 0);
	return 0;
}

int joinUserToRoom(Message message, int local, int respond)
{
	// Attempt joining user, and handle failure
	int result = 0;
	int room_index = findRoomIndex(rooms, message.target);
	if(room_index < 0)
	{
		createRoom(message, local, 0);
		buffer.type = CREATE_REMOTE_ROOM;
	}
    if((result = addUserToRoom(users, rooms, message.source, message.target)) < 0)
	{
		// Sending response to endpoint that request failed
		buffer.type = RESPONSE_ERROR;
		strcpy(buffer.message.content, "Cannot join room.\0");
		if(respond) msgsnd(key_response, &buffer, sizeof(buffer.message), 0);
		return result;
	}

	// Send response status for endpoint through service queue
	buffer.type = RESPONSE_OK;
	if(respond) msgsnd(key_response, &buffer, sizeof(buffer.message), 0);

	// Inform other servers about user join
	buffer.type = SERVER_JOIN_USER_TO_ROOM;
	if(local)
		for(int i = 0; i < servers_count; i++)
		{
			msgsnd(servers[i], &buffer, sizeof(buffer.message), 0);
			//msgrcv(servers[i], &buffer, sizeof(buffer.message), -SERVER_TYPE_RESPONSE_MAX, 0);
		}

	return 0;
}

int leaveUserFromRoom(Message message, int local, int respond)
{
	// Attempt leaving user, and handle failure
	int result = 0;
    if((result = clearUserFromRoom(users, rooms, message.source, message.target, 1)) < 0)
	{
		// Sending response to endpoint that request failed
		buffer.type = RESPONSE_ERROR;
		strcpy(buffer.message.content, "Cannot leave room.");
		if(respond) msgsnd(key_response, &buffer, sizeof(buffer.message), 0);
		return result;
	}

	// Send response status for endpoint through service queue
	buffer.type = RESPONSE_OK;
	if(respond) msgsnd(key_response, &buffer, sizeof(buffer.message), 0);

	// Inform other servers about users leaving
	buffer.type = SERVER_LEAVE_USER_FROM_ROOM;
	if(local)
		for(int i = 0; i < servers_count; i++)
		{
			msgsnd(servers[i], &buffer, sizeof(buffer.message), 0);
			//msgrcv(servers[i], &buffer, sizeof(buffer.message), -SERVER_TYPE_RESPONSE_MAX, 0);
		}

	return 0;
}

int forwardToClient(Message message, int respond, int override_index)
{
	int user_index;
	// Getting user from message if not specified in override_index
	if(override_index >= 0)
		user_index = override_index;
	else
		user_index = findUserIndex(users, message.target);

	if(user_index < 0)
	{
		// Sending response to endpoint that request failed
		buffer.type = RESPONSE_ERROR;
		strcpy(buffer.message.content, "Cannot send message.");
		if(respond) msgsnd(key_response, &buffer, sizeof(buffer.message), 0);
		return -1;
	}

	// Sending response to endpoint that request was processed
	buffer.type = RESPONSE_OK;
	if(respond) msgsnd(key_response, &buffer, sizeof(buffer.message), 0);

	// Sending message to user if user is connected to this server, or to users server otherwise
	if(users[user_index].is_direct)
	{
		buffer.type = CLIENTAPP_MESSAGE_RECEIVE;
		buffer.message = message;
		msgsnd(users[user_index].key, &buffer, sizeof(buffer.message), 0);
		//msgrcv(users[user_index].key2, &buffer, sizeof(buffer.message), -CLIENTAPP_TYPE_RESPONSE_MAX, 0);
	}
	else
	{
		buffer.type = SERVER_SEND_TO_USER;
		buffer.message = message;
		msgsnd(users[user_index].key, &buffer, sizeof(buffer.message), 0);
		//msgrcv(users[user_index].key2, &buffer, sizeof(buffer.message), -SERVER_TYPE_RESPONSE_MAX, 0);
	}
	return 0;
}

int forwardToRoom(Message message, int respond)
{
	int room_index = findRoomIndex(rooms, message.target);
	if(room_index < 0)
	{
		// Sending response to endpoint that request failed
		buffer.type = RESPONSE_ERROR;
		strcpy(buffer.message.content, "Cannot send message.");
		if(respond) msgsnd(key_response, &buffer, sizeof(buffer.message), 0);
		return -1;
	}

	// Sending response to endpoint that request was processed
	buffer.type = RESPONSE_OK;
	if(respond) msgsnd(key_response, &buffer, sizeof(buffer.message), 0);

	if(rooms[room_index].is_direct)
	{
		buffer.type = SERVER_ADD_MESSAGE_TO_ROOM;
		printf("ta\n");
		for(int i = 0; i < servers_count; i++)
			msgsnd(servers[i], &buffer, sizeof(buffer.message), 0);
		printf("tb\n");
		addPast(rooms, message);
		printf("tc\n");
		char buf[MAX_NAME_SIZE];
		char buf2[MAX_NAME_SIZE];
		strcpy(buf, message.source);
		strcpy(buf2, message.target);
		sprintf(message.source, "%s@%s", buf, buf2);
		// Sending messages to users if room was originally created on this server
		for(int i = 0; i < MAX_USERS_IN_ROOM; i++)
			if(rooms[room_index].users[i] >= 0)
			{
				strcpy(message.target, users[rooms[room_index].users[i]].name);
				forwardToClient(message, 0, -1);
			}
	}
	else
	{
		// Forwarding message to rooms original server otherwise
		buffer.type = SERVER_SEND_TO_ROOM;
		msgsnd(rooms[room_index].key, &buffer, sizeof(buffer.message), 0);
		//msgrcv(rooms[room_index].key, &buffer, sizeof(buffer.message), -SERVER_TYPE_RESPONSE_MAX, 0);
	}

	return 0;
}

int forwardToAll(Message message, int respond)
{
	// Sending response to endpoint that request was processed
	buffer.type = RESPONSE_OK;
	buffer.message = message;
	if(respond) msgsnd(key_response, &buffer, sizeof(buffer.message), 0);

	for(int user_index = 0; user_index < MAX_USERS; user_index++)
	{
		if(!users[user_index].initialized) continue;
		// Sending message to user if user is connected to this server, or to users server otherwise
		if(users[user_index].is_direct)
		{
			buffer.type = CLIENTAPP_MESSAGE_RECEIVE;
			msgsnd(users[user_index].key, &buffer, sizeof(buffer.message), 0);
			//msgrcv(users[user_index].key, &buffer, sizeof(buffer.message), -CLIENTAPP_TYPE_RESPONSE_MAX, 0);
		}
		else
		{
			buffer.type = SERVER_SEND_TO_USER;
			msgsnd(users[user_index].key, &buffer, sizeof(buffer.message), 0);
			//msgrcv(users[user_index].key, &buffer, sizeof(buffer.message), -SERVER_TYPE_RESPONSE_MAX, 0);
		}
	}
	return 0;
}

int sendUserList(Message message)
{
	strcpy(message.content, "\n");
	for(int i = 0; i < MAX_USERS; i++)
	{
		if(!users[i].initialized)
			continue;
		strcat(message.content, users[i].name);
		strcat(message.content, "\n");
	}

	strcpy(message.target, message.source);
	strcpy(message.source, "Server - list of users");
	forwardToClient(message, 1, -1);
	return 0;
}

int sendRoomList(Message message)
{
	strcpy(message.content, "\n");
	for(int i = 0; i < MAX_ROOMS; i++)
	{
		if(!rooms[i].initialized)
			continue;
		strcat(message.content, rooms[i].name);
		strcat(message.content, "\n");
	}

	strcpy(message.target, message.source);
	strcpy(message.source, "Server - list of rooms");
	forwardToClient(message, 1, -1);
	return 0;
}

int sendUsersInRoomList(Message message)
{
	int room_index = findRoomIndex(rooms, message.target);
	if(room_index < 0)
	{
		buffer.type = RESPONSE_ERROR;
		strcpy(buffer.message.content, "Cannot find room.");
		msgsnd(key_response, &buffer, sizeof(buffer.message), 0);
		return -1;
	}

	strcpy(message.content, "\n");
	for(int i = 0; i < MAX_USERS_IN_ROOM; i++)
	{
		if(rooms[room_index].users[i] < 0)
			continue;
		if(!users[rooms[room_index].users[i]].initialized)
			continue;
		strcat(message.content, users[rooms[room_index].users[i]].name);
		strcat(message.content, "\n");
	}

	strcpy(message.source, "Server - list of users in room ");
	strcat(message.source, message.target);
	strcpy(message.target, message.source);
	forwardToClient(message, 1, -1);
	return 0;
}