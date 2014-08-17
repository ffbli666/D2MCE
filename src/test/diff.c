#include <stdio.h>
#include <stdlib.h>
#include "../d2mce.h"
#include "../sharememory.h"

#define TYPE char
#define SIZE size
int main(int argc, char *argv[]){
	int i;
	double time;
	unsigned int size = atoi(argv[1]);
	TYPE *A;
	TYPE *B;
	struct diff *diff;
	A=malloc(sizeof(TYPE)*size);
	B=malloc(sizeof(TYPE)*size);
printf("%u=n", SIZE/4);
	for(i=0;i<SIZE;i++){
		A[i] = i;
			B[i] = i;
		if( i >SIZE/4 && i<SIZE/2)
			B[i] = 1;
		else if(i>SIZE*0.75)
			B[i] = 0;
		else
			B[i] = i;
	}
/*
	printf("A:");
    for(i=0;i<SIZE;i++){
        printf("%d ", A[i]);
    }
	printf("\nB:");
    for(i=0;i<SIZE;i++){
        printf("%d ", B[i]);
    }
	printf("\n");
*/
	time = - d2mce_time();
	diff = createDiff(A, B, SIZE*sizeof(TYPE));
	time += d2mce_time();
	printf("diff size: %u\n", diff->size);
	printf("diff offset: %d\n",((struct diff_header*)diff->data)->offset);
	printf("diff length: %d\n",((struct diff_header*)diff->data)->length);
	printf("time %f\n", time);
	printf("\n");	
	return 1;	
}
