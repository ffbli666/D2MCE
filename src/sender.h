#ifndef __SENDER_H_
#define __SENDER_H_

//#include <unistd.h>
#include <sys/socket.h>
#include <semaphore.h>
#include <pthread.h>

#include "define.h"
#include "socket.h"
#include "wqueue.h"
#include "thread.h"
#include "network.h"
#include "memory.h"


int sender_init();
int sender_destroy();
void *sender(void *argv);
int sender_close();



#endif 

