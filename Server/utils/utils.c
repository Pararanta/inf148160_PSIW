#include "utils.h"
#include "string.h"
#include "signal.h"
#include <stdio.h>

int getKey(char * path, int project)
{
    key_t key = ftok(path, project);
    int msg = msgget(key, IPC_CREAT | 0666);
    if(msg == -1) msg = msgget(key, 0666);
	return msg;
}

int addPast(Room * rooms, Message message)
{
    int room_index = findRoomIndex(rooms, message.target);
    message.initialized = 1;
    if(room_index < 0)
        return 1; 

    for(int i = MAX_PAST_MESSAGES-1; i > 0; i--)
        rooms[room_index].past[i] = rooms[room_index].past[i-1];

    rooms[room_index].past[0] = message;

    return 0;
}

// Returns users index if found, -1 otherwise
int findUserIndex(User * users, char * username)
{
    for(int i = 0; i < MAX_USERS; i++)
        if(users[i].initialized && !strcmp(users[i].name, username)) return i;
    return -1;
}

int findRoomIndex(Room * rooms, char * roomname)
{
    for(int i = 0; i < MAX_ROOMS; i++)
        if(rooms[i].initialized && !strcmp(rooms[i].name, roomname)) return i;
    return -1;
}

int isUserInRoom(Room * rooms, User * users, char * username, char * roomname)
{
    int user_index = findUserIndex(users, username);
    int room_index = findRoomIndex(rooms, roomname);

    if(user_index < 0 || room_index < 0) return 1;  

    for(int i = 0; i < MAX_USERS_IN_ROOM; i++)
        if(rooms[room_index].initialized && rooms[room_index].users[i] == user_index) return 1;
    return 0;
}

int doUsersShareRoom(User * users, char * username_a, char * username_b){

    int user_a_index = findUserIndex(users, username_a);
    int user_b_index = findUserIndex(users, username_b);

    if(user_a_index < 0 || user_b_index < 0) return 1;

    for(int i = 0; i < MAX_ROOMS_IN_USER; i++)
        for(int j = 0; j < MAX_ROOMS_IN_USER; j++)
            if(users[user_a_index].rooms[i] < 0 || users[user_b_index].rooms[j] < 0) continue;
            else if(users[user_a_index].rooms[i] == users[user_b_index].rooms[j]) return 1;
    return 0;
}

int addUser(User * users, Room * rooms, Message message, int remote)
{
    (void)rooms;
    if(findUserIndex(users, message.source) >= 0)
        return -2;

    int index = -1;

    for(int i = 0; i < MAX_USERS; i++)
        if(!users[i].initialized)
        {
            users[i].initialized = 1;
            users[i].is_direct = !remote;
            users[i].key = message.payload[0];
            users[i].key2 = message.payload[1];
            strcpy(users[i].name, message.source);

            for(int j = 0; j < MAX_ROOMS_IN_USER; j++)
                users[i].rooms[j] = -1;

            index = i;
            break;
        }

    return index;
}

int addRoom(User * users, Room * rooms, Message message, int remote)
{
    (void)users;
    if(findRoomIndex(rooms, message.target) >= 0)
        return -2;

    int index = -1;

    for(int i = 0; i < MAX_ROOMS; i++)
        if(!rooms[i].initialized)
        {
            rooms[i].initialized = 1;
            rooms[i].is_direct = !remote;
            rooms[i].key = message.payload[0];

            strcpy(rooms[i].name, message.target);
            for(int j = 0; j < MAX_PAST_MESSAGES; j++)
                rooms[i].past[j].initialized = 0;
            for(int j = 0; j < MAX_USERS_IN_ROOM; j++)
                rooms[i].users[j] = -1;

            index = i;
            break;
        }

    return index;
}

int clearRoom(User * users, Room * rooms, char * roomname, int handle_users)
{
    int room_index;
    if((room_index = findRoomIndex(rooms, roomname)) < 0)
        return -1;

    if(handle_users)
        for(int i = 0; i < MAX_USERS_IN_ROOM; i++)
            clearUserFromRoom(users, rooms, users[rooms[room_index].users[i]].name, roomname, 0);
    rooms[room_index].initialized = 0;

    return 0;
}

int clearUser(User * users, Room * rooms, char * username)
{
    int user_index;
    if((user_index = findUserIndex(users, username)) < 0)
        return -1;

    for(int i = 0; i < MAX_ROOMS_IN_USER; i++)
        clearUserFromRoom(users, rooms, rooms[users[user_index].rooms[i]].name, username, 1);
    users[user_index].initialized = 0;

    // Clear off listener and heartbeak of process
    if(users[user_index].is_direct)
    {
        kill(users[user_index].heartbeat_pid, SIGKILL);
        kill(users[user_index].listener_pid, SIGKILL);
    }

    return 0;
}

int addUserToRoom(User * users, Room * rooms, char * username, char * roomname)
{

    int user_index = findUserIndex(users, username);
    int room_index = findRoomIndex(rooms, roomname);

    if(user_index < 0 || room_index < 0)
        return -1;

    int user_in_room_index = 0;
    int room_in_user_index = 0;

    for(; user_in_room_index < MAX_USERS_IN_ROOM; user_in_room_index++)
        if(rooms[room_index].users[user_in_room_index] < 0) break;

    for(; room_in_user_index < MAX_ROOMS_IN_USER; room_in_user_index++)
        if(users[user_index].rooms[room_in_user_index] < 0) break;

    if(user_in_room_index == MAX_USERS_IN_ROOM || room_in_user_index == MAX_ROOMS_IN_USER)
        return -2;

    rooms[room_index].users[user_in_room_index] = user_index;
    users[user_index].rooms[room_in_user_index] = room_index;

    return 0;
}

int clearUserFromRoom(User * users, Room * rooms, char * username, char * roomname, int check_room)
{
    int user_index = findUserIndex(users, username);
    int room_index = findRoomIndex(rooms, roomname);

    if(user_index < 0 || room_index < 0)
        return 1;

    for(int i = 0; i < MAX_USERS_IN_ROOM; i++)
        if(rooms[room_index].users[i] == user_index) rooms[room_index].users[i] = -1;

    for(int i = 0; i < MAX_ROOMS_IN_USER; i++)
        if(users[user_index].rooms[i] == room_index) users[user_index].rooms[i] = -1;

    if(check_room)
    {
        int i = 0;
        for(; i < MAX_USERS_IN_ROOM; i++)
            if(rooms[room_index].users[i] > 0) break;
        if(i == MAX_USERS_IN_ROOM) clearRoom(users, rooms, roomname, 0);
    }
    return 0;
}