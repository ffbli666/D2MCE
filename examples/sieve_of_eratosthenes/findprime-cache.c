/*
 * this program use "the Sieve of Eratosthenes" algorithm
 */
#include "mtime.h"
#include <stdio.h>
#include <stdlib.h>
#define NUMBER_COUNT 100
#define CACHE_SIZE 32 //need even
//#define PRINT_RESULT
int main(int argc, char *argv[])
{
	char *_n;
	int i,j,k;
	int prime = 2; //the prime
	int _number_count = NUMBER_COUNT;
	double time;
	int count=1;
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
//init the _N to 0
	for ( i=0 ; i<_number_count ; i++)
		_n[i]=0;
//start find the prime
	time = -D2MCE_Mtime();/*
//every Loop have process how length data to improve cache hit rate
	for( i=1 ; i<_number_count ; i+=CACHE_SIZE){
		for(prime =3; prime*prime <i+CACHE_SIZE ;prime+=2){
			if(_n[prime] ==1)
                        	continue;
			k=i/prime;
			if(k<2)
				k++;
			for( j=prime*k ; j<i+CACHE_SIZE ; j+=prime){
				_n[j]=1;
			}
		}
	}*/
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
	 * end find prime
	 * print the result
	 */
	printf("\ndata size:%d\n", _number_count);
#ifdef PRINT_RESULT
	printf("<%d prime have:\n",_number_count);
	printf("2 ");
#endif
	for ( i =3 ; i <_number_count ; i+=2) {
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
