#include <stdio.h>
#include <stdlib.h>
#include "../d2mce.h"

int main(int argc, char *argv[]){
	int i,j;
	double time;
	d2mce_barrier_t b1;
	d2mce_mutex_t m1;
	d2mce_sem_t s1;
	int times = atoi(argv[1]);
	int nodes = atoi(argv[2]);
	int sizes = atoi(argv[3]);
	int node_id;
	int *A;

	d2mce_init();
	node_id = d2mce_join("fetch", "eps", 0);
	d2mce_barrier_init(&b1, "b1");
	d2mce_mutex_init(&m1, "m1");
	d2mce_sem_init(&s1, "s1", 0);
	A = d2mce_malloc("A", sizeof(int)*sizes*sizes);
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
	d2mce_barrier(&b1, nodes);
	time = -d2mce_time();
	if(node_id == 1){
		for(i=0;i<sizes;i++)
			A[i]=i+100;
		d2mce_store(A);
		for(i=1;i<nodes;i++)
			d2mce_sem_post(&s1);
	}
	else if(node_id == 0){
		d2mce_sem_wait(&s1);
	}
	else{
		d2mce_sem_wait(&s1);
		d2mce_load(A);
	}
	if(node_id == 1){
		for(i=0;i<sizes;i++)
			A[i+sizes]=i+50;
		d2mce_store(A);
		d2mce_barrier(&b1, nodes);
	}
	else{
		d2mce_barrier(&b1, nodes);
		d2mce_load(A);
 	}
	time += d2mce_time();
	
	int check=0;
	for(i=0;i<sizes;i++){
//		printf("%d", A[i]);
		if(A[i]!=i+100)
			check=1;
	}
    for(i=0;i<sizes;i++){
//		printf("%d", A[i]);
		if(A[i+sizes]!=i+50)
			check=1;
	}

	for(i=2;i<sizes;i++){
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
