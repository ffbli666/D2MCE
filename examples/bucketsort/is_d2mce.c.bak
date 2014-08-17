/*
	radix sort (IS, integer sorting)
    sequence version 
    version: 1.0
    date: 2008/6/18
    author: Zong Ying Lyu
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "../../include/d2mce.h"
#define RANDOM_SEED	2882
#define ARRAY_SIZE	100
#define NODES		1
//#define ENABLE_SHOW_BUCKET
int startend(int myid,int nodes,unsigned int totalsize ,unsigned int* start,unsigned int* end)
{
    int block_size;
    block_size = totalsize / nodes;
    if( totalsize % nodes != 0)
        block_size ++;
    *start = myid * block_size;
    *end = *start + block_size;
    if (*end > totalsize)
        *end = totalsize;
    return 1;
}

int intcmp(int *a, int *b){
	return (*a>*b)?1:0;
}
double utime()
{
    double gettime;
    struct timeval now;
    gettimeofday(&now, 0);
    gettime = 1000000*now.tv_sec + now.tv_usec;
    gettime /= 1000000;
    return gettime;
}

int print_usage()
{
    printf(" Usage: sor_signal [options]\n");
    printf(" Option:\n");
    printf(" \t[ -s <vector size> ] (default: %d)\n", ARRAY_SIZE);
	
    return 1;
}

int main(int argc, char *argv[]){
	int i ;
	int iterations;
	int *source_array;
	int *order_array;
	unsigned int **_bucket;
	unsigned int *bucket;
	int *number_length;
	int lsd;
	int n = 1;
	double time;
	double time2;
	unsigned int size = ARRAY_SIZE;
	int nodes = NODES;
	int node_id;
	unsigned int start;
	unsigned int end;
	FILE *file;
	d2mce_barrier_t b1;
	d2mce_mutex_t m1;
	srand( RANDOM_SEED );
	
    if (argc > 1) {
        for ( i = 1 ; i < argc ;) {
            if (strcmp(argv[i],"-s") == 0) {
                size = atoi(argv[i+1]);
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

	d2mce_init();
	node_id = d2mce_join("isd2mce","eps",0);
    d2mce_mutex_init(&m1,"m1");
    d2mce_barrier_init(&b1,"b1");
	order_array = malloc(sizeof(int)*size);
	source_array = d2mce_malloc("order_array", sizeof(int)*size);
	number_length = d2mce_malloc("number_lenght", sizeof(int));
	//bucket = d2mce_malloc("bucket",sizeof(unsigned int)*10);
	startend(node_id,nodes, size, &start, &end);
	printf("init..\n");
	printf("start %d end %d\n", start,end);
	/*
		create source data  
	*/
//	printf("source array:\n");
	if(node_id == 0){
        file = fopen("input.data","r");
        if (file==NULL) {
            printf("can't not open the input.data file\n");
            return -1;
        }
		fread(number_length, sizeof(int), 1, file);
        fread(source_array, sizeof(int), n, file);
        fclose(file);
		d2mce_barrier(&b1, nodes);
	}else{
		d2mce_barrier(&b1, nodes);
		d2mce_load(source_array);
		d2mce_load(number_length);
	}
	bucket = d2mce_malloc("bucket",sizeof(unsigned int)*10* *number_length);

	_bucket = (unsigned int **)malloc( sizeof(unsigned int*)* *number_length);
	for(i=0;i<*number_length;i++)
		_bucket[i] = (unsigned int*)malloc(sizeof(unsigned int)*10);

	d2mce_barrier(&b1, nodes);
	printf("IS sorting start\n");
	time = -utime();
	time2 = time;
	for( iterations = 0 ; iterations<*number_length ; iterations++){
		printf("iterstion = %d\n",iterations+1);
	/*
		init array;
	*/
	    for( i=0 ; i<10 ; i++){
            _bucket[iterations][i]=0;
			bucket[i]=0;
	    }
	/*
		chose into bucket
	*/
		for( i=start  ; i<end ; i++){
			lsd = (source_array[i]/n) % 10 ;
			_bucket[iterations][lsd]++;
		}
		for(i=1;i<10;i++){
            _bucket[iterations][i] += _bucket[iterations][i-1];
        }
		n *=10 ;
	}
	//merge bucket
    d2mce_mutex_lock(&m1);
    d2mce_load(bucket);
	for(iterations=0;iterations<*number_length; iterations++){
	    for(i=0;i<10;i++){
    		bucket[iterations*10 + i] += _bucket[iterations][i];
	   	}
	}
    d2mce_store(bucket);
    d2mce_mutex_unlock(&m1);
    d2mce_barrier(&b1, nodes);
	if(node_id == 0){
		d2mce_load(bucket);
		time2 += utime();
		n =1;
		for(iterations=0;iterations<*number_length; iterations++){
		    for(i=size-1; i>=0; i--){
    		    lsd = (source_array[i]/n) % 10;
		        order_array[--bucket[iterations*10 + lsd]] = source_array[i];
		    }
			memcpy(source_array, order_array, sizeof(unsigned int)*size);
			n *=10;
		}
	}else{
		time2 += utime();
	}
	time += utime();
	if(node_id ==0){
		printf("check result:\n"); 
		for( i=1 ; i<size ; i++){
//			printf("%d ", order_array[i]);
			if(order_array[i]<order_array[i-1] ){
				printf("check error%d<%d %d\n", order_array[i], order_array[i-1], i);
				break;
			}
		}
		printf("\n");
	}
/*
    printf("order result:\n");
    for( i=0 ; i<size ; i++){
        printf("%d ", order_array[i]);
    }
    printf("\n");
*/
    printf("Result:\n");
    printf("\tTIME\tTIME2\tSize\n");
    printf("\t%f\t%f\t%d\n", time, time2, size);
	d2mce_finalize();

	return 0;
}
