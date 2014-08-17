/*
# Description:
#   d2mce.c
#
# Copyright (C) 2006- by EPS(Embedded and Parallel Systems Lab) @ NTUT CSIE
#
# Date: $Date: 2007/12/24 02:44:39 $
# Version: $Revision: 1.6 $
#
# History:
#
# $Log: d2mce.c,v $
# Revision 1.6  2007/12/24 02:44:39  ffbli
# dos2unix
#
# Revision 1.5  2007/12/20 13:08:14  ffbli
# add log
#
#
*/
#include "d2mce.h"

int d2mce_init()
{
	bzero((void*)&g_group, sizeof(g_group));
	if(read_config()<0){
		printf("Error: read_config()\n");
		return -1;
	}
	mem_init();
	network_init();
	sync_init();
	thread_init();
	connectSelf();
	return 1;
}

int d2mce_finalize()
{
	int sockfd;	
	//unsigned short msg;
	struct finalize_s2d_req final_req;
	struct sockaddr_in server_addr;
	
	//send to daemon to exit
	if(g_group.coordinator.main_id == g_group.node_id && g_group.group_id !=0){
  		final_req.msg_type = MSG_FINALIZE;
		final_req.group_id = g_group.group_id;
		bzero(&server_addr, sizeof(server_addr));
		sockfd = UDP_init(g_system_conf.broadcast_ip, g_system_conf.demon_port, &server_addr);
		UDP_setBroadcast(sockfd);
		UDP_setTTL(sockfd, 1);
    	sendto(sockfd, (void*) &final_req, sizeof(struct finalize_s2d_req),
            0, (struct sockaddr*) &server_addr, sizeof(server_addr));
		if(readable_timer(sockfd, 1, 0) == 0){
			printf("Error: Not send the exit msg to the d2mced\n");
		}
		else{
			recvfrom(sockfd, (void*) &final_req, sizeof(struct finalize_s2d_req), 0,NULL, NULL);
			if(final_req.msg_type == MSG_FINALIZE_OK)
				printf("daemon exit ok!!\n");
		}
	}
	//send to other node to finalize
	finalize_req();
	sync_destroy();
	thread_destroy();
	ctrl_destroy();
	network_destroy();
/*
	if(g_group.barrier_table != NULL)
		hashTableDestroy(g_group.barrier_table);
	if(g_group.mutex_table != NULL)
		hashTableDestroy(g_group.mutex_table);
	if(g_group.sm_table != NULL)
		hashTableDestroy(g_group.sm_table);
	if(g_group.sem_table != NULL)
		hashTableDestroy(g_group.sem_table);
*/
	mem_destroy();
	return 1;
}


int d2mce_join(char *app_name, char *group_name, enum d2mce_join_option option)
{
    int sockfd;
    socklen_t socklen = sizeof(struct sockaddr_in);
    ssize_t recv_len;
	void *buf = NULL;
	char *new_app_name;
	char *new_group_name;
	struct node node;
	struct node *self;
 	struct sockaddr_in server_addr, recv_addr;
	struct join_d2s_reply join_reply;
	buf = mem_malloc(JOIN_BUF_SIZE);
	/*
		app_name & group_name check over max length
	*/
	new_app_name = str_malloc(app_name, D2MCE_APP_NAME_LEN, 1);
//	strncpy(new_app_name, app_name, D2MCE_APP_NAME_LEN);
	memcpy(new_app_name, app_name, strlen(app_name));
	new_app_name[strlen(app_name)] = 0;
	memcpy(&((struct join_s2d_req*)buf)->group_instance ,g_system_conf.hwaddr, sizeof(unsigned int));
	if( option == D2MCE_GROUP_NEW){
		new_group_name = str_malloc(group_name, D2MCE_GROUP_NAME_LEN, sizeof(unsigned int)+1);
		snprintf(new_group_name, D2MCE_GROUP_NAME_LEN, "%s%d", group_name, ((struct join_s2d_req*)buf)->group_instance);
	}
	else{
		new_group_name = str_malloc(group_name, D2MCE_GROUP_NAME_LEN, 1);
//		strncpy(new_group_name, group_name, D2MCE_GROUP_NAME_LEN);
		memcpy(new_group_name, group_name, strlen(group_name));
		new_group_name[strlen(group_name)]=0;
	}
	g_group.app_name = new_app_name;
	g_group.group_name = new_group_name;

	/* 
		packet join_request header 
	*/
    ((struct join_s2d_req*)buf)->msg_type = MSG_JOIN_REQ;
	((struct join_s2d_req*)buf)->app_len = strlen(new_app_name);
	((struct join_s2d_req*)buf)->group_len = strlen(new_group_name);
    ((struct join_s2d_req*)buf)->port = g_system_conf.port;
	memcpy( buf+sizeof(struct join_s2d_req), new_app_name, ((struct join_s2d_req*)buf)->app_len);
	memcpy( buf+sizeof(struct join_s2d_req)+strlen(new_app_name), new_group_name, ((struct join_s2d_req*)buf)->group_len);   
	/*
		UDP broadcast to the daemon
	*/
    bzero(&recv_addr,sizeof(recv_addr));
    bzero(&server_addr, sizeof(server_addr));
	sockfd = UDP_init(g_system_conf.broadcast_ip, g_system_conf.demon_port, &server_addr);
	UDP_setBroadcast(sockfd);
	UDP_setTTL(sockfd, 1);
    sendto(sockfd, buf, sizeof(struct join_s2d_req)+((struct join_s2d_req*)buf)->app_len+((struct join_s2d_req*)buf)->group_len,
            0, (struct sockaddr*) &server_addr, sizeof(server_addr));
	mem_free(buf);
	/*
		wait join_reply and do the join_reply process
	*/
	if(readable_timer(sockfd, 1, 0) == 0){
		printf("Error: Not find the d2mced, please run the d2mced\n");
		close(sockfd);
		exit(-1);
	}
	bzero(&join_reply, sizeof(struct join_d2s_reply));
    recv_len = recvfrom(sockfd, (void*) &join_reply, sizeof(struct join_d2s_reply), 0, (struct sockaddr *) &recv_addr, &socklen);
	close(sockfd);
	g_group.sm_table = hashTableCreate(SM_HASH_SIZE);
	//first join, I am coordinator.
	if(join_reply.msg_type == MSG_JOIN_OK){
	 	node.id = 0;
		node.ip = g_system_conf.ip;
		node.port = g_system_conf.port;
		self = (struct node*)tableGetRow(g_group.node_table, MAX_NODE_NUM-1);
		node.recv_fd = self->recv_fd;
		node.send_fd = self->send_fd;
		self->id = 0;
		self->recv_fd = 0;
		self->send_fd = 0;
		tableAdd( g_group.node_table, (void*)&node, 0);
		g_group.node_table->index = 1;
		g_group.node_table->use = 1;
		
		g_group.group_id = join_reply.group_id;
		g_group.node_id = 0;
		g_group.node_num = 1;
		g_group.coordinator.main_id = 0;
		g_group.coordinator.mutex_id = 0;
		g_group.coordinator.sem_id = 0;
		g_group.coordinator.barrier_id = 0;
		g_group.barrier_table = hashTableCreate(BARRIER_HASH_SIZE);
		g_group.mutex_table = hashTableCreate(MUTEX_HASH_SIZE);
		g_group.sem_table = hashTableCreate(SEM_HASH_SIZE);
		return 0;		
	}
	//not coordinator, so send join request to the coordinator
	else if(join_reply.msg_type == MSG_JOIN_REPLY && recv_len == sizeof(struct join_d2s_reply)) {
//		printf("%d %d %d \n", join_reply.group_id,join_reply.ip, join_reply.port);
		return join_grant_req(join_reply.group_id, join_reply.id, join_reply.ip, join_reply.port);
	}
	return -1;
}

int d2mce_exit()
{
	exit_manager_req();
 	return 1;
}

int d2mce_probe(char *app_name, char *group_name, d2mce_ginfo_t **group_list, int group_num)
{
	int i;
	int count = 0;
	int sockfd;
	socklen_t socklen = sizeof(struct sockaddr_in);
	ssize_t recv_len;
	void *buf;
	struct sockaddr_in server_addr, recv_addr;
	struct probe_d2s_reply probe_reply;
	
	for(i=0; i<group_num; i++)
		group_list[i] = malloc(sizeof(d2mce_ginfo_t));

	/*
		app_name & group_name check over max length
	*/
	buf = mem_malloc(JOIN_PROBE_SIZE);
	//probe_req header
	((struct probe_s2d_req*)buf)->msg_type = MSG_PROBE_REQ;
	if(app_name !=NULL){
		((struct probe_s2d_req*)buf)->app_len = strlen(app_name);
		memcpy( buf+sizeof(struct probe_s2d_req), app_name, ((struct probe_s2d_req*)buf)->app_len);
	}
	else
		((struct probe_s2d_req*)buf)->app_len = 0;
	if(group_name !=NULL){
		((struct probe_s2d_req*)buf)->group_len = strlen(group_name);
		memcpy( buf+sizeof(struct probe_s2d_req)+((struct probe_s2d_req*)buf)->app_len, group_name
				,((struct probe_s2d_req*)buf)->group_len);
	}
	else
		((struct probe_s2d_req*)buf)->group_len = 0;
	
	((struct probe_s2d_req*)buf)->group_num = group_num;

	bzero(&server_addr, sizeof(server_addr));
	bzero(&recv_addr,sizeof(recv_addr));
	sockfd = UDP_init(g_system_conf.broadcast_ip, g_system_conf.demon_port, &server_addr);
	UDP_setBroadcast(sockfd);
	UDP_setTTL(sockfd, 1);
	sendto(sockfd, buf, sizeof(struct probe_s2d_req)+((struct probe_s2d_req*)buf)->app_len+((struct probe_s2d_req*)buf)->group_len,
			0, (struct sockaddr*) &server_addr, sizeof(server_addr));
	mem_free(buf);
	for(i=0;i<group_num;i++){
		if(readable_timer(sockfd, 0, 300) == 0)
			continue;
		recv_len = recvfrom(sockfd, (void*)&probe_reply , sizeof(struct probe_d2s_reply), 0, (struct sockaddr *) &recv_addr, &socklen);
		if(probe_reply.msg_type == MSG_PROBE_REPLY && recv_len == sizeof(struct probe_d2s_reply)){
			group_list[count]->app_name = malloc(strlen(probe_reply.app_name));
			group_list[count]->group_name = malloc(strlen(probe_reply.group_name));
//			strncpy(group_list[count]->app_name, probe_reply.app_name, D2MCE_APP_NAME_LEN);
			memcpy(group_list[count]->app_name, probe_reply.app_name, strlen(probe_reply.app_name));
//			strncpy(group_list[count]->group_name, probe_reply.group_name, D2MCE_APP_NAME_LEN);
			memcpy(group_list[count]->group_name, probe_reply.group_name, strlen(probe_reply.group_name));
			group_list[count]->group_instance = probe_reply.group_instance;
			group_list[count]->ip = probe_reply.ip;
			group_list[count]->port = probe_reply.port;
			group_list[count]->id = probe_reply.id;
			count++;
		}
	}
	close(sockfd);
	return count;
}

int d2mce_join_gid(d2mce_ginfo_t *ginfo)
{
	char *new_app_name;
	char *new_group_name;
	/*
		app_name & group_name check over max length
	*/
	new_app_name = str_malloc(ginfo->app_name, D2MCE_APP_NAME_LEN, 1);
//	strncpy(new_app_name, ginfo->app_name, D2MCE_APP_NAME_LEN);
	memcpy(new_app_name, ginfo->app_name, strlen(ginfo->app_name));
	new_app_name[strlen(ginfo->app_name)] = 0;
	new_group_name = str_malloc(ginfo->group_name, D2MCE_GROUP_NAME_LEN, 1);
//	strncpy(new_group_name, ginfo->group_name, D2MCE_GROUP_NAME_LEN);
	memcpy(new_group_name, ginfo->group_name, strlen(ginfo->group_name));
	new_group_name[strlen(ginfo->group_name)] = 0;
	g_group.app_name = new_app_name;
	g_group.group_name = new_group_name;

	return join_grant_req(ginfo->group_instance, ginfo->id , ginfo->ip, ginfo->port);

}



void* d2mce_malloc(char* sm_name, size_t size)
{
	char *new_sm_name;
	void *buf;
	unsigned int hash_id;
	struct sm_header *sm_header;
	struct sm_info *new_sm;
	new_sm_name = str_malloc(sm_name, SM_NAME_LEN, 1);
//	strncpy(new_sm_name, sm_name, SM_NAME_LEN);
	memcpy(new_sm_name, sm_name, strlen(sm_name));
	new_sm_name[strlen(sm_name)]=0;
	hash_id = hash_str(new_sm_name)%SM_HASH_SIZE;
		
	//coordinator
	if(g_group.node_id == g_group.coordinator.mutex_id){	
		sm_header = hashTableSearch(g_group.sm_table, hash_id, new_sm_name);		
		//have register
		if(sm_header != NULL){
			if(sm_header->size != size)
				return NULL;
			//have ohter node to register & home node not me
			if(sm_header->info == NULL){
				new_sm = mallocSM(size, INVALID, NO);				
				if(new_sm == NULL)
					return NULL;
				new_sm->header = sm_header;
				sm_header->info = new_sm;
			}
			return (new_sm+sizeof(struct sm_info));
		}
		//don't register & i am home node
		sm_header = createSM(hash_id, new_sm_name, g_group.node_id, size);
		hashTableInsert(g_group.sm_table, (struct hashheader*)sm_header);
		if(sm_header == NULL)
			return NULL;
		new_sm = mallocSM(size, SHARED, YES);
		if(new_sm == NULL)
			return NULL;
		new_sm->header = sm_header;
		sm_header->info = new_sm;
		return ((void*)new_sm+sizeof(struct sm_info));				
	}
	//not coordinator
	else{
		sm_header = hashTableSearch(g_group.sm_table, hash_id, new_sm_name);		
		//have register
		if(sm_header != NULL){
			if(sm_header->size != size)
				return NULL;
			new_sm = sm_header->info;
			return (new_sm+sizeof(struct sm_info));
		}
		buf = mem_malloc(SM_INIT_SIZE);
		((struct sm_init_req*)buf)->req.msg_type = MSG_SM_INIT;
		((struct sm_init_req*)buf)->hash_id = hash_id;
		((struct sm_init_req*)buf)->name_len = strlen(new_sm_name);
		((struct sm_init_req*)buf)->size = size;
		memcpy( buf+sizeof(struct sm_init_req), new_sm_name, ((struct sm_init_req*)buf)->name_len);
		sendRecv(g_group.coordinator.main_id ,
			buf, sizeof(struct sm_init_req)+((struct sm_init_req*)buf)->name_len,
			buf, sizeof(struct request_header));
		if(((struct sm_init_reply*)buf)->req.msg_type == MSG_SM_FAILED)
			return NULL;
		//i am first register & i am home node
		if(((struct sm_init_reply*)buf)->req.msg_type == MSG_SM_OK){			
			sm_header = createSM(hash_id, new_sm_name, g_group.node_id, size);
			hashTableInsert(g_group.sm_table, (struct hashheader*)sm_header);
			if(sm_header == NULL)
				return NULL;
			new_sm = mallocSM(size, SHARED, YES);
			if(new_sm == NULL)
				return NULL;
			new_sm->header = sm_header;
			sm_header->info = new_sm;
		}
		//have register & home node not me
		else{
			sm_header = createSM(hash_id, new_sm_name,((struct sm_init_reply*)buf)->home_node, size);
			hashTableInsert(g_group.sm_table, (struct hashheader*)sm_header);
			if(sm_header == NULL)
				return NULL;
			new_sm = mallocSM(size, INVALID, NO);
			if(new_sm == NULL)
				return NULL;
			new_sm->header = sm_header;
			sm_header->info = new_sm;
		}
		mem_free(buf);
	}
	return ((void*)new_sm+sizeof(struct sm_info));

}

int d2mce_set_home(void *share_memory){
	struct sm_info *sm_info;
	if(share_memory==NULL)
		return -1;
	sm_info = share_memory - sizeof(struct sm_info);
	if(sm_info->header==NULL)
		return -1;
	return sm_sethome_req(sm_info);
}


int d2mce_load(void *share_memory)
{
#ifdef CONF_SM_AUTO_HOME_MIGARATION
	int score;
#endif
	void *buf = NULL;
	struct sm_info *sm_info;
	struct sm_header *sm_header;	
	if(share_memory==NULL)
		return -1;
	sm_info = share_memory - sizeof(struct sm_info);
	if(sm_info->header==NULL)
		return -1;
	sm_header = sm_info->header;
	pthread_mutex_lock(&sm_header->lock);
	if(sm_info->state == INVALID){
		sm_info->state = SHARED;
		buf = mem_malloc(sizeof(struct sm_req)+strlen(sm_header->hash.name)+sm_header->size);
//		buf = malloc(sizeof(struct sm_req)+strlen(sm_header->hash.name)+sm_header->size);
		((struct sm_req*)buf)->hash_id = sm_header->hash.id;
		((struct sm_req*)buf)->name_len = strlen(sm_header->hash.name);
		memcpy( buf+sizeof(struct sm_req), sm_header->hash.name, ((struct sm_req*)buf)->name_len);
		//home node
		if(sm_header->home_node == g_group.node_id){
			((struct sm_req*)buf)->req.msg_type = MSG_SM_FETCH;
			sendRecv(sm_info->last_node.id ,
				buf, sizeof(struct sm_req)+((struct sm_req*)buf)->name_len,
				buf, sizeof(struct request_header)+sm_header->size);
			memcpy(share_memory , buf+sizeof(struct request_header), sm_header->size);
#ifdef CONF_SM_AUTO_HOME_MIGARATION
			score = sm_score_add(sm_info, g_group.node_id, 1);
			if(score >=SM_HOME_MIGRATION_THRESHOLD)
				sm_score_zero(sm_info);
#endif
		}
		//not home node
		else{
			((struct sm_req*)buf)->req.msg_type = MSG_SM_READMISS;
			sendRecv(sm_header->home_node ,
				buf, sizeof(struct sm_req)+((struct sm_req*)buf)->name_len,
				buf, sizeof(struct request_header)+sm_header->size);
			memcpy(share_memory , buf+sizeof(struct request_header), sm_header->size);
			//node have move
			//if(sm_header->home_node != ((struct request_header*)buf)->src_node)
			//	sm_header->home_node = ((struct request_header*)buf)->src_node;
		}
	}
	pthread_mutex_unlock(&sm_header->lock);	
	if(buf != NULL){
		mem_free(buf);
//		free(buf);
	}
 	return 1;
}


int d2mce_store(void *share_memory)
{
#ifdef CONF_SM_AUTO_HOME_MIGARATION
	int score;
#endif
	int i;
	int *node;
	int seq_number;
	void *recv_buf;
	void *buf = NULL;
	unsigned int send_size;
	sem_t *sem;
	struct sm_info *sm_info;
	struct sm_header *sm_header;	
	if(share_memory==NULL)
		return -1;
	sm_info = share_memory - sizeof(struct sm_info);
	if(sm_info->header==NULL)
		return -1;
	sm_header = sm_info->header;
	pthread_mutex_lock(&sm_header->lock);
#ifdef CONF_SM_EVENT_DRIVER
	for(i=0;i<sm_info->event.wait_count;i++)
		sem_post(&sm_info->event.sem);
	sm_info->event.wait_count = 0;
#endif	
#ifdef CONF_SM_DISSEMINATE_UPDATE			
	if(sm_header->home_node == g_group.node_id){
		//send update
		if(sm_info->disseminate->use > 0){
			send_size = sizeof(struct sm_req)+strlen(sm_header->hash.name)+1+sm_header->size;
			buf = mem_malloc(sizeof(struct sm_req)+strlen(sm_header->hash.name)+1+sm_header->size);
			((struct sm_req*)buf)->req.msg_type = MSG_SM_UPDATE;
			((struct sm_req*)buf)->hash_id = sm_header->hash.id;
			((struct sm_req*)buf)->name_len = strlen(sm_header->hash.name);
			memcpy(buf+sizeof(struct sm_req), sm_header->hash.name, ((struct sm_req*)buf)->name_len);
			memcpy(buf+sizeof(struct sm_req)+((struct sm_req*)buf)->name_len+1, share_memory, sm_header->size);
			for(i=0;i<sm_info->disseminate->use;i++){
				node = sm_info->disseminate->row[i];
				if(*node != g_group.node_id)
					sendTo(*node, buf , send_size);
			}
		}
#ifdef CONF_SM_AUTO_HOME_MIGARATION
		score = sm_score_add(sm_info, g_group.node_id, 2);
		if(score >=SM_HOME_MIGRATION_THRESHOLD)
			sm_score_zero(sm_info);
#endif
	}
#endif	
	if(sm_info->state == SHARED || sm_info->state == INVALID){
		sm_info->state = EXCLUSIVE;
		sm_info->last_node.id = g_group.node_id;						
		//home node
		if(sm_header->home_node == g_group.node_id){
			//send invalid
			//home node and no node need to send invalid
			if(sm_info->need->use > 0){
				if(buf == NULL){
					buf = mem_malloc(SM_BUF_SIZE);
				}
				((struct sm_req*)buf)->hash_id = sm_header->hash.id;
				((struct sm_req*)buf)->name_len = strlen(sm_header->hash.name);
				memcpy( buf+sizeof(struct sm_req), sm_header->hash.name, ((struct sm_req*)buf)->name_len);
				if(isAcquire()){
					((struct sm_req*)buf)->req.msg_type = MSG_SM_INVALID;			
					sem = malloc(sizeof(sem_t));
					if(sem==NULL){
						mem_free(buf);
						pthread_mutex_unlock(&sm_header->lock);
						return -1;
					}
					sem_init(sem, 0, 0);
					sm_info->last_node.sem = sem;
					addWaitSem(sem);
				}
				else
					((struct sm_req*)buf)->req.msg_type = MSG_SM_INVALID_NOREPLY;
				sm_info->sync = sm_info->need->use;
				send_size = sizeof(struct sm_req)+((struct sm_req*)buf)->name_len;
				for(i=0; i<sm_info->need->use; i++){
					node = sm_info->need->row[i];
					if(*node != g_group.node_id)
						sendTo(*node, buf, send_size);
					*node = -1;
				}
				sm_info->need->index = 0;
				sm_info->need->use = 0; 
				pthread_mutex_unlock(&sm_header->lock);
			}
		}
		//not home node
		else{
			pthread_mutex_unlock(&sm_header->lock);
			buf = mem_malloc(SM_BUF_SIZE);
			((struct sm_req*)buf)->hash_id = sm_header->hash.id;
			((struct sm_req*)buf)->name_len = strlen(sm_header->hash.name);
			memcpy( buf+sizeof(struct sm_req), sm_header->hash.name, ((struct sm_req*)buf)->name_len);
			if(isAcquire()){
				recv_buf = mem_malloc(64);
				((struct sm_req*)buf)->req.msg_type = MSG_SM_WRITEMISS;
				seq_number = Send(sm_header->home_node, buf,sizeof(struct sm_req)+((struct sm_req*)buf)->name_len,
									recv_buf, sizeof(struct request_header));
				addWait(seq_number);
			}
			else{
				((struct sm_req*)buf)->req.msg_type = MSG_SM_WRITEMISS_NOREPLY;
				sendTo(sm_header->home_node, buf ,sizeof(struct sm_req)+((struct sm_req*)buf)->name_len);
			}			
		}
	}
	pthread_mutex_unlock(&sm_header->lock);
	if(buf != NULL)
		mem_free(buf);
	return 1;
}

int d2mce_mload(void *share_memory, unsigned int offset, unsigned int length){
	void *buf = NULL;
	struct sm_info *sm_info;
	struct sm_header *sm_header;
	if(length == 0)
		return 1;
	if(share_memory==NULL)
		return -1;
	sm_info = share_memory - sizeof(struct sm_info);
	if(sm_info->header==NULL)
		return -1;
	sm_header = sm_info->header;
	if(offset <0 || length <0)
		return -1;
	if(offset+length > sm_header->size)
		return -1;
	pthread_mutex_lock(&sm_header->lock);
	if(sm_header->home_node == g_group.node_id){
		if(sm_info->state == INVALID){			
			sm_info->state = SHARED;
			buf = mem_malloc(sizeof(struct sm_req)+strlen(sm_header->hash.name)+sm_header->size);			
			((struct sm_req*)buf)->req.msg_type = MSG_SM_FETCH;
			((struct sm_req*)buf)->hash_id = sm_header->hash.id;
			((struct sm_req*)buf)->name_len = strlen(sm_header->hash.name);
			memcpy( buf+sizeof(struct sm_req), sm_header->hash.name, ((struct sm_req*)buf)->name_len);
			sendRecv(sm_info->last_node.id ,
				buf, sizeof(struct sm_req)+((struct sm_req*)buf)->name_len,
				buf, sizeof(struct request_header)+sm_header->size);
			memcpy(share_memory , buf+sizeof(struct request_header), sm_header->size);
		}
	}
	else{
		sm_info->state = SHARED;
		buf = mem_malloc(sizeof(struct sm_multi_req)+strlen(sm_header->hash.name)+length);
		((struct sm_multi_req*)buf)->req.msg_type = MSG_SM_MULTIREAD;
		((struct sm_multi_req*)buf)->hash_id = sm_header->hash.id;
		((struct sm_multi_req*)buf)->name_len = strlen(sm_header->hash.name);
		((struct sm_multi_req*)buf)->offset = offset;
		((struct sm_multi_req*)buf)->length = length;
		memcpy( buf+sizeof(struct sm_multi_req), sm_header->hash.name, ((struct sm_multi_req*)buf)->name_len);
		sendRecv(sm_header->home_node ,
			buf, sizeof(struct sm_multi_req)+((struct sm_multi_req*)buf)->name_len,
			buf, sizeof(struct request_header)+length);
		memcpy(share_memory+offset , buf+sizeof(struct request_header), length);		
//		if(sm_header->home_node != ((struct request_header*)buf)->src_node)
//			sm_header->home_node = ((struct request_header*)buf)->src_node;
	}
	pthread_mutex_unlock(&sm_header->lock);	
	if(buf !=NULL)
		mem_free(buf);
	return 1;	
}

int d2mce_mstore(void *share_memory, unsigned int offset, unsigned int length)
{
	int i;
	int *node;
	void *buf = NULL;
	unsigned int send_size;
	struct sm_info *sm_info;
	struct sm_header *sm_header;	
	if(length == 0)
		return 1;
	if(share_memory==NULL)
		return -1;
	sm_info = share_memory - sizeof(struct sm_info);
	if(sm_info->header==NULL)
		return -1;
	sm_header = sm_info->header;
	if(offset <0 || length <0)
		return -1;
	if(offset+length > sm_header->size)
		return -1;
	pthread_mutex_lock(&sm_header->lock);
#ifdef CONF_SM_EVENT_DRIVER
	for(i=0;i<sm_info->event.wait_count;i++)
		sem_post(&sm_info->event.sem);
	sm_info->event.wait_count = 0;
#endif	
	//home node only send invalid
	if(sm_header->home_node == g_group.node_id){
		buf = mem_malloc(SM_BUF_SIZE);
		sm_info->state = SHARED;
		sm_info->last_node.id =  sm_header->home_node;
		if(sm_info->need->use == 0){
			pthread_mutex_unlock(&sm_header->lock);
			mem_free(buf);
			return 1;
		}		
		((struct sm_req*)buf)->req.msg_type = MSG_SM_INVALID_NOREPLY;
		((struct sm_req*)buf)->hash_id = sm_header->hash.id;
		((struct sm_req*)buf)->name_len = strlen(sm_header->hash.name);
		memcpy( buf+sizeof(struct sm_req), sm_header->hash.name, ((struct sm_req*)buf)->name_len);
		send_size = sizeof(struct sm_req)+((struct sm_req*)buf)->name_len;
		for(i=0; i<sm_info->need->use; i++){
			node = sm_info->need->row[i];
			if(*node != g_group.node_id)
				sendTo(*node, buf, send_size);
			*node = -1;
		}
		sm_info->sync = 0;
		sm_info->need->index = 0;
		sm_info->need->use = 0; 
		pthread_mutex_unlock(&sm_header->lock);
		mem_free(buf);
		return 1;
	}
	//not home node
	send_size = sizeof(struct sm_multi_req)+strlen(sm_header->hash.name)+length+1;
	buf = mem_malloc(send_size);
	sm_info->state = INVALID;
	sm_info->last_node.id = sm_header->home_node;
	((struct sm_multi_req*)buf)->hash_id = sm_header->hash.id;
	((struct sm_multi_req*)buf)->name_len = strlen(sm_header->hash.name);
	memcpy(buf+sizeof(struct sm_multi_req), sm_header->hash.name, ((struct sm_multi_req*)buf)->name_len);
	memcpy(buf+sizeof(struct sm_multi_req)+((struct sm_multi_req*)buf)->name_len+1, share_memory+offset, length);
	pthread_mutex_unlock(&sm_header->lock);
	((struct sm_multi_req*)buf)->req.msg_type = MSG_SM_MULTIWRITE;	
	((struct sm_multi_req*)buf)->offset = offset;
	((struct sm_multi_req*)buf)->length = length;
	sendTo(sm_header->home_node, buf , send_size);
//	sendrecv
	if(buf!=NULL)
		mem_free(buf);
	return 1;
}

int d2mce_mutex_init(d2mce_mutex_t *mutex, char *mutex_name){
	char *new_mutex_name;
	void *buf;
	struct mutex_lock *new_mutex;
	new_mutex_name = str_malloc(mutex_name, MUTEX_NAME_LEN, 1);
//	strncpy(new_mutex_name, mutex_name, MUTEX_NAME_LEN);
	memcpy(new_mutex_name, mutex_name, strlen(mutex_name));
	new_mutex_name[strlen(mutex_name)] = 0;
	mutex->id = hash_str(new_mutex_name)%MUTEX_HASH_SIZE;
	mutex->name = new_mutex_name;
	//coordinator
	if(g_group.node_id == g_group.coordinator.mutex_id){
		new_mutex = hashTableSearch(g_group.mutex_table, mutex->id, mutex->name);		
		if(new_mutex != NULL)
			return 1;
		insertNewMutex(mutex->id, new_mutex_name);
		return 1;
	}
	//not coordinator
	else{
		buf = mem_malloc(MUTEX_BUF_SIZE);
		((struct mutex_req*)buf)->req.msg_type = MSG_MUTEX_INIT;
		((struct mutex_req*)buf)->hash_id = mutex->id;
		((struct mutex_req*)buf)->name_len = strlen(new_mutex_name);
		memcpy( buf+sizeof(struct mutex_req), new_mutex_name, ((struct mutex_req*)buf)->name_len);
		sendRecv(g_group.coordinator.mutex_id ,
				buf, sizeof(struct mutex_req)+((struct mutex_req*)buf)->name_len,
				buf, sizeof(struct request_header));
		if(((struct request_header*)buf)->msg_type == MSG_MUTEX_FAILED){
			mem_free(buf);
			return -1;
		}
		mem_free(buf);
	}
	return 1;
}

int d2mce_mutex_lock(d2mce_mutex_t *mutex){
//	char wait = TRUE;
	void *buf;
//	sem_t sem;
//	struct mutex_lock *find;
//	struct req_node_info node;
	//coordinator
/*	if(g_group.node_id == g_group.coordinator.mutex_id){		
		find = hashTableSearch(g_group.mutex_table, mutex->id, mutex->name);
		if(find == NULL)
			return -1;
		node.id = g_group.node_id;
		sem_init(&sem, 0, 0);
		node.sem = &sem;
		pthread_mutex_lock(&find->lock);
		if(find->mutex == FALSE){
			find->mutex = TRUE;
			find->owner = g_group.node_id;
			wait = FALSE;
		}
		else
			queuePush(find->queue, (void*)&node);
		pthread_mutex_unlock(&find->lock);
		if(wait == TRUE)
			sem_wait(node.sem);
	}
	//not coordinator
	else{*/
		buf = mem_malloc(MUTEX_BUF_SIZE);
		((struct mutex_req*)buf)->req.msg_type = MSG_MUTEX_LOCK;
		((struct mutex_req*)buf)->hash_id = mutex->id;
		((struct mutex_req*)buf)->name_len = strlen(mutex->name);
		memcpy(buf+sizeof(struct mutex_req), mutex->name, ((struct mutex_req*)buf)->name_len);
		sendRecv(g_group.coordinator.mutex_id,
				buf , sizeof(struct mutex_req)+ ((struct mutex_req*)buf)->name_len,
				buf, sizeof(struct request_header));
		if(((struct request_header*)buf)->msg_type == MSG_MUTEX_FAILED){
			mem_free(buf);
			return -1;
		}
		mem_free(buf);
//	}
	acquire();
	return 1;

}
int d2mce_mutex_unlock(d2mce_mutex_t *mutex){
	void *buf;
//	struct mutex_lock *find;
//	struct req_node_info *get;
//	struct reply_header reply;
	release();
	//coordinator
/*	if(g_group.node_id == g_group.coordinator.mutex_id){
		find = hashTableSearch(g_group.mutex_table, mutex->id, mutex->name);
		if(find == NULL)
			return -1;
		if(find->owner != g_group.node_id)		
			return -1;
		reply.msg_type = MSG_MUTEX_OK;
		pthread_mutex_lock(&find->lock);
		if(find->mutex == TRUE){
			get=(struct req_node_info*)queuePop(find->queue);
			if(get==NULL){
				find->mutex = FALSE;
				find->owner = -1;
			}
			else{
				find->owner = get->id;
				if(get->id == g_group.coordinator.mutex_id)
					sem_post(get->sem);
				else{
					reply.seq_number = get->seq_number; 
					sendTo(get->id, (void*)&reply, sizeof(struct request_header));
				}
			
			}		
		}	
		pthread_mutex_unlock(&find->lock);
	}
	//not coordinator
	else{*/
		buf = mem_malloc(MUTEX_BUF_SIZE);
		((struct mutex_req*)buf)->req.msg_type = MSG_MUTEX_UNLOCK;
		((struct mutex_req*)buf)->hash_id = mutex->id;
		((struct mutex_req*)buf)->name_len = strlen(mutex->name);
		memcpy(buf+sizeof(struct mutex_req), mutex->name, ((struct mutex_req*)buf)->name_len);
		sendTo(g_group.coordinator.mutex_id, buf, sizeof(struct mutex_req)+((struct mutex_req*)buf)->name_len);
/*		sendRecv(g_group.coordinator.mutex_id, (void*)&buf , sizeof(struct mutex_req)+ ((struct mutex_req*)buf)->name_len,
			(void*)&reply, sizeof(struct request_header));
		if(reply.msg_type == MSG_MUTEX_FAILED)
			return -1;
*/			
		mem_free(buf);
//	}
	return 1;
}

int d2mce_sem_init(d2mce_sem_t *sem, char *sem_name, unsigned int value){
	char *new_sem_name;
	void *buf;	
	struct sem *new_sem;
	new_sem_name = str_malloc(sem_name, SEM_NAME_LEN, 1);
//	strncpy(new_sem_name, sem_name, SEM_NAME_LEN);
	memcpy(new_sem_name, sem_name, strlen(sem_name));
	new_sem_name[strlen(sem_name)] = 0;

	sem->id = hash_str(new_sem_name)%SEM_HASH_SIZE;
	sem->name = new_sem_name;
	//coordinator
	if(g_group.node_id == g_group.coordinator.sem_id){
		new_sem = hashTableSearch(g_group.sem_table, sem->id, sem->name);		
		if(new_sem != NULL)
			return 1;
		insertNewSem(sem->id, new_sem_name, value);
		return 1;
	}
	//not coordinator
	else{
		buf = mem_malloc(SEM_INIT_SIZE);
		((struct sem_init_req*)buf)->req.msg_type = MSG_SEM_INIT;
		((struct sem_init_req*)buf)->hash_id = sem->id;
		((struct sem_init_req*)buf)->name_len = strlen(new_sem_name);
		((struct sem_init_req*)buf)->value = value;
		memcpy( buf+sizeof(struct sem_init_req), new_sem_name, ((struct sem_init_req*)buf)->name_len);
		sendRecv(g_group.coordinator.sem_id ,
					buf, sizeof(struct sem_init_req)+((struct sem_init_req*)buf)->name_len,
					buf, sizeof(struct request_header));
		if(((struct request_header*)buf)->msg_type == MSG_SEM_FAILED){
			mem_free(buf);
			return -1;
		}
		mem_free(buf);
	}
	return 1;
}
int d2mce_sem_post(d2mce_sem_t *sem){
	void *buf;
//	struct sem *find;
//	struct req_node_info *get;
//	struct reply_header reply;
	release();
	//coordinator
/*
	if(g_group.node_id == g_group.coordinator.sem_id){		
		find = hashTableSearch(g_group.sem_table, sem->id, sem->name);
		if(find == NULL)
			return -1;
		reply.msg_type = MSG_MUTEX_OK;
		pthread_mutex_lock(&find->lock);
		find->value++;
		if(find->value >0){
			get = queuePop(find->queue);
			if(get!=NULL){
				find->value--;
				if(get->id == g_group.coordinator.sem_id)
					sem_post(get->sem);
				else{
					reply.seq_number = get->seq_number; 
					sendTo(get->id, (void*)&reply, sizeof(struct request_header));
				}
			}
		}
		pthread_mutex_unlock(&find->lock);

	}
	//not coordinator
	else{*/
		buf = mem_malloc(SEM_BUF_SIZE);
		((struct sem_req*)buf)->req.msg_type = MSG_SEM_POST;
//		((struct sem_req*)buf)->req.seq_number = 0;
		((struct sem_req*)buf)->hash_id = sem->id;
		((struct sem_req*)buf)->name_len = strlen(sem->name);
		memcpy(buf+sizeof(struct sem_req), sem->name, ((struct sem_req*)buf)->name_len);
		sendTo(g_group.coordinator.sem_id, buf, sizeof(struct sem_req)+((struct sem_req*)buf)->name_len);
//		sendRecv(g_group.coordinator.sem_id, (void*)&buf , sizeof(struct sem_req)+ ((struct sem_req*)buf)->name_len,
//			(void*)&reply, sizeof(struct request_header));
//		if(reply.msg_type == MSG_SEM_FAILED)
//			return -1;
		mem_free(buf);
//	}
	return 1;

}
int d2mce_sem_wait(d2mce_sem_t *d2mce_sem){
//	char wait = TRUE;
	void *buf;
//	sem_t sem;
//	struct sem *find;
//	struct req_node_info node;
	//coordinator
/*
	if(g_group.node_id == g_group.coordinator.sem_id){		
		find = hashTableSearch(g_group.sem_table, d2mce_sem->id, d2mce_sem->name);
		if(find == NULL)
			return -1;
		node.id = g_group.node_id;
		sem_init(&sem, 0, 0);
		node.sem = &sem;	
		pthread_mutex_lock(&find->lock);
		if(find->value >0){
			wait=FALSE;
			find->value--;
		}
		else
			queuePush(find->queue, (void*)&node);
		pthread_mutex_unlock(&find->lock);
		if(wait==TRUE)
			sem_wait(node.sem);
	}
	//not coordinator
	else{*/
		buf = mem_malloc(SEM_BUF_SIZE);
		((struct sem_req*)buf)->req.msg_type = MSG_SEM_WAIT;
		((struct sem_req*)buf)->hash_id = d2mce_sem->id;
		((struct sem_req*)buf)->name_len = strlen(d2mce_sem->name);
		memcpy(buf+sizeof(struct sem_req), d2mce_sem->name, ((struct sem_req*)buf)->name_len);
		sendRecv(g_group.coordinator.sem_id, 
					buf , sizeof(struct sem_req)+ ((struct sem_req*)buf)->name_len,
					buf, sizeof(struct request_header));
		if(((struct request_header*)buf)->msg_type == MSG_SEM_FAILED){
			mem_free(buf);
			return -1;
		}
		mem_free(buf);
//	}
	return 1;
}

int d2mce_barrier_init(d2mce_barrier_t *barrier, char *barrier_name){
	char *new_barrier_name;
	void *buf;
	struct barrier *new_barrier;
	new_barrier_name = str_malloc(barrier_name, BARRIER_NAME_LEN, 1);
//	strncpy(new_barrier_name, barrier_name, BARRIER_NAME_LEN);
	memcpy(new_barrier_name,barrier_name, strlen(barrier_name));
	new_barrier_name[strlen(barrier_name)] = 0;
	barrier->id = hash_str(new_barrier_name)%BARRIER_HASH_SIZE;
	barrier->name = new_barrier_name;
	//coordinator
	if(g_group.node_id == g_group.coordinator.barrier_id){
		new_barrier = hashTableSearch(g_group.barrier_table, barrier->id, barrier->name);		
		if(new_barrier != NULL)
			return 1;
		insertNewBarrier(barrier->id, new_barrier_name);
		return 1;
	}
	//not coordinator
	else{
		buf = mem_malloc(BARRIER_INIT_SIZE);
		((struct barrier_init_req*)buf)->req.msg_type = MSG_BARRIER_INIT;
		((struct barrier_init_req*)buf)->hash_id = barrier->id;
		((struct barrier_init_req*)buf)->name_len = strlen(new_barrier_name);
		memcpy( buf+sizeof(struct barrier_init_req), new_barrier_name, ((struct barrier_init_req*)buf)->name_len);
		sendRecv(g_group.coordinator.barrier_id ,
					buf, sizeof(struct barrier_init_req)+((struct barrier_init_req*)buf)->name_len,
					buf, sizeof(struct request_header));
		if(((struct request_header*)buf)->msg_type == MSG_BARRIER_FAILED){
			mem_free(buf);
			return -1;
		}
		mem_free(buf);
	}
	return 1;
}

int d2mce_barrier(d2mce_barrier_t *barrier, unsigned int wait_counter){
//	char wait = TRUE;
	void *buf;
//	sem_t sem;
//	struct barrier *find;
//	struct req_node_info *get;
//	struct req_node_info node;
//	struct reply_header reply;
	release();
	if(wait_counter <= 1)
		return 1;
	//coordinator
/*	if(g_group.node_id == g_group.coordinator.barrier_id){
		node.id = g_group.node_id;
		sem_init(&sem, 0, 0);
		node.sem = &sem;
		node.seq_number = -1;
		find = hashTableSearch(g_group.barrier_table, barrier->id, barrier->name);
		if(find == NULL)
			return -1;
		pthread_mutex_lock(&find->lock);
		if(find->state == OFF){
			find->state = ON;
			find->wait_counter = wait_counter;
		}
		find->wait_counter--;
		if(find->wait_counter!=0)
			queuePush(find->queue, (void*)&node);
		else if(find->wait_counter==0){
			wait = FALSE;
			reply.msg_type = MSG_BARRIER_OK;
			while(find->queue->use>0){
				get = (struct req_node_info*)queuePop(find->queue); 		
				if(get->id != g_group.node_id){
					reply.seq_number = get->seq_number;
					sendTo(get->id, (void*)&reply, sizeof(struct request_header));
				}
			}
			find ->state=OFF;
		}
		pthread_mutex_unlock(&find->lock);
		if(wait == TRUE)
			sem_wait(node.sem);
		
	}
	//not coordinator
	else{*/
		buf = mem_malloc(BARRIER_BUF_SIZE);
		((struct barrier_req*)buf)->req.msg_type = MSG_BARRIER_REQ;
		((struct barrier_req*)buf)->hash_id = barrier->id;
		((struct barrier_req*)buf)->wait_counter = wait_counter;
		((struct barrier_req*)buf)->name_len = strlen(barrier->name);
		memcpy(buf+sizeof(struct barrier_req), barrier->name, ((struct barrier_req*)buf)->name_len);
		sendRecv(g_group.coordinator.barrier_id,
			buf , sizeof(struct barrier_req)+ ((struct barrier_req*)buf)->name_len,
			buf, sizeof(struct request_header));
		if(((struct request_header*)buf)->msg_type == MSG_BARRIER_FAILED){
			mem_free(buf);
			return -1;
		}
		mem_free(buf);
//	}
	return 1;
}


int d2mce_mutex_rw(d2mce_mutex_t* mutex, int num, ...)
{
	int i;
	void *ptr;
	char *action;
	va_list va;
	if (num <= 0)
		return -1;
	va_start(va, num);
	d2mce_mutex_lock(mutex);
	for(i=0; i<num; i++){
		ptr = va_arg(va, void*);	
		action = va_arg(va, char*);	
		if (strcmp( "r", action) == 0)
			d2mce_load(ptr);
		else if(strcmp("w", action) == 0)
			d2mce_store(ptr);
		else if(strcmp("rw", action) ==0){
			d2mce_load(ptr);
			d2mce_store(ptr);
		}
		else
			printf("Error:d2mce_mutex_rw action is not find\n");
	}
	va_end(va);			
	return 1;
}

int d2mce_sem_wait_rw(d2mce_sem_t* sem, int num, ...)
{
	int i;
	void *ptr;
	char *action;
	va_list va;
	if (num <= 0)
		return -1;
	va_start(va, num);
	d2mce_sem_wait(sem);
	for(i=0; i<num; i++){
		ptr = va_arg(va, void*);	
		action = va_arg(va, char*);	
		if (strcmp( "r", action) == 0)
			d2mce_load(ptr);
		else if(strcmp("w", action) == 0)
			d2mce_store(ptr);
		else if(strcmp("rw", action) ==0){
			d2mce_load(ptr);
			d2mce_store(ptr);
		}
		else
			printf("Error:d2mce_mutex_rw action is not find\n");
	}
	va_end(va);	
 	return 1;
}

__inline__ int d2mce_getNodeNum(){
	return g_group.node_num;
}

__inline__ int d2mce_isCoordinator(){
 	return (g_group.coordinator.main_id == g_group.node_id);
}

double d2mce_time(){
  double gettime;
  struct timeval now;
  gettimeofday(&now, 0);
  gettime = 1000000*now.tv_sec + now.tv_usec;
  gettime /= 1000000;
  return gettime;
}

double d2mce_stime(){
  struct timeval now;
  gettimeofday(&now, 0);
  return now.tv_sec;
}

double d2mce_utime(){
  struct timeval now;
  gettimeofday(&now, 0);
  return now.tv_usec;
}


#ifdef CONF_SM_DISSEMINATE_UPDATE
int d2mce_update_register(void* share_memory){
	int index;
	void *buf;
	struct sm_info *sm_info;
	struct sm_header *sm_header;	
	if(share_memory==NULL)
		return -1;
	sm_info = share_memory - sizeof(struct sm_info);
	if(sm_info->header==NULL)
		return -1;
	sm_header = sm_info->header;
	//home node
	if(sm_header->home_node == g_group.node_id){
		pthread_mutex_lock(&sm_header->lock);
		if(searchNode(sm_info->disseminate, g_group.node_id)==-1){
			index = tableGetEmpty(sm_info->disseminate);
			if(index == -1){
				pthread_mutex_unlock(&sm_header->lock);
				return -1;
			}
		}
		tableAdd(sm_info->disseminate, (void*)&g_group.node_id, index);
		pthread_mutex_unlock(&sm_header->lock);
		return 1;
	}
	//not home node
	buf = mem_malloc(SM_BUF_SIZE);
	((struct sm_req*)buf)->req.msg_type = MSG_SM_UPDATE_REGISTER;
	((struct sm_req*)buf)->hash_id = sm_header->hash.id;
	((struct sm_req*)buf)->name_len = strlen(sm_header->hash.name);
	memcpy( buf+sizeof(struct sm_req), sm_header->hash.name, ((struct sm_req*)buf)->name_len);
	sendTo(sm_header->home_node, buf ,sizeof(struct sm_req)+((struct sm_req*)buf)->name_len);
	mem_free(buf);
  	return 1;
}
int d2mce_update_unregister(void* share_memory){
	int index;
	void *buf;
	struct sm_info *sm_info;
	struct sm_header *sm_header;	
	if(share_memory==NULL)
		return -1;
	sm_info = share_memory - sizeof(struct sm_info);
	if(sm_info->header==NULL)
		return -1;
	sm_header = sm_info->header;
	//home node
	if(sm_header->home_node == g_group.node_id){
		pthread_mutex_lock(&sm_header->lock);
		index = searchNode(sm_info->disseminate, g_group.node_id);
		if(index >= 0)
			tableRemove(sm_info->disseminate , index);
		pthread_mutex_unlock(&sm_header->lock);
		return 1;
	}
	//not home node
	buf = mem_malloc(SM_BUF_SIZE);
	((struct sm_req*)buf)->req.msg_type = MSG_SM_UPDATE_UNREGISTER;
	((struct sm_req*)buf)->hash_id = sm_header->hash.id;
	((struct sm_req*)buf)->name_len = strlen(sm_header->hash.name);
	memcpy( buf+sizeof(struct sm_req), sm_header->hash.name, ((struct sm_req*)buf)->name_len);
	sendTo(sm_header->home_node, buf ,sizeof(struct sm_req)+((struct sm_req*)buf)->name_len);
	mem_free(buf);
  	return 1;
}

#endif


int d2mce_set_barrier_manager(){
	return barrier_imanager_req();
}
int d2mce_set_sem_manager(){
	return sem_imanager_req();
}
int d2mce_set_mutex_manager(){
	return mutex_imanager_req();
}
int d2mce_set_resource_manager(){
	return main_imanager_req();
}
#ifdef CONF_SM_EVENT_DRIVER

int d2mce_wait_update(void* share_memory){
	struct sm_info *sm_info;
	struct sm_header *sm_header;	
	if(share_memory==NULL)
		return -1;
	sm_info = share_memory - sizeof(struct sm_info);
	if(sm_info->header==NULL)
		return -1;
	sm_header = sm_info->header;
	pthread_mutex_lock(&sm_header->lock);
	sm_info->event.wait_count++;
	pthread_mutex_unlock(&sm_header->lock);
	sem_wait(&sm_info->event.sem);
	return 1;
}

int d2mce_trywait_update(void* share_memory){
    struct sm_info *sm_info;
    struct sm_header *sm_header;
    if(share_memory==NULL)
        return -1;
    sm_info = share_memory - sizeof(struct sm_info);
    if(sm_info->header==NULL)
        return -1;
    sm_header = sm_info->header;
    if (sem_trywait(&sm_info->event.sem) == 0)
        return 1;
    else
        return -1;
    return 1;
}


#endif

int d2mce_acq(){
	acquire();
	return 1;
}

int print_info(){	
	int i;
	char line[16];
	unsigned int ip = htonl(g_system_conf.ip);
	unsigned int broadcast = htonl(g_system_conf.broadcast_ip);
	struct node **node_table;
	printf("my ip = %s \n", inet_ntop(AF_INET, &ip , line, 16 ));
	printf("port = %d \n", g_system_conf.port);
	printf("broadcast ip = %s \n", inet_ntop(AF_INET, &broadcast, line, 16 ));
	printf("mac = %2x:%2x:%2x:%2x:%2x:%2x\n", g_system_conf.hwaddr[0], g_system_conf.hwaddr[1], g_system_conf.hwaddr[2],
			g_system_conf.hwaddr[3], g_system_conf.hwaddr[4], g_system_conf.hwaddr[5]);
	node_table =(struct node**)getTable(g_group.node_table);
	for(i=0; i<g_group.node_table->use; i++){
		printf("%d\t%s\t%d\t%d\t%d\n", node_table[i]->id, inet_ntop(AF_INET, &node_table[i]->ip , line, 16 ), node_table[i]->port, node_table[i]->recv_fd, node_table[i]->send_fd);
	}
	return 1;
}

int print_overhead(){	
	printf("msg count:%u\n", g_overhead.msg_count);
	printf("msg byte :%u\n", g_overhead.msg_size);
	return 1;
}
