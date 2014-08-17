#include "receiver.h"
#define MSG_TYPE check.msg_type
#define SEQ_NUMBER check.seq_number

int toQueue(struct role_info *role, int sockfd, unsigned int recv_size);

int receiver_init(){
	if(g_recv_info.count == 0){
		g_recv_info.count = 1;
		return 1;
	}
	g_recv_info.count++;
	return -1;
}
int receiver_destroy(){
	if(g_recv_info.count == 1){
		g_recv_info.count = 0;
		return 1;
	}
	g_recv_info.count--;
	return -1;
}

void *receiver(void *argv){
	int i;
	int listenfd, connfd;
	int find;
	void* recv_buf;
	unsigned int recv_size;
	fd_set rset;
    socklen_t child_len;
    struct sockaddr_in child_addr;
	struct node **node_table;
	struct receiver_table **receiver_table;
	struct request_header check;
	receiver_init();
	node_table = (struct node**)getTable(g_group.node_table);
	receiver_table = (struct receiver_table**)getTable(g_receiver_table);
	listenfd = TCP_listen(g_system_conf.ip, 0);
	g_system_conf.port = get_listen_port(listenfd);
	fdSet(listenfd);
	sem_post(&g_thread_sem);
	while(1) {
		usleep(45);
		rset = g_fdset;
		select(g_maxfd+1, &rset, NULL, NULL, NULL);
		//new connect
		if(FD_ISSET( listenfd, &rset))	{
			child_len = sizeof(struct sockaddr_in);
			if( (connfd = accept( listenfd, (struct sockaddr *) &child_addr, &child_len) ) == -1){
            	printf("Error: accept()");
	           	continue;
    	    }
			find = getNodeID(ntohl(child_addr.sin_addr.s_addr));
			if(find >=0)
				node_table[find]->recv_fd = connfd;
			fdSet(connfd);
			continue;
		}
		//check new connect want join?
		if( connfd>0 && FD_ISSET(connfd, &rset) && find<0){
			recv(connfd, (void*)&check, sizeof(struct request_header), MSG_PEEK);
			recv_buf = mem_malloc(check.size);
			recv_size = 0;
			do{
				recv_size += recv(connfd, recv_buf + recv_size,	check.size - recv_size, 0);				
			}while(check.size > recv_size);
			if(recv_size <= 0) {
				mem_free(recv_buf);
				fdClear(connfd);
				close(connfd);
				connfd = -1;
			}
			else if(MSG_TYPE == MSG_JOIN_GRANT){
				*((int*)(recv_buf+sizeof(struct join_s2c_req))) = connfd;
				pthread_mutex_lock(&g_ctrl_info.lock);
				wqueuePush(g_ctrl_info.wqueue, recv_buf);
				pthread_mutex_unlock(&g_ctrl_info.lock);
				sem_post(&g_ctrl_info.sem); 
				connfd = -1;
			}
		}

		//check aready connect have receive data
		for (i = 0; i< MAX_NODE_NUM ; i++)	{	
			if(node_table[i]->recv_fd <= 0)
				continue;
			if (FD_ISSET( node_table[i]->recv_fd, &rset)) {
				recv_size = recv(node_table[i]->recv_fd, (void*)&check, sizeof(struct request_header), MSG_PEEK);
//				printf("recv!! msg:%u src_node:%u src_seq:%u size:%u des_seq:%u\n",check.msg_type,check.src_node,check.src_seq_number, check.size, check.seq_number);s
				//old connect will disconnect (this is not normal exit)
				if(recv_size <= 0) {
					recv_buf =  mem_malloc(check.size);
					recv_size = recv(node_table[i]->recv_fd, recv_buf, check.size, 0);
					mem_free(recv_buf);
					fdClear(node_table[i]->recv_fd);
					close(node_table[i]->recv_fd);
					node_table[i]->recv_fd = -1;
				}
				//do
				else if( MSG_TYPE == MSG_D2MCE_TERMINAL){
					recv_buf =  mem_malloc(check.size);
					recv_size = recv(node_table[i]->recv_fd, recv_buf, check.size, 0);
					mem_free(recv_buf);
					goto end_receiver;
				}
				//computing thread
				else if (MSG_TYPE == MSG_JOIN_REPLY || MSG_TYPE == MSG_JOIN_OK || MSG_TYPE == MSG_JOIN_FAILED ||
							MSG_TYPE == MSG_BARRIER_OK || MSG_TYPE == MSG_BARRIER_FAILED || MSG_TYPE == MSG_IBAR_REPLY || MSG_TYPE == MSG_IBAR_READY ||
							MSG_TYPE == MSG_MUTEX_OK ||  MSG_TYPE == MSG_MUTEX_FAILED || MSG_TYPE == MSG_IMUTEX_REPLY || MSG_TYPE == MSG_IMUTEX_READY ||
							MSG_TYPE == MSG_SEM_OK || MSG_TYPE == MSG_SEM_FAILED || MSG_TYPE == MSG_ISEM_REPLY || MSG_TYPE == MSG_ISEM_READY ||
							MSG_TYPE == MSG_SM_OK || MSG_TYPE == MSG_SM_FAILED || MSG_TYPE == MSG_SM_INIT_REPLY ||
							MSG_TYPE == MSG_FINALIZE_OK ||
							MSG_TYPE == MSG_SM_IAMHOME_REPLY ||
							MSG_TYPE == MSG_IMAIN_REPLY || MSG_TYPE == MSG_IMAIN_READY ||
							MSG_TYPE == MSG_YOU_OK){
					receiver_table[SEQ_NUMBER]->recv_size=0;		
//					receiver_table[SEQ_NUMBER]->recv_size = recv(node_table[i]->recv_fd, receiver_table[SEQ_NUMBER]->buf, check.size, 0);
					do{
						receiver_table[SEQ_NUMBER]->recv_size += recv(node_table[i]->recv_fd,
							receiver_table[SEQ_NUMBER]->buf + receiver_table[SEQ_NUMBER]->recv_size,
							check.size-receiver_table[SEQ_NUMBER]->recv_size, 0);
					}while(check.size > receiver_table[SEQ_NUMBER]->recv_size);
					sem_post(&receiver_table[SEQ_NUMBER]->sem);
				}
				//main
				else if(MSG_TYPE == MSG_JOIN_NEWNODE_ADD || MSG_TYPE == MSG_IMAIN_MANAGER || MSG_TYPE == MSG_NEWMAIN_MANAGER){
					toQueue(&g_ctrl_info, node_table[i]->recv_fd, check.size);
					sem_post(&g_ctrl_info.sem);
				}
				//exit
				else if(MSG_TYPE == MSG_YOU_MANAGER || MSG_TYPE == MSG_NODE_EXIT ){					
					toQueue(&g_ctrl_info, node_table[i]->recv_fd, check.size);
					sem_post(&g_ctrl_info.sem);
				}
				//finalize
				else if(MSG_TYPE == MSG_FINALIZE){					
					toQueue(&g_ctrl_info, node_table[i]->recv_fd, check.size);
					sem_post(&g_ctrl_info.sem);
				}
				//barrier
				else if(MSG_TYPE == MSG_BARRIER_INIT || MSG_TYPE == MSG_BARRIER_REQ ||
						MSG_TYPE == MSG_IBAR_MANAGER || MSG_TYPE == MSG_NEWBAR_MANAGER ){ //MSG_TYPE == MSG_IBAR_READY)
					toQueue(&g_ctrl_info, node_table[i]->recv_fd, check.size);
					sem_post(&g_ctrl_info.sem);
				}
				//mutex
				else if(MSG_TYPE == MSG_MUTEX_INIT || MSG_TYPE == MSG_MUTEX_LOCK || MSG_TYPE == MSG_MUTEX_UNLOCK ||
						MSG_TYPE == MSG_IMUTEX_MANAGER || MSG_TYPE == MSG_NEWMUTEX_MANAGER){
					toQueue(&g_ctrl_info, node_table[i]->recv_fd, check.size);
					sem_post(&g_ctrl_info.sem);

				}
				//semaphore
				else if(MSG_TYPE == MSG_SEM_INIT || MSG_TYPE == MSG_SEM_POST || MSG_TYPE == MSG_SEM_WAIT ||
						MSG_TYPE == MSG_ISEM_MANAGER || MSG_TYPE == MSG_NEWSEM_MANAGER){
					toQueue(&g_ctrl_info, node_table[i]->recv_fd, check.size);
					sem_post(&g_ctrl_info.sem);

				}
				//share memory
				else if(MSG_TYPE == MSG_SM_INIT || MSG_TYPE == MSG_SM_FETCH || MSG_TYPE == MSG_SM_INVALID ||
						MSG_TYPE == MSG_SM_INVALID_NOREPLY || MSG_TYPE == MSG_SM_READMISS || MSG_TYPE == MSG_SM_WRITEMISS ||
						MSG_TYPE == MSG_SM_WRITEMISS_NOREPLY || MSG_TYPE == MSG_SM_INVALID_OK ||
						MSG_TYPE == MSG_SM_MULTIREAD ||	MSG_TYPE == MSG_SM_MULTIWRITE ||
						MSG_TYPE == MSG_SM_IAMHOME || MSG_TYPE == MSG_SM_HOME_READY ||MSG_TYPE == MSG_SM_NEWHOME ||
						MSG_TYPE == MSG_SM_UHOME){
					toQueue(&g_data_info, node_table[i]->recv_fd, check.size);
					sem_post(&g_data_info.sem);
//					toQueue(&g_ctrl_info, node_table[i]->recv_fd, check.size);
//					sem_post(&g_ctrl_info.sem);

				}
#ifdef CONF_SM_DISSEMINATE_UPDATE
				else if(MSG_TYPE == MSG_SM_UPDATE_REGISTER || MSG_TYPE == MSG_SM_UPDATE_UNREGISTER || MSG_TYPE == MSG_SM_UPDATE){
					
					toQueue(&g_data_info, node_table[i]->recv_fd, check.size);
					sem_post(&g_data_info.sem);
//					toQueue(&g_ctrl_info, node_table[i]->recv_fd, check.size);
//					sem_post(&g_ctrl_info.sem);
				}
#endif
				else{
					printf("Receiver Error MSG %d size%d from %d\n", check.msg_type, check.size, i);
					recv_buf =  mem_malloc(check.size);
					recv_size = recv(node_table[i]->recv_fd, recv_buf, check.size, 0);
					mem_free(recv_buf);
				}
			}
		}
	}
end_receiver:
	close(listenfd);
	receiver_destroy();
	return NULL;
}
int receiver_close(){
	struct request_header msg;
	msg.msg_type= MSG_D2MCE_TERMINAL;
	sendTo(g_group.node_id, (void*)&msg, sizeof(struct request_header));
	return 1;
}

int getNodeID(unsigned int node_ip){
	int i=0;
	struct node **node_table;
	node_table = (struct node**)getTable(g_group.node_table);
	for(i=0; i<MAX_NODE_NUM; i++){
		if( node_table[i]->ip == node_ip && node_table[i]->recv_fd == -1)
			return i;
	}
	return -1;
}

int toQueue(struct role_info *role, int sockfd, unsigned int recv_size){
	int full;
	void *recv_data;
	unsigned int size;
		
	recv_data = mem_malloc(recv_size);
	size = 0;
	do{
		size += recv(sockfd, recv_data + size, recv_size-size, 0);
	}while(recv_size > size);
	
	pthread_mutex_lock(&role->lock);
	full = wqueuePush(role->wqueue, recv_data);
	if(full == -1){
		role->wqueue = wqueueFat(role->wqueue, role->wqueue->wqueue_size);
		if(role->wqueue == NULL){
			printf("Receiver Error:data queue fat error\n");
			exit(1);
		}
		full = wqueuePush(role->wqueue, recv_data);
		if(full == -1){
			printf("Receiver Error:data queue full \n");
			exit(1);
		}
	}
	pthread_mutex_unlock(&role->lock);
	return 1;
}


