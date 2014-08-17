#include <stdio.h>
#include <stdlib.h>
#include "../d2mce.h"
int main(int argc, char *argv[]){
	int i;
	double time;
	d2mce_mutex_t m1;
	d2mce_mutex_t m2;
	d2mce_barrier_t b1;
	char input;
	int times = atoi(argv[1]);
	int nodes = atoi(argv[2]);
	int node_id;

	d2mce_init();
	node_id = d2mce_join("mutex","eps", 0);
	d2mce_barrier_init(&b1, "b1");
	d2mce_mutex_init(&m1, "m1");
	d2mce_mutex_init(&m2, "m2");
	d2mce_barrier(&b1, nodes);
	time = -d2mce_time();
	for(i=0;i<times;i++){
		//if(node_id == 1){
		if(d2mce_mutex_lock(&m1)<0)
			printf("lock error\n");
		if(d2mce_mutex_unlock(&m1)<0)
			printf("unlock error\n");
        if(d2mce_mutex_lock(&m2)<0)
            printf("lock error\n");
        if(d2mce_mutex_unlock(&m2)<0)
            printf("unlock error\n");

//		}
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
