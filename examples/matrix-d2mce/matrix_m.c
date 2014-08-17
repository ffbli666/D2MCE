#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/d2mce.h"

#define RANDOM_SEED 2882        //random seed
#define NODES 1 	//this is want to test numbers of nodes. if <=1 dynamice joining, if >1 restriction nodes
//#define PRINT_INFO
#define VECTOR_SIZE 512
#define MATRIX_SIZE (VECTOR_SIZE * VECTOR_SIZE)
//#define CHECK

int main(int argc, char *argv[]){
	int node_id;
	int *A, *B, *C, *_C;
	double time;
	int i,j,k;
	int _vector_size = VECTOR_SIZE;
	int _matrix_size = VECTOR_SIZE * VECTOR_SIZE;
	int nodes = NODES;
	int scatter_size;
	int sum;
	if(argc > 1){
		for( i = 1 ; i < argc ;){
			if(strcmp(argv[i],"-n") == 0){
				nodes = atoi(argv[i+1]);
				i+=2;
			}
			else if(strcmp(argv[i],"-s") == 0){
				_vector_size = atoi(argv[i+1]);
				_matrix_size =_vector_size * _vector_size;
				i+=2;
			}
			else{
				printf("the argument only have:\n");
				printf("-n: the nodes of number ex: -n 100\n");
				printf("-s: the size of vector ex: -s 256\n");
				return 0;
			}
		}
	}
	_C=(int *)malloc( sizeof(int) * _vector_size * _vector_size );
	bzero(_C, sizeof(int) * _vector_size * _vector_size);

	if(_vector_size % nodes !=0){
		return 0;
	}
	scatter_size = _vector_size / nodes;
	srand( RANDOM_SEED );

	d2mce_mutex_t mutex;
	d2mce_barrier_t barrier;
/* D2MCE library initialization */
	d2mce_init();
/* Join and get task id */
	node_id = d2mce_join("matrix", "eps308", 0);
/* Lock initialization */
	d2mce_mutex_init(&mutex,"mutex");
	d2mce_barrier_init(&barrier,"barrier");
/* Shared data allocation */
	A = d2mce_malloc( "A", sizeof(int) * _matrix_size);
	B = d2mce_malloc( "B", sizeof(int) * _matrix_size);
	C = d2mce_malloc( "C", sizeof(int) * _matrix_size);
/* Initialization of the matrix A and B by node-0 */
	if (node_id == 0){
/* matrix initialization */
		d2mce_mutex_lock(&mutex);
		for( i=0 ; i<_matrix_size ; i++){
			*(A + i) = rand()%10;
			*(B + i) = rand()%10;
		}
		d2mce_store(A);
		d2mce_store(B);
		d2mce_mutex_unlock(&mutex);
	}
	else { 
/* Load A, B */
		d2mce_mutex_lock(&mutex);

		d2mce_load(A);
		d2mce_load(B);
		d2mce_mutex_unlock(&mutex);
	}
	d2mce_barrier(&barrier, nodes);
//	show_node_table();
	time = -d2mce_time();
/* Local computation */
	for( i = node_id*scatter_size; i < node_id*scatter_size + scatter_size; i++){
		for ( j = 0; j < _vector_size; j++){
			sum = 0;
			for ( k = 0; k < _vector_size; k++){
				sum += A[i*_vector_size+k] * B[k*_vector_size+j];
			}
//			_C[i*(_vector_size) + j] = sum;
			C[i*_vector_size+j] = sum;
		}
	}
//	d2mce_mutex_lock(&mutex);
//	d2mce_load(C);
/*
	for( i = node_id*scatter_size ; i < node_id*scatter_size + scatter_size; i++){
		for( j = 0 ; j < _vector_size ; j++){
			C[i*_vector_size+j]=_C[i*(_vector_size) + j];
		}
	}
*/
	d2mce_mstore(C,  node_id*scatter_size*_vector_size*sizeof(int), scatter_size*_vector_size*sizeof(int));
//	d2mce_mutex_unlock(&mutex);

	d2mce_barrier(&barrier, nodes);
	if(node_id == 0)
		d2mce_load(C);
	time += d2mce_time();

#ifdef PRINT_INFO
	for(i =0 ; i <_matrix_size;i++){
		printf("%d ",C[i]);
		if( (i+1)%_vector_size == 0)
			printf("\n");
	}

#endif

#ifdef CHECK
	if(node_id == 0){
	int error=0;
    bzero(_C, sizeof(int) * _vector_size * _vector_size);

	for( i = 0; i < _vector_size; i++){
		for ( j = 0; j < _vector_size; j++){
			sum = 0;
			for ( k = 0; k < _vector_size; k++){
				sum += A[i*_vector_size+k] * B[k*_vector_size+j];
			}
			_C[i*(_vector_size) + j] = sum;
		}
	}
	for( i = 0; i < _matrix_size; i++){
		if(_C[i] != C[i]){
			error=1;
			printf("%d %d\n", _C[i], C[i]);
			break;
		}
	}

	printf("error: %s\n", (error)?"true":"false");
	}
#endif
	if (node_id == 0){
/* print the result of the matrix initialization */
		printf("\nVector Size:%d\n", _vector_size);
		printf("Matrix size:%d\n", _matrix_size);
		printf("Processing time:%f\n", time);
	}
/* Terminate D2MCE environment */
	print_overhead();
	d2mce_finalize();	
	return 0;
}
