#include <stdio.h>
#include "utils.h"

int getKey(char * path, int project)
{
    key_t key = ftok(path, project);
    int msg = msgget(key, IPC_CREAT | 0666);
    if(msg == -1) msg = msgget(key, 0666);
	return msg;
}