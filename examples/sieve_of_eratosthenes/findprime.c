/*
 * this program use "the Sieve of Eratosthenes" algorithm
 */
#include "mtime.h"
#include <stdio.h>
#include <stdlib.h>
#define NUMBER_COUNT 100
#define PRINT_RESULT
int main(int argc, char *argv[])
{
	char *_n;
	int i,j;
	int prime = 2;
	int _number_count = NUMBER_COUNT;
	int count;
	double time;
	if (argc > 1) {
		for ( i = 1 ; i < argc ;) {
			if (strcmp(argv[i],"-s") == 0) {
				_number_count = atoi(argv[i+1]);
				i+=2;
			} else {
				printf("the argument only have:\n");
				printf("-s: the size of vector ex: -s 256\n");
				return 0;
			}
		}
	}
	_n = (char *)malloc( sizeof(char) * _number_count);
//init the _N to 0 and the even is 1
	for ( i=0 ; i<_number_count ; i++) {
		if ( i >2 && i%2==0)
			_n[i]=1;
		else
			_n[i]=0;
	}
	time = -D2MCE_Mtime();
//start find the prime
//don't find even, because even(should >2) is not prime
	for ( i =3 ; i <_number_count ; i+=2) {
		if (_n[i] != 1) {
			prime=i;
			if ( prime*prime >_number_count)
				break;
			for ( j=prime+prime ; j < _number_count ; j+=prime)
				_n[j]=1;
		}
	}
	time += D2MCE_Mtime();
	/*
	 *end find prime
	 *print the result
	 */
	count = 0;
	printf("\ndata size:%d\n", _number_count);
#ifdef PRINT_RESULT
	printf("<%d prime have:\n",_number_count);
#endif
	for ( i =2 ; i <_number_count ; i++) {
		if (_n[i]==0) {
			count++;
#ifdef PRINT_RESULT
			printf("%d ", i);
#endif
		}
	}
	printf("\nfind prime count:%d", count);
	printf("\nprocess time=%f\n",time);

	return 0;
}
