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
	TYPE *A;
	int one_size = sizes*sizes/nodes;

	d2mce_init();
	node_id = d2mce_join("load","eps", 0);
	d2mce_barrier_init(&b1, "b1");
	d2mce_mutex_init(&m1,"m1");
	A = d2mce_malloc("AA", sizeof(TYPE)*sizes*sizes);
	if(A==NULL)
		printf("malloc error\n");
	for(i=0;i<sizes*sizes;i++)
		A[i]=0;

	time = -d2mce_time();
	for(i=0;i<times;i++){
		if(node_id == 0){
			for(i=0;i<sizes*sizes;i++){
				A[i]=i;
			}
			d2mce_store(A);
		}
		d2mce_barrier(&b1, nodes);
//	printf("offset=%d length=%d\n", node_id*one_size, one_size);
		d2mce_mload(A, node_id*one_size*sizeof(TYPE), one_size*sizeof(TYPE));
	}
	time +=d2mce_time();
	int check=0;
	int a=0;
	for(i=node_id*one_size; i<node_id*one_size+one_size; i++){
		if(A[i]!=i){
			check=1;
			printf("%d=%d\n", i, A[i]);
			break;
		}
	}
/*   	for(i=0;i<sizes*sizes;i++){
        printf("%d=%d\n", i, A[i]);
	}
*/
	printf("\ttimes\tnodes\ttime\n");
	printf("\t%d\t%d\t%f\n", times, nodes, time);
	printf("error: %s %d i=%d\n", (check)?"true":"false", a, i);
//	sleep(1);
//	print_info();
	d2mce_finalize();
	
	return 1;	
}
