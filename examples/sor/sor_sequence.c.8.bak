/*
    SOR(successive over relaxation)
    sequence version & 8 direction
    version: 1.0
    date: 2008/6/18
    author: Zong Ying Lyu
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#define RANDOM_SEED	3838
#define SIZE		128
#define OMEGA		0.5
#define EPS			1.0e-5
#define ITERATIONS	100//3600
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
	printf(" Usage: sor_sequence [options]\n");
	printf(" Option:\n");
	printf(" \t[ -s <vector size> ] (default: %d)\n", SIZE);
	printf(" \t[ -i <iterations> ] (default: %d)\n", ITERATIONS);
	return 1;
}

int main(int argc, char *argv[])
{
	int i,j,k;
	int n;
	double *data;
	int iterations;
	FILE *file;
	double time;
	double omega;
	double eps;
	double temp,err1;
	int row_size;
	n=SIZE;
	eps=EPS;
	omega=OMEGA;
	iterations = ITERATIONS;

	if (argc > 1) {
		for ( i = 1 ; i < argc ;) {
			if (strcmp(argv[i],"-s") == 0) {
				n = atoi(argv[i+1]);
				n = n;
				i+=2;
			} else if (strcmp(argv[i],"-i") == 0) {
				iterations = atoi(argv[i+1]);
				i+=2;
			} else {
				print_usage();
				return 0;
			}
		}
	}
	data = malloc(sizeof(double)* (n+2) * (n+2));
	file = fopen("input.data","r");
	if (file==NULL) {
		printf("can't not open the input.data file\n");
		return -1;
	}
	fread(data, sizeof(double), (n+2)*(n+2), file);
	fclose(file);
	row_size = n+2;
	printf("sequence SOR processing...\n");
	time = -utime();
	for (k=1; k<iterations; k++)  {
		err1 = 0.0;
		for (i=1; i<=n; i+=2)  {
			for (j=1; j<=n; j++)  {
				temp=0.125*( data[(i-1)*row_size+j]+data[(i+1)*row_size+j]+data[i*row_size+j-1]+data[i*row_size+j+1]+
							 data[(i-1)*row_size+j-1]+data[(i-1)*row_size+j+1]+data[(i+1)*row_size+j-1]+data[(i+1)*row_size+j+1] )-data[i*row_size+j];
				data[i*row_size+j]+=omega*temp;
				if (temp < 0) 
					temp=-temp;
				if (temp > err1) 
					err1=temp;
			}
		}
        for (i=2; i<=n; i+=2)  {
            for (j=1; j<=n; j++)  {
                temp=0.125*( data[(i-1)*row_size+j]+data[(i+1)*row_size+j]+data[i*row_size+j-1]+data[i*row_size+j+1]+
                             data[(i-1)*row_size+j-1]+data[(i-1)*row_size+j+1]+data[(i+1)*row_size+j-1]+data[(i+1)*row_size+j+1])-data[i*row_size+j];
                data[i*row_size+j]+=omega*temp;
                if (temp < 0)
                    temp=-temp;
                if (temp > err1)
                    err1=temp;
            }
        }


		if (err1 <= eps) 
			break;
	
	}
	time+=utime();
	printf("\n====================================================\n");
	printf("Result:\n");
	printf("\tTIME\tVector_Size\tLoops\tErr\n");
	printf("\t%f\t%d\t%d\t%.5e\n", time, n, k, err1);
	file = fopen("sequence_output.data","w");
	if(file == NULL){
		printf("can't not open sequence_output.data\n");
		return -1;
	}
	fwrite(data, sizeof(double), (n+2)*(n+2), file);
	fclose(file);

	return 0;
}
