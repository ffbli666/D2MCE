#include "../../src/d2mce.h"
#include <stdio.h>
#include <stdlib.h>
#define RANDOM_SEED 2882        //random seed
#define NODES 1 	//this is want to test numbers of nodes. if <=1 dynamice joining, if >1 restriction nodes
//#define PRINT_INFO
//#define CHECK
#define VECTOR_SIZE 512
#define MATRIX_SIZE (VECTOR_SIZE * VECTOR_SIZE)
#define CHUNK_SIZE 12
//#define OPENMP
int main(int argc, char *argv[]){
	int task_id, local_index = 0 ;
	int *A, *B, *C, *_C, *schedule_index , *initialized = 0;
	double time;
	int i,j,k;
	int _vector_size = VECTOR_SIZE;
	int _matrix_size = VECTOR_SIZE * VECTOR_SIZE;
	int nodes = NODES;
	int chunk_size = CHUNK_SIZE;
	int start;
	int end;
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
	_C=(int *)malloc( sizeof(int) * _vector_size * (1+_vector_size) );
	bzero(_C, sizeof(int) * _vector_size * (1+_vector_size));
 	srand( RANDOM_SEED );

	d2mce_mutex_t mutex;
	d2mce_barrier_t barrier;
/* D2MCE library initialization */
	if (d2mce_init() <0 ){
		printf("d2mce init failured\n");
		return -1;
	}
/* Join and get task id */
  	task_id = d2mce_join("dynamic matrix", "abc", 0);

/* Lock initialization */
	d2mce_mutex_init(&mutex,"mutex");
	d2mce_barrier_init(&barrier,"barrier");
/* Shared data allocation */
	A = d2mce_malloc( "A", sizeof(int)* _matrix_size);
	B = d2mce_malloc( "B", sizeof(int)* _matrix_size);
	C = d2mce_malloc( "C", sizeof(int)* _matrix_size);
	schedule_index = d2mce_malloc("schedule_index", sizeof(int));
	initialized = d2mce_malloc("initialized", sizeof(int));
/* Initialization of the matrix A and B by node-0 */
	if (task_id == 0){
/* matrix initialization */
		d2mce_mutex_rw(&mutex, 4, A, "rw", B, "rw", initialized, "rw", schedule_index, "rw");
		for( i=0 ; i<_matrix_size ; i++){
        	        *(A + i) = rand()%10;
	                *(B + i) = rand()%10;
                }
/* mark the initialization flag as done */
		*schedule_index = 0;
		*initialized = 1;
 		//d2mce_store(initialized);
 		d2mce_mutex_unlock(&mutex);
 	}
	else { 
/* wait for the matrix  initialization to complete */
		while (! initialized) {
			d2mce_mutex_rw(&mutex, 1,initialized,"r");
			d2mce_mutex_unlock(&mutex);
//			d2mce_load(initialized);
		}
/* Load A, B and schedule_index */
		d2mce_load(A);
		d2mce_load(B);
	}
	//sleep(100);
	if(nodes >1)
		d2mce_barrier(&barrier, nodes);
	time = -d2mce_time();
	while (1) {
/* task scheduling */
		//d2mce_mutex_rw(&mutex, 1, schedule_index, "rw");
		d2mce_mutex_lock(&mutex);
		d2mce_load(schedule_index);
		start = (*schedule_index);
		end = start + chunk_size;
		if(end > _vector_size)
			end = _vector_size;
		(*schedule_index)+=chunk_size;
//		printf("%d %d\n", start ,end);
		d2mce_store(schedule_index);
		d2mce_mutex_unlock(&mutex);
/* all task finished? */
		if (start >= _vector_size) break;
/* Local computation */

#ifdef OPENMP
    #pragma omp parallel for private(sum, j , k)
#endif
		for(i=start; i<end ; i++){
			for ( j = 0; j < _vector_size; j++){
				sum = 0;
				for ( k = 0; k < _vector_size; k++){
					 sum += A[i*_vector_size+k] * B[k*_vector_size+j];
				}
				C[i*(_vector_size) + j] = sum;
/* mark the locally modified row in _C */
				_C[i*(_vector_size+1) + _vector_size] = 1;
			}
		}
	}
/* write back to C from _C */
//	d2mce_mutex_rw(&mutex, 1, C, "rw");
/* apply the locally modified part of _C into C */

#ifdef OPENMP
    #pragma omp parallel for private(sum, j)
#endif
	for( i = 0 ; i < _vector_size ; i++){
		if( _C[i*(_vector_size+1) +_vector_size]){
			/*
			for( j = 0 ; j < _vector_size ; j++){
				C[i*_vector_size+j]=_C[i*(_vector_size+1) + j];
			}*/
			d2mce_mstore(C,  (i*_vector_size)*sizeof(int), _vector_size*sizeof(int));
		}
	}
//	d2mce_mutex_unlock(&mutex);
//	d2mce_barrier("wait", 2);
	d2mce_barrier(&barrier, d2mce_getNodeNum());
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
    int error=0;
    bzero(_C, sizeof(int) * _vector_size * _vector_size);

    for( i = 0; i < _vector_size; i++){
        for ( j = 0; j < _vector_size; j++){
            for ( k = 0; k < _vector_size; k++){
                _C[i*(_vector_size) + j] += A[i*_vector_size+k] * B[k*_vector_size+j];
            }
        }
    }
    for( i = 0; i < _matrix_size; i++){
        if(_C[i] != C[i]){
            error=1;
            printf("%d %d\n", _C[i], C[i]);
            break;
        }
    }

    printf("error: %s\n", (error)?"error!":"no error");
#endif

	if (task_id == 0){
/* print the result of the matrix initialization */
		printf("\nVector Size:%d\n", _vector_size);
		printf("Matrix size:%d\n", _matrix_size);
		printf("Processing time:%f\n", time);
	}
//	print_info();
/* Terminate D2MCE environment */
	print_overhead();
	d2mce_finalize();
	return 0;
}
