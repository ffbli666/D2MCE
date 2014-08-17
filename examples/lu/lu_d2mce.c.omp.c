/*
 	LU decomposition 
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
#define TYPE	double
#define SIZE	1024
#define NODES 1
int print_usage()
{
    printf(" Usage: lu_d2mce [options]\n");
    printf(" Option:\n");
    printf(" \t[ -s <vector size> ] (default: %d)\n", SIZE);
	printf(" \t[ -n <number of nodes> ] (default: %d)\n", NODES);
    return 1;
}
int startend(int myid, int nodes, int i, int totalsize, int *start, int *end){
    int block_size;
    block_size = (totalsize-i) / nodes;
	if((totalsize-i)<=nodes){
		if( myid !=0){
			*start = totalsize+2;
			*end = totalsize+1;
			return 1;
		}else{
			*start = i;
			*end = totalsize;
			return 1;
		}
	}
    if( (totalsize-i) % nodes != 0)
		block_size ++;
//	printf("block %d\n", block_size);
    *start = myid * block_size+i;
    *end = *start + block_size;
    if (*end > totalsize)
        *end = totalsize;
    return 1;
}
int main(int argc, char *argv[])
{
	int n;
	int i,j,k;
	int nodes;
	TYPE *col;
	TYPE *row;
	TYPE *matrix;
	TYPE *_matrix;
	double time;
	FILE *file;
	int node_id;
	int start;
	int end;
    d2mce_barrier_t b1;
    d2mce_mutex_t m1;
	nodes = NODES;
	n =SIZE;
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
    printf("init...\n");
    d2mce_init();
    node_id = d2mce_join("lu","d2mce_orig", 0);
    printf("my node id:%d\n", node_id);
    d2mce_mutex_init(&m1,"m1");
    d2mce_barrier_init(&b1,"b1");
    matrix = d2mce_malloc("matrix", sizeof(TYPE)* n * n);
	_matrix =  malloc( sizeof(TYPE)* n * n);
	col = malloc( sizeof(TYPE)*n);
	row = malloc( sizeof(TYPE)*n);
	if(node_id == 0){
	    file = fopen("input.data","r");
	    if (file==NULL) {
	        printf("can't not open the input.data file\n");
	        return -1;
	    }
	    fread(matrix, sizeof(TYPE), n*n, file);
	    fclose(file);
	}else{
		d2mce_load(matrix);
	}

/*
	matrix[0] =1;
	matrix[1] =1;
	matrix[2] =1;
	matrix[n] = 20;
	matrix[n+1] = 24;
	matrix[n+2] = 30;
	matrix[2*n] = 12;
	matrix[2*n+1] = 10;
	matrix[2*n+2] = 8;
*/
	d2mce_barrier(&b1, nodes);
	printf("d2mce LU decomposition processing...\n");
	/* start to the procedure of LU decomposition */
	time = -d2mce_time();
	for (k = 0; k<n-1; k++) {
		startend(node_id, nodes, k, n, &start, &end);
		if(start == n+2)
			break;
		printf("%d start%d end%d\n", k, start, end);
		for (i = k; i<n; i++) {
	    	col[i] = matrix[i*n+k];
 		}
	    for (i = k+1; n<n; i++) {
    		matrix[k*n+i] /= col[k];
	    }
	    for (i = k+1; i<n; i++) {
    		row[i] = matrix[k*n+i];
 		}
		if(node_id == 0);
			start++;
	    for (i = start ; i<end; i++) {
	   		for (j = k+1; j<n; j++) {
				_matrix[i*n+j] = matrix[i*n+j] - row[i] * col[j];
      		}
    	}
		d2mce_mutex_lock(&m1);
		d2mce_load(matrix);
		for (i = start ; i<end; i++) {
            for (j = k+1; j<n; j++) {
                matrix[i*n+j] = _matrix[i*n+j];
            }
        }
		d2mce_store(matrix);
		d2mce_mutex_unlock(&m1);
		if((n-k) > nodes)
			d2mce_barrier(&b1, nodes);
		d2mce_load(matrix);
	}

	time += d2mce_time();
	printf("result\n");
	printf("\tTIME\tVector_Size\n");
	printf("\t%f\t%d*%d\n", time, n,n);
	if(node_id == 0){
	    file = fopen("d2mce_output.data","w");
	    fwrite(matrix, sizeof(TYPE), n*n, file);
	    fclose(file);
	}
	d2mce_finalize();
/*
    for(i=0;i<n;i++){
        for(j=0;j<n;j++)
			printf("%f ", matrix[i*n+j]);
		printf("\n");
	}
*/
	free(_matrix);
	free(row);
	free(col);
	return 1;
}
