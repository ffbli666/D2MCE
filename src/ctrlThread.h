#ifndef __CTRLTHREAD_H_
#define __CTRLTHREAD_H_
#include <pthread.h>
#include <semaphore.h>
//#include "network.h"
#include "thread.h"
#include "join.h"
#include "wqueue.h"
#include "barrier.h"
#include "mutexlock.h"
#include "semaphore.h"
#include "finalize.h"
#include "sharememory.h"
#include "manager.h"
#include "exit.h"

struct role_info g_ctrl_info;


int ctrl_init();
int ctrl_destroy();

void *ctrlThread(void *argv);
int ctrlThread_close();

#endif

