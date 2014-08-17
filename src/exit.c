#include "exit.h"


int chose_one(int inode_id){
	unsigned int node = -1;
	srand(time(NULL));
	while(node != inode_id && node != -1 )
		node = rand()%g_group.node_num;
	return node;
}

int you_manager_reply(void *request){
	void *buf;
	if(((struct umanager_req*)request)->manager.main == YES)
		main_imanager_req();
	if(((struct umanager_req*)request)->manager.mutex == YES)
		mutex_imanager_req();
	if(((struct umanager_req*)request)->manager.sem == YES)
		sem_imanager_req();
	if(((struct umanager_req*)request)->manager.barrier == YES)
		barrier_imanager_req();
	buf = mem_malloc(sizeof(struct request_header));
	((struct request_header*)buf)->msg_type = MSG_YOU_OK;
	((struct request_header*)buf)->seq_number = ((struct umanager_req*)request)->req.src_seq_number;
	sendTo(((struct umanager_req*)request)->req.src_node, buf, sizeof(struct request_header));	
	mem_free(buf);
	return 1;
}

int exit_manager_req(){
	int i;
	char move = NO;
	void *buf;
	unsigned int sendnode;
	struct node *node;
	buf = mem_malloc(sizeof(struct umanager_req));
	bzero(buf, sizeof(struct umanager_req));
	//send to the chose node , that will do the original node's manager
	if(g_group.coordinator.main_id == g_group.node_id){
		((struct umanager_req*)buf)->manager.main = YES;
		move = YES;
	}
	if(g_group.coordinator.mutex_id == g_group.node_id){
		((struct umanager_req*)buf)->manager.mutex = YES;
		move = YES;
	}
	if(g_group.coordinator.sem_id == g_group.node_id){
		((struct umanager_req*)buf)->manager.sem = YES;
		move = YES;
	}
	if(g_group.coordinator.barrier_id == g_group.node_id){
		((struct umanager_req*)buf)->manager.barrier = YES;
		move = YES;
	}
	if(move == YES){
	    sendnode = chose_one(g_group.node_id);
	    ((struct umanager_req*)buf)->req.msg_type = MSG_YOU_MANAGER;
		sendRecv(sendnode, buf, sizeof(struct umanager_req), buf, sizeof(struct request_header));
	}
	//send to other node to notice this node will exit
	((struct request_header*)buf)->msg_type = MSG_NODE_EXIT;
	for( i=0; i<g_group.node_table->table_size; i++){
		node =(struct node*)tableGetRow(g_group.node_table, i);
		if(node->id != -1 && node->id != g_group.node_id){
			sendTo(node->id , buf, sizeof(struct request_header));
		}
	}
	mem_free(buf);
	return 1;
}

int node_exit_reply(void *request){
	remove_node(((struct request_header*)request)->src_node);
	g_group.node_num--;
	return 1;
}

int remove_node(unsigned int node_id){
	struct node *node;
	node = g_group.node_table->row[node_id];
	if(node->recv_fd >= 0)
		close(node->recv_fd);
	if(node->send_fd >= 0)
		close(node->send_fd);
	tableRemove(g_group.node_table, node_id);
	return 1;
}

