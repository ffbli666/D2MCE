/*
	main manager
	resource manager
	node join, share memory info table, node exit
	
*/

#include "manager.h"

int main_imanager_reply(void *request){
	int i, j;
	void *buf;
//	int index;
	int sockfd;	
	int *node;
	unsigned int src_node;
	unsigned int sm_num = 0;
	unsigned int offset;
	unsigned int *send_node;
	struct sm_header *now;
	struct sockaddr_in server_addr;
	src_node = ((struct request_header*)request)->src_node;
	buf = mem_malloc(sizeof(struct imain_reply) + g_group.sm_table->item_num*200);
	((struct imain_reply*)buf)->req.msg_type = MSG_IMAIN_REPLY;
	((struct imain_reply*)buf)->req.seq_number = ((struct request_header*)request)->src_seq_number;
	offset = sizeof(struct imain_reply);
	for(i=0; i<g_group.sm_table->table_size; i++){
		now = (struct sm_header*)g_group.sm_table->row[i];
		while(now!=NULL){
			sm_num++;
			//max 200 byte of one mutex 24+32+4*32 = 184
			((struct main_sm_info*)(buf+offset))->hash_id = now->hash.id;
			((struct main_sm_info*)(buf+offset))->name_len = strlen(now->hash.name);
			((struct main_sm_info*)(buf+offset))->home_node = now->home_node;
			((struct main_sm_info*)(buf+offset))->size = now->size;
			((struct main_sm_info*)(buf+offset))->count = now->count;
			((struct main_sm_info*)(buf+offset))->users_count = now->users->use;
			offset += sizeof(struct main_sm_info);
			memcpy(buf+offset, now->hash.name, strlen(now->hash.name));
			offset += strlen(now->hash.name);
			//node id in the table
			for(j=0;j<now->users->table_size;j++){
				node = now->users->row[j];
				if(*node != -1){
					*((unsigned int*)(buf+offset)) = *node;
					offset += sizeof(unsigned int);
				}			
			}
			
			now = (struct sm_header*)now->hash.next;
		}
	}
	((struct imain_reply*)buf)->sm_num = sm_num;
	//send mutex info & wait new mutex ready
	sendRecv(src_node, buf, offset, buf, sizeof(struct request_header));
	g_group.coordinator.main_id = ((struct request_header*)request)->src_node;
	//send to other node the
	((struct new_manager_req*)buf)->req.msg_type = MSG_NEWMAIN_MANAGER;
	((struct new_manager_req*)buf)->new_manager = src_node;
	for(i=0;i<g_group.node_table->table_size;i++){
		send_node = tableGetRow(g_group.node_table, i);
		if(*send_node != -1 && *send_node != g_group.node_id && *send_node != src_node)
			sendTo(*send_node, buf, sizeof(struct new_manager_req));
	}
	//send to demon
	((struct main_s2d_req*)buf)->msg_type = MSG_NEWMAIN_MANAGER;
	((struct main_s2d_req*)buf)->app_len = strlen(g_group.app_name);
	((struct main_s2d_req*)buf)->group_len = strlen(g_group.group_name);
	((struct main_s2d_req*)buf)->ip = ((struct node*)g_group.node_table->row[src_node])->ip;
	((struct main_s2d_req*)buf)->port = ((struct node*)g_group.node_table->row[src_node])->port;	
	((struct main_s2d_req*)buf)->id = src_node;
	memcpy( buf+sizeof(struct main_s2d_req), g_group.app_name, ((struct main_s2d_req*)buf)->app_len);
	memcpy( buf+sizeof(struct main_s2d_req)+strlen(g_group.app_name), g_group.group_name, ((struct main_s2d_req*)buf)->group_len);   
	bzero(&server_addr, sizeof(server_addr));
	sockfd = UDP_init(g_system_conf.broadcast_ip, g_system_conf.demon_port, &server_addr);
	UDP_setBroadcast(sockfd);
	UDP_setTTL(sockfd, 1);
	sendto(sockfd, buf, sizeof(struct main_s2d_req)+((struct main_s2d_req*)buf)->app_len+((struct main_s2d_req*)buf)->group_len,
		0, (struct sockaddr*)&server_addr, sizeof(server_addr));

	mem_free(buf);
	return 1;	

}
int main_newmanager_reply(void *request){
	g_group.coordinator.main_id = ((struct new_manager_req*)request)->new_manager;
	return 1;
}
int main_imanager_req(){
	int i, j;
	int index;
	void *buf;
	unsigned int offset;
	unsigned int users_count;
	unsigned int src_node;
	unsigned int seq_number;
	unsigned int node_id;
	struct sm_header *find;
	char *sm_name;
	if(g_group.coordinator.main_id == g_group.node_id)
		return 1;
	//max 33 mutex
	buf = mem_malloc(MAIN_IMAIN_SIZE);
	((struct request_header*)buf)->msg_type = MSG_IMAIN_MANAGER;
	sendRecv(g_group.coordinator.main_id, buf, sizeof(struct request_header),
			buf, MSG_IMAIN_MANAGER);
	if(g_group.sm_table == NULL){
		g_group.sm_table = hashTableCreate(SM_HASH_SIZE);
	}
	
	src_node = ((struct imain_reply*)buf)->req.src_node;
	seq_number = ((struct imain_reply*)buf)->req.src_seq_number;
	offset = sizeof(struct imain_reply);
	//copy share memory data
	for(i=0; i<((struct imain_reply*)buf)->sm_num; i++){
		//mutex name
		sm_name = malloc(((struct main_sm_info*)(buf+offset))->name_len);
		memcpy(sm_name, buf+offset+sizeof(struct main_sm_info), ((struct main_sm_info*)(buf+offset))->name_len);
		sm_name[((struct main_sm_info*)(buf+offset))->name_len] = 0;
		printf("%s", sm_name);
		//search mutex
		find = hashTableSearch(g_group.sm_table, ((struct main_sm_info*)(buf+offset))->hash_id, sm_name);
		if(find == NULL){
			//create new
			find = createSM(((struct main_sm_info*)(buf+offset))->hash_id, sm_name, 
						((struct main_sm_info*)(buf+offset))->home_node, ((struct main_sm_info*)(buf+offset))->size);
			hashTableInsert(g_group.sm_table, (struct hashheader*)find);
		}
		//copy 
		find->count = ((struct main_sm_info*)(buf+offset))->count;
		//copy queue info
		users_count = ((struct main_sm_info*)(buf+offset))->users_count;
		offset += sizeof(struct main_sm_info)+strlen(sm_name);
		if(find->users == NULL)
			find->users = tableCreate(MAX_NODE_NUM, sizeof(unsigned int));
		for(j=0; j< users_count; j++){
			node_id = *((unsigned int*)(buf+offset));
			if(searchNode(find->users, src_node)==-1){
				index = tableGetEmpty(find->users);
				if(index == -1)
					return -1;
				tableAdd(find->users, (void*)&src_node, index);
			}
			offset += sizeof(unsigned int);
		}
	}	
	g_group.coordinator.main_id = g_group.node_id;
	((struct request_header*)buf)->msg_type = MSG_IMAIN_READY;
	((struct request_header*)buf)->seq_number = seq_number;
	sendTo(src_node ,buf, sizeof(struct request_header));
	mem_free(buf);
	return 1;
}



