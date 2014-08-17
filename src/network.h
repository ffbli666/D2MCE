#ifndef __NETWORK_H_
#define __NETWORK_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#include "define.h"
#include "socket.h"
#include "table.h"
//#include "queue.h"
#include "stack.h"
#include "wqueue.h"
#include "memory.h"

fd_set g_fdset;
int g_maxfd;
// buf


//receiver
struct table *g_receiver_table;
struct receiver_table{
	int event_seq_number; //need long?
	sem_t sem;
	ssize_t recv_size;
	void *buf;
};

//sender
struct sender_info{
	int count;
//	struct queue *queue;
	struct wqueue *queue;
	sem_t sem;
	pthread_mutex_t lock;
}g_sender_info;

struct sender_table{
	unsigned int id;
	void *send_buf;
	unsigned int send_size;
	unsigned int msg;
	sem_t sem;
};
//request header
//#pragma pack(push, 2)
struct request_header{
	unsigned short msg_type;
	unsigned int size;
	unsigned int seq_number;		//destination
	unsigned int src_node;			//source id
	unsigned int src_seq_number;	//source seq_number
};
//#pragma pack(pop)


#define reply_header request_header

struct new_manager_req{
	struct request_header req;
	unsigned int new_manager;
};

//request node info
struct req_node_info{
	unsigned int id;
	unsigned int seq_number;
};

struct sem_node_info{
	unsigned int id;
	sem_t *sem;
	unsigned int seq_number;
};

//d2mce_init use;
int network_init();
int network_destroy();
int connectSelf();

int sendTo(unsigned int node_id, void *send_buf, size_t send_size);
int sendRecv(unsigned int node_id, void *send_buf, size_t send_size, void *recv_buf, size_t recv_size);

//only use in release
//this will get seq_number, not wait this time
int Send(unsigned int node_id, void *send_buf, size_t send_size, void *recv_buf, size_t recv_size);
//recv req_number data, wait the data receive
void* Recv(unsigned int seq_number);


int forward(unsigned int node_id, void *send_buf);


//only use in join failed
int sendNonConnect(unsigned int ip, unsigned short port, void *send_buf, int send_size);



int fdSet(int set_fd);
int fdClear(int clr_fd);

#endif 

