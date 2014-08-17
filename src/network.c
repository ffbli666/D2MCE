#include "network.h"

//function
unsigned long getEventSeqNumber();

//variable
pthread_mutex_t lock_event_seq_number;
pthread_mutex_t lock_receiver_table;

unsigned long event_seq_number;

int network_init(){
	struct node **node_table;
	struct receiver_table **receiver_table;
	int i;
	event_seq_number = 1;
	g_overhead.msg_count = 0;
	g_overhead.msg_size = 0;
	//init fd;
    FD_ZERO(&g_fdset);
	
	//init computing table;
	g_receiver_table = tableCreate( RECEIVER_TABLE_SIZE, sizeof(struct receiver_table));
	receiver_table = (struct receiver_table**)getTable(g_receiver_table);
	for(i=0;i<RECEIVER_TABLE_SIZE;i++){
		sem_init(&receiver_table[i]->sem, 0, 0);
	}
	
	g_group.node_table = tableCreate(MAX_NODE_NUM, sizeof(struct node));
	node_table = (struct node**)getTable(g_group.node_table);
//	for(i=0;i<MAX_NODE_NUM;i++){
//		node_table[i]->id = -1;
//	}

	return SUCCESS;
}
int network_destroy(){
	int i;
	struct node **node_table;
	struct receiver_table **receiver_table;
	receiver_table = (struct receiver_table**)getTable(g_receiver_table);
	for(i=0;i<RECEIVER_TABLE_SIZE;i++){
		sem_destroy(&receiver_table[i]->sem);
	}
	tableDestroy(g_receiver_table);
	
	node_table = (struct node**)getTable(g_group.node_table);
	//close socket fd
	for(i=0;i<g_group.node_table->use;i++){
		close(node_table[i]->recv_fd);
		close(node_table[i]->send_fd);
	}	
	tableDestroy(g_group.node_table);
//	printf("network destroy ok!\n");
	return SUCCESS;
}
int connectSelf(){
	struct node *node;
	node = (struct node*)tableGetRow(g_group.node_table, MAX_NODE_NUM-1);
	node->ip = g_system_conf.ip;
	node->recv_fd = -1;
	node->send_fd = TCP_connect(g_system_conf.ip,g_system_conf.port);
	//tableAdd(g_group.node_table, (void*)&node, MAX_NODE_NUM-1);
	return 1;
}

// send data to the node use node_id , coordinator use.
int sendTo(unsigned int node_id, void *send_buf, size_t send_size){
	struct sender_table *send=NULL;
/*
	pthread_mutex_lock(&g_sender_info.lock);
	send = queueGetPush(g_sender_info.queue);	
	pthread_mutex_unlock(&g_sender_info.lock);	
*/
	send = mem_malloc(sizeof(struct sender_table));
	if(send == NULL){
		printf("sendto send error %d", send->msg);
		exit(1);
	}
	((struct request_header*)send_buf)->size = send_size;
	((struct request_header*)send_buf)->src_node = g_group.node_id;
	send->id = node_id;
	send->send_buf = send_buf;
	send->send_size = send_size;
	send->msg = ((struct request_header*)send_buf)->msg_type;
	sem_init(&send->sem,0,0);
	if(send_buf == NULL){
		printf("sendto sendbuf error %d", send->msg);
		exit(1);
	}
	pthread_mutex_lock(&g_sender_info.lock);
	wqueuePush(g_sender_info.queue,send);	
	pthread_mutex_unlock(&g_sender_info.lock);
	sem_post(&g_sender_info.sem);
	sem_wait(&send->sem);
	mem_free(send);
	return 1;
}


int sendRecv(unsigned int node_id, void *send_buf, size_t send_size, void *recv_buf, size_t recv_size){
	int index;
	struct receiver_table *recv;
	struct sender_table *send;

	pthread_mutex_lock(&lock_receiver_table);
	index = tableGetEmpty(g_receiver_table);
	if(index == -1){
		printf("not enoguh");
		g_receiver_table = tableFat(g_receiver_table, g_receiver_table->table_size);
		index = tableGetEmpty(g_receiver_table);
	}
	recv = tableGetRow(g_receiver_table, index);
//	recv->event_seq_number = getEventSeqNumber();
	recv->event_seq_number = 1;
	pthread_mutex_unlock(&lock_receiver_table);
	recv->buf = recv_buf;
	recv->recv_size = recv_size;

	//send
/*
	pthread_mutex_lock(&g_sender_info.lock);
	send = queueGetPush(g_sender_info.queue);
	pthread_mutex_unlock(&g_sender_info.lock);
*/
	send = mem_malloc(sizeof(struct sender_table));
	if(send == NULL){
		printf("sendrecv send error %d", send->msg);
		exit(1);
	}
	((struct request_header*)send_buf)->size = send_size;
	((struct request_header*)send_buf)->src_node = g_group.node_id;
	((struct request_header*)send_buf)->src_seq_number = index;
	send->id = node_id;
	send->send_buf = send_buf;
	send->send_size = send_size;
	send->msg = ((struct request_header*)send_buf)->msg_type;
	sem_init(&send->sem,0,0);
	if(send_buf == NULL){
		printf("sendrecv error %d", send->msg);
		exit(1);
	}
	pthread_mutex_lock(&g_sender_info.lock);
	wqueuePush(g_sender_info.queue, send);
	pthread_mutex_unlock(&g_sender_info.lock);
	sem_post(&g_sender_info.sem);
	sem_wait(&send->sem);
	mem_free(send);
	//waiting the data recv		
	sem_wait(&recv->sem);
	recv->event_seq_number = -1;
	recv->buf = NULL;
	//get the data and return;	
	g_overhead.msg_count++;
	g_overhead.msg_size += send_size;
	return recv->recv_size;
}

int Send(unsigned int node_id, void *send_buf, size_t send_size, void *recv_buf, size_t recv_size){	
	int index;
	struct receiver_table *recv;
	struct sender_table *send;
	pthread_mutex_lock(&lock_receiver_table);
	index = tableGetEmpty(g_receiver_table);	
	if(index == -1){
		printf("not enoguh");
		g_receiver_table = tableFat(g_receiver_table, g_receiver_table->table_size);
		index = tableGetEmpty(g_receiver_table);
	}
	recv = tableGetRow(g_receiver_table, index);
//	recv->event_seq_number = getEventSeqNumber();
	recv->event_seq_number = 1;
	pthread_mutex_unlock(&lock_receiver_table);
	recv->buf = recv_buf;
	recv->recv_size = recv_size;
	
	
/*
	pthread_mutex_lock(&g_sender_info.lock);
	send = queueGetPush(g_sender_info.queue);
	pthread_mutex_unlock(&g_sender_info.lock);
*/
	send = mem_malloc(sizeof(struct sender_table));
	if(send == NULL){
		printf("send send error %d", send->msg);
		exit(1);
	}
	((struct request_header*)send_buf)->size = send_size;
	((struct request_header*)send_buf)->src_node = g_group.node_id;
	((struct request_header*)send_buf)->src_seq_number = index;
	send->id = node_id;
	send->send_buf = send_buf;
	send->send_size = send_size;
	send->msg = ((struct request_header*)send_buf)->msg_type;
	sem_init(&send->sem,0,0);
	pthread_mutex_lock(&g_sender_info.lock);
	wqueuePush(g_sender_info.queue, send);
	pthread_mutex_unlock(&g_sender_info.lock);
	sem_post(&g_sender_info.sem);
	sem_wait(&send->sem);
	mem_free(send);
	return index;
}

void* Recv(unsigned int seq_number){
	void *buf;
	struct receiver_table *recv;
	recv = tableGetRow(g_receiver_table, seq_number);
	//waiting the data recv 	
	sem_wait(&recv->sem);
	buf = recv->buf;
	recv->event_seq_number = -1;
	recv->buf = NULL;
	//get the data and return;
	return buf;
}

int forward(unsigned int node_id, void *send_buf){
	struct sender_table *send=NULL;
	send = mem_malloc(sizeof(struct sender_table));
/*
	pthread_mutex_lock(&g_sender_info.lock);
	send = queueGetPush(g_sender_info.queue);	
	pthread_mutex_unlock(&g_sender_info.lock);	
*/
	if(send == NULL){
		printf("forward send error %d", send->msg);
		exit(1);
	}
	send->id = node_id;
	send->send_buf = send_buf;
	send->send_size = ((struct request_header*)send_buf)->size;
	sem_init(&send->sem,0,0);
	pthread_mutex_lock(&g_sender_info.lock);
	wqueuePush(g_sender_info.queue, send);	
	pthread_mutex_unlock(&g_sender_info.lock);
	sem_post(&g_sender_info.sem);
	sem_wait(&send->sem);
	mem_free(send);
	return 1;
}



int sendNonConnect(unsigned int ip, unsigned short port, void *send_buf, int send_size){
	int socket_fd;
	socket_fd = TCP_connect( ip, port);
	send(socket_fd, send_buf , send_size, 0);
	return 1;
}



unsigned long getEventSeqNumber(){
	unsigned long getnumber;
	pthread_mutex_lock(&lock_event_seq_number);
	getnumber = event_seq_number;
	event_seq_number ++;
	pthread_mutex_unlock(&lock_event_seq_number);
	return getnumber;
}


int fdSet(int set_fd){
	FD_SET(set_fd, &g_fdset);
	if(set_fd > g_maxfd)
		g_maxfd = set_fd;
	return 1;
}

int fdClear(int clr_fd){
	int i;
	struct node **node_table;
	node_table = (struct node**)getTable(g_group.node_table);
	FD_CLR(clr_fd, &g_fdset);
	if(clr_fd == g_maxfd){
		g_maxfd=0;
		for(i=0;i<MAX_NODE_NUM;i++){
			if(node_table[i]->recv_fd > g_maxfd && node_table[i]->recv_fd != clr_fd )
				g_maxfd = node_table[i]->recv_fd;
		}
	}	
	return 1;
}

