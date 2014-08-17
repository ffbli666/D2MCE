/* 
# Description:
#   d2mce.h
#
# Copyright (C) 2006- by EPS(Embedded and Parallel Systems Lab) @ NTUT CSIE
#
# Date: $Date: 2007/12/24 02:44:39 $
# Version: $Revision: 1.6 $
#
# History:
#
# $Log: d2mce.h,v $
# Revision 1.6  2007/12/24 02:44:39  ffbli
# dos2unix
#
# Revision 1.5  2007/12/20 13:08:14  ffbli
# add log
#
#
*/
#ifndef __D2MCE_H_
#define __D2MCE_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

#include "network.h"
#include "join.h"
#include "common.h"
#include "define.h"
#include "ctrlThread.h"
#include "barrier.h"
#include "mutexlock.h"
#include "semaphore.h"
#include "sharememory.h"
#include "synchronization.h"
#include "finalize.h"
#include "memory.h"
#include "manager.h"

typedef struct d2mce_ginfo_s{
	char *app_name;
	char *group_name;
	int group_instance;
	unsigned int ip;               //coordinator ip
	unsigned short int port;       //coordinator port
	unsigned int id;
}d2mce_ginfo_t;

enum d2mce_join_option{
	D2MCE_GROUP_ANY = 0,
	D2MCE_GROUP_NEW
};

typedef struct d2mce_mutex_s{
	unsigned int id;
	char *name;
	unsigned int manager_id;
//	struct group *group;
}d2mce_mutex_t;


typedef struct d2mce_barrier_s{
	unsigned int id;
	char *name;
	unsigned int manager_id;
//	struct group *group;
}d2mce_barrier_t;



typedef struct d2mce_sem_s{
	unsigned int id;
	char *name;
	unsigned int manager_id;
//	struct group *group;	
}d2mce_sem_t;

//init & finalize
int d2mce_init();
int d2mce_finalize();
//dynamic join & exit
int d2mce_join(char *app_name, char *group_name, enum d2mce_join_option option);
int d2mce_exit();
int d2mce_probe(char *app_name, char *group_name, d2mce_ginfo_t **group_list, int group_num);
int d2mce_join_gid(d2mce_ginfo_t *ginfo);

//share memory
void* d2mce_malloc(char *sm_name, size_t size);
//i am home node
int d2mce_set_home(void *share_memory);
int d2mce_load(void *share_memory);
int d2mce_store(void *share_memory);
//share memory: multi write protocol
int d2mce_mload(void *share_memory, unsigned int offset, unsigned int length);
int d2mce_mstore(void *share_memory, unsigned int offset, unsigned int length);

//mutex 
int d2mce_mutex_init(d2mce_mutex_t *mutex, char *mutex_name);
int d2mce_mutex_lock(d2mce_mutex_t *mutex);
int d2mce_mutex_unlock(d2mce_mutex_t *mutex);
//semaphore
int d2mce_sem_init(d2mce_sem_t *d2mce_sem, char *sem_name, unsigned int value);
int d2mce_sem_post(d2mce_sem_t *d2mce_sem);
int d2mce_sem_wait(d2mce_sem_t *d2mce_sem);
//barrier
int d2mce_barrier_init(d2mce_barrier_t *barrier, char *barrier_name);
int d2mce_barrier(d2mce_barrier_t *barrier, unsigned int wait_counter);

__inline__ int d2mce_getNodeNum();
__inline__ int d2mce_isCoordinator();


int d2mce_mutex_rw(d2mce_mutex_t* mutex, int num, ...);
int d2mce_sem_wait_rw(d2mce_sem_t* sem, int num, ...);

//get second have decimal ex 1.123456
double d2mce_time();
//get second
double d2mce_stime();
//get micro second
double d2mce_utime();

#ifdef CONF_SM_DISSEMINATE_UPDATE
//share memory
//disseminate udate
int d2mce_update_register(void* share_memory);
int d2mce_update_unregister(void* share_memory);
#endif

int d2mce_set_barrier_manager();
int d2mce_set_sem_manager();
int d2mce_set_mutex_manager();
int d2mce_set_resource_manager();
#ifdef CONF_SM_EVENT_DRIVER
//event
int d2mce_wait_update(void* share_memory);
int d2mce_trywait_update(void* share_memory);

#endif

int d2mce_acq();

#endif // D2MCE_H_


//debug info
int print_info();
int print_overhead();

