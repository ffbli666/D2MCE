#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include "../pointtable.h"

struct pointtable *tb;
sem_t gsem;
struct abc{
	int a;
	int b;
	sem_t sem;
};

void set(int i){
	struct abc a;
	int index;
	a.a=1*i;
	a.b=2*i;
	sem_init(&a.sem, 0, 0);
	index = pointTableGetEmpty(tb);
	pointTableAttach(tb, &a, index);
	sem_post(&gsem);
	sem_wait(&a.sem);
}

void *get(){
	struct abc *a;
	int i;
	for(i=0;i<100;i++){
		sem_wait(&gsem);
		a = pointTableDetach(tb,0);
		printf("h%d:%d %d\n",i, a->a, a->b);
		if(a==NULL)
			printf( "h%d:null\n",i);
		sem_post(&a->sem);
	}
	return NULL;
}
void *another(){
	int i;
	for(i=50;i<100;i++){
		set(i);
	}	
	return NULL;
}

int main(int argc, char* argv[]){
	struct abc *a;
	int i;
	pthread_t p1,p2;
	sem_init(&gsem,0,0);

	tb = pointTableCreate(10);
	pthread_create(&p1, NULL, get, NULL);
	pthread_create(&p2, NULL, another, NULL);
	for(i=0;i<50;i++){
		set(i);
	}

	return 1;
}
