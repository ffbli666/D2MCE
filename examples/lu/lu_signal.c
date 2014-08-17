/*
 	LU decomposition 
	sequence version
	version: 1.0
	date: 2008/6/18
	author: Zong Ying Lyu

example:
	source matrix
		1,1,1
		20,24,30
		12,10,8
	result:
 		1.000 1.000 1.000
	    20.000 4.000 2.500
	 	12.000 -2.000 1.000


*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#define TYPE	double
#define SIZE	1024
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
    printf(" Usage: lu_signal [options]\n");
    printf(" Option:\n");
    printf(" \t[ -s <vector size> ] (default: %d)\n", SIZE);
    return 1;
}

int main(int argc, char *argv[])
{
	int n;
	int i,j,k;
	TYPE temp;
	TYPE *matrix;
	TYPE *U;
	TYPE *L;
	double time;
	FILE *file;
	n =SIZE;
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
	matrix = malloc(sizeof(TYPE)*n*n);
	U = malloc(sizeof(TYPE)*n*n);
	L = malloc(sizeof(TYPE)*n*n);
	bzero(U, sizeof(TYPE)*n*n);
	bzero(L, sizeof(TYPE)*n*n);
    file = fopen("input.data","r");
    if (file==NULL) {
        printf("can't not open the input.data file\n");
        return -1;
    }
    fread(matrix, sizeof(TYPE), n*n, file);
    fclose(file);

/*
	matrix[0] =1;
	matrix[1] =1;
	matrix[2] =1;
	matrix[n] = 20;
	matrix[n+1] = 24;
	matrix[n+2] = 30;
	matrix[2*n] = 12;
	matrix[2*n+1] = 10;
	matrix[2*n+2] = 8;
*/
	/* start to the procedure of LU decomposition */
	time = -utime();
	for (i=0; i<n; i++) 
		L[i*n]=matrix[i*n];
	for (i=0; i<n ;i++) 
		U[i]=matrix[i]/L[0];
	for(j=1; j<n; j++){
    	for(i=j; i<n;i++){
	        temp=0.0;
	        for(k=0;k<=j-1;k++){
               temp=temp+L[i*n+k]*U[k*n+j];
            }
           	L[i*n+j]=matrix[i*n+j]-temp;
        }
	    U[j*n+j]=1.0;
	    for(i=j+1;i<n;i++){
    	    temp=0.0;
	        for(k=0; k<=j-1;k++){
		        temp=temp+L[j*n+k]*U[k*n+i];
	        }
	        U[j*n+i]=(matrix[j*n+i]-temp)/L[j*n+j];
	    }
   	}
	//merge U and L	
	for(i=0;i<n;i++){
		for(j=i+1;j<n;j++)
			matrix[i*n+j] = U[i*n+j];
		for(j=i;j<n;j++)
			matrix[j*n+i] = L[j*n+i];
	}
	time += utime();
	printf("result\n");
	printf("\tTIME\tVector_Size\n");
	printf("\t%f\t%d*%d\n", time, n,n);

    file = fopen("signal_output.data","w");
    fwrite(matrix, sizeof(TYPE), n*n, file);
    fclose(file);

/*
    for(i=0;i<n;i++){
        for(j=0;j<n;j++)
			printf("%f ", matrix[i*n+j]);
		printf("\n");
	}
*/
	return 1;
}
