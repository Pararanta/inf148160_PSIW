#ifndef UTILS_HEADER
#define UTILS_HEADER

	#include "../types/types.h"

int getKey(char * path, int project);
int addPast(Room * rooms, Message message);
int findUserIndex(User * users, char * username);
int findRoomIndex(Room * rooms, char * roomname);
int isUserInRoom(Room * rooms, User * users, char * username, char * roomname);
int doUsersShareRoom(User * users, char * username_a, char * username_b);
int addUser(User * users, Room * rooms, Message message, int remote);
int addRoom(User * users, Room * rooms, Message message, int remote);
int clearRoom(User * users, Room * rooms, char * roomname, int handle_users);
int clearUser(User * users, Room * rooms, char * username);
int addUserToRoom(User * users, Room * rooms, char * username, char * roomname);
int clearUserFromRoom(User * users, Room * rooms, char * username, char * roomname, int check_room);
#endif