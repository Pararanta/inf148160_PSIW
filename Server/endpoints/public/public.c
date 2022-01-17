#include "public.h"
#include "../../service/service.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>

static MessageBuffer buffer;

int start_public(int key, int service_key, int service_key_response)
{
    printf(ANSI_COLOR_BLUE"info"ANSI_COLOR_RESET" - public: listening on key "ANSI_COLOR_BLUE"%d"ANSI_COLOR_RESET"\n", key);
    while(msgrcv(key, &buffer, sizeof(buffer.message), PUBLIC_REQUEST_NEW_CLIENT, 0))
        if(buffer.type == PUBLIC_REQUEST_NEW_CLIENT)
        {
            printf("user %s\n", buffer.message.source);
            buffer.type = CREATE_LOCAL_USER;
            msgsnd(service_key, &buffer, sizeof(buffer.message), 0);
            msgrcv(service_key_response, &buffer, sizeof(buffer.message), -TYPE_RESPONSE_MAX, 0);
            if(buffer.type == RESPONSE_OK)
            {
                buffer.type = PUBLIC_RESPONSE_NEW_CLIENT_OK;
                strcpy(buffer.message.content, "User established");
            }
            else
            {
                buffer.type = PUBLIC_RESPONSE_NEW_CLIENT_ERROR;
                strcpy(buffer.message.content, "Could not establish user");
            }
            msgsnd(buffer.message.payload[0], &buffer, sizeof(buffer.message), 0);
        }

    exit(0);
    return 0;
}