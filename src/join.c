#include "join.h"

int join_grant_req( unsigned int group_id, unsigned int id, unsigned int ip, unsigned short port){
	int i;
	int recv_len,offset;
	void *buf;
	struct node node;
	struct node *self;
	buf = mem_malloc(JOIN_GRANT_SIZE);
	((struct join_s2c_req*)buf)->req.msg_type = MSG_JOIN_GRANT;
	((struct join_s2c_req*)buf)->ip = g_system_conf.ip;
	((struct join_s2c_req*)buf)->port = g_system_conf.port;
	g_group.group_id = group_id;
	node.id = id;
	node.ip = ip;
	node.port = port;
	node.recv_fd = -1;
	node.send_fd = -1;
	tableAdd( g_group.node_table, (void*)&node, node.id);
	g_group.node_table->index=1;
	recv_len = sendRecv(id , buf, sizeof(struct join_s2c_req), buf, JOIN_GRANT_SIZE);
	if(((struct join_c2s_ok*)buf)->req.msg_type == MSG_JOIN_FAILED){
		mem_free(buf);
		printf("Join failed\n");
		return -1;
	}
	g_group.node_id = ((struct join_c2s_ok*)buf)->node_id;
	g_group.node_num = ((struct join_c2s_ok*)buf)->node_num;		
	g_group.coordinator.main_id = ((struct join_c2s_ok*)buf)->req.src_node;
	g_group.coordinator.mutex_id = ((struct join_c2s_ok*)buf)->mutex_id;
	g_group.coordinator.sem_id = ((struct join_c2s_ok*)buf)->sem_id;
	g_group.coordinator.barrier_id = ((struct join_c2s_ok*)buf)->barrier_id;
	offset = sizeof(struct join_c2s_ok);
	g_group.node_table->use = 0;
	for(i = 0; i < g_group.node_num ; i++)	{
		node.id = ((struct join_node_info*)(buf+offset))->id;
		node.ip = ((struct join_node_info*)(buf+offset))->ip;
		node.port = ((struct join_node_info*)(buf+offset))->port;
		if(i == g_group.node_id){
			self = (struct node*)tableGetRow(g_group.node_table, MAX_NODE_NUM-1);
			node.recv_fd = self->recv_fd;
			node.send_fd = self->send_fd;
			self->id = 0;
			self->recv_fd = 0;
			self->send_fd = 0;
		}
		else if(((struct node*)g_group.node_table->row[i])->id != -1){
			node.recv_fd = ((struct node*)g_group.node_table->row[i])->recv_fd;
			node.send_fd = ((struct node*)g_group.node_table->row[i])->send_fd;
		}
		else{
			node.recv_fd = -1;
			node.send_fd = -1;
		}
		tableAdd(g_group.node_table, (void*)&node, node.id);
		offset += sizeof(struct join_node_info);
	}
	mem_free(buf);
	return g_group.node_id;
}

int join_grant_reply(void *request){
	int i;
	int index;
	int seq_number;
	int offset;
	void *buf;
	struct node node;
	struct node *getnode;
	struct join_c2c_newNode join_newNode;
	index = tableGetEmpty(g_group.node_table);	
	if(index < 0){
		i=MSG_JOIN_FAILED;
		sendNonConnect(((struct join_s2c_req*)request)->req.src_node, ((struct join_s2c_req*)request)->port,
			(void*)&i,sizeof(int));
		return 1;
	}
	//add the node to the node table
	node.id = index;
	node.ip = ((struct join_s2c_req*)request)->ip;
	node.port = ((struct join_s2c_req*)request)->port;
	node.recv_fd = *((int*)(request+sizeof(struct join_s2c_req)));
	node.send_fd = -1;
	seq_number = ((struct join_s2c_req*)request)->req.src_seq_number;
	tableAdd(g_group.node_table, (void*)&node, node.id);
	g_group.node_num++;
	//send other node have new node join
	join_newNode.req.msg_type =	MSG_JOIN_NEWNODE_ADD;
//	join_newNode.req.src_node = g_group.node_id;
	join_newNode.node_info.id = node.id;
	join_newNode.node_info.ip = node.ip;
	join_newNode.node_info.port = node.port;
	for(i=0; i<g_group.node_table->table_size; i++){
		if(i == node.id || i == g_group.node_id)
			continue;
		getnode = tableGetRow(g_group.node_table, i);
		if(getnode->id != -1)
			sendTo( i, (void*)&join_newNode, sizeof(struct join_c2c_newNode));
	}

	//send node table to the new node
	buf = mem_malloc(JOIN_NEWNODE_SIZE);
	((struct join_c2s_ok*)buf)->req.msg_type = MSG_JOIN_OK;
	((struct join_c2s_ok*)buf)->req.seq_number = seq_number;
	((struct join_c2s_ok*)buf)->group_id = g_group.group_id;
	((struct join_c2s_ok*)buf)->node_id = node.id;
	((struct join_c2s_ok*)buf)->node_num = g_group.node_table->use;
	((struct join_c2s_ok*)buf)->mutex_id = g_group.coordinator.mutex_id;
	((struct join_c2s_ok*)buf)->sem_id = g_group.coordinator.sem_id;
	((struct join_c2s_ok*)buf)->barrier_id = g_group.coordinator.barrier_id;
	offset = sizeof(struct join_c2s_ok);
	for( i=0; i<g_group.node_table->table_size; i++){
		getnode = tableGetRow(g_group.node_table, i);
		if(getnode->id != -1){
			((struct join_node_info*)(buf+offset))->id = getnode->id;
			((struct join_node_info*)(buf+offset))->ip = getnode->ip;
			((struct join_node_info*)(buf+offset))->port = getnode->port;
			offset += sizeof(struct join_node_info);
		}
	}
	sendTo( node.id, buf, offset);
	mem_free(buf);
	return 1;
}

int join_newNodeAdd(struct join_c2c_newNode *request){
	struct node node;
	node.id = request->node_info.id;
	node.ip = request->node_info.ip;
	node.port = request->node_info.port;
	node.recv_fd = -1;
	node.send_fd = -1;
	tableAdd(g_group.node_table, (struct node*)&node, node.id);
	g_group.node_num++;
	return 1;
}


