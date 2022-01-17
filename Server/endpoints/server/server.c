#include "server.h"
#include "../../service/service.h"
#include <stdio.h>
#include <stdlib.h>
static MessageBuffer buffer;

int callService(int service_key, int service_key_response, int call_type, Message message)
{
    // In theory its as valid as a = a, but I will still leave it here in case I would need to implement some overriting functionality in the future
    buffer.message = message;
    buffer.type = call_type;
    msgsnd(service_key, &buffer, sizeof(buffer.message), 0);
    msgrcv(service_key_response, &buffer, sizeof(buffer.message), -TYPE_RESPONSE_MAX, 0);

    return buffer.type == RESPONSE_ERROR;
}

int start_server(int key, int service_key, int service_key_response)
{
    printf(ANSI_COLOR_BLUE"info"ANSI_COLOR_RESET" - server: listening on key "ANSI_COLOR_BLUE"%d"ANSI_COLOR_RESET"\n", key);
    // Receive messages from other servers and forward them to service with appropriate call type
    while(msgrcv(key, &buffer, sizeof(buffer.message), -SERVER_TYPE_REQUEST_MAX, 0))
    {
        if(buffer.type == SERVER_CREATE_USER)
            callService(service_key, service_key_response, CREATE_REMOTE_USER, buffer.message);
		if(buffer.type == SERVER_CREATE_ROOM)
            callService(service_key, service_key_response, CREATE_REMOTE_ROOM, buffer.message);
		if(buffer.type == SERVER_REMOVE_USER)
        	callService(service_key, service_key_response, REMOVE_REMOTE_USER, buffer.message);

        if(buffer.type == SERVER_SEND_TO_USER)
        	callService(service_key, service_key_response, FORWARD_TO_CLIENT, buffer.message);
        if(buffer.type == SERVER_SEND_TO_ROOM)
        	callService(service_key, service_key_response, FORWARD_TO_ROOM, buffer.message);

        if(buffer.type == SERVER_JOIN_USER_TO_ROOM)
        	callService(service_key, service_key_response, JOIN_USER_TO_ROOM_REMOTE, buffer.message);
        if(buffer.type == SERVER_LEAVE_USER_FROM_ROOM)
        	callService(service_key, service_key_response, LEAVE_USER_FROM_ROOM_REMOTE, buffer.message);
        
        if(buffer.type == SERVER_ADD_MESSAGE_TO_ROOM)
        	callService(service_key, service_key_response, ADD_MESSAGE_TO_ROOM, buffer.message);
    }

    exit(0);
    return 0;
}