/*
	All pairs shortest path (ASP) or Floyd’s Algorithm
    sequence version
    version: 1.0
    date: 2008/6/18
    author: Zong Ying Lyu
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#define RANDOM_SEED	3838
#define SIZE 1024
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
	printf(" Usage: asp_sequence [options]\n");
	printf(" Option:\n");
	printf(" \t[ -s <vector size> ] (default: %d)\n", SIZE);
	return 1;
}

int main(int argc, char *argv[])
{
	int i,j,k;
	int n;
	double *data;
	FILE *file;
	double time;
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
	data = malloc(sizeof(double)* n * n);
	file = fopen("input.data","r");
	if (file==NULL) {
		printf("can't not open the input.data file\n");
		return -1;
	}
	fread(data, sizeof(double), n*n, file);
	fclose(file);
	printf("sequence ASP processing...\n");
	time = -utime();
	for (k = 0; k < n; k++)
		for (i = 0; i < n; i++)
			for (j = 0; j < n; j++)
				if (data[i*n+j] > (data[i*n+k] + data[k*n+j]))
					data[i*n+j] = data[i*n+k] + data[k*n+j];
	time+=utime();
	printf("Result:\n");
	printf("\tTIME\tVector_Size\n");
	printf("\t%f\t%d\n", time, n);
	file = fopen("sequence_output.data","w");
	fwrite(data, sizeof(double), n*n, file);
	fclose(file);

	return 0;
}
