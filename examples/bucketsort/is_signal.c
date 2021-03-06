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
#define RANDOM_SEED 2882
#define ARRAY_SIZE 100
//#define ENABLE_SHOW_BUCKET
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
	int *init_array;
	int *source_array;
	int *order_array;
	int bucket[10];
	int number_length = 0;
	int lsd;
	int n = 1;
	char get_number[10];
	double time;
	int size = ARRAY_SIZE;
	srand( RANDOM_SEED );
	
    if (argc > 1) {
        for ( i = 1 ; i < argc ;) {
            if (strcmp(argv[i],"-s") == 0) {
                size = atoi(argv[i+1]);
                i+=2;
            } else {
                print_usage();
                return 0;
            }
        }
    }
	source_array = malloc(sizeof(int)*size);
	order_array = malloc(sizeof(int)*size);
	init_array =  malloc(sizeof(int)*size);

	printf("init..\n");
	/*
		create source data  
	*/
//	printf("source array:\n");
	for( i=0 ; i<size ; i++){
		source_array[i] = rand()%1000+1;
		order_array[i] = source_array[i];
		init_array[i] = source_array[i];
//		printf("%d ", source_array[i]);
		sprintf(get_number, "%d", source_array[i]);
		if( strlen( get_number ) > number_length)
			number_length = strlen( get_number );

	}
//	printf("\n");
	printf("IS sorting start\n");
	time = -utime();

	for( iterations = 0 ; iterations<number_length ; iterations++){
		printf("iterstion = %d\n", iterations);
	/*
		init array;
	*/
        for( i=0 ; i<10 ; i++){
            bucket[i]=0;
//			order_array[i] = 0;
        }


	/*
		chose into bucket
	*/
		for( i=0 ; i<size ; i++){
			lsd = (source_array[i]/n) % 10 ;
			bucket[lsd]++;
		}
	/*
 		
	*/
		for(i=1;i<10;i++){
			bucket[i] += bucket[i-1];
		}
	/*
		order to order_array
	*/

		for(i=size-1; i>=0; i--){
			lsd = (source_array[i]/n) % 10;
			order_array[--bucket[lsd]] = source_array[i];
		}
		for(i=0;i<size;i++){
			source_array[i] = order_array[i];
		}

	/*
		change number digit. increase *10 of every times
	*/
		n *=10 ;
	}
	time += utime();
/*
	    for(i=size-1; i>=0; i--){
            lsd = (source_array[i]/n) % 10;
            order_array[--bucket[lsd]] = source_array[i];
        }
        for(i=0;i<size;i++){
            source_array[i] = order_array[i];
        }
*/
	qsort(init_array, size, sizeof(int), intcmp);
	printf("check result:\n"); 
	for( i=0 ; i<size ; i++){
//		printf("%d ", order_array[i]);
		if(order_array[i] != init_array[i]){
			printf("check error\n");
			break;
		}
	}
	printf("\n");
/*
    printf("order result:\n");
    for( i=0 ; i<size ; i++){
        printf("%d ", source_array[i]);
    }
    printf("\n");
*/
    printf("Result:\n");
    printf("\tTIME\tSize\n");
    printf("\t%f\t%d\n", time, size);


	return 0;
}
