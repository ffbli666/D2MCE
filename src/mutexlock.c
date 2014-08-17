#include "mutexlock.h"

struct mutex_lock* createNewMutex(unsigned int hash_id, char *name){
	struct mutex_lock *new_mutex;
	new_mutex = malloc(sizeof(struct mutex_lock));
	new_mutex->hash.id = hash_id;
	new_mutex->hash.next = NULL;
	new_mutex->hash.name = name;
	new_mutex->mutex = FALSE;
	pthread_mutex_init(&new_mutex->lock, NULL);
	new_mutex->queue = queueCreate(MAX_NODE_NUM+1, sizeof(struct req_node_info));
	return new_mutex;
}

int insertNewMutex(unsigned int hash_id, char *name){
	struct mutex_lock *new_mutex;
	new_mutex = createNewMutex(hash_id, name);
	hashTableInsert(g_group.mutex_table, (struct hashheader*)new_mutex);
	return 1;
}
int mutex_init_reply(void *request){
	char *new_mutex_name;
	struct request_header reply;
	struct mutex_lock *new_mutex;
	new_mutex_name = malloc(((struct mutex_req*)request)->name_len+1);
	new_mutex_name[((struct mutex_req*)request)->name_len] = 0;
	memcpy(new_mutex_name, request+sizeof(struct mutex_req), ((struct mutex_req*)request)->name_len);
	new_mutex = hashTableSearch(g_group.mutex_table, ((struct mutex_req*)request)->hash_id, new_mutex_name);
	if(new_mutex == NULL){
		insertNewMutex(((struct mutex_req*)request)->hash_id, new_mutex_name);
	}else{
		free(new_mutex_name);
	}
	reply.msg_type = MSG_MUTEX_OK;
	reply.seq_number = ((struct mutex_req*)request)->req.src_seq_number;
	sendTo(((struct mutex_req*)request)->req.src_node, (void*)&reply, sizeof(struct request_header));
	return 1;
}

int mutex_lock_reply(void *request){
	char send = FALSE;
	struct request_header reply;
	struct mutex_lock *find;
	struct req_node_info node;
	node.id = ((struct mutex_req*)request)->req.src_node;
	node.seq_number = ((struct mutex_req*)request)->req.src_seq_number;
	*(char*)(request+sizeof(struct mutex_req)+((struct mutex_req*)request)->name_len)=0;
	find = hashTableSearch(g_group.mutex_table, ((struct mutex_req*)request)->hash_id, (request+sizeof(struct mutex_req)));	
	reply.seq_number = node.seq_number;
	if(find == NULL){
		return -1;
	}
	
	pthread_mutex_lock(&find->lock);
	if(find->mutex == FALSE){
		find->mutex = TRUE;
		find->owner = node.id;
		send=TRUE;
	}
	else
		queuePush(find->queue, (void*)&node);
	pthread_mutex_unlock(&find->lock);
		
	if(send == TRUE){
		reply.msg_type = MSG_MUTEX_OK;
		sendTo(((struct mutex_req*)request)->req.src_node, (void*)&reply, sizeof(struct request_header));
	}
	return 1;
}

int mutex_unlock_reply(void *request){	
	struct request_header reply;
	struct mutex_lock *find;
	struct req_node_info *get;	
	*(char*)(request+sizeof(struct mutex_req)+((struct mutex_req*)request)->name_len)=0;
	find = hashTableSearch(g_group.mutex_table, ((struct mutex_req*)request)->hash_id, (request+sizeof(struct mutex_req)));
	if(find == NULL){
		return -1;
	}	
	if(find->owner != ((struct mutex_req*)request)->req.src_node){		
		return -1;
	}
	reply.msg_type = MSG_MUTEX_OK;
	pthread_mutex_lock(&find->lock);
	if(find->mutex == TRUE){
		get=(struct req_node_info*)queuePop(find->queue);		
		if(get==NULL){
			find->mutex = FALSE;
//			find->owner = -1;
		}
		else{
			find->owner = get->id;
/*			if(get->id == g_group.coordinator.mutex_id)
				sem_post(get->sem);
			else{*/
				reply.seq_number = get->seq_number; 
				sendTo(get->id, (void*)&reply, sizeof(struct request_header));
//			}

		}
	}
	else
		return -1;
	pthread_mutex_unlock(&find->lock);
	return 1;
}

int mutex_imanager_reply(void *request){
	int i, j;
	void *buf;
	int index;
	unsigned int mutex_num = 0;
	unsigned int offset;
	unsigned int *send_node;
	struct req_node_info *node;
	struct mutex_lock *now;
	//need add lock
	buf = mem_malloc(sizeof(struct imutex_reply) + g_group.mutex_table->item_num*310);
	((struct imutex_reply*)buf)->req.msg_type = MSG_IMUTEX_REPLY;
	((struct imutex_reply*)buf)->req.seq_number = ((struct request_header*)request)->src_seq_number;
	offset = sizeof(struct imutex_reply);
	for(i=0; i<g_group.mutex_table->table_size; i++){
		now = (struct mutex_lock*)g_group.mutex_table->row[i];
		while(now!=NULL){
			mutex_num++;
			//max 305 byte of one mutex 16+32+1+8*32 = 305
			((struct mutex_info*)(buf+offset))->hash_id = now->hash.id;
			((struct mutex_info*)(buf+offset))->name_len = strlen(now->hash.name);
			((struct mutex_info*)(buf+offset))->mutex = now->mutex;
			((struct mutex_info*)(buf+offset))->owner = now->owner;
			((struct mutex_info*)(buf+offset))->queue_count = now->queue->use;
			offset += sizeof(struct mutex_info);
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
			now = (struct mutex_lock*)now->hash.next;
		}
	}
	((struct imutex_reply*)buf)->mutex_num = mutex_num;
	//send mutex info & wait new mutex ready
	sendRecv(((struct request_header*)request)->src_node, buf, offset, buf, sizeof(struct request_header));
	g_group.coordinator.mutex_id = ((struct request_header*)request)->src_node;
	//send to other node the
	((struct new_manager_req*)buf)->req.msg_type = MSG_NEWMUTEX_MANAGER;
	((struct new_manager_req*)buf)->new_manager = ((struct request_header*)request)->src_node;
	for(i=0;i<g_group.node_table->table_size;i++){
		send_node = tableGetRow(g_group.node_table, i);
		if(*send_node != -1 && *send_node != g_group.node_id && *send_node != ((struct request_header*)request)->src_node)
			sendTo(*send_node, buf, sizeof(struct new_manager_req));
	}
	mem_free(buf);
	return 1;	
	
}
int mutex_newmanager_reply(void *request){
	g_group.coordinator.mutex_id = ((struct new_manager_req*)request)->new_manager;
	return 1;
}
int mutex_imanager_req(){
	int i, j;
	void *buf;
	unsigned int offset;
	unsigned int queue_count;
	unsigned int src_node;
	unsigned int seq_number;
	struct mutex_lock *find;
	struct req_node_info node;
	char *mutex_name;
	if(g_group.coordinator.mutex_id == g_group.node_id)
		return 1;
	//max 33 mutex
	buf = mem_malloc(MUTEX_IMUTEX_SIZE);
	((struct request_header*)buf)->msg_type = MSG_IMUTEX_MANAGER;
	sendRecv(g_group.coordinator.mutex_id, buf, sizeof(struct request_header),
			buf, MUTEX_IMUTEX_SIZE);
	if(g_group.mutex_table == NULL){
		g_group.mutex_table = hashTableCreate(MUTEX_HASH_SIZE);
	}
	src_node = ((struct imutex_reply*)buf)->req.src_node;
	seq_number = ((struct imutex_reply*)buf)->req.src_seq_number;
	offset = sizeof(struct imutex_reply);
	for(i=0; i<((struct imutex_reply*)buf)->mutex_num; i++){
		//mutex name
		mutex_name = malloc(((struct mutex_info*)(buf+offset))->name_len);
		memcpy(mutex_name, buf+offset+sizeof(struct mutex_info), ((struct mutex_info*)(buf+offset))->name_len);
		mutex_name[((struct mutex_info*)(buf+offset))->name_len] = 0;
		//search mutex
		find = hashTableSearch(g_group.mutex_table, ((struct mutex_info*)(buf+offset))->hash_id, mutex_name);
		if(find == NULL){
			//create new
			find = createNewMutex(((struct mutex_info*)(buf+offset))->hash_id, mutex_name);
			hashTableInsert(g_group.mutex_table, (struct hashheader*)find);
		}else{
			if(find->queue->use!=0){
				while(queuePop(find->queue)!=NULL);
			}
		}
		//copy 
		find->mutex = ((struct mutex_info*)(buf+offset))->mutex;
		find->owner = ((struct mutex_info*)(buf+offset))->owner;
		//copy queue info
		queue_count = ((struct mutex_info*)(buf+offset))->queue_count;
		offset += sizeof(struct mutex_info)+strlen(mutex_name);
//		printf("!!!!mutex:%s owner:%d", find->hash.name, find->owner);
		for(j=0; j< queue_count;j++){
			node.id = ((struct req_node_info*)(buf+offset))->id;
			node.seq_number = ((struct req_node_info*)(buf+offset))->seq_number;
//			printf(" node:%d id:%d seq:%d", j, node.id, node.seq_number);
			queuePush(find->queue, (void*)&node);
			offset += sizeof(struct req_node_info);
		}
//		printf("\n");
		
	}	
	g_group.coordinator.mutex_id = g_group.node_id;
	((struct request_header*)buf)->msg_type = MSG_IMUTEX_READY;
	((struct request_header*)buf)->seq_number = seq_number;
	sendTo(src_node ,buf, sizeof(struct request_header));
	mem_free(buf);
	return 1;
}

