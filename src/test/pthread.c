#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "../d2mce.h"
int *A;
int nodes;
d2mce_mutex_t m1;
d2mce_barrier_t b1;

void* thread(void* argv){
	int i;
	int times = *((int*)argv);
	d2mce_barrier(&b1, nodes*2);
    for(i=0;i<times;i++){
        d2mce_mutex_lock(&m1);
        d2mce_load(A);
        d2mce_store(A);
        d2mce_mutex_unlock(&m1);
    }
	return NULL;
}
int main(int argc, char *argv[]){
	int i,j;
	double time;
	d2mce_sem_t s1;
	pthread_t pid;
	int times = atoi(argv[1]);
	nodes = atoi(argv[2]);
	int sizes = atoi(argv[3]);
	int node_id;
	int *b;
	int *c;
	d2mce_init();
	node_id = d2mce_join("pthread", "eps", 0);

	d2mce_barrier_init(&b1, "b1");
	d2mce_mutex_init(&m1, "m1");
	d2mce_sem_init(&s1, "s1", 0);
	A = d2mce_malloc("Aaa", sizeof(int)*sizes*sizes);
	b = d2mce_malloc("b", sizeof(int));
	c = d2mce_malloc("ccc", sizeof(int));
	if(A==NULL)
		printf("malloc error\n");
	if(node_id == 0){
		for(i=0;i<sizes;i++){
			for(j=0;j<sizes;j++){
				A[i*sizes+j]=j+i*sizes;
			}
		}
	}
	else{
		if(d2mce_load(A)<0)
			printf("load error\n");
	}
	pthread_create( &pid, NULL, thread, &times);
	d2mce_barrier(&b1, nodes*2);
	time = -d2mce_time();
	for(i=0;i<times;i++){
		d2mce_mutex_lock(&m1);
		d2mce_load(A);
		d2mce_store(A);
		d2mce_mutex_unlock(&m1);
	}
	time += d2mce_time();
	int check=0;
	for(i=1;i<sizes;i++){
		for(j=0;j<sizes;j++){
			if(A[i*sizes+j]!=j+i*sizes){
				check=1;
				break;
			}
		}
		if(check)
			break;
	}
	printf("\ttimes\tnodes\ttime\n");
	printf("\t%d\t%d\t%f\n", times, nodes, time);
	printf("error: %s %d\n", (check)?"true":"false", j+i*sizes);
	//print_info();
	sleep(1);
	d2mce_finalize();
	
	return 1;	
}
