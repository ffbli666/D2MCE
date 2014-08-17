#include "thread.h"

struct table *g_thread_table;

int addReceiver(int i);
int addSender(int i);
int addCtrl(int i);
int addData(int i);


int thread_init(){
	int i;
	g_thread_table = tableCreate(THREAD_NUM, sizeof(struct thread));

	addSender(0);
	addReceiver(1);
	addCtrl(2);
	addData(3);

	g_recv_info.count = 0;
	g_ctrl_info.count = 0;
	g_data_info.count = 0;
	g_sender_info.count = 0;
	ctrl_init();
	data_init();
	sem_init(&g_thread_sem, 0, 0);
	for(i=0;i<g_thread_table->use;i++)
		threadStart(i);
	for(i=0;i<g_thread_table->use;i++)
		sem_wait(&g_thread_sem);
	return 1;
}
int thread_destroy(){	
	int i;		
	struct thread **thread_table;
	thread_table = (struct thread**)getTable(g_thread_table);
	for(i=(g_thread_table->use-1); i>=0; i--)
		thread_table[i]->close();
	for(i=0; i<g_thread_table->use; i++)
		threadJoin(i);
	data_destroy();
	ctrl_destroy();
	return 1;
}

int threadStart(int index){
	struct thread *thread;
	thread = (struct thread*)tableGetRow(g_thread_table, index);
	if(thread->enable == YES)
		return -1;
	else{
		thread->enable = YES;
		pthread_create( &thread->pid, NULL, thread->start, (void*)index);
	}
	return 1;
}

int threadKill(int index){
	struct thread *thread;
	thread = (struct thread*)tableGetRow(g_thread_table, index);
	if(thread->enable == NO)
		return -1;
	else{
		thread->enable = NO;
		pthread_kill(thread->pid, SIGKILL);	
	}
	return 1;
}


int threadJoin(int index){
	struct thread *thread;
	thread = (struct thread*)tableGetRow(g_thread_table, index);
	if(thread->enable == NO)
		return -1;
	else{
		thread->enable = NO;		
		pthread_join(thread->pid, NULL);
	}
	return 1;
}

int addReceiver(int i){
	struct thread *thread;
	thread = (struct thread*)tableGetRow(g_thread_table, i);
	thread->enable = NO;
	thread->who = RECEIVER_TID;
	thread->start = receiver;
	thread->close = receiver_close;
	g_thread_table->use++;
	return 1;
}
int addSender(int i){
	struct thread *thread;
	thread = (struct thread*)tableGetRow(g_thread_table, i);
	thread->enable = NO;
	thread->who = SENDER_TID;
	thread->start = sender;
	thread->close = sender_close;
	g_thread_table->use++;
	return 1;
}

int addCtrl(int i){
	struct thread *thread;
	thread = (struct thread*)tableGetRow(g_thread_table, i);
	thread->enable = NO;
	thread->who = CTRL_TID;
	thread->start = ctrlThread;
	thread->close = ctrlThread_close;
	g_thread_table->use++;
	return 1;
}
int addData(int i){
	struct thread *thread;
	thread = (struct thread*)tableGetRow(g_thread_table, i);
	thread->enable = NO;
	thread->who = DATA_TID;
	thread->start = dataThread;
	thread->close = dataThread_close;
	g_thread_table->use++;
	return 1;
}


