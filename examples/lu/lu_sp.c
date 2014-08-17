/*
 *  * LU.c
 *   *
 *    * A prgram to d an LU decomposition.
 *     *  http://kallipolis.com/openmp/LU.c
 *      */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

//#define SIZE 500

int main(int argc, char *argv[])
{
	double start, stop; /* for keeping track of running time */
	double *matrix;
	double  *col, *row;
	char *filename;
	FILE *fp;
	int i, j, k, n;

	if (argc <= 1) {
		printf ("./xxx vector_size input_filename\n");
		return 1;
	} else {
		n = atoi(argv[1]);
	}

	if ((fp = fopen("input.data", "r")) == NULL) {
		printf("fopen read error\n");
		return 1;
	}
	matrix = (double *)malloc(sizeof(double) * n * n);
	fread(matrix, sizeof(double), n*n, fp);
	fclose(fp);

/*
	for (i = 0; i< n; i++) {
		for (j = 0; j< n; j++)
			printf("%.0f ", matrix[i*n+j]);
		printf("\n");
	}

*/
	col = (double *)malloc(sizeof(double) * n );
	row = (double *)malloc(sizeof(double) * n );

	/* preload A with random values */
	/*
		for (i = 0; i<SIZE; i++)
			for (j = 0; j<SIZE; j++)
				A[i][j] = rand();
	*/

	/* time start now */
	start = time(0);

	for (k=0;k < n-1;k++) {
		for (i=k;i < n;i++)
			col[i] = matrix[i*n+k];
		for (i=k+1;i < n;i++)
			matrix[k*n+i] /= col[k];
		for (i=k+1;i < n;i++)
			row[i] = matrix[k*n+i];
		for (i=k+1;i < n;i++)	{
			for (j=k+1;j < n;j++)	{
				matrix[i*n+j] -= col[i] * row[j];
			}
		}
	}

	/* we're done so stop the timer */
	stop = time(0);
/*
	for (i = 0; i< n; i++) {
		for (j = 0; j< n; j++)
//			printf("%.3f ", matrix[i*n+j]);
			printf("%.3f ", matrix[i][j]);
		printf("\n");
	}
*/
	printf("Completed decomposition in %.3lf seconds\n", difftime(stop, start));

    if ((fp = fopen("seqence.data", "w")) == NULL) {
        printf("fopen read error\n");
        return 1;
    }
    fwrite(matrix, sizeof(double), n*n, fp);
    fclose(fp);


	//free(matrix);
	free(col);
	free(row);
	return 0;
}




