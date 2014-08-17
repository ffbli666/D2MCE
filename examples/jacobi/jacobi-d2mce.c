/*
    jacobi to Solving a System of Linear Equations
    d2mce version
    version: 1.0
    date: 2008/6/18
    author: Zong Ying Lyu
*/

/* jacobi method (math solutiong)
 * Usage: jacobi [-s <vector number> ]
 * 		 [-i <iteration> ]
 * 		 [-v <init value> ]
 * 		 [-n <nodes number> ]
 * 		 [-r <dynamic computing row size>]
 */
#include "../../src/d2mce.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define VECTOR_SIZE	3
#define ITERATION	100
#define INITVAL 	10
#define NODES		1
#define RANDOM_SEED	2882
//#define OPENMP

void usage_info();//print the usage information
double create_init_value();

int main(int argc, char *argv[]){
	int i,j,k;
	int _vector_size = VECTOR_SIZE;
	int _iteration = ITERATION;
	double _initval = INITVAL;
	int _nodes = NODES;
	int _column = VECTOR_SIZE+1;
	int ans_index=0;
	int change=1;
	d2mce_mutex_t lock;
	d2mce_barrier_t b1;
	int node_id;
	int *initialized;
//	int local_index;
	double computing;
	double *matrix;
	double *ans;
	double *_ans[2];
	double time;
	int comp_size = 0;
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
			else if( !strcmp(argv[i], "-n")){
				if(argv[i+1] == NULL){
					usage_info();
					return 0;
				}
				_nodes = atoi(argv[i+1]);
				i+=2;
			}
			else{
				usage_info();
				return 0;
			}
		}
	}
	comp_size = _vector_size /_nodes;
	if(_vector_size%_nodes !=0)
		comp_size++;

	srand( RANDOM_SEED );
	d2mce_init();
	node_id = d2mce_join("jacobi", "eps308", 0);
	printf("my id%d\n", node_id);
	d2mce_mutex_init(&lock, "lock");
	d2mce_barrier_init(&b1, "b1");
// init value 

	initialized = (int*)d2mce_malloc("initialized", sizeof(int));
	matrix = (double*) d2mce_malloc("matrix", sizeof(double)* _vector_size * _column );
	ans = (double*) d2mce_malloc("ans", sizeof(double)* _vector_size ); //the old ans;
//	ans[1] = (double*) d2mce_malloc("ans[1]", sizeof(double), _vector_size );
	_ans[0] = (double*) malloc(sizeof(double) * _vector_size ); //the new ans;
	_ans[1] = (double*) malloc(sizeof(double) * _vector_size ); //local index to check the data is dirty
	
	if(node_id == 0){
		d2mce_mutex_rw(&lock, 3, initialized, "rw",  matrix, "rw", ans, "rw"); 
		for( i=0; i < _vector_size; i++){
			ans[i] = (double)_initval;
			_ans[0][i] = (double)_initval;
			_ans[1][i] = 0.0;
			for( j=0; j< _column ; j++){
				if(i == j)
					matrix[i*_column + j] = 0;
				else
					matrix[i*_column + j] = create_init_value();
			}
		}
		*initialized = 0;	
		d2mce_mutex_unlock(&lock);
	}
	else{
		while (! initialized) {
			d2mce_mutex_rw(&lock, 1,initialized,"r");
			d2mce_mutex_unlock(&lock);
		}
		d2mce_mutex_rw(&lock, 2, matrix, "r", ans, "r" );
		d2mce_mutex_unlock(&lock);
        for( i=0; i < _vector_size; i++){
			_ans[0][i] = ans[i];
			_ans[1][i] = 0.0;
		}
	}
	if(_nodes >1)
		d2mce_barrier(&b1, _nodes);
	
/*
	for( i=0; i < _vector_size; i++){
		for( j=0; j< _column ; j++){
			printf("%f ", matrix[i*_column + j]);
		}
		printf("\n");
	}
	printf("\n\n");
	for( j=0; j< _column ; j++){
		printf("%f ", ans[j]);
	}
	printf("\n\n");

	for( j=0; j< _column ; j++){
		printf("%f ", _ans[0][j]);
	}
	printf("\n\n");
	for( j=0; j< _column ; j++){
		printf("%f ", _ans[1][j]);
	}
	printf("\n\n");
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
	time = -d2mce_time();
	for( i=0; i<_iteration; i++){
		for( j=node_id*comp_size; j<(node_id*comp_size+comp_size) && j<_vector_size; j++){
			computing =0;
			for( k=0; k<_vector_size; k++){
				if(j!=k)
					computing += matrix[j*_column + k]*_ans[ans_index][k];
			}
			computing += matrix[j*_column + _vector_size];
			_ans[ans_index + change][j] = computing;
		}
		ans_index += change;
		change = -change;
//		d2mce_barrier(&b1, _nodes);
        d2mce_mutex_rw(&lock, 1, ans, "rw");
        for( j=node_id*comp_size; j<(node_id*comp_size+comp_size) && j<_vector_size; j++){
            ans[j] = _ans[ans_index][j];
        }
        d2mce_mutex_unlock(&lock);
		d2mce_barrier(&b1, _nodes);
		d2mce_load(ans);
		if( i != _iteration){
		#ifdef OPENMP
	        #pragma omp parallel for
		#endif
	        for( j=0; j<_vector_size; j++){
    	        _ans[ans_index][j] = ans[j];
	        }
		}

	}
	time += d2mce_time();
/*
 * print the result information
 */
/*
	printf("the ans:\n");
	for( i=0; i < _vector_size; i++){
		printf("%f ", ans[i]);
	}
	printf("\n");
*/
	printf("Result:\n");
 	printf("\tTIME\tVector_Size\n");
    printf("\t%f\t%d\n", time, _vector_size);
    file = fopen("d2mce_output.data","w");
    fwrite(ans, sizeof(double), _vector_size, file);
    fclose(file);
	print_overhead();
	d2mce_finalize();
	free(_ans[0]);	
	free(_ans[1]);
	return 0;
}

void usage_info(){
	printf(" Usage: jacobi [ -s <vector number> ] (default: %d)\n", VECTOR_SIZE);
	printf("               [ -i <iteration> ] (default: %d)\n", ITERATION);
	printf("               [ -v <init value> ] (default: %d)\n", INITVAL);
	printf("               [ -n <nodes number> ] (default: %d)\n", NODES);
}

double create_init_value(){
	int negative=rand()%2;
	double value = (double)(rand()%10+1) / (rand()%10+1);	
	if( negative )
		value=-value ;
	return value;	
}

