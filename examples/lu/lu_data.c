/*
    LU decomposition
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
#define TYPE double
int print_usage()
{
	printf(" Usage: lu_data [options]\n");
	printf(" Option:\n");
	printf(" \t[ -s <vector size> ] (default: %d)\n", SIZE);

	return 1;
}
int main(int argc, char *argv[])
{
	int i,j;
	int n;
	TYPE *data;
	FILE *file;
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
	data = malloc(sizeof(TYPE)* n * n);
	for (i=0; i<n; i++) {
		for (j=0; j<n; j++) {
			// there will create 3 digit float
			data[i*n+j] = (double)rand()/10000000;
		}
	}
	file = fopen("input.data","w");
	fwrite(data, sizeof(TYPE), n*n, file);
	fclose(file);

	return 0;
}


