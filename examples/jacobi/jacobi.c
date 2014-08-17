/*
    jacobi to Solving a System of Linear Equations
    sequence version
    version: 1.0
    date: 2008/6/18
    author: Zong Ying Lyu
*/

/* jacobi method (math solutiong)
 * Usage: jacobi [-s <vector number> ]
 * 		 [-i <iteration> ]
 * 		 [-v <init value> ]
 */
#include "mtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define VECTOR_SIZE	3
#define ITERATION	100
#define INITVAL 	10

#define RANDOM_SEED	2882

void usage_info();//print the usage information
double create_init_value();

int main(int argc, char *argv[]){
	int i,j,k;
	int _vector_size = VECTOR_SIZE;
	int _iteration = ITERATION;
	double _initval = INITVAL;
	int _column = VECTOR_SIZE+1;
	int ans_index=0;
	int change=1;
	double computing;
	double *matrix;
	double *ans[2];// in start ,0 is old , 1 is new,but it's will change in running time;
	double time;
	FILE *file;
// processing argument
	if(argc > 1){
		for( i = 1 ; i < argc ;){
			if( !strcmp(argv[i], "-s" )){
				if(argv[i+1] == NULL){
					usage_info();
					return 0;
				}
				_vector_size = atoi(argv[i+1]);
				_column = _vector_size + 1;
				i+=2;
			}
			else if( !strcmp(argv[i], "-i")){
				if(argv[i+1] == NULL){
					usage_info();
					return 0;
				}
				_iteration = atoi(argv[i+1]);
				i+=2;
			}
 			else if( !strcmp(argv[i], "-v")){
				if(argv[i+1] == NULL){
					usage_info();
					return 0;
			}
			_initval = atof(argv[i+1]);
			i+=2;
			}
		    else{
				usage_info();
				return 0;
			}
		}
	}
	srand( RANDOM_SEED );

// init value 
	matrix = (double*) malloc( sizeof(double) * _vector_size * _column );
	ans[0] = (double*) malloc( sizeof(double) * _vector_size );
	ans[1] = (double*) malloc( sizeof(double) * _vector_size );
	for( i=0; i < _vector_size; i++){
		ans[0][i] = _initval;
		for( j=0; j< _column ; j++){
			if(i == j)
				matrix[i*_column + j] = 0;
			else
				matrix[i*_column + j] = create_init_value();
		}
//		matrix[i*_column + j] =1 ;
	}
/*
	for( i=0; i < _vector_size; i++){
		for( j=0; j< _column ; j++){
			printf("%f ", matrix[i*_column + j]);
		}
		printf("\n");
	}
*/

/*
	ans[0][0]=-1.5;
	ans[0][1]=2.5;
	ans[0][2]=-0.5;
	matrix[0]=-0;
	matrix[1]=-0.125;
	matrix[2]=0.25;
	matrix[3]=-(double)11/8;
	matrix[_column]=-(double)2/9;
	matrix[_column+1]=0;
	matrix[_column+2]=-(double)1/9;
	matrix[_column+3]=(double)22/9;
	matrix[2*_column]=(double)1/11;
	matrix[2*_column+1]=(double)2/11;
	matrix[2*_column+2]=0;
	matrix[2*_column+3]=-(double)15/11;
*/

/*
 * jacobi method computing
 */
	time = -d2mce_mtime();
	for( i=0; i<_iteration; i++){
		for( j=0; j<_vector_size; j++){
			//computing = matrix[i*_column + _vector_size];
			computing =0;
			for( k=0; k<_vector_size; k++){
				if(j!=k)
					computing += matrix[j*_column + k]*ans[ans_index][k];
			}
			computing += matrix[j*_column + _vector_size];
			ans[ans_index + change][j] = computing;
			printf ("%d  ", computing);
		}
		printf("\n");
		ans_index += change;
//		for( j=0; j<_vector_size; j++){
//			printf("%f ", ans[ans_index][j]);
//		}
//		printf("\n");
		change = -change;
	}
	time += d2mce_mtime();
/*
 * print the result information
 */

	printf("the ans:\n");
	for( i=0; i < _vector_size; i++){
		printf("%f ", ans[ans_index][i]);
	}

    printf("Result:\n");
    printf("\tTIME\tVector_Size\n");
    printf("\t%f\t%d\n", time, _vector_size);

	file = fopen("sequence_output.data","w");
	fwrite(ans[ans_index], sizeof(double), _vector_size, file);
	fclose(file);
    
	
	return 0;
}

void usage_info(){
	printf(" Usage: jacobi [ -s <vector number> ] (default: %d)\n", VECTOR_SIZE);
	printf("               [ -i <iteration> ] (default: %d)\n", ITERATION);
	printf("               [ -v <init value> ] (default: %d)\n", INITVAL);
}

double create_init_value(){
	int negative=rand()%2;
	double value = (double)(rand()%10+1) / (rand()%10+1);	
	if( negative )
		value=-value ;
	return value;	
}

