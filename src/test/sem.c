#include <stdio.h>
#include <stdlib.h>
#include "../d2mce.h"
int main(int argc, char *argv[]){
	int i,j;
	double time;
	d2mce_barrier_t b1;
	d2mce_sem_t s1;
	char input;
	int times = atoi(argv[1]);
	int nodes = atoi(argv[2]);
	int node_id;

	d2mce_init();
	node_id = d2mce_join("sem","eps", 0);
    if(node_id == (nodes-1))
        printf("my id:%d i am post\n", node_id);
    else
        printf("my id:%d\n", node_id);
	d2mce_barrier_init(&b1, "b1");
	d2mce_sem_init(&s1, "s1", 0);
	d2mce_barrier(&b1, nodes);
	time = -d2mce_time();
	for(i=0;i<times;i++){
		if(node_id == (nodes-1)){
			for(j=1;j<nodes;j++){
				if(d2mce_sem_post(&s1)<0)
					printf("post error\n");
			}
		}
		else{
			if(d2mce_sem_wait(&s1)<0)
				printf("wait error\n");
		}
//		d2mce_barrier(&b1,nodes);
	}
	time +=d2mce_time();
	d2mce_barrier(&b1, nodes);
	printf("\ttimes\tnodes\ttime\n");
	printf("\t%d\t%d\t%f\n", times, nodes, time);
	sleep(1);
	//print_info();
	d2mce_finalize();	
	return 1;	
}
