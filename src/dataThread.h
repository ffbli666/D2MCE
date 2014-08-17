#ifndef __DATATHREAD_H_
#define __DATATHREAD_H_
#include <pthread.h>
#include <semaphore.h>
//#include "network.h"
#include "thread.h"
#include "join.h"
#include "wqueue.h"
#include "sharememory.h"

struct role_info g_data_info;


int data_init();
int data_destroy();

void *dataThread(void *argv);
int dataThread_close();

#endif

