#include "barrier.h"


struct barrier* createNewBarrier(unsigned int hash_id, char *name){
	struct barrier *new_barrier;
	new_barrier = malloc(sizeof(struct barrier));
	new_barrier->hash.id = hash_id;
	new_barrier->hash.next = NULL;
	new_barrier->hash.name = name;
	new_barrier->wait_counter = 0;
	new_barrier->state = OFF;
	pthread_mutex_init(&new_barrier->lock, NULL);
	new_barrier->queue = queueCreate(MAX_NODE_NUM+1, sizeof(struct req_node_info));
	return new_barrier;
}

int insertNewBarrier(unsigned int hash_id, char *name){
	struct barrier *new_barrier;
	new_barrier = createNewBarrier(hash_id, name);
	hashTableInsert(g_group.barrier_table, (struct hashheader*)new_barrier);
	return 1;
}

int barrier_init_reply(void *request){
	char *new_barrier_name;
	struct request_header reply;
	struct barrier *new_barrier;
	new_barrier_name = malloc(((struct barrier_init_req*)request)->name_len+1);
	new_barrier_name[((struct barrier_init_req*)request)->name_len] = 0;
	memcpy(new_barrier_name, request+sizeof(struct barrier_init_req), ((struct barrier_init_req*)request)->name_len);
	new_barrier = hashTableSearch(g_group.barrier_table, ((struct barrier_init_req*)request)->hash_id, new_barrier_name);
	if(new_barrier == NULL){
		insertNewBarrier(((struct barrier_init_req*)request)->hash_id, new_barrier_name);
	}else{
		free(new_barrier_name);
	}
	reply.msg_type = MSG_BARRIER_OK;
	reply.seq_number = ((struct barrier_init_req*)request)->req.src_seq_number;
	sendTo(((struct barrier_init_req*)request)->req.src_node, (void*)&reply, sizeof(struct request_header));
	return 1;
}

int barrier_reply(void *request){
	void *buf = NULL;
	struct barrier *find;
	struct req_node_info node;
	struct req_node_info *get;
	buf = mem_malloc(sizeof(struct request_header));
	node.id = ((struct barrier_req*)request)->req.src_node;
	node.seq_number = ((struct barrier_req*)request)->req.src_seq_number;
	*(char*)(request+sizeof(struct barrier_req)+((struct barrier_req*)request)->name_len)=0;
	find = hashTableSearch(g_group.barrier_table, ((struct barrier_req*)request)->hash_id, (request+sizeof(struct barrier_req)));
	if(find == NULL){
		((struct request_header*)buf)->msg_type = MSG_BARRIER_FAILED;
		((struct request_header*)buf)->seq_number = node.seq_number;
		sendTo(node.id, buf, sizeof(struct request_header));
		mem_free(buf);
		return -1;
	}
	pthread_mutex_lock(&find->lock);
	if(find->state == OFF){
		find->state = ON;
		find->wait_counter = ((struct barrier_req*)request)->wait_counter;
	}
	find->wait_counter--;
	queuePush(find->queue, (void*)&node);
	if(find->wait_counter==0){
		((struct request_header*)buf)->msg_type = MSG_BARRIER_OK;
		while(find->queue->use>0){
			get = (struct req_node_info*)queuePop(find->queue);
			((struct request_header*)buf)->seq_number = get->seq_number;
			sendTo(get->id, buf, sizeof(struct request_header));
		}
		find->state=OFF;
	}
	pthread_mutex_unlock(&find->lock);
	mem_free(buf);
	return 1;
}

int barrier_imanager_reply(void *request){
	int i, j;
	void *buf = NULL;
	int index;
	unsigned int barrier_num = 0;
	unsigned int offset;
	unsigned int *send_node;
	struct req_node_info *node;
	struct barrier *now;
	buf = mem_malloc(sizeof(struct ibarrier_reply) + g_group.barrier_table->item_num*310);
	((struct ibarrier_reply*)buf)->req.msg_type = MSG_IBAR_REPLY;
	((struct ibarrier_reply*)buf)->req.seq_number = ((struct request_header*)request)->src_seq_number;
	offset = sizeof(struct ibarrier_reply);
	for(i=0; i<g_group.barrier_table->table_size; i++){
		now = (struct barrier*)g_group.barrier_table->row[i];
		while(now!=NULL){
			barrier_num++;
			//max 310 byte of one barrier 20+32+1+8*32 = 309
			((struct barrier_info*)(buf+offset))->hash_id = now->hash.id;
			((struct barrier_info*)(buf+offset))->name_len = strlen(now->hash.name);
			((struct barrier_info*)(buf+offset))->wait_counter = now->wait_counter;
			((struct barrier_info*)(buf+offset))->state = now->state;
			((struct barrier_info*)(buf+offset))->queue_count = now->queue->use;
			offset += sizeof(struct barrier_info);
			memcpy(buf+offset, now->hash.name, strlen(now->hash.name));
			offset += strlen(now->hash.name);
			//node info in the queue
			for(j=0;j<now->queue->use;j++){
				index = now->queue->front + j;
				if(index >= now->queue->queue_size)
					index -= now->queue->queue_size;
				node = now->queue->row[index];
				((struct req_node_info*)(buf+offset))->id = node->id;
				((struct req_node_info*)(buf+offset))->seq_number = node->seq_number;
				offset += sizeof(struct req_node_info);
			}
			now = (struct barrier*)now->hash.next;
		}
	}
	((struct ibarrier_reply*)buf)->barrier_num = barrier_num;
	//send barrier info & wait new barrier ready
	sendRecv(((struct request_header*)request)->src_node, buf, offset, buf, sizeof(struct request_header));
	g_group.coordinator.barrier_id = ((struct request_header*)request)->src_node;
	//send to other node the
	((struct new_manager_req*)buf)->req.msg_type = MSG_NEWBAR_MANAGER;
	((struct new_manager_req*)buf)->new_manager = ((struct request_header*)request)->src_node;
	for(i=0;i<g_group.node_table->table_size;i++){
		send_node = tableGetRow(g_group.node_table, i);
		if(*send_node != -1 && *send_node != g_group.node_id && *send_node != ((struct request_header*)request)->src_node)
			sendTo(*send_node, buf, sizeof(struct new_manager_req));
	}
	mem_free(buf);
	return 1;	
}

int barrier_newmanager_reply(void *request){
	g_group.coordinator.barrier_id = ((struct new_manager_req*)request)->new_manager;
	return 1;
}

int barrier_imanager_req(){
	int i, j;
	void *buf = NULL;
	unsigned int offset;
	unsigned int queue_count;
	unsigned int src_node;
	unsigned int seq_number;
	struct barrier *find;
	struct req_node_info node;
	char *barrier_name;
	if(g_group.coordinator.barrier_id == g_group.node_id)
		return 1;
	//max 33 barrier
	buf = mem_malloc(BARRIER_IBAR_SIZE);
	((struct request_header*)buf)->msg_type = MSG_IBAR_MANAGER;
	sendRecv(g_group.coordinator.barrier_id, buf, sizeof(struct request_header),
			buf, BARRIER_IBAR_SIZE);
	if(g_group.barrier_table == NULL){
		g_group.barrier_table = hashTableCreate(BARRIER_HASH_SIZE);
	}
	src_node = ((struct ibarrier_reply*)buf)->req.src_node;
	seq_number = ((struct ibarrier_reply*)buf)->req.src_seq_number;
	offset = sizeof(struct ibarrier_reply);
	for(i=0; i<((struct ibarrier_reply*)buf)->barrier_num; i++){
		//barrier name
		barrier_name = malloc(((struct barrier_info*)(buf+offset))->name_len);
		memcpy(barrier_name, buf+offset+sizeof(struct barrier_info), ((struct barrier_info*)(buf+offset))->name_len);
		barrier_name[((struct barrier_info*)(buf+offset))->name_len] = 0;
		//search barrier
		find = hashTableSearch(g_group.barrier_table, ((struct barrier_info*)(buf+offset))->hash_id, barrier_name);
		if(find == NULL){
			//create new
			find = createNewBarrier(((struct barrier_info*)(buf+offset))->hash_id, barrier_name);
			hashTableInsert(g_group.barrier_table, (struct hashheader*)find);
		}else{
			if(find->queue->use!=0){
				while(queuePop(find->queue)!=NULL);
			}
		}
		//copy state
		find->wait_counter = ((struct barrier_info*)(buf+offset))->wait_counter;
		find->state = ((struct barrier_info*)(buf+offset))->state;
		//copy queue info
		queue_count = ((struct barrier_info*)(buf+offset))->queue_count;
		offset += sizeof(struct barrier_info)+strlen(barrier_name);
		for(j=0; j< queue_count;j++){
			node.id = ((struct req_node_info*)(buf+offset))->id;
			node.seq_number = ((struct req_node_info*)(buf+offset))->seq_number;
			queuePush(find->queue, (void*)&node);
			offset += sizeof(struct req_node_info);
		}
	}	
	g_group.coordinator.barrier_id = g_group.node_id;
	((struct request_header*)buf)->msg_type = MSG_IBAR_READY;
	((struct request_header*)buf)->seq_number = seq_number;
	sendTo(src_node ,buf, sizeof(struct request_header));
	mem_free(buf);
	return 1;
}


