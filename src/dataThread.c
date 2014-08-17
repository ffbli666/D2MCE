#include "dataThread.h"

#define MSG_TYPE *((unsigned short*)buf)
pthread_mutex_t lock_data_init;

int data_init()
{
	//init data_wqueue
	pthread_mutex_lock(&lock_data_init);
	if (g_data_info.count == 0) {
		sem_init(&g_data_info.sem, 0, 0);
		g_data_info.wqueue = wqueueCreate( DATA_WQUEUE_SIZE);
	}
	g_data_info.count++;
	pthread_mutex_unlock(&lock_data_init);
	return 1;
}
int data_destroy()
{
	pthread_mutex_lock(&lock_data_init);
	if (g_data_info.count == 1) {
		sem_destroy(&g_data_info.sem);
		wqueueDestroy(g_data_info.wqueue);
//		printf("dataThread data destroy ok!\n");
	}
	g_data_info.count--;
	pthread_mutex_unlock(&lock_data_init);
	return -1;
}


void *dataThread(void *argv)
{
	void *buf;
//	data_init();
	sem_post(&g_thread_sem);
	while (1) {
		sem_wait(&g_data_info.sem);
		pthread_mutex_lock(&g_data_info.lock);
		buf = wqueuePop(g_data_info.wqueue);
		pthread_mutex_unlock(&g_data_info.lock);
		if (buf == NULL) {
			printf("Error:dataThread\n");
			continue;
		}
		//process
		if ( MSG_TYPE == MSG_D2MCE_TERMINAL) {
			mem_free(buf);
			break;
		}
		//share memory
		else if (MSG_TYPE == MSG_SM_INIT) {
			sm_init_reply(buf);
		} else if (MSG_TYPE == MSG_SM_READMISS) {
			sm_readmiss_reply(buf);
		} else if (MSG_TYPE == MSG_SM_FETCH) {
			sm_fetch_reply(buf);
		} else if (MSG_TYPE == MSG_SM_WRITEMISS || MSG_TYPE == MSG_SM_WRITEMISS_NOREPLY) {
			sm_writemiss_reply(buf);
		} else if (MSG_TYPE == MSG_SM_INVALID || MSG_TYPE == MSG_SM_INVALID_NOREPLY) {
			sm_invalid_reply(buf);
		} else if (MSG_TYPE == MSG_SM_INVALID_OK) {
			sm_invalid_ok(buf);
		} else if (MSG_TYPE == MSG_SM_MULTIREAD) {
			sm_multiread_reply(buf);
		} else if (MSG_TYPE == MSG_SM_MULTIWRITE) {
			sm_multiwrite_reply(buf);
		} else if (MSG_TYPE == MSG_SM_IAMHOME) {
			sm_sethome_reply(buf);
		} else if (MSG_TYPE == MSG_SM_NEWHOME) {
			sm_newhome_reply(buf);
		} else if (MSG_TYPE == MSG_SM_HOME_READY) {
			sm_homeready_reply(buf);
		}
#ifdef CONF_SM_DISSEMINATE_UPDATE
		else if (MSG_TYPE == MSG_SM_UPDATE_REGISTER) {
			sm_update_register_reply(buf);
		} else if (MSG_TYPE == MSG_SM_UPDATE_UNREGISTER) {
			sm_update_unregister_reply(buf);
		} else if (MSG_TYPE == MSG_SM_UPDATE) {
			sm_update_reply(buf);
		}
#endif
#ifdef CONF_SM_AUTO_HOME_MIGARATION
		else if (MSG_TYPE == MSG_SM_UHOME) {
			sm_uhome_reply(buf);
		}
#endif

		//free buf
		mem_free(buf);
	}
//	data_destroy();
	return NULL;
}

int dataThread_close()
{
	void *buf;
	buf = mem_malloc(64);
	((struct request_header*)buf)->msg_type = MSG_D2MCE_TERMINAL;
	pthread_mutex_lock(&g_data_info.lock);
	wqueuePush(g_data_info.wqueue, buf);
	pthread_mutex_unlock(&g_data_info.lock);
	sem_post(&g_data_info.sem);
	return 1;
}

