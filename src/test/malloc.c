#include <stdio.h>
#include <stdlib.h>
#include "../d2mce.h"
int main(int argc, char *argv[]){
	int i;
	double time;
	d2mce_barrier_t b1;
	char input;
	int times = atoi(argv[1]);
	int nodes = atoi(argv[2]);
	int node_id;
	int *a;
	d2mce_init();
	node_id = d2mce_join("barrier","eps", 0);
	d2mce_barrier_init(&b1, "b1");
	a = d2mce_malloc("a", 100);
	d2mce_load(a);

	time = -d2mce_time();
//	for(i=0;i<times;i++){
//		if(d2mce_barrier(&b1, nodes)<0)
//			printf("barrier error\n");
//	}
	time +=d2mce_time();
//	printf("\ttimes\tnodes\ttime\n");
//	printf("\t%d\t%d\t%f\n", times, nodes, time);
	//print_info();
	sleep(1);
	d2mce_finalize();
	
	return 1;	
}
