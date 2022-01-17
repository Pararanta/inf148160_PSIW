/*
* Type declaration.
* Declaration of types used by whole application.
*/

#ifndef MODEL_HEADER
#define MODEL_HEADER

	#include "sys/ipc.h"
	#include "sys/types.h"
	#include <sys/msg.h>

	#define ANSI_COLOR_RED     "\x1b[31m"
	#define ANSI_COLOR_GREEN   "\x1b[32m"
	#define ANSI_COLOR_YELLOW  "\x1b[33m"
	#define ANSI_COLOR_BLUE    "\x1b[34m"
	#define ANSI_COLOR_MAGENTA "\x1b[35m"
	#define ANSI_COLOR_CYAN    "\x1b[36m"
	#define ANSI_COLOR_RESET   "\x1b[0m"

	#define MAX_MESSAGE_SIZE 4095
	#define MAX_NAME_SIZE 63
	#define MAX_USERS 255
	#define MAX_ROOMS 255
	#define MAX_USERS_IN_ROOM 255
	#define MAX_ROOMS_IN_USER 255
	#define MAX_PAST_MESSAGES 10
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

	typedef struct
	{
	char initialized;
	char is_direct;
	int users[MAX_USERS_IN_ROOM];
	char name[MAX_NAME_SIZE];
	Message past[MAX_PAST_MESSAGES];
	int key;
	} Room;

	typedef struct
	{
	char initialized;
	char is_direct;
	int rooms[MAX_ROOMS_IN_USER];
	char name[MAX_NAME_SIZE];
	int key;
	int key2;
	pid_t listener_pid;
	pid_t heartbeat_pid;
	} User;

#endif