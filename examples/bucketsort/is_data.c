/*
	Integer sorting use radix sort based on counting sorting 
    use to create input data
    version: 1.0
    date: 2008/6/18
    author: Zong Ying Lyu
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define RANDOM_SEED	3838
#define SIZE 1024
int print_usage()
{
	printf(" Usage: is_data [options]\n");
	printf(" Option:\n");
	printf(" \t[ -s <number size> ] (default: %d)\n", SIZE);

	return 1;
}
int main(int argc, char *argv[])
{
	int i;
	unsigned int n;
	int *data;
	FILE *file;
	int number_length=0;
	char get_number[10];	
	n=SIZE;
	if (argc > 1) {
		for ( i = 1 ; i < argc ;) {
			if (strcmp(argv[i],"-s") == 0) {
				n = atoi(argv[i+1]);
				i+=2;
			} else {
				print_usage();
				return 0;
			}
		}
	}
	srand(RANDOM_SEED);
	data = malloc(sizeof(int)* n);
	for(i=0; i<n; i++){
		data[i] = rand()%1000+1;
        sprintf(get_number, "%d", data[i]);
        if( strlen( get_number ) > number_length)
	        number_length = strlen( get_number );
	}
	file = fopen("input.data","w");
	fwrite(&number_length, sizeof(int), 1, file);
	fwrite(data, sizeof( int), n, file);
	fclose(file);
	
	return 0;
}
