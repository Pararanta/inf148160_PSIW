#include "main.h"
#include "signal.h"
#include "./endpoints/server/server.h"
#include "./endpoints/public/public.h"
#include "./service/service.h"
#include "stdlib.h"
#include "utils/utils.h"
#include <unistd.h>
#include <stdio.h>

int server_endpoint_key;
pid_t server_pid;
int public_endpoint_key;
pid_t public_pid;
int key;
int key_response;
int servers_count;
int * servers;
pid_t * servers_pids;

User users[MAX_USERS];
Room rooms[MAX_ROOMS];

void handleExit(int sig)
{
	(void)sig;
    kill(public_pid, SIGKILL);
    kill(server_pid, SIGKILL);
	for(int i = 0; i < MAX_USERS; i++)
		if(users[i].initialized)
		{
			kill(users[i].heartbeat_pid, SIGKILL);
			kill(users[i].listener_pid, SIGKILL);
		}
    exit(0);
	return;
}

int main(int argc, char ** argv)
{
    // Handling program exit
    signal(SIGINT, handleExit);
	signal(SIGTERM, handleExit);

	if(argc < 9)
	{
		printf(ANSI_COLOR_RED"error"ANSI_COLOR_RESET" - expected parameters:\
		"ANSI_COLOR_MAGENTA"internal_server_key_file internal_server_project_char"ANSI_COLOR_RESET"\
		"ANSI_COLOR_YELLOW"internal_server_response_key_file internal_server_response_project_char"ANSI_COLOR_RESET"\
		"ANSI_COLOR_RED"server_endpoint_key_file server_endpoint_project_char"ANSI_COLOR_RESET"\
		"ANSI_COLOR_YELLOW"public_endpoint_key_file public_endpoint_project_char"ANSI_COLOR_RESET"\
		["ANSI_COLOR_BLUE"connected_servers1_endpoint_key_file connected_servers1_endpoint_project_char ..."ANSI_COLOR_RESET"]\n");
		exit(1);
	}
	if(argc < 10)
	{
		printf(ANSI_COLOR_YELLOW"warrning"ANSI_COLOR_RESET": you can specify additional parameters: ["ANSI_COLOR_BLUE"connected_servers1_endpoint_key_file connected_servers1_endpoint_project_char ..."ANSI_COLOR_RESET"]\n");
	}

	// Get key for this server
	key = getKey(argv[1], argv[2][0]);
	key_response = getKey(argv[3], argv[4][0]);
	// Get key for this servers endpoint
	server_endpoint_key = getKey(argv[5], argv[6][0]);

    // Get key for public enpoint
    public_endpoint_key = getKey(argv[7], argv[8][0]);

    if(!(public_pid = fork())) return start_public(public_endpoint_key, key, key_response);
    if(!(server_pid = fork())) return start_server(server_endpoint_key, key, key_response);

	// Get keys for other servers
	servers_count = (argc-9)/2;
	servers = calloc(servers_count, sizeof(int));
	for(int i = 0; i < servers_count; i += 2)
	{
		int skey = getKey(argv[i*2+9], argv[i*2+10][0]);
		servers[i] = skey;
		printf(ANSI_COLOR_BLUE"info"ANSI_COLOR_RESET": aware of server with server key "ANSI_COLOR_BLUE"%d"ANSI_COLOR_RESET"\n", skey);
	}

    // starting service on main process, and ending program if the service ends
    return start_service(key);
}