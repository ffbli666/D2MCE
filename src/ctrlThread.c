#include "ctrlThread.h"

#define MSG_TYPE *((unsigned short*)buf)
pthread_mutex_t lock_ctrl_init;

int ctrl_init(){
	//init ctrl_queue
	pthread_mutex_lock(&lock_ctrl_init);
	if(g_ctrl_info.count == 0){		
		sem_init(&g_ctrl_info.sem, 0, 0);
		g_ctrl_info.wqueue = wqueueCreate( CONTROL_WQUEUE_SIZE);
	}
	g_ctrl_info.count++;
	pthread_mutex_unlock(&lock_ctrl_init);
	return 1;
}
int ctrl_destroy(){
	pthread_mutex_lock(&lock_ctrl_init);	
	if( g_ctrl_info.count == 1){
		sem_destroy(&g_ctrl_info.sem);		
		wqueueDestroy(g_ctrl_info.wqueue);
//		printf("ctrlThread data destroy ok!\n");
	}
	g_ctrl_info.count--;
	pthread_mutex_unlock(&lock_ctrl_init);
	return -1;
}


void *ctrlThread(void *argv){
	void *buf;
//	ctrl_init();
	sem_post(&g_thread_sem);
	while(1){
		sem_wait(&g_ctrl_info.sem);
		pthread_mutex_lock(&g_ctrl_info.lock);
		buf = wqueuePop(g_ctrl_info.wqueue);
		pthread_mutex_unlock(&g_ctrl_info.lock);
		if(buf == NULL){
			printf("Error:ctrlThread\n");
			continue;
		}

		//process
		if( MSG_TYPE == MSG_D2MCE_TERMINAL){
			mem_free(buf);
			break;
		}
		//JOIN
		else if(MSG_TYPE == MSG_JOIN_GRANT){
			pthread_mutex_lock(&g_group.coordinator.main_lock);
			if(g_group.coordinator.main_id == g_group.node_id){
				join_grant_reply(buf);
				pthread_mutex_unlock(&g_group.coordinator.main_lock);

			}
			else{
				pthread_mutex_unlock(&g_group.coordinator.main_lock);
				forward(g_group.coordinator.main_id, buf);
			}
		}
		else if(MSG_TYPE == MSG_JOIN_NEWNODE_ADD){
			join_newNodeAdd((struct join_c2c_newNode*)buf);
		}
		else if(MSG_TYPE == MSG_IMAIN_MANAGER){
			pthread_mutex_lock(&g_group.coordinator.main_lock);
			if(g_group.coordinator.main_id == g_group.node_id){
				main_imanager_reply(buf);
				pthread_mutex_unlock(&g_group.coordinator.main_lock);
			}
			else{
				pthread_mutex_unlock(&g_group.coordinator.main_lock);
				forward(g_group.coordinator.main_id, buf);
			}
		}
		else if(MSG_TYPE == MSG_NEWMAIN_MANAGER){
			main_newmanager_reply(buf);
		}
		//exit
		else if(MSG_TYPE == MSG_YOU_MANAGER){
			you_manager_reply(buf);
		}
		else if(MSG_TYPE == MSG_NODE_EXIT){
			node_exit_reply(buf);
		}
		//finalize
		else if(MSG_TYPE == MSG_FINALIZE){
			finalize_reply((struct request_header*)buf);
		}
		//Barrier
		else if(MSG_TYPE == MSG_BARRIER_INIT){
			pthread_mutex_lock(&g_group.coordinator.barrier_lock);
			if(g_group.coordinator.barrier_id == g_group.node_id){
				barrier_init_reply(buf);
				pthread_mutex_unlock(&g_group.coordinator.barrier_lock);				
			}
			else{
				pthread_mutex_unlock(&g_group.coordinator.barrier_lock);				
				forward(g_group.coordinator.barrier_id, buf);
			}
		}
		else if(MSG_TYPE == MSG_BARRIER_REQ){
			pthread_mutex_lock(&g_group.coordinator.barrier_lock);			
			if(g_group.coordinator.barrier_id == g_group.node_id){
				barrier_reply(buf);	
				pthread_mutex_unlock(&g_group.coordinator.barrier_lock);
			}
			else{
				pthread_mutex_unlock(&g_group.coordinator.barrier_lock);				
				forward(g_group.coordinator.barrier_id, buf);
			}
		}
		else if(MSG_TYPE == MSG_IBAR_MANAGER){
			pthread_mutex_lock(&g_group.coordinator.barrier_lock);			
			if(g_group.coordinator.barrier_id == g_group.node_id){
				barrier_imanager_reply(buf);			
				pthread_mutex_unlock(&g_group.coordinator.barrier_lock);
			}
			else{
				pthread_mutex_unlock(&g_group.coordinator.barrier_lock);				
				forward(g_group.coordinator.barrier_id, buf);
			}
		}		
		else if(MSG_TYPE == MSG_NEWBAR_MANAGER){
			barrier_newmanager_reply(buf);
		}
		//Mutex lock
		else if(MSG_TYPE == MSG_MUTEX_INIT){
			pthread_mutex_lock(&g_group.coordinator.mutex_lock);
			if(g_group.coordinator.mutex_id == g_group.node_id){
				mutex_init_reply(buf);
				pthread_mutex_unlock(&g_group.coordinator.mutex_lock);
			}
			else{
				pthread_mutex_unlock(&g_group.coordinator.mutex_lock);
				forward(g_group.coordinator.mutex_id, buf);
			}
		}
		else if(MSG_TYPE == MSG_MUTEX_LOCK){
			pthread_mutex_lock(&g_group.coordinator.mutex_lock);
			if(g_group.coordinator.mutex_id == g_group.node_id){
				mutex_lock_reply(buf);
				pthread_mutex_unlock(&g_group.coordinator.mutex_lock);
			}
			else{
				pthread_mutex_unlock(&g_group.coordinator.mutex_lock);
				forward(g_group.coordinator.mutex_id, buf);
			}
		}		
		else if(MSG_TYPE == MSG_MUTEX_UNLOCK){
			pthread_mutex_lock(&g_group.coordinator.mutex_lock);	
			if(g_group.coordinator.mutex_id == g_group.node_id){
				mutex_unlock_reply(buf);
				pthread_mutex_unlock(&g_group.coordinator.mutex_lock);	
			}
			else{
				pthread_mutex_unlock(&g_group.coordinator.mutex_lock);	
				forward(g_group.coordinator.mutex_id, buf);
			}
		}		
		else if(MSG_TYPE == MSG_IMUTEX_MANAGER){
			pthread_mutex_lock(&g_group.coordinator.mutex_lock);	
			if(g_group.coordinator.mutex_id == g_group.node_id){
				pthread_mutex_unlock(&g_group.coordinator.mutex_lock);	
				mutex_imanager_reply(buf);			
			}
			else{
				pthread_mutex_unlock(&g_group.coordinator.mutex_lock);	
				forward(g_group.coordinator.mutex_id, buf);
			}
		}		
		else if(MSG_TYPE == MSG_NEWMUTEX_MANAGER){
			mutex_newmanager_reply(buf);
		}
		//Semaphore
		else if(MSG_TYPE == MSG_SEM_INIT){
			pthread_mutex_lock(&g_group.coordinator.sem_lock);
			if(g_group.coordinator.sem_id == g_group.node_id){
				sem_init_reply(buf);			
				pthread_mutex_unlock(&g_group.coordinator.sem_lock);
			}
			else{
				pthread_mutex_unlock(&g_group.coordinator.sem_lock);
				forward(g_group.coordinator.sem_id, buf);
			}
		}
		else if(MSG_TYPE == MSG_SEM_POST){
			pthread_mutex_lock(&g_group.coordinator.sem_lock);
			if(g_group.coordinator.sem_id == g_group.node_id){
				sem_post_reply(buf);
				pthread_mutex_unlock(&g_group.coordinator.sem_lock);
			}
			else{
				pthread_mutex_unlock(&g_group.coordinator.sem_lock);
				forward(g_group.coordinator.sem_id, buf);
			}
		}		
		else if(MSG_TYPE == MSG_SEM_WAIT){
			pthread_mutex_lock(&g_group.coordinator.sem_lock);
			if(g_group.coordinator.sem_id == g_group.node_id){
				sem_wait_reply(buf);
				pthread_mutex_unlock(&g_group.coordinator.sem_lock);
			}
			else{
				pthread_mutex_unlock(&g_group.coordinator.sem_lock);
				forward(g_group.coordinator.sem_id, buf);
			}
		}		
		else if(MSG_TYPE == MSG_ISEM_MANAGER){
			pthread_mutex_lock(&g_group.coordinator.sem_lock);
			if(g_group.coordinator.sem_id == g_group.node_id){
				sem_imanager_reply(buf);
				pthread_mutex_unlock(&g_group.coordinator.sem_lock);
			}
			else{
				pthread_mutex_unlock(&g_group.coordinator.sem_lock);
				forward(g_group.coordinator.sem_id, buf);
			}
		}		
		else if(MSG_TYPE == MSG_NEWSEM_MANAGER){
			sem_newmanager_reply(buf);
		}
		//share memory 
/*
		else if(MSG_TYPE == MSG_SM_INIT){
			sm_init_reply(buf);
		}
		else if(MSG_TYPE == MSG_SM_READMISS){
			sm_readmiss_reply(buf);
		}
		else if(MSG_TYPE == MSG_SM_FETCH){
			sm_fetch_reply(buf);
		}
		else if(MSG_TYPE == MSG_SM_WRITEMISS || MSG_TYPE == MSG_SM_WRITEMISS_NOREPLY){
			sm_writemiss_reply(buf);
		}
		else if(MSG_TYPE == MSG_SM_INVALID || MSG_TYPE == MSG_SM_INVALID_NOREPLY){
			sm_invalid_reply(buf);
		}
		else if(MSG_TYPE == MSG_SM_INVALID_OK){
			sm_invalid_ok(buf);
		}
		else if(MSG_TYPE == MSG_SM_MULTIREAD){
			sm_multiread_reply(buf);
		}
		else if(MSG_TYPE == MSG_SM_MULTIWRITE){
			sm_multiwrite_reply(buf);
		}		
		else if(MSG_TYPE == MSG_SM_IAMHOME){
			sm_sethome_reply(buf);
		}
		else if(MSG_TYPE == MSG_SM_NEWHOME){
			sm_newhome_reply(buf);
		}
		else if(MSG_TYPE == MSG_SM_HOME_READY){
			sm_homeready_reply(buf);
		}
#ifdef CONF_SM_DISSEMINATE_UPDATE
		else if(MSG_TYPE == MSG_SM_UPDATE_REGISTER){
			sm_update_register_reply(buf);
		}
		else if (MSG_TYPE == MSG_SM_UPDATE_UNREGISTER) {
			sm_update_unregister_reply(buf);
		}
		else if(MSG_TYPE == MSG_SM_UPDATE){
			sm_update_reply(buf);
		}
#endif
#ifdef CONF_SM_AUTO_HOME_MIGARATION
		else if(MSG_TYPE == MSG_SM_UHOME){
			sm_uhome_reply(buf);
		}
#endif
*/
		mem_free(buf);
	}
//	ctrl_destroy();
	return NULL;
}

int ctrlThread_close(){
	void* buf;
	buf = mem_malloc(64);
	((struct request_header*)buf)->msg_type = MSG_D2MCE_TERMINAL;
	pthread_mutex_lock(&g_ctrl_info.lock);
	wqueuePush(g_ctrl_info.wqueue, buf);
	pthread_mutex_unlock(&g_ctrl_info.lock);
	sem_post(&g_ctrl_info.sem);
	return 1;
}

