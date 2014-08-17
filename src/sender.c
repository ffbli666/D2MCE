#include "sender.h"


int sender_init(){
//	int i;
//	struct sender_table *sender;
	if(g_sender_info.count == 0){
		g_sender_info.count = 1;
		sem_init(&g_sender_info.sem, 0, 0);
//		g_sender_info.queue = queueCreate(SENDER_TABLE_SIZE, sizeof(struct sender_table));
		g_sender_info.queue = wqueueCreate(SENDER_TABLE_SIZE);
/*
		for(i=0; i<SENDER_TABLE_SIZE; i++){
			sender = g_sender_info.queue->row[i];
			sem_init(&sender->sem, 0, 0);
		}
*/
		return 1;
	}
	g_sender_info.count++;
	return -1;
}
int sender_destroy(){
	if(g_sender_info.count == 1){
		g_sender_info.count = 0;
		wqueueDestroy(g_sender_info.queue);
		return 1;
	}
	g_sender_info.count--;
	return -1;
}

void *sender(void *argv){
	ssize_t sent_size;
	struct sender_table *get;
	struct node **node_table;
	sender_init();
	sem_post(&g_thread_sem);
	node_table = (struct node**)getTable(g_group.node_table);
	while(1) {
//		usleep(1000);
		sem_wait(&g_sender_info.sem);
		pthread_mutex_lock(&g_sender_info.lock);
		get = wqueuePop(g_sender_info.queue);
		pthread_mutex_unlock(&g_sender_info.lock);
		if(get==NULL){
			printf("Error:sender\n");
			exit(1);
		}
		if(get->id == -1)
			break;
//		req = get->send_buf;
		if(get->send_buf == NULL){
			printf("send memory error");
			printf("msg %u, id %u , size %u\n",get->msg, get->id, get->send_size);
			exit(1);
//			continue;
		}
//		struct request_header *req;
//		req = get->send_buf;
//		printf("send!!msg:%u srcnode:%u src_seq%u desnode:%u des_seq:%u, size:%u \n",req->msg_type,
//		req->src_node,req->src_seq_number,	get->id, req->seq_number, get->send_size);
		
		if(node_table[get->id]->send_fd < 0)
			node_table[get->id]->send_fd = TCP_connect(node_table[get->id]->ip, node_table[get->id]->port);
				
		sent_size = 0;
		do{
			sent_size += send(node_table[get->id]->send_fd, get->send_buf+sent_size, get->send_size-sent_size, 0);
		}while(get->send_size > sent_size);
			
		g_overhead.msg_count++;
		g_overhead.msg_size += get->send_size;
		sem_post(&get->sem);
/*		
		get->id = -2;
		get->msg = 0;
		get->send_size = 0;
		get->send_buf = NULL;
*/
	}
	sender_destroy();
	return NULL;
}
int sender_close(){
	struct sender_table *get;
	get = mem_malloc(sizeof(struct sender_table));
	get->id = -1 ;
	pthread_mutex_lock(&g_sender_info.lock);
	wqueuePush(g_sender_info.queue, get);
	pthread_mutex_unlock(&g_sender_info.lock);	
	sem_post(&g_sender_info.sem);
	return 1;
}


