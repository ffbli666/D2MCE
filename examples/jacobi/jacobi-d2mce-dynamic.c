/*
    jacobi to Solving a System of Linear Equations
    beta version
    version: 0.1
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
#include "../../../src/d2mce.c"
#include <stdio.h>
#include <stdlib.h>

#define VECTOR_SIZE	3
#define ITERATION	5
#define INITVAL 	10
#define NODES		1
#define COMPUTER_ROW	1
#define RANDOM_SEED	2882

void usage_info();//print the usage information
double create_init_value();

int main(int argc, char *argv[]){
	int i,j,k;
	int _vector_size = VECTOR_SIZE;
	int _iteration = ITERATION;
	double _initval = INITVAL;
	int _nodes = NODES;
	int _column = VECTOR_SIZE+1;
	int _computer_row = COMPUTER_ROW;
	int ans_index=0;
	int change=1;
	d2mce_mutex_t lock;
	int task_id;
	int *initialized;
	int *schedule_index;
	int local_index;
	double computing;
	double *matrix;
	double *ans;
	double *_ans[2];
	double time;

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
			else if( !strcmp(argv[i], "-r")){
				if(argv[i+1] == NULL){
					usage_info();
					return 0;
				}
				_computer_row = atoi(argv[i+1]);
				i+=2;
 			}
			else{
				usage_info();
				return 0;
			}
		}
	}
	srand( RANDOM_SEED );
	d2mce_init();
	task_id = d2mce_join("jacobi","abc", 0);
	d2mce_mutex_init(&lock, "lock");

// init value 

	initialized = (int*)d2mce_malloc("initialized", sizeof(int), 1, 0);
	schedule_index = (int*)d2mce_malloc("schedule_index", sizeof(int), 1, 0);
	matrix = (double*) d2mce_malloc("matrix", sizeof(double), _vector_size * _column, 0 );
	ans = (double*) d2mce_malloc("ans", sizeof(double), _vector_size, 0 ); //the old ans;
//	ans[1] = (double*) d2mce_malloc("ans[1]", sizeof(double), _vector_size );
	_ans[0] = (double*) malloc(sizeof(double) * _vector_size ); //the new ans;
	_ans[1] = (double*) malloc(sizeof(double) * _vector_size ); //local index to check the data is dirty
	
	if(task_id == 0){
		d2mce_mutex_rw(&lock, 4, initialized, "rw", schedule_index, "rw", matrix, "rw",
					ans, "rw"); 
		for( i=0; i < _vector_size; i++){
			ans[i] = (double)_initval;
			_ans[0][i] = 0;
			_ans[1][i] = 0;
			for( j=0; j< _column ; j++){
				if(i == j)
					matrix[i*_column + j] = 0;
				else
					matrix[i*_column + j] = create_init_value();
			}
		}
		*schedule_index = 0;
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
	}
	if(_nodes >1)
		d2mce_barrier("wait all init", _nodes);
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
		if(i > 0){
			for(j=0; j<_vector_size; j++)
				_ans[1][j] =0;
		}
		while(1){
			computing =0;
			d2mce_mutex_rw(&lock, 1, schedule_index, "rw");
			local_index = *schedule_index;
			*schedule_index += _computer_row;
			d2mce_mutex_unlock(&lock);
			if(local_index >=_vector_size) 
				break;
			
			for( k=0; k<_vector_size; k++){
				if(local_index!=k)
					computing += matrix[local_index*_column + k]*ans[k];
			}
			computing += matrix[local_index*_column + _vector_size];
			_ans[0][local_index] = computing;
			_ans[1][local_index] = 1;
		}
		if(task_id == 0){
			d2mce_mutex_rw(&lock, 2, ans, "rw", schedule_index,"rw");
			for(j =0 ; j<_vector_size;j++){
				if(_ans[1][j] == 1)
					ans[j] =_ans[0][j];
				}
				*schedule_index=0;
				d2mce_mutex_unlock(&lock);
			}
		else{
			d2mce_mutex_rw(&lock, 1, ans, "rw");
			for(j =0 ; j<_vector_size;j++){
				if(_ans[1][j] == 1)
					ans[j] =_ans[0][j];
		        }
			d2mce_mutex_unlock(&lock);
		}
		d2mce_barrier("wait all write back to the ans", _nodes);
		d2mce_mutex_rw(&lock, 1, ans, "r");
		d2mce_mutex_unlock(&lock);
	}
	time += d2mce_mtime();
/*
 * print the result information
 */
	printf("the ans:\n");
	for( i=0; i < _vector_size; i++){
		printf("%f ", ans[i]);
	}
	printf("\n");
	printf("execute time:%f\n", time);
	d2mce_exit();	
	return 0;
}

void usage_info(){
	printf(" Usage: jacobi [ -s <vector number> ] (default: %d)\n", VECTOR_SIZE);
	printf("               [ -i <iteration> ] (default: %d)\n", ITERATION);
	printf("               [ -v <init value> ] (default: %d)\n", INITVAL);
	printf("               [ -n <nodes number> ] (default: %d)\n", NODES);
	printf("               [ -r <dynamic computing row size> ] (default: %d)\n", COMPUTER_ROW);
}

double create_init_value(){
	int negative=rand()%2;
	double value = (double)(rand()%10+1) / (rand()%10+1);	
	if( negative )
		value=-value ;
	return value;	
}

