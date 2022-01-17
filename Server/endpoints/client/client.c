#include "client.h"
#include "../../service/service.h"
#include "string.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

static int service_key;
static int service_key_response;
static MessageBuffer buffer_heartbeat;
static MessageBuffer buffer_message;

int callServiceClient(int service_key, int service_key_response, int call_type, Message message, User * user)
{
    // In theory its as valid as a = a, but I will still leave it here in case I would need to implement some overriting functionality in the future
    buffer_message.message = message;
    strcpy(buffer_message.message.source, user->name);
    buffer_message.type = call_type;
    msgsnd(service_key, &buffer_message, sizeof(buffer_message.message), 0);
    printf("a\n");
    msgrcv(service_key_response, &buffer_message, sizeof(buffer_message.message), -TYPE_RESPONSE_MAX, 0);
    printf("b\n");
    if(buffer_message.type != RESPONSE_OK) buffer_message.type = CLIENT_RESPONSE_ERROR;
    if(buffer_message.type == RESPONSE_OK) buffer_message.type = CLIENT_RESPONSE_OK;
    msgsnd(user->key, &buffer_message, sizeof(buffer_message.message), 0);
    return buffer_message.type == RESPONSE_ERROR;
}

int start_listener(User * user)
{
    (void)user;
    while(msgrcv(user->key2, &buffer_message, sizeof(buffer_message.message), -CLIENT_REQUEST_MAX, 0))
    {
        if(buffer_message.type == CLIENT_SEND_TO_USER)
        {
            buffer_message.message.payload[0] = 0;
            callServiceClient(service_key, service_key_response, FORWARD_TO_CLIENT, buffer_message.message, user);
        }
		if(buffer_message.type == CLIENT_SEND_TO_ROOM)
        {
            buffer_message.message.payload[0] = 1;
            callServiceClient(service_key, service_key_response, FORWARD_TO_ROOM, buffer_message.message, user);
        }
		if(buffer_message.type == CLIENT_JOIN_ROOM)
            callServiceClient(service_key, service_key_response, JOIN_USER_TO_ROOM_LOCAL, buffer_message.message, user);
		if(buffer_message.type == CLIENT_LEAVE_ROOM)
            callServiceClient(service_key, service_key_response, LEAVE_USER_FROM_ROOM_LOCAL, buffer_message.message, user);
        if(buffer_message.type == CLIENT_LOGOUT)
            callServiceClient(service_key, service_key_response, REMOVE_LOCAL_USER, buffer_message.message, user);

        if(buffer_message.type == CLIENT_REQUEST_USER_LIST)
            callServiceClient(service_key, service_key_response, SEND_USER_LIST_TO_CLIENT, buffer_message.message, user);
        if(buffer_message.type == CLIENT_REQUEST_ROOM_LIST)
            callServiceClient(service_key, service_key_response, SEND_ROOM_LIST_TO_CLIENT, buffer_message.message, user);
        if(buffer_message.type == CLIENT_REQUEST_USERS_IN_ROOM_LIST)
            callServiceClient(service_key, service_key_response, SEND_USERS_IN_ROOM_LIST_TO_CLIENT, buffer_message.message, user);
        if(buffer_message.type == CLIENT_REQUEST_PAST)
            callServiceClient(service_key, service_key_response, SEND_PAST_TO_CLIENT, buffer_message.message, user);
    }
    exit(0);
    return 0;
}

int start_hearbeat(User * user)
{
    sleep(4);
    while(1)
    {
        buffer_heartbeat.type = CLIENT_HEARTBEAT_REQUEST;
        msgsnd(user->key, &buffer_heartbeat, sizeof(buffer_heartbeat.message), 0);
        sleep(2);
        if(msgrcv(user->key, &buffer_heartbeat, sizeof(buffer_heartbeat.message), CLIENT_HEARTBEAT_RESPONSE, 0 | IPC_NOWAIT) < 0){
            buffer_heartbeat.type = REMOVE_LOCAL_USER;
            strcpy(buffer_heartbeat.message.source, user->name);
            msgsnd(service_key, &buffer_heartbeat, sizeof(buffer_heartbeat.message), 0);
        }
    }
    exit(0);
    return 0;
}

int client_start(User * user, int key, int key_response)
{
    // Assigning service_key at the start so it is visable for child processes
    service_key = key;
    service_key_response = key_response;
    printf(ANSI_COLOR_BLUE"info"ANSI_COLOR_RESET" - client: client "ANSI_COLOR_YELLOW"%s"ANSI_COLOR_RESET" listening on keys "ANSI_COLOR_BLUE"%d"ANSI_COLOR_RESET" and "ANSI_COLOR_BLUE"%d"ANSI_COLOR_RESET"\n", user->name, user->key, user->key2);
    // Create process for handling requests from user
    if(!(user->listener_pid = fork()))
        return start_listener(user);

    // Create process managing heartbeat
    if(!(user->heartbeat_pid = fork()))
        return start_hearbeat(user);

    // pids assigned to user structure will be visable for service since this function is called directly by it
    return 0;
}