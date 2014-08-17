#include "semaphore.h"

struct sem* createNewSem(unsigned int hash_id, char *name, unsigned value){
	struct sem *new_sem;
	new_sem = malloc(sizeof(struct sem));
	new_sem->hash.id = hash_id;
	new_sem->hash.next = NULL;
	new_sem->hash.name = name;
	new_sem->value = value;
	pthread_mutex_init(&new_sem->lock, NULL);
	new_sem->queue = queueCreate(MAX_NODE_NUM+1, sizeof(struct req_node_info));
	return new_sem;
}

int insertNewSem(unsigned int hash_id, char *name, unsigned int value){
	struct sem *new_sem;
	new_sem = createNewSem(hash_id, name, value);
	hashTableInsert(g_group.sem_table, (struct hashheader*)new_sem);
	return 1;
}
int sem_init_reply(void *request){
	char *new_sem_name;
	struct request_header reply;
	struct sem *new_sem;
	new_sem_name = malloc(((struct sem_init_req*)request)->name_len+1);
	new_sem_name[((struct sem_init_req*)request)->name_len]=0;
	memcpy(new_sem_name, request+sizeof(struct sem_init_req), ((struct sem_init_req*)request)->name_len);
	new_sem = hashTableSearch(g_group.sem_table, ((struct sem_init_req*)request)->hash_id, new_sem_name);
	if(new_sem == NULL){
		insertNewSem(((struct sem_init_req*)request)->hash_id, new_sem_name, ((struct sem_init_req*)request)->value);
	}else{
		free(new_sem_name);
	}
	reply.msg_type = MSG_SEM_OK;
	reply.seq_number = ((struct sem_init_req*)request)->req.src_seq_number;
	sendTo(((struct sem_req*)request)->req.src_node, (void*)&reply, sizeof(struct request_header));
	return 1;
}


int sem_post_reply(void *request){	
	struct sem *find;
	struct req_node_info *get;	
	struct reply_header reply;
	*(char*)(request+sizeof(struct sem_req)+((struct sem_req*)request)->name_len)=0;
	find = hashTableSearch(g_group.sem_table, ((struct sem_req*)request)->hash_id, (request+sizeof(struct sem_req)));
	if(find == NULL){
		return -1;
	}		
	reply.msg_type = MSG_SEM_OK;
	pthread_mutex_lock(&find->lock);
	find->value++;		//MAX = 65535 maybe over!! need check
	if(find->value >0){
		get = queuePop(find->queue);
		if(get!=NULL){
			find->value--;			
//			if(get->id == g_group.coordinator.sem_id)
//				sem_post(get->sem);
//			else{
				reply.seq_number = get->seq_number; 
				sendTo(get->id, (void*)&reply, sizeof(struct request_header));
//			}
		}
	}
	pthread_mutex_unlock(&find->lock);

	return 1;
}


int sem_wait_reply(void *request){
	char send = FALSE;
	struct request_header reply;
	struct sem *find;
	struct req_node_info node;
	node.id = ((struct sem_req*)request)->req.src_node;
	node.seq_number = ((struct sem_req*)request)->req.src_seq_number;
	*(char*)(request+sizeof(struct sem_req)+((struct sem_req*)request)->name_len)=0;
	find = hashTableSearch(g_group.sem_table, ((struct sem_req*)request)->hash_id, (request+sizeof(struct sem_req)));
	
	reply.seq_number = node.seq_number;
	if(find == NULL){
		reply.msg_type = MSG_SEM_FAILED;
		sendTo(node.id, (void*)&reply, sizeof(struct request_header));
		return -1;
	}
	
	pthread_mutex_lock(&find->lock);
	if(find->value >0){
		send=TRUE;
		find->value--;
	}
	else
		queuePush(find->queue, (void*)&node);
	pthread_mutex_unlock(&find->lock);
	if(send==TRUE){
		reply.msg_type = MSG_SEM_OK;
		sendTo(node.id, (void*)&reply, sizeof(struct request_header));		
	}
	return 1;
}

int sem_imanager_reply(void *request){
	int i, j;
	void *buf;
	int index;
	unsigned int sem_num = 0;
	unsigned int offset;
	unsigned int *send_node;
	struct req_node_info *node;
	struct sem *now;
	buf = mem_malloc(sizeof(struct isem_reply) + g_group.sem_table->item_num*310);
	((struct isem_reply*)buf)->req.msg_type = MSG_ISEM_REPLY;
	((struct isem_reply*)buf)->req.seq_number = ((struct request_header*)request)->src_seq_number;
	offset = sizeof(struct isem_reply);
	for(i=0; i<g_group.sem_table->table_size; i++){
		now = (struct sem*)g_group.sem_table->row[i];
		while(now!=NULL){
			sem_num++;
			//max 305 byte of one sem 16+32+1+8*32 = 305
			((struct sem_info*)(buf+offset))->hash_id = now->hash.id;
			((struct sem_info*)(buf+offset))->name_len = strlen(now->hash.name);
			((struct sem_info*)(buf+offset))->value = now->value;
			((struct sem_info*)(buf+offset))->queue_count = now->queue->use;
			offset += sizeof(struct sem_info);
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
			now = (struct sem*)now->hash.next;
		}
	}
	((struct isem_reply*)buf)->sem_num = sem_num;

	//send sem info & wait new sem ready
	sendRecv(((struct request_header*)request)->src_node, buf, offset, buf, sizeof(struct request_header));
	g_group.coordinator.sem_id = ((struct request_header*)request)->src_node;
	//send to other node the
	((struct new_manager_req*)buf)->req.msg_type = MSG_NEWSEM_MANAGER;
	((struct new_manager_req*)buf)->new_manager = ((struct request_header*)request)->src_node;
	for(i=0;i<g_group.node_table->table_size;i++){
		send_node = tableGetRow(g_group.node_table, i);
		if(*send_node != -1 && *send_node != g_group.node_id && *send_node != ((struct request_header*)request)->src_node)
			sendTo(*send_node, buf, sizeof(struct new_manager_req));
	}
	mem_free(buf);
	return 1;	
}
int sem_newmanager_reply(void *request){
	g_group.coordinator.sem_id = ((struct new_manager_req*)request)->new_manager;
	return 1;
}
int sem_imanager_req(){
	int i, j;
	void *buf;
	unsigned int offset;
	unsigned int queue_count;
	unsigned int src_node;
	unsigned int seq_number;
	struct sem *find;
	struct req_node_info node;
	char *sem_name;
	if(g_group.coordinator.sem_id == g_group.node_id)
		return 1;
	//max 33 sem
	buf = mem_malloc(SEM_ISEM_SIZE);
	((struct request_header*)buf)->msg_type = MSG_ISEM_MANAGER;
	sendRecv(g_group.coordinator.sem_id, buf, sizeof(struct request_header),
			buf, SEM_ISEM_SIZE);
	if(g_group.sem_table == NULL){
		g_group.sem_table = hashTableCreate(SEM_HASH_SIZE);
	}
	src_node = ((struct isem_reply*)buf)->req.src_node;
	seq_number = ((struct isem_reply*)buf)->req.src_seq_number;
	offset = sizeof(struct isem_reply);
	for(i=0; i<((struct isem_reply*)buf)->sem_num; i++){
		//sem name
		sem_name = malloc(((struct sem_info*)(buf+offset))->name_len);
		memcpy(sem_name, buf+offset+sizeof(struct sem_info), ((struct sem_info*)(buf+offset))->name_len);
		sem_name[((struct sem_info*)(buf+offset))->name_len] = 0;
		//search sem
		find = hashTableSearch(g_group.sem_table, ((struct sem_info*)(buf+offset))->hash_id, sem_name);
		if(find == NULL){
			//create new
			find = createNewSem(((struct sem_info*)(buf+offset))->hash_id, sem_name, ((struct sem_info*)(buf+offset))->value);
			hashTableInsert(g_group.sem_table, (struct hashheader*)find);
		}else{
			if(find->queue->use!=0){
				while(queuePop(find->queue)!=NULL);
			}
		}
		//copy queue info
		queue_count = ((struct sem_info*)(buf+offset))->queue_count;
		offset += sizeof(struct sem_info)+strlen(sem_name);
		for(j=0; j< queue_count;j++){
			node.id = ((struct req_node_info*)(buf+offset))->id;
			node.seq_number = ((struct req_node_info*)(buf+offset))->seq_number;
			queuePush(find->queue, (void*)&node);
			offset += sizeof(struct req_node_info);
		}
	}	
	g_group.coordinator.sem_id = g_group.node_id;
	((struct request_header*)buf)->msg_type = MSG_ISEM_READY;
	((struct request_header*)buf)->seq_number = seq_number;
	sendTo(src_node ,buf, sizeof(struct request_header));
	mem_free(buf);
	return 1;
}

