/*
	matrix multiplication
	matrix A
	matrix B 
	matrix C 
	size : A = B =C
	computing : A * B = C	
*/
#include "mtime.h"
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define RANDOM_SEED 2882	//random seed
#define VECTOR_SIZE 4		//sequare matrix width the same to height		 
#define MATRIX_SIZE (VECTOR_SIZE * VECTOR_SIZE)	//total size of MATRIX

int  main(int argc, char *argv[]){
	int i,j,k;
	int node_id;
	int *AA; //sequence use & check the d2mce right or fault
	int *BB; //sequence use
	int *CC; //sequence use
	int computing;
	int _vector_size = VECTOR_SIZE;
	int _matrix_size = MATRIX_SIZE;
	double time;
	char c[10];
	if(argc > 1){
		for( i = 1 ; i < argc ;){
			if(strcmp(argv[i],"-s") == 0){
				_vector_size = atoi(argv[i+1]);
				_matrix_size =_vector_size * _vector_size;
				i+=2;
			}
			else{
				printf("the argument only have:\n");
				printf("-s: the size of vector ex: -s 256\n");
				return 0;
			}
		}
	}
	AA =(int *)malloc(sizeof(int) * _matrix_size);
	BB =(int *)malloc(sizeof(int) * _matrix_size);
	CC =(int *)malloc(sizeof(int) * _matrix_size);
	
	srand( RANDOM_SEED );

/*
	create matrix A and Matrix B
*/
	for( i=0 ; i< _matrix_size ; i++){
		AA[i] = rand()%10; 
		BB[i] = rand()%10;
	}
	time = -D2MCE_Mtime();

/*
	computing C = A * B
*/
	#pragma omp parallel for private(computing, j , k)
	for( i=0 ; i < _vector_size ; i++){
		for( j=0 ; j < _vector_size ; j++){
			CC[ i*_vector_size + j ] =0;
			for( k=0 ; k < _vector_size ; k++)
				CC[ i*_vector_size + j ]+= AA[ i*_vector_size + k ] * BB[ k*_vector_size + j ];
		}
	}
	time += D2MCE_Mtime();	

	printf("\nVector_size:%d\n", _vector_size);
	printf("Matrix_size:%d\n", _matrix_size);
	printf("Processing time:%f\n", time);
	for( j=0 ; j<_vector_size ; j++){
    	for( k=0 ; k<_vector_size ; k++)
			printf("%d ", CC[j*_vector_size + k]);
		printf("\n");
	}

	return 0;

}
