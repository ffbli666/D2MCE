#include "finalize.h"

struct finalize_info final;
sem_t final_sem;


int finalize_init();
int finalize_destroy();

int finalize_init(){
	if(final.node == NULL){
		final.count = 0;
		final.node = tableCreate(MAX_NODE_NUM, sizeof(struct finalize_node));
		sem_init(&final_sem, 0, 0);
	}
	return 1;	
}

int finalize_destroy(){
	final.count = 0;
	sem_destroy(&final_sem);
	tableDestroy(final.node);
	return 1;
}

int finalize_req(){
	struct request_header req;
	int i;
	struct finalize_node *get;
	struct request_header reply;
	//main node
	if(g_group.node_id == g_group.coordinator.main_id){
		pthread_mutex_lock(&final.lock);
		finalize_init();
		final.count++;
		if(final.count == g_group.node_num){
			reply.msg_type = MSG_FINALIZE_OK;
			for(i=0; i<final.node->use; i++){
				get = (struct finalize_node*)tableGetRow(final.node, i);
				reply.seq_number = get->seq_number;
				sendTo(get->id , (void*)&reply, sizeof(struct request_header));
			}
			finalize_destroy();
			pthread_mutex_unlock(&final.lock);
		}
		else{
			pthread_mutex_unlock(&final.lock);
			sem_wait(&final_sem);
		}		
		return 1;
	}
	//not main node
//	sleep(1);
	req.msg_type = MSG_FINALIZE;
	sendRecv(g_group.coordinator.main_id, (void*)&req, sizeof(struct request_header),
		(void*)&req, sizeof(struct request_header));
//	printf("finalize_ok!\n");
	return 1;
}

int finalize_reply(struct request_header *request){
	int i;
	struct finalize_node node;
	struct finalize_node *get;
	struct request_header reply;
	pthread_mutex_lock(&final.lock);
	finalize_init();
	node.id = request->src_node;
	node.seq_number= request->src_seq_number;
	tableAdd(final.node, (void*)&node, final.node->use);
	final.count++;
	if(final.count == g_group.node_num){
		reply.msg_type = MSG_FINALIZE_OK;
		for(i=0; i<final.node->use; i++){
			get = (struct finalize_node*)tableGetRow(final.node, i);
			reply.seq_number = get->seq_number;
			sendTo(get->id , (void*)&reply, sizeof(struct request_header));
		}
		sem_post(&final_sem);
		finalize_destroy();
	}
	pthread_mutex_unlock(&final.lock);
	return 1;
}

