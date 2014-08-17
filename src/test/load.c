#include <stdio.h>
#include <stdlib.h>
#include "../d2mce.h"

#define TYPE int
int main(int argc, char *argv[]){
	int i,j;
	double time;
	d2mce_barrier_t b1;
	d2mce_mutex_t m1;
	int times = atoi(argv[1]);
	int nodes = atoi(argv[2]);
	int sizes = atoi(argv[3]);
	int node_id;
	int input;
	TYPE *A;
	d2mce_init();
	node_id = d2mce_join("load","eps", 0);
	d2mce_barrier_init(&b1, "b1");
	d2mce_mutex_init(&m1,"m1");
	A = d2mce_malloc("A", sizeof(TYPE)*sizes*sizes);
	if(A==NULL)
		printf("malloc error\n");
	if(node_id == 0){
		for(i=0;i<sizes;i++){
			for(j=0;j<sizes;j++){
				A[i*sizes+j]=1;
			}
		}
	}
	d2mce_barrier(&b1, nodes);
	
	time = -d2mce_time();
	for(i=0;i<times;i++){
		d2mce_load(A);
		d2mce_barrier(&b1, nodes);

	}
//	d2mce_barrier(&b1, nodes);
//	d2mce_load(A);
//	d2mce_barrier(&b1, nodes);
	time +=d2mce_time();
	int check=0;
	int a=0;
	for(i=0;i<sizes;i++){
		for(j=0;j<sizes;j++){
			if(A[i*sizes+j]!=1){
				check=1;
				a=j+i*sizes;
				struct join_c2c_newNode *no;
				no = A[i*sizes+j];
			}
			if(check==1)
				printf("%u ", A[i*sizes+j]);
		}
		if(check==1)
			break;
	}
	printf("\ttimes\tnodes\ttime\n");
	printf("\t%d\t%d\t%f\n", times, nodes, time);
	printf("error: %s %d\n", (check)?"true":"false", a);
//	sleep(1);
//	print_info();
	d2mce_finalize();
	
	return 1;	
}
