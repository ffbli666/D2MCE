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
	TYPE temp;
	TYPE *col;
	TYPE *row;
	TYPE *matrix;
	TYPE *U;
	TYPE *L;
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
	col = d2mce_malloc("col", sizeof(TYPE)*n);
	row = d2mce_malloc("row", sizeof(TYPE)*n);
	U = malloc(sizeof(TYPE)*n*n);
	L = malloc(sizeof(TYPE)*n*n);
	bzero(U, sizeof(TYPE)*n*n);
	bzero(L, sizeof(TYPE)*n*n);
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
	for (i=0; i<n; i++) 
		L[i*n]=matrix[i*n];
	for (i=0; i<n ;i++) 
		U[i]=matrix[i]/L[0];
	for(j=1; j<n; j++){
		printf("%d\n",j);
		startend(node_id, nodes, j, n, &start, &end);
		if(start == (n+2))
			break;
		//computing L
    	for(i=start; i<end;i++){
	        temp=0.0;
	        for(k=0;k<=j-1;k++){
               temp=temp+L[i*n+k]*U[k*n+j];
            }
           	L[i*n+j]=matrix[i*n+j]-temp;
        }
		//merge L and send to all
		d2mce_mutex_lock(&m1);
		d2mce_load(col);
		for(i=start; i<end;i++){
			col[i] = L[i*n+j];
		}
		d2mce_store(col);
		d2mce_mutex_unlock(&m1);
		if((n-j) > nodes)
			d2mce_barrier(&b1, nodes);
		d2mce_load(col);
        for(i=j; i<n;i++){
            L[i*n+j] = col[i];
        }

		//computing U
	    U[j*n+j]=1.0;
		if(node_id==0)
			start++;
	    for(i=start;i<end;i++){
    	    temp=0.0;
	        for(k=0; k<=j-1;k++){
		        temp=temp+L[j*n+k]*U[k*n+i];
	        }
	        U[j*n+i]=(matrix[j*n+i]-temp)/L[j*n+j];
	    }
		//merge U and send to all
        d2mce_mutex_lock(&m1);
        d2mce_load(row);
        for(i=start; i<end;i++){
            row[i] = U[j*n+i];
        }
        d2mce_store(row);
        d2mce_mutex_unlock(&m1);
		if((n-j) > nodes)
	        d2mce_barrier(&b1, nodes);
		d2mce_load(row);
        for(i=j; i<n;i++){
            U[j*n+i] = row[i];
        }

   	}
	if(node_id == 0){
		//merge U and L	
		for(i=0;i<n;i++){
			for(j=i+1;j<n;j++)
				matrix[i*n+j] = U[i*n+j];
			for(j=i;j<n;j++)
				matrix[j*n+i] = L[j*n+i];
		}
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
	free(U);
	free(L);
	return 1;
}
