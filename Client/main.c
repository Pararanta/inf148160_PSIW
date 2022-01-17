#include <stdio.h>
#include <stdlib.h>
#include "types/types.h"
#include "utils/utils.h"
#include "string.h"
#include <sys/prctl.h>
#include <signal.h>
#include <unistd.h>

int server_key;
int our_key;
int our_key2;
pid_t main_ppid;
pid_t hearbeat_pid;
pid_t listener_pid;

MessageBuffer buffer;

/*
*   Nie jestem pewien czy mogę korzystać z prctl, teoretycznie mógłbym to rozwiązać tak jak na serwerze, to jest zapisać pid procesów potomnych i zamykać je przy zamknięciu procesu.
*   Jednak o ile w serwerze zakładam poprawne zamknięcie, to mam wrażenie, że założenie poprawnego zamknięcia przy cliencie byłoby błędem,
*   sygnał heartbeat realizujemy przecież właśnie dlatego, że nie ufamy klientowi, że się poprawnie rozłączy.
*/

int printResponse(MessageBuffer buf)
{
    if(buf.type == CLIENT_RESPONSE_OK) printf(ANSI_COLOR_GREEN"OK\n"ANSI_COLOR_RESET);
    else printf(ANSI_COLOR_RED"NOT OK\n"ANSI_COLOR_RESET);
    return 0;
}

int start_hearbeat()
{
    int r = prctl(PR_SET_PDEATHSIG, SIGTERM);
    if (r == -1) { perror(0); exit(1); }
    if (getppid() != main_ppid)
        exit(1);
    
    while(msgrcv(our_key, &buffer, sizeof(buffer.message), CLIENT_HEARTBEAT_REQUEST, 0))
    {
        buffer.type = CLIENT_HEARTBEAT_RESPONSE;
        msgsnd(our_key, &buffer, sizeof(buffer.message), 0);
    }

    exit(0);
    return 0;
}

int start_listener()
{
    int r = prctl(PR_SET_PDEATHSIG, SIGTERM);
    if (r == -1) { perror(0); exit(1); }
    if (getppid() != main_ppid)
        exit(1);

    while(msgrcv(our_key, &buffer, sizeof(buffer.message), CLIENTAPP_MESSAGE_RECEIVE, 0))
    {
        printf(ANSI_COLOR_BLUE"%s"ANSI_COLOR_RESET": "ANSI_COLOR_YELLOW"%s"ANSI_COLOR_RESET"\n", buffer.message.source, buffer.message.content);
        buffer.type = CLIENTAPP_MESSAGE_RECEIVED;
    }

    exit(0);
    return 0;
}

int main(int argc, char ** argv)
{
    (void)argv;
    if(argc < 8)
    {
        printf(ANSI_COLOR_RED"error"ANSI_COLOR_RESET" - expected parameters: "ANSI_COLOR_MAGENTA"server_public_key_file server_public_project_char"ANSI_COLOR_RESET" "ANSI_COLOR_RED"this_client_key_file this_client_project_char"ANSI_COLOR_RESET" "ANSI_COLOR_GREEN"this_client_key_file2 this_client_project_char2"ANSI_COLOR_RESET" "ANSI_COLOR_YELLOW"\"user name\""ANSI_COLOR_RESET"\n");
        exit(1);
    }

    server_key = getKey(argv[1], argv[2][0]);
    our_key = getKey(argv[3], argv[4][0]);
    our_key2 = getKey(argv[5], argv[6][0]);
    printf(ANSI_COLOR_BLUE"info"ANSI_COLOR_RESET": attempting connection to server with public key "ANSI_COLOR_BLUE"%d"ANSI_COLOR_RESET", using personal keys "ANSI_COLOR_BLUE"%d"ANSI_COLOR_RESET" and "ANSI_COLOR_BLUE"%d"ANSI_COLOR_RESET"\n", server_key, our_key, our_key2);

    printf("Command formats:\n\troom [join/leave] "ANSI_COLOR_YELLOW"\"room name\""ANSI_COLOR_RESET"\n"\
    "\tmessage ["ANSI_COLOR_GREEN"user"ANSI_COLOR_RESET"/"ANSI_COLOR_BLUE"room"ANSI_COLOR_RESET"] "ANSI_COLOR_YELLOW"\"user/room name\" \"message content\""ANSI_COLOR_RESET"\n"\
    "\t"ANSI_COLOR_RED"exit"ANSI_COLOR_RESET"\n"\
    "\t"ANSI_COLOR_BLUE"list"ANSI_COLOR_RESET" ["ANSI_COLOR_GREEN"users"ANSI_COLOR_RESET"/"ANSI_COLOR_BLUE"rooms"ANSI_COLOR_RESET"/"ANSI_COLOR_YELLOW"usersInRoom"ANSI_COLOR_RESET"] "ANSI_COLOR_YELLOW"\"room name if usersInRoom command\""ANSI_COLOR_RESET"\n"\
    "\t"ANSI_COLOR_MAGENTA"past"ANSI_COLOR_RESET" "ANSI_COLOR_YELLOW"\"room name\""ANSI_COLOR_RESET"\n"\
    );

    buffer.type = PUBLIC_REQUEST_NEW_CLIENT;
    buffer.message.payload[0] = our_key;
    buffer.message.payload[1] = our_key2;
    strcpy(buffer.message.source, argv[7]);

    msgsnd(server_key, &buffer, sizeof(buffer.message), 0);
    msgrcv(our_key, &buffer, sizeof(buffer.message), -CLIENT_RESPONSE_MAX, 0);

    if(buffer.type != PUBLIC_RESPONSE_NEW_CLIENT_OK)
    {
        printf(ANSI_COLOR_RED"error"ANSI_COLOR_RESET" - server: Server response not ok for client creation\n");
    }

    main_ppid = getpid();

    if(!(hearbeat_pid = fork())) start_hearbeat();
    if(!(listener_pid = fork())) start_listener();
    printf("Connected\n");

    char command_buffers[4][255];
    char ending_char;
    (void)ending_char;
    while(1)
    {
        scanf("%255s", command_buffers[0]);
        if(!strcmp(command_buffers[0], "exit"))
        {
            buffer.type = CLIENT_LOGOUT;
            msgsnd(our_key2, &buffer, sizeof(buffer.message), 0);
            msgrcv(our_key, &buffer, sizeof(buffer.message), -CLIENT_RESPONSE_MAX, 0);
            kill(hearbeat_pid, SIGKILL);
            kill(listener_pid, SIGKILL);
            exit(0);
        }
        else if(!strcmp(command_buffers[0], "room"))
        {
            scanf("%255s %255s", command_buffers[1], command_buffers[2]);
            if(!strcmp(command_buffers[1], "join"))
            {
                buffer.type = CLIENT_JOIN_ROOM;
                strcpy(buffer.message.target, command_buffers[2]);
                msgsnd(our_key2, &buffer, sizeof(buffer.message), 0);
                msgrcv(our_key, &buffer, sizeof(buffer.message), -CLIENT_RESPONSE_MAX, 0);
                printResponse(buffer);
            }else if(!strcmp(command_buffers[1], "leave"))
            {
                buffer.type = CLIENT_LEAVE_ROOM;
                strcpy(buffer.message.target, command_buffers[2]);
                msgsnd(our_key2, &buffer, sizeof(buffer.message), 0);
                msgrcv(our_key, &buffer, sizeof(buffer.message), -CLIENT_RESPONSE_MAX, 0);
                printResponse(buffer);
            }else
            {
                printf(ANSI_COLOR_RED"error"ANSI_COLOR_RESET" - unrecognized command: "ANSI_COLOR_RED"%s"ANSI_COLOR_RESET"\n", command_buffers[1]);
                continue;
            }
        }else if(!strcmp(command_buffers[0], "message"))
        {
            scanf("%255s %255s %255s", command_buffers[1], command_buffers[2], command_buffers[3]);
            if(!strcmp(command_buffers[1], "user"))
            {
                buffer.type = CLIENT_SEND_TO_USER;
                strcpy(buffer.message.target, command_buffers[2]);
                strcpy(buffer.message.content, command_buffers[3]);
                msgsnd(our_key2, &buffer, sizeof(buffer.message), 0);
                msgrcv(our_key, &buffer, sizeof(buffer.message), -CLIENT_RESPONSE_MAX, 0);
                printResponse(buffer);
            }else if(!strcmp(command_buffers[1], "room"))
            {
                buffer.type = CLIENT_SEND_TO_ROOM;
                strcpy(buffer.message.target, command_buffers[2]);
                strcpy(buffer.message.content, command_buffers[3]);
                msgsnd(our_key2, &buffer, sizeof(buffer.message), 0);
                msgrcv(our_key, &buffer, sizeof(buffer.message), -CLIENT_RESPONSE_MAX, 0);
                printResponse(buffer);
            }else
            {
                printf(ANSI_COLOR_RED"error"ANSI_COLOR_RESET" - unrecognized command: "ANSI_COLOR_RED"%s"ANSI_COLOR_RESET"\n", command_buffers[1]);
                continue;
            }
        }else if(!strcmp(command_buffers[0], "list"))
        {
            scanf("%255s", command_buffers[1]);
            if(!strcmp(command_buffers[1], "users"))
            {
                buffer.type = CLIENT_REQUEST_USER_LIST;
                msgsnd(our_key2, &buffer, sizeof(buffer.message), 0);
                msgrcv(our_key, &buffer, sizeof(buffer.message), -CLIENT_RESPONSE_MAX, 0);
                printResponse(buffer);
            }else if(!strcmp(command_buffers[1], "rooms"))
            {
                buffer.type = CLIENT_REQUEST_ROOM_LIST;
                msgsnd(our_key2, &buffer, sizeof(buffer.message), 0);
                msgrcv(our_key, &buffer, sizeof(buffer.message), -CLIENT_RESPONSE_MAX, 0);
                printResponse(buffer);
            }else if(!strcmp(command_buffers[1], "usersInRoom"))
            {
                scanf("%255s", command_buffers[2]);
                buffer.type = CLIENT_REQUEST_USERS_IN_ROOM_LIST;
                strcpy(buffer.message.target, command_buffers[2]);
                msgsnd(our_key2, &buffer, sizeof(buffer.message), 0);
                msgrcv(our_key, &buffer, sizeof(buffer.message), -CLIENT_RESPONSE_MAX, 0);
                printResponse(buffer);
            }else
            {
                printf(ANSI_COLOR_RED"error"ANSI_COLOR_RESET" - unrecognized command: "ANSI_COLOR_RED"%s"ANSI_COLOR_RESET"\n", command_buffers[1]);
                continue;
            }
        }else if(!strcmp(command_buffers[0], "past"))
        {
            scanf("%255s", command_buffers[1]);
                buffer.type = CLIENT_REQUEST_PAST;
                strcpy(buffer.message.target, command_buffers[1]);
                msgsnd(our_key2, &buffer, sizeof(buffer.message), 0);
                msgrcv(our_key, &buffer, sizeof(buffer.message), -CLIENT_RESPONSE_MAX, 0);
                printResponse(buffer);
        }else
        {
            printf(ANSI_COLOR_RED"error"ANSI_COLOR_RESET" - unrecognized command: "ANSI_COLOR_RED"%s"ANSI_COLOR_RESET"\n", command_buffers[0]);
            continue;
        }
    }

    return 0;
}