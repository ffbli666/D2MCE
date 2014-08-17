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
	int *b;
	int *c;
	d2mce_init();
	node_id = d2mce_join("ibarrier", "eps", 0);
//	if(node_id ==1)
//		d2mce_ibarrier_manager();
	d2mce_barrier_init(&b1, "b1");
	d2mce_mutex_init(&m1, "m1");
	d2mce_sem_init(&s1, "s1", 0);
//	if(node_id ==1)
//		d2mce_ibarrier_manager();

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
	time = -d2mce_time();
	for(i=0;i<times;i++){
		d2mce_barrier(&b1, nodes);
		if(i==10 && node_id ==1)
			d2mce_set_barrier_manager();
//		else if(i==10)
//			sleep(5);
		if(node_id == 1){
			for(j=0;j<10;j++){
				d2mce_load(A);
				d2mce_store(A);
			}
		}
		else{
			d2mce_load(A);
		}
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
