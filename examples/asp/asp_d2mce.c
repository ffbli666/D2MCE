/*
    All pairs shortest path (ASP) or Floyd’s Algorithm
	d2mce version
    version: 1.0
    date: 2008/6/18
    author: Zong Ying Lyu
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "../../include/d2mce.h"
#define RANDOM_SEED	3838
#define SIZE 1024
#define NODES 1
//#define OPENMP
int print_usage()
{
	printf(" Usage: asp_d2mce [options]\n");
	printf(" Option:\n");
	printf(" \t[ -s <vector size> ] (default: %d)\n", SIZE);
	printf(" \t[ -n <number of nodes> ] (default: %d)\n", NODES);

	return 1;
}

int main(int argc, char *argv[])
{
	int i,j,k;
	int n;
	int *data;
	int *_data;
	int *tmp;
	FILE *file;
	double time;
	int node_id;
	int nodes;
	int block_size;
	int root;
	d2mce_barrier_t b1;
	d2mce_mutex_t m1;

	n=SIZE;
	nodes = NODES;
	if (argc > 1) {
		for ( i = 1 ; i < argc ;) {
			if (strcmp(argv[i],"-s") == 0) {
				n = atoi(argv[i+1]);
				i+=2;
			} else if (strcmp(argv[i],"-n") == 0) {
				nodes = atoi(argv[i+1]);
				i+=2;
			} else {
				print_usage();
				return 0;
			}
		}
	}
	block_size = n / nodes;
	if (n % nodes !=0) {
		printf("size error\n");
		return -1;
	}
	printf("init...\n");
	d2mce_init();
	node_id = d2mce_join("asp","d2mce_orig", 0);
	printf("my node id:%d\n", node_id);
	d2mce_mutex_init(&m1,"m1");
	d2mce_barrier_init(&b1,"b1");
	data = d2mce_malloc("data", sizeof(int)* n * n);
	tmp = d2mce_malloc("tmp", sizeof(int)*n);

	_data = malloc(sizeof(int)*n*n);
	//read data
	if (node_id == 0) {
		file = fopen("input.data","r");
		if (file==NULL) {
			printf("can't not open the input.data file\n");
			return -1;
		}
		fread(data, sizeof(int), n*n, file);
		fclose(file);
	} else {
		d2mce_load(data);
	}
	memcpy(_data, data, sizeof(int)*n*n);
	d2mce_barrier(&b1, nodes);
	//processing
	printf("d2mce ASP processing...\n");
	time = -d2mce_time();

	for (k = 0; k < n; k++) {
		root = k/block_size;
		if(node_id == root){
			for(j=0;j<n;j++)
				tmp[j] = _data[k*n+j];
			d2mce_acq();
			d2mce_store(tmp);
			d2mce_barrier(&b1, nodes);
		}else{
			d2mce_barrier(&b1, nodes);
			d2mce_load(tmp);
		}
#ifdef OPENMP
        #pragma omp parallel for private(j)
#endif
		for (i = node_id * block_size; i < node_id *block_size + block_size; i++){
			for (j = 0; j < n; j++) {
				if (_data[i*n+j] > (_data[i*n+k] + tmp[j]))
					_data[i*n+j] = _data[i*n+k] + tmp[j];
			}
		}
		d2mce_barrier(&b1, nodes);
	}
	//gather

	d2mce_mutex_lock(&m1);
    d2mce_load(data);
#ifdef OPENMP
        #pragma omp parallel for private(j)
#endif
 	for (i = node_id * block_size; i < node_id *block_size + block_size; i++)
	    for (j = 0; j < n; j++)
    	    data[i*n+j] = _data[i*n+j];
    d2mce_store(data);
    d2mce_mutex_unlock(&m1);

//	d2mce_mstore(data,  node_id * block_size * n * sizeof(int), block_size * n * sizeof(int));

    d2mce_barrier(&b1, nodes);
	if(node_id == 0)
		d2mce_load(data);
	time+=d2mce_time();
	printf("Result:\n");
	printf("\tTIME\tVector_Size\n");
	printf("\t%f\t%d\n", time, n);
	if(node_id == 0){
		file = fopen("d2mce_output.data","w");
		fwrite(data, sizeof(int), n*n, file);
		fclose(file);
	}
	print_overhead();
	d2mce_finalize();
	free(_data);
	return 0;
}
