#include "sharememory.h"

int searchNode(struct table *node, unsigned int node_id){
	int i;
	unsigned int *get;
	for(i=0;i<node->use;i++){
		get = node->row[i];
		if(*get == node_id)
			return i;
	}
	return -1;
}

void* createSM(unsigned int hash_id, char *name, unsigned int home_node, unsigned int size){
	int i;
	unsigned int *node;
	struct sm_header *sm_header;
	sm_header = malloc(sizeof(struct sm_header));
	if(sm_header == NULL)
		return NULL;
	sm_header->hash.id = hash_id;
	sm_header->hash.next = NULL;
	sm_header->hash.name = name;
	sm_header->home_node = home_node;
	sm_header->size = size;
	sm_header->info = NULL;
	pthread_mutex_init(&sm_header->lock, NULL);
//	sm_header->count = 1;
	if(home_node == g_group.node_id){
		sm_header->users = tableCreate(MAX_NODE_NUM, sizeof(unsigned int));
		for(i=0;i<MAX_NODE_NUM;i++){
			node = sm_header->users->row[i];
			*node = -1;
		}
	}
	return sm_header;
}

void* mallocSM(unsigned int size, char state, int ishome){
	struct sm_info *new_sm;
	new_sm = malloc(sizeof(struct sm_info)+size);
	if(new_sm == NULL)
		return NULL;
	new_sm->header = NULL;
	new_sm->last_node.id = 0;
	new_sm->last_node.seq_number = -1;
	new_sm->state = state;
#ifdef	CONF_SM_EVENT_DRIVER
	sem_init(&new_sm->event.sem,0,0);
	new_sm->event.wait_count=0;
#endif
	if(ishome == YES){
		new_sm->need = tableCreate(MAX_NODE_NUM, sizeof(unsigned int));
#ifdef CONF_SM_DISSEMINATE_UPDATE
		new_sm->disseminate = tableCreate(MAX_NODE_NUM, sizeof(unsigned int));
#endif
#ifdef CONF_SM_AUTO_HOME_MIGARATION
		new_sm->scoreboard = tableCreate(MAX_NODE_NUM, sizeof(int));
		sm_score_zero(new_sm);
#endif
	}
	return new_sm;
}

int sm_init_reply(void *request){
	int index;
	char *new_sm_name;
	unsigned int src_node;
	unsigned int name_len;
	struct request_header reply;
	struct sm_init_reply init_reply;
	struct sm_header *sm_header;
 	name_len = ((struct sm_init_req*)request)->name_len;
	new_sm_name = malloc(name_len+1);
	memcpy(new_sm_name, request+sizeof(struct sm_init_req), name_len);
	new_sm_name[name_len]=0;
	src_node = ((struct sm_req*)request)->req.src_node;
	sm_header = hashTableSearch(g_group.sm_table, ((struct sm_init_req*)request)->hash_id, new_sm_name);
	//not register then create
	if(sm_header == NULL){
		sm_header = createSM(((struct sm_init_req*)request)->hash_id, new_sm_name, 
					src_node, ((struct sm_init_req*)request)->size);
		hashTableInsert(g_group.sm_table, (struct hashheader*)sm_header);
		if(sm_header == NULL){			
			reply.msg_type = MSG_SM_FAILED;
			reply.seq_number = ((struct sm_init_req*)request)->req.src_seq_number;
			sendTo(src_node, (void*)&reply, sizeof(struct request_header));
			return -1;
		}
		reply.msg_type = MSG_SM_OK;
		reply.seq_number = ((struct sm_init_req*)request)->req.src_seq_number;
		sendTo(src_node, (void*)&reply, sizeof(struct request_header));
		return 1;
	}else{
		free(new_sm_name);
	}
	if(sm_header->size != ((struct sm_init_req*)request)->size){		
		reply.msg_type = MSG_SM_FAILED;
		reply.seq_number = ((struct sm_init_req*)request)->req.src_seq_number;
		sendTo(src_node, (void*)&reply, sizeof(struct request_header));
		return -1;
	}
//	sm_header->count++;
	pthread_mutex_lock(&sm_header->lock);
	if(searchNode(sm_header->users, src_node)==-1){
		index = tableGetEmpty(sm_header->users);
		if(index == -1)
			return -1;
		tableAdd(sm_header->users, (void*)&src_node, index);
	}
	pthread_mutex_unlock(&sm_header->lock);
	init_reply.req.msg_type = MSG_SM_INIT_REPLY;
	init_reply.req.seq_number = ((struct sm_init_req*)request)->req.src_seq_number;
	init_reply.home_node = sm_header->home_node;
	sendTo(((struct sm_req*)request)->req.src_node, (void*)&init_reply, sizeof(struct sm_init_reply));
	return 1;
}


int sm_readmiss_reply(void *request){
#ifdef	CONF_SM_EVENT_DRIVER
	int i;
#endif
#ifdef CONF_SM_AUTO_HOME_MIGARATION
	int score;
#endif
	int index;
	unsigned int src_node;
	void *buf = NULL;
	struct request_header reply;
	struct sm_header *find;
	struct sm_info *info;
	*(char*)(request+sizeof(struct sm_req)+((struct sm_req*)request)->name_len)=0;
	src_node = ((struct sm_req*)request)->req.src_node;
	find = hashTableSearch(g_group.sm_table, ((struct sm_req*)request)->hash_id, (request+sizeof(struct sm_req))); 
	if(find == NULL || find->info == NULL){
		reply.msg_type = MSG_SM_FAILED;
		reply.seq_number = ((struct sm_req*)request)->req.src_seq_number;
		sendTo(src_node, (void*)&reply, sizeof(struct request_header));
		return -1;
	}
	info = find->info;
	buf = mem_malloc(sizeof(struct sm_req)+strlen(find->hash.name)+find->size);
//	buf = malloc(sizeof(struct sm_req)+strlen(find->hash.name)+find->size);
	pthread_mutex_lock(&find->lock);
	if(find->home_node == g_group.node_id){
		if(info->state == INVALID && info->last_node.id != src_node){
			//fetch the last node data
			((struct sm_req*)buf)->req.msg_type = MSG_SM_FETCH;
			((struct sm_req*)buf)->hash_id = find->hash.id;
			((struct sm_req*)buf)->name_len = strlen(find->hash.name);
			memcpy(buf+sizeof(struct sm_req), find->hash.name, ((struct sm_req*)buf)->name_len);
			//mabye deadlock
			sendRecv(info->last_node.id , buf, sizeof(struct sm_req)+((struct sm_req*)buf)->name_len,
					buf, sizeof(struct request_header)+find->size);
			memcpy((void*)info+sizeof(struct sm_info), buf+sizeof(struct request_header), find->size);
			info->state=SHARED;
#ifdef CONF_SM_EVENT_DRIVER
			for(i=0;i<info->event.wait_count;i++)
				sem_post(&info->event.sem);
			info->event.wait_count = 0;
#endif
		}
		else{
			info->state=SHARED;
			((struct request_header*)buf)->msg_type = MSG_SM_OK;
			memcpy(buf+sizeof(struct request_header), (void*)info+sizeof(struct sm_info), find->size);			
		}
#ifdef CONF_SM_DISSEMINATE_UPDATE
		if(searchNode(info->disseminate, src_node)==-1 && searchNode(info->need, src_node)==-1)
#else
		if(searchNode(info->need, src_node)==-1)
#endif
		{
			index = tableGetEmpty(info->need);
			if(index == -1){
				pthread_mutex_unlock(&find->lock);
				if(buf!=NULL)
					mem_free(buf);
				return -1;
			}
			tableAdd(info->need, (void*)&src_node, index);
		}
		pthread_mutex_unlock(&find->lock);		
		((struct request_header*)buf)->seq_number = ((struct sm_req*)request)->req.src_seq_number;
		sendTo(src_node, buf, sizeof(struct request_header)+find->size);
#ifdef CONF_SM_AUTO_HOME_MIGARATION
		score = sm_score_add(info, src_node, 1);
		if(score >=SM_HOME_MIGRATION_THRESHOLD){
			if(find->home_node != src_node )
				sm_uhome_req(info, src_node);
			sm_score_zero(info);
		}
#endif
	}
	else{//not home node
		pthread_mutex_unlock(&find->lock);
		forward(find->home_node, request);
	}
	mem_free(buf);
//	free(buf);
	return 1;
}

//only last node
int sm_fetch_reply(void *request){
	void *buf;
	struct reply_header reply;
	struct sm_header *find;
	struct sm_info *info;
	*(char*)(request+sizeof(struct sm_req)+((struct sm_req*)request)->name_len)=0;
	find = hashTableSearch(g_group.sm_table, ((struct sm_req*)request)->hash_id, (request+sizeof(struct sm_req))); 
	if(find == NULL || find->info == NULL){
//		printf("fetch not found\n");
		reply.msg_type = MSG_SM_FAILED;
		reply.seq_number = ((struct sm_req*)request)->req.src_seq_number;
		sendTo(((struct sm_req*)request)->req.src_node, (void*)&reply, sizeof(struct request_header));
		return -1;
	}
	info = find->info;	
	buf = mem_malloc(sizeof(struct request_header)+find->size);
	
	((struct request_header*)buf)->msg_type = MSG_SM_OK;
	((struct request_header*)buf)->seq_number = ((struct sm_req*)request)->req.src_seq_number;
//	pthread_mutex_lock(&find->lock);
	info->state = SHARED;
	memcpy(buf+sizeof(struct request_header), (void*)info+sizeof(struct sm_info), find->size);
//	pthread_mutex_unlock(&find->lock);
	sendTo(((struct sm_req*)request)->req.src_node, buf, sizeof(struct request_header)+find->size);

	mem_free(buf);
	return 1;
}

int sm_writemiss_reply(void *request){
	int i;
#ifdef CONF_SM_AUTO_HOME_MIGARATION
	int score;
#endif
	unsigned int *node;
	unsigned int send_size;
	void *buf = NULL;
	struct sm_header *find;
	struct sm_info *info;
	struct request_header reply;
	*(char*)(request+sizeof(struct sm_req)+((struct sm_req*)request)->name_len)=0;
	find = hashTableSearch(g_group.sm_table, ((struct sm_req*)request)->hash_id, (request+sizeof(struct sm_req))); 
	if(find == NULL || find->info == NULL){
//		printf("writemiss not found\n");
		return -1;
	}
	info = find->info;
	pthread_mutex_lock(&find->lock);
	if(find->home_node == g_group.node_id){
		info->last_node.id = ((struct sm_req*)request)->req.src_node;
		info->last_node.seq_number = ((struct sm_req*)request)->req.src_seq_number;
		info->state = INVALID;
#ifdef CONF_SM_DISSEMINATE_UPDATE			
		//send update
		if(info->disseminate->use > 0){
			//get buf
			send_size = sizeof(struct sm_req)+strlen(find->hash.name)+1+find->size;
			buf = mem_malloc(send_size);
				
			//fetch
			((struct sm_req*)buf)->req.msg_type = MSG_SM_FETCH;
			((struct sm_req*)buf)->hash_id = find->hash.id;
			((struct sm_req*)buf)->name_len = strlen(find->hash.name);
			memcpy(buf+sizeof(struct sm_req), find->hash.name, ((struct sm_req*)buf)->name_len);
			sendRecv(info->last_node.id , buf, sizeof(struct sm_req)+((struct sm_req*)buf)->name_len,
					buf, sizeof(struct request_header)+find->size);
			memcpy((void*)info+sizeof(struct sm_info), buf+sizeof(struct request_header), find->size);
			info->state=SHARED;
			//send update
			((struct sm_req*)buf)->req.msg_type = MSG_SM_UPDATE;
			((struct sm_req*)buf)->hash_id = find->hash.id;
			((struct sm_req*)buf)->name_len = strlen(find->hash.name);
			memcpy(buf+sizeof(struct sm_req), find->hash.name, ((struct sm_req*)buf)->name_len);
			memcpy(buf+sizeof(struct sm_req)+((struct sm_req*)buf)->name_len+1, (void*)info+sizeof(struct sm_info), find->size);
			for(i=0;i<info->disseminate->use;i++){
				node = info->disseminate->row[i];
				if(*node != g_group.node_id && *node != ((struct sm_req*)request)->req.src_node)
					sendTo(*node, buf , send_size);
			}
		}
#endif	
#ifdef CONF_SM_EVENT_DRIVER
		for(i=0;i<info->event.wait_count;i++)
			sem_post(&info->event.sem);
		info->event.wait_count = 0;
#endif
		//send invalide	
		if(buf == NULL){
			buf = mem_malloc(SM_BUF_SIZE);
		}
		if(((struct sm_req*)request)->req.msg_type == MSG_SM_WRITEMISS)
			((struct sm_req*)buf)->req.msg_type = MSG_SM_INVALID;
		else
			((struct sm_req*)buf)->req.msg_type = MSG_SM_INVALID_NOREPLY;
		((struct sm_req*)buf)->hash_id = find->hash.id;
		((struct sm_req*)buf)->name_len = strlen(find->hash.name);
		memcpy(buf+sizeof(struct sm_req), find->hash.name, ((struct sm_req*)buf)->name_len);
		info->sync = info->need->use;
		send_size = sizeof(struct sm_req)+((struct sm_req*)buf)->name_len;
		for(i=0; i<info->need->use; i++){
			node = info->need->row[i];
			if(*node == info->last_node.id || *node == g_group.node_id){
				info->sync--;
			}
			else
				sendTo(*node, buf, send_size);
			*node = -1;
		}
		if(info->sync==0 && ((struct sm_req*)request)->req.msg_type == MSG_SM_WRITEMISS){
			reply.msg_type = MSG_SM_OK;
			reply.seq_number = info->last_node.seq_number;
			sendTo(info->last_node.id, (void*)&reply, sizeof(struct request_header));
		}
#ifdef CONF_SM_DISSEMINATE_UPDATE
		if(searchNode(info->disseminate, info->last_node.id)==-1){
#endif
			tableAdd(info->need, (void*)&info->last_node.id , 0);
			info->need->index = 1;
			info->need->use = 1;	
#ifdef CONF_SM_DISSEMINATE_UPDATE
		}
		else{
			info->need->index = 0;
			info->need->use = 0;
		}
#endif
		pthread_mutex_unlock(&find->lock);
#ifdef CONF_SM_AUTO_HOME_MIGARATION
		score = sm_score_add(info, ((struct sm_req*)request)->req.src_node, 2);
		if(score >=SM_HOME_MIGRATION_THRESHOLD){
			printf("home %d\n", ((struct sm_req*)request)->req.src_node);
			if(find->home_node != ((struct sm_req*)request)->req.src_node )
				sm_uhome_req(info, ((struct sm_req*)request)->req.src_node);
			sm_score_zero(info);
		}
#endif
	}
	else{//not home node
		pthread_mutex_unlock(&find->lock);
		forward(find->home_node, request);
	}
	if(buf != NULL)
		mem_free(buf);
	return 1;

	
}

//pair of the write miss
int sm_invalid_ok(void *request){
	struct reply_header reply;
	struct sm_header *find;
	struct sm_info *info;
	*(char*)(request+sizeof(struct sm_req)+((struct sm_req*)request)->name_len)=0;
	find = hashTableSearch(g_group.sm_table, ((struct sm_req*)request)->hash_id, (request+sizeof(struct sm_req))); 
	if(find == NULL || find->info == NULL){
//		printf("invalid ok not found\n");
		return -1;
	}
	info = find->info;
	reply.msg_type = MSG_SM_OK;
	pthread_mutex_lock(&find->lock);
	info->sync--;
	if(info->sync==0){
		if(info->last_node.id == find->home_node){
			sem_post(info->last_node.sem);
			info->last_node.sem=NULL;
		}
		else{
			reply.seq_number = info->last_node.seq_number;
			sendTo(info->last_node.id, (void*)&reply, sizeof(struct request_header));
		}
	}
	pthread_mutex_unlock(&find->lock);
	return 1;		
}



int sm_invalid_reply(void *request){
#ifdef	CONF_SM_EVENT_DRIVER
	int i;
#endif
	void *buf;
	struct reply_header reply;
	struct sm_header *find;
	struct sm_info *info;
	*(char*)(request+sizeof(struct sm_req)+((struct sm_req*)request)->name_len)=0;
	find = hashTableSearch(g_group.sm_table, ((struct sm_req*)request)->hash_id, (request+sizeof(struct sm_req)));
	if(find == NULL || find->info == NULL){
//		printf("invalid reply not found\n");
		reply.msg_type = MSG_SM_FAILED;
		reply.seq_number = ((struct sm_req*)request)->req.src_seq_number;
		sendTo(((struct sm_req*)request)->req.src_node, (void*)&reply, sizeof(struct request_header));
		return -1;
	}
	info = find->info;
	pthread_mutex_lock(&find->lock);
	info->state = INVALID;
#ifdef CONF_SM_EVENT_DRIVER
	for(i=0;i<info->event.wait_count;i++)
		sem_post(&info->event.sem);
	info->event.wait_count = 0;
#endif
	pthread_mutex_unlock(&find->lock);
	if(((struct sm_req*)request)->req.msg_type == MSG_SM_INVALID_NOREPLY){
		return 1;
	}
	buf = mem_malloc(SM_BUF_SIZE);
	((struct sm_req*)buf)->req.msg_type = MSG_SM_INVALID_OK;
	((struct sm_req*)buf)->hash_id = find->hash.id;
	((struct sm_req*)buf)->name_len = strlen(find->hash.name);
	memcpy(buf+sizeof(struct sm_req), find->hash.name, ((struct sm_req*)buf)->name_len);
	sendTo(((struct sm_req*)request)->req.src_node, buf, sizeof(struct sm_req)+((struct sm_req*)buf)->name_len);
	mem_free(buf);
	return 1;
}


int sm_multiread_reply(void *request){
#ifdef	CONF_SM_EVENT_DRIVER
	int i;
#endif
	unsigned int src_node;
	void *buf;
	struct request_header reply;
	struct sm_header *find;
	struct sm_info *info;
	*(char*)(request+sizeof(struct sm_multi_req)+((struct sm_multi_req*)request)->name_len)=0;
	src_node = ((struct sm_multi_req*)request)->req.src_node;
	find = hashTableSearch(g_group.sm_table, ((struct sm_multi_req*)request)->hash_id, (request+sizeof(struct sm_multi_req))); 
	if(find == NULL || find->info == NULL){
		reply.msg_type = MSG_SM_FAILED;
		reply.seq_number = ((struct sm_multi_req*)request)->req.src_seq_number;
		sendTo(src_node, (void*)&reply, sizeof(struct request_header));
		return -1;
	}
	info = find->info;
	pthread_mutex_lock(&find->lock);
	if(find->home_node == g_group.node_id){
		if(info->state == INVALID && info->last_node.id != src_node){
			buf = mem_malloc(sizeof(struct sm_multi_req)+strlen(find->hash.name)+find->size);
			((struct sm_req*)buf)->req.msg_type = MSG_SM_FETCH;
			((struct sm_req*)buf)->hash_id = find->hash.id;
			((struct sm_req*)buf)->name_len = strlen(find->hash.name);
			memcpy(buf+sizeof(struct sm_req), find->hash.name, ((struct sm_req*)buf)->name_len);
			//mabye deadlock
			sendRecv(info->last_node.id , buf, sizeof(struct sm_req)+((struct sm_req*)buf)->name_len,
					buf, sizeof(struct request_header)+find->size);
			memcpy((void*)info+sizeof(struct sm_info), buf+sizeof(struct request_header), find->size);
			info->state=SHARED;
#ifdef CONF_SM_EVENT_DRIVER
			for(i=0;i<info->event.wait_count;i++)
				sem_post(&info->event.sem);
			info->event.wait_count = 0;
#endif
		}
		else{
			buf = mem_malloc(sizeof(struct request_header)+((struct sm_multi_req*)request)->length);
		}
		memcpy(buf+sizeof(struct request_header), 
				(void*)info+sizeof(struct sm_info)+((struct sm_multi_req*)request)->offset,
				((struct sm_multi_req*)request)->length);			
		pthread_mutex_unlock(&find->lock);
		((struct request_header*)buf)->msg_type = MSG_SM_OK;
		((struct request_header*)buf)->seq_number = ((struct sm_multi_req*)request)->req.src_seq_number;
		sendTo(src_node, buf, sizeof(struct request_header)+((struct sm_multi_req*)request)->length);
	}
	else{
		pthread_mutex_unlock(&find->lock);
		forward(find->home_node, request);
	}
	if(buf != NULL)
		mem_free(buf);
	return 1;
}


//only home node
int sm_multiwrite_reply(void *request){
	int i;
	unsigned int src_node;
	unsigned int *node;
	void *buf=NULL;
	struct sm_header *find;
	struct sm_info *info;
	*(char*)(request+sizeof(struct sm_multi_req)+((struct sm_multi_req*)request)->name_len)=0;
	src_node = ((struct sm_multi_req*)request)->req.src_node;
	find = hashTableSearch(g_group.sm_table, ((struct sm_multi_req*)request)->hash_id, (request+sizeof(struct sm_multi_req))); 
	if(find == NULL || find->info == NULL){
		return -1;
	}
	info = find->info;
	pthread_mutex_lock(&find->lock);
	if(find->home_node == g_group.node_id){
		memcpy((void*)info+sizeof(struct sm_info)+((struct sm_multi_req*)request)->offset,
				request+sizeof(struct sm_multi_req)+((struct sm_multi_req*)request)->name_len+1 ,
				((struct sm_multi_req*)request)->length);
#ifdef CONF_SM_EVENT_DRIVER
		for(i=0;i<info->event.wait_count;i++)
			sem_post(&info->event.sem);
		info->event.wait_count = 0;
#endif
		//send invalid
		info->last_node.id = find->home_node;
		info->sync = 0;
		if(info->need->use > 0){
			buf = mem_malloc(SM_BUF_SIZE);
			((struct sm_req*)buf)->req.msg_type = MSG_SM_INVALID_NOREPLY;
			((struct sm_req*)buf)->hash_id = find->hash.id;
			((struct sm_req*)buf)->name_len = strlen(find->hash.name);
			memcpy(buf+sizeof(struct sm_req), find->hash.name, ((struct sm_req*)buf)->name_len);
			for(i=0; i<info->need->use; i++){
				node = info->need->row[i];
				if(*node != src_node && *node != g_group.node_id)
					sendTo(*node, buf, sizeof(struct sm_req)+((struct sm_req*)buf)->name_len);
				*node = -1;
			}
			mem_free(buf);
		}
		info->need->index = 0;
		info->need->use = 0;	
		info->state = SHARED;
		pthread_mutex_unlock(&find->lock);
	}
	else{
		pthread_mutex_unlock(&find->lock);
		forward(find->home_node, request);
	}
	return 1;
}



/*
	diff content : offset, length, data
*/
struct diff* createDiff(void* orig, void* twin, unsigned int size){
	char find;
	unsigned int i;
	unsigned int max_size;
	unsigned int diff_offset = 0;
	struct diff* diff;
	struct diff_header diff_header;

	diff_header.offset = 0;
	diff_header.length = 0;
	max_size = size*DIFF_SIZE_RATE;
	diff = mem_malloc(sizeof(struct diff));
	diff->data = mem_malloc(max_size);
	find = NO;
	for(i=0;i<size;i++){
		
//		if(memcmp(orig+i, twin+i, 1)!=0 && find == NO){
		if(*((char*)orig+i)!=*((char*)twin+i) && find == NO){
			find = YES;
			diff_header.offset = i;
			diff_header.length = 1;	
		}
//		else if(memcmp(orig+i, twin+i, 1)!=0 && find == YES){
		else if(*((char*)orig+i)!=*((char*)twin+i) && find == YES){
			diff_header.length++;
		}
//		else if(memcmp(orig+i, twin+i, 1)!=0 && find == YES){
		else if(*((char*)orig+i)!=*((char*)twin+i) && find == YES){
			if((diff_offset+sizeof(struct diff_header)+diff_header.length) > max_size)
				return NULL;
			memcpy(diff->data+diff_offset+sizeof(struct diff_header), orig+diff_header.offset, diff_header.length);
			memcpy(diff->data+diff_offset, (void*)&diff_header, sizeof(struct diff_header));
			diff_offset += sizeof(struct diff_header)+diff_header.length;
			diff_header.length = 0;
			find = NO;
		}
	}
	if(find == YES){
		if((diff_offset+sizeof(struct diff_header)+diff_header.length) > max_size)
			return NULL;
		memcpy(diff->data+diff_offset+sizeof(struct diff_header), orig+diff_header.offset, diff_header.length);
		memcpy(diff->data+diff_offset, (void*)&diff_header, sizeof(struct diff_header));
		diff_offset += sizeof(struct diff_header)+diff_header.length;
	}
	diff->size = diff_offset;
	return diff;
}

#ifdef CONF_SM_DISSEMINATE_UPDATE
int sm_update_register_reply(void *request){
	int index;
	unsigned int src_node;
	struct sm_header *find;
	struct sm_info *info;
	*(char*)(request+sizeof(struct sm_req)+((struct sm_req*)request)->name_len)=0;
	src_node = ((struct sm_req*)request)->req.src_node;
	find = hashTableSearch(g_group.sm_table, ((struct sm_req*)request)->hash_id, (request+sizeof(struct sm_req))); 
	if(find == NULL || find->info == NULL){
		return -1;
	}
	info = find->info;
	pthread_mutex_lock(&find->lock);
	if(find->home_node == g_group.node_id){
		if(searchNode(info->disseminate, src_node)==-1){
			index = tableGetEmpty(info->disseminate);
			if(index == -1){
				pthread_mutex_unlock(&find->lock);
				return -1;
			}
			tableAdd(info->disseminate, (void*)&src_node, index);
		}
		pthread_mutex_unlock(&find->lock);
	}	
	else{
		pthread_mutex_unlock(&find->lock);
		forward(find->home_node, request);
	}
	return 1;		
		
}
int sm_update_unregister_reply(void *request){
	int index;
	unsigned int src_node;
	struct sm_header *find;
	struct sm_info *info;
	*(char*)(request+sizeof(struct sm_req)+((struct sm_req*)request)->name_len)=0;
	src_node = ((struct sm_req*)request)->req.src_node;
	find = hashTableSearch(g_group.sm_table, ((struct sm_req*)request)->hash_id, (request+sizeof(struct sm_req))); 
	if(find == NULL || find->info == NULL){
		return -1;
	}
	info = find->info;
	pthread_mutex_lock(&find->lock);
	if(find->home_node == g_group.node_id){
		index = searchNode(info->disseminate, src_node);
		if(index >= 0)
			tableRemove(info->disseminate , index);
		pthread_mutex_unlock(&find->lock);
	}	
	else{
		pthread_mutex_unlock(&find->lock);
		forward(find->home_node, request);
	}
	return 1;		
		
}

int sm_update_reply(void *request){
#ifdef	CONF_SM_EVENT_DRIVER
	int i;
#endif
	struct sm_header *find;
	struct sm_info *info;
	*(char*)(request+sizeof(struct sm_req)+((struct sm_req*)request)->name_len)=0;
	find = hashTableSearch(g_group.sm_table, ((struct sm_req*)request)->hash_id, (request+sizeof(struct sm_req)));
	if(find == NULL || find->info == NULL){
		return -1;
	}
	info = find->info;
	pthread_mutex_lock(&find->lock);
	info->last_node.id = ((struct sm_req*)request)->req.src_node;
	info->state = SHARED;
	memcpy((void*)info+sizeof(struct sm_info),
				request+sizeof(struct sm_req)+((struct sm_req*)request)->name_len+1 ,
				find->size);
#ifdef CONF_SM_EVENT_DRIVER
	for(i=0;i<info->event.wait_count;i++)
		sem_post(&info->event.sem);
	info->event.wait_count = 0;
#endif
	pthread_mutex_unlock(&find->lock);
	return 1;		
}
#endif

int sm_sethome_reply(void *request){
	int i;
	void *buf;
	unsigned int offset;
	struct sm_header *find;
	struct sm_info *info;
	struct request_header reply;
	*(char*)(request+sizeof(struct sm_req)+((struct sm_req*)request)->name_len)=0;
	find = hashTableSearch(g_group.sm_table, ((struct sm_req*)request)->hash_id, (request+sizeof(struct sm_req)));
	if(find == NULL || find->info == NULL){
		reply.msg_type = MSG_SM_FAILED;
		reply.seq_number = ((struct sm_req*)request)->req.src_seq_number;
		sendTo(((struct sm_req*)request)->req.src_node, (void*)&reply, sizeof(struct request_header));
		return -1;
	}
	info = find->info;
	buf = mem_malloc(SM_IHOME_SIZE);
	((struct sm_ihome_reply*)buf)->req.msg_type = MSG_SM_IAMHOME_REPLY;
	((struct sm_ihome_reply*)buf)->req.seq_number = ((struct sm_req*)request)->req.src_seq_number;
	pthread_mutex_lock(&find->lock);
	if(find->home_node == g_group.node_id){
//		((struct sm_ihome_reply*)buf)->count = find->count;
		((struct sm_ihome_reply*)buf)->last_node = info->last_node.id;
		((struct sm_ihome_reply*)buf)->users_count = find->users->use;
		((struct sm_ihome_reply*)buf)->need_count = info->need->use;
		offset = sizeof(struct sm_ihome_reply);		
		for(i=0;i<find->users->use;i++){
			memcpy(buf+offset, find->users->row[i], sizeof(unsigned int));
			offset += sizeof(unsigned int);
		}
		for(i=0;i<info->need->use;i++){
			memcpy(buf+offset, info->need->row[i], sizeof(unsigned int));
			offset += sizeof(unsigned int);
		}
#ifdef CONF_SM_DISSEMINATE_UPDATE
		((struct sm_ihome_reply*)buf)->disseminate_count = info->disseminate->use;
		for(i=0;i<info->disseminate->use;i++){
			memcpy(buf+offset, info->disseminate->row[i], sizeof(unsigned int));
			offset += sizeof(unsigned int);
		}
#endif
		sendTo(((struct sm_req*)request)->req.src_node, buf, offset);
		pthread_mutex_unlock(&find->lock);	
	}
	else{
		pthread_mutex_unlock(&find->lock);
		forward(find->home_node, request);
	}
	mem_free(buf);
	return 1;
}

int sm_newhome_reply(void *request){	
	void *buf;
	struct sm_header *find;
	struct sm_info *info;
	*(char*)(request+sizeof(struct sm_newhome_req)+((struct sm_newhome_req*)request)->name_len)=0;
	find = hashTableSearch(g_group.sm_table, ((struct sm_newhome_req*)request)->hash_id, (request+sizeof(struct sm_newhome_req)));
	if(find == NULL || find->info == NULL){
		return -1;
	}
	info = find->info;
	pthread_mutex_lock(&find->lock);
	find->home_node = ((struct sm_newhome_req*)buf)->new_home;
	pthread_mutex_unlock(&find->lock);
	return 1;
}

int sm_homeready_reply(void *request){
	int i;
	void *buf;
	unsigned int *node;
	struct sm_header *find;
	struct sm_info *info;
	*(char*)(request+sizeof(struct sm_req)+((struct sm_req*)request)->name_len)=0;
	find = hashTableSearch(g_group.sm_table, ((struct sm_req*)request)->hash_id, (request+sizeof(struct sm_req)));
	if(find == NULL || find->info == NULL){
		return -1;
	}
	info = find->info;
	pthread_mutex_lock(&find->lock);
	if(find->home_node == g_group.node_id){
		buf = mem_malloc(sizeof(struct sm_newhome_req)+SM_NAME_LEN);
		find->home_node = ((struct sm_req*)request)->req.src_node;
		((struct sm_newhome_req*)buf)->req.msg_type = MSG_SM_NEWHOME;
		((struct sm_newhome_req*)buf)->hash_id = find->hash.id;
		((struct sm_newhome_req*)buf)->name_len = strlen(find->hash.name);
		((struct sm_newhome_req*)buf)->new_home = ((struct sm_req*)request)->req.src_node;
		memcpy( buf+sizeof(struct sm_newhome_req), find->hash.name, ((struct sm_newhome_req*)buf)->name_len);
		for(i=0;i<find->users->use;i++){
			node = find->users->row[i];
			if(*node != g_group.node_id && *node != ((struct sm_req*)request)->req.src_node)
				sendTo(*node, buf, sizeof(struct sm_newhome_req)+((struct sm_newhome_req*)buf)->name_len);	
		}
		pthread_mutex_unlock(&find->lock);
		mem_free(buf);
	}
	else{		
		pthread_mutex_unlock(&find->lock);
		forward(find->home_node, request);
	}
	return 1;	
}

int sm_sethome_req(struct sm_info *sm_info){
	int i;
	void *buf;
	void *recv_buf;
	unsigned int offset;
	struct sm_header *sm_header;	
	sm_header = sm_info->header;
	if(sm_header->home_node == g_group.node_id)
		return 1;
	buf = mem_malloc(SM_IHOME_SIZE);
	recv_buf = mem_malloc(SM_IHOME_SIZE);
	((struct sm_req*)buf)->req.msg_type = MSG_SM_IAMHOME;
	((struct sm_req*)buf)->hash_id = sm_header->hash.id;
	((struct sm_req*)buf)->name_len = strlen(sm_header->hash.name);
	memcpy( buf+sizeof(struct sm_req), sm_header->hash.name, ((struct sm_req*)buf)->name_len);
	sendRecv(sm_header->home_node, buf ,sizeof(struct sm_req)+((struct sm_req*)buf)->name_len,
				recv_buf, SM_IHOME_SIZE);
	pthread_mutex_lock(&sm_header->lock);	
	sm_info->last_node.id = ((struct sm_ihome_reply*)recv_buf)->last_node;
	//copy users
	offset = sizeof(struct sm_ihome_reply);	
	if(	sm_header->users == NULL){
		sm_header->users = tableCreate(g_group.node_table->table_size, sizeof(unsigned int));
	}
	for(i=0;i<((struct sm_ihome_reply*)recv_buf)->users_count; i++){
		tableAdd(sm_header->users, recv_buf+offset, i);
		offset += sizeof(unsigned int);
	}
	sm_header->users->index = ((struct sm_ihome_reply*)recv_buf)->users_count;
	//copy need
	if(	sm_info->need == NULL){
		sm_info->need = tableCreate(g_group.node_table->table_size , sizeof(unsigned int));
	}
	for(i=0;i<((struct sm_ihome_reply*)recv_buf)->need_count;i++){
		tableAdd(sm_info->need, recv_buf+offset, i);
		offset += sizeof(unsigned int);
	}
	sm_info->need->index = ((struct sm_ihome_reply*)recv_buf)->need_count;
	//copy disseminate
#ifdef CONF_SM_DISSEMINATE_UPDATE
	if(	sm_info->disseminate == NULL){
		sm_info->disseminate = tableCreate(g_group.node_table->table_size , sizeof(unsigned int));
	}
	for(i=0;i<((struct sm_ihome_reply*)recv_buf)->disseminate_count;i++){
		tableAdd(sm_info->disseminate, recv_buf+offset, i);
		offset += sizeof(unsigned int);
	}
	sm_info->disseminate->index = ((struct sm_ihome_reply*)recv_buf)->disseminate_count;
#endif
	//init ok then send ready to the old home
	((struct sm_req*)buf)->req.msg_type = MSG_SM_HOME_READY;
	sendTo(sm_header->home_node, buf ,sizeof(struct sm_req)+((struct sm_req*)buf)->name_len);
	sm_header->home_node = g_group.node_id;
#ifdef CONF_SM_AUTO_HOME_MIGARATION
	sm_info->scoreboard = tableCreate(MAX_NODE_NUM, sizeof(int));
	sm_score_zero(sm_info);
#endif
	pthread_mutex_unlock(&sm_header->lock);
	mem_free(buf);
	mem_free(recv_buf);
	return 1;
}


#ifdef CONF_SM_AUTO_HOME_MIGARATION
	int sm_uhome_reply(void *request){
		struct sm_header *find;
		struct sm_info *info;
		*(char*)(request+sizeof(struct sm_req)+((struct sm_req*)request)->name_len)=0;
		find = hashTableSearch(g_group.sm_table, ((struct sm_req*)request)->hash_id, (request+sizeof(struct sm_req)));
		if(find == NULL || find->info == NULL){
			return -1;
		}
		info = find->info;
		if(find->home_node != g_group.node_id)
			sm_sethome_req(info);
		return 1;
	}
	int sm_uhome_req(struct sm_info *sm_info, unsigned int send_node){
		void *buf;
		struct sm_header *sm_header;
//		struct sm_req 
		sm_header = sm_info->header;
		buf = mem_malloc(SM_BUF_SIZE);
		((struct sm_req*)buf)->req.msg_type = MSG_SM_UHOME;
		((struct sm_req*)buf)->hash_id = sm_header->hash.id;
		((struct sm_req*)buf)->name_len = strlen(sm_header->hash.name);
		memcpy( buf+sizeof(struct sm_req), sm_header->hash.name, ((struct sm_req*)buf)->name_len);
		sendTo(send_node , buf , sizeof(struct sm_req)+((struct sm_req*)buf)->name_len);			
		mem_free(buf);
		return 1;
	}
	int sm_score_add(struct sm_info *sm_info, unsigned int node_id, int addscore){
		int *score;
		score = sm_info->scoreboard->row[node_id];
		*score += addscore;
		return *score;
	}
	int sm_score_zero(struct sm_info *sm_info){
		int i;
		int *score;
		for(i=0;i<sm_info->scoreboard->table_size;i++){
			score = sm_info->scoreboard->row[i];
			*score = 0;
		}
		return 1;
	}
#endif

