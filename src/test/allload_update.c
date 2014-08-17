#include <stdio.h>
#include <stdlib.h>
#include "../d2mce.h"

#define TYPE int
int main(int argc, char *argv[]){
	int i,j;
	double time;
	d2mce_barrier_t b1;
	d2mce_barrier_t b2;
	d2mce_mutex_t m1;
	d2mce_sem_t s1;
	int times = atoi(argv[1]);
	int nodes = atoi(argv[2]);
	int sizes = atoi(argv[3]);
	int node_id;
	int check=0;
	double time1=0;
	TYPE *A, *B;
	d2mce_init();
	node_id = d2mce_join("load","eps", 0);
	d2mce_barrier_init(&b1, "b1");
	A = d2mce_malloc("A", sizeof(TYPE)*sizes);
	d2mce_update_register(A);
	if(A==NULL)
		printf("malloc error\n");
	d2mce_barrier(&b1, nodes);
	time = -d2mce_time();
	for(i=0;i<times;i++){

//		printf("\n%d\n", i);
		if(node_id == 0){
			for(j=0;j<sizes;j++){
                A[j]=i+j;
            }
			d2mce_acq();
//			time1= -d2mce_time();
      	    d2mce_store(A);
//			time1+= d2mce_time();
	//		time1= -d2mce_time();
			d2mce_barrier(&b1, nodes);
	//		time1+= d2mce_time();
//			usleep(1000);	
//			for(j=1;j<nodes;j++)
//				d2mce_sem_post(&s1);
		}
		else{
//			d2mce_sem_wait(&s1);
	//		 time1= -d2mce_time();
			d2mce_barrier(&b1, nodes);
	//		time1+= d2mce_time();
//			usleep(1000);
//			time1= -d2mce_time();
			d2mce_load(A);
//			time1+= d2mce_time();

          for(j=0;j<sizes;j++){
                if(A[j]!=i+j){
                    printf("error:%d",*A);
                    check =1;
                }
            }

		}
//		printf("%f ", time1);

//	d2mce_barrier(&b1,nodes);
	}
	time +=d2mce_time();
	printf("\ttimes\tnodes\ttime\n");
	printf("\t%d\t%d\t%f\n", times, nodes, time);
	printf("error: %s\n", (check)?"true":"false");
//	sleep(1);
//	print_info();
	print_overhead();
	d2mce_finalize();
	
	return 1;	
}
