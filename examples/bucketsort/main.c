/*
	radix sort
*/
#include "../../src/d2mce.c"

#include <stdio.h>
#include <stdlib.h>

#define RANDOM_SEED 2882		//random seed
#define NODE_SIZE 10 		//the numbers of element. if node > 0 then the total size will TOTAL_SIZE * NODES
#define NODES 4			//this is numbers of nodes. minimum is 1. don't use < 1. max is 10
#define TOTAL_SIZE (NODE_SIZE * NODES)	//total size of all nodes' array sum

//#define ENABLE_SHOW_BUCKET
//#define ENABLE_SHOW_PRINT
int  main(void)
{
	int i,j,k;
	int iterations;				//need iterations
	int lsd;				//the mod number
	int n = 1;				//digits
	int swaptemp;
	int *numtemp;
	char get_number[10];			//integer convert string
	int check_order[TOTAL_SIZE];
	int check = 1;
	d2mce_mutex_t m1;
	int node_id;
	int* order_array;
	int* number_length;
	int* bucket;
	int* bucket_numbers;
	int node_source;
	double starttime, endtime;
	int base;
	// get start time

	d2mce_init();
	node_id = d2mce_join("is", "eps", 0);
	srand( RANDOM_SEED + node_id);
	//srand( RANDOM_SEED);
	d2mce_mutex_init(&l1,"lock1");

	order_array = d2mce_malloc( "order_array", sizeof(int) * TOTAL_SIZE );	//order data and result
	number_length = d2mce_malloc( "number_length", sizeof(int) );		//need numbers of iterations
	bucket_numbers = d2mce_malloc( "bucket_numbers", sizeof(int) * 10 );	//every bucket's numbers of element
	bucket = d2mce_malloc( "bucket", sizeof(int) * 10 * TOTAL_SIZE );		//radix bucket 0~9
	numtemp = d2mce_malloc( "numtemp", sizeof(int) );
	if ( node_id == 0) {
		d2mce_lock(l1);
		d2mce_store(number_length);
		*number_length =0;
		d2mce_unlock(l1);
	}
	d2mce_Barrier("wait init bucket", NODES );

	/*
		create source data
	*/
	for ( i=0 ; i<NODE_SIZE ; i++) {
		node_source = rand()%100000;
		sprintf(get_number, "%d", node_source);
		d2mce_Lock(l1);

		d2mce_Load(order_array);
		d2mce_Exclusive(order_array);
		*(order_array + (node_id * NODE_SIZE) + i) = node_source;

		d2mce_Load(number_length);
		if ( strlen( get_number ) > *number_length) {
			d2mce_Exclusive(number_length);
			*number_length = strlen( get_number );
		}

		d2mce_Unlock(l1);
	}
	printf("\n");
	d2mce_Barrier("wait all node create data", NODES );
	/*
		sequence order array get source data
	*/
	d2mce_Lock(l1);
	d2mce_Load( order_array );
	d2mce_Unlock(l1);
#ifdef ENABLE_SHOW_PRINT
	printf("source data:\n");
#endif
	for ( i =0 ;i< TOTAL_SIZE ; i++) {
		check_order[i] = *(order_array + i);
#ifdef ENABLE_SHOW_PRINT
		printf("%d ", *(order_array + i));
#endif
	}
	printf("\n\n\n");
	for ( iterations = 0 ; iterations<*number_length ; iterations++) {
		d2mce_Barrier("for loop", NODES );
		/*
			init array;
		*/
		if ( node_id == 0) {
			d2mce_Lock(l1);
			d2mce_Exclusive(bucket_numbers);
			d2mce_Exclusive(bucket);
			d2mce_Exclusive(numtemp);
			*numtemp=0;
			for ( i=0 ; i<10 ; i++)
				*(bucket_numbers + i) = 0;
			for ( i=0 ; i<10 ; i++) {
				for ( j=0 ; j<TOTAL_SIZE ; j++) {
					*(bucket + j + i*TOTAL_SIZE) = 0;
				}
			}
			d2mce_Unlock(l1);
		}
		d2mce_Barrier("wait init bucket", NODES );

		/*
			chose into bucket
		*/
		d2mce_Lock(l1);

		d2mce_Load(order_array);
		d2mce_Unlock(l1);
		for ( i=0 ; i<NODE_SIZE ; i++) {
			lsd = ( *(order_array + i + (node_id * NODE_SIZE) ) /n) % 10 ;
			d2mce_Lock(l1);
			d2mce_Load(bucket);
			d2mce_Load(bucket_numbers);
			d2mce_Exclusive(bucket);

			*(bucket+(lsd)+ (*(bucket_numbers + lsd)* 10)) = *(order_array + i +(node_id * NODE_SIZE));

			d2mce_Exclusive(bucket_numbers);
			*(bucket_numbers + lsd) +=1;
			d2mce_Unlock(l1);
		}

		d2mce_Barrier("wait data into bucket", NODES );
		/*
			sort every bucket array
		*/
		d2mce_Lock(l1);
		d2mce_Load(bucket);
		d2mce_Load(bucket_numbers);
		d2mce_Unlock(l1);

		for ( i=node_id ; i<10 ; i += NODES) {
			for ( j=0 ; j<*(bucket_numbers + i)-1; j++) {
				for ( k= j+1 ; k<*(bucket_numbers + i); k++) {
					if ( *(bucket + j*10 + i) > *(bucket + k*10 +i)) {
						swaptemp =*(bucket + j*10 +i);
						*(bucket + j*10 + i) =  *(bucket + k*10 + i);
						*(bucket + k*10 + i) = swaptemp;
					}
				}
			}
		}


		/*
			order to order_array
		*/
		for ( i=node_id ; i<10 ; i += NODES) {
			base = 0;
			for ( k=0 ; k < i ;k++)
				base += *(bucket_numbers+k);

			for ( j=0 ; j< *(bucket_numbers+i) ; j++) {
				d2mce_Lock(l1);
				d2mce_Load(order_array);
				d2mce_Load(numtemp);
				d2mce_Exclusive(order_array);
				d2mce_Exclusive(numtemp);
				*(order_array + base + j) = *(bucket + i  + j * 10);
				*numtemp += 1;
				d2mce_Unlock(l1);

			}
		}

#ifdef ENABLE_SHOW_BUCKET
		/*
			show bucket
		*/
		d2mce_Barrier("show  bucket", NODES );
		d2mce_Lock(l1);
		d2mce_Load(bucket);
		d2mce_Unlock(l1);
		printf("input %d:\n", n);
		for ( i=0 ; i<TOTAL_SIZE ; i++) {
			for ( j=0 ; j<10 ; j++) {
				printf("%d\t", *(bucket + (i * 10) + j));
			}
			printf("\n");
		}

#endif
		/*
			change number digit. increase *10 of every times
		*/
		n *=10 ;
	}
	/*
		show result
	*/
	d2mce_Barrier("show  order result", NODES );
	d2mce_Lock(l1);
	d2mce_Load(order_array);
	d2mce_Unlock(l1);
	for ( i=0 ; i < (TOTAL_SIZE-1) ; i++) {
		for ( j= i+1 ; j < TOTAL_SIZE ; j++) {
			if ( check_order[i] > check_order[j]) {
				swaptemp = check_order[i];
				check_order[i] = check_order[j];
				check_order[j] = swaptemp;
			}
		}
	}
	/*
		check result
	*/

	for ( i=0 ; i<TOTAL_SIZE ; i++) {
		if (check_order[i] != *(order_array + i)) {
			check = 0;
			break;
		}
	}
#ifdef ENABLE_SHOW_PRINT
	printf("\nd2mce result:\n");
	for ( i=0 ; i<TOTAL_SIZE ; i++) {
		printf("%d ", *(order_array + i));
	}
	printf("\n\nsequence result:\n");
	for ( i=0 ; i<TOTAL_SIZE ; i++) {
		printf("%d ", check_order[i]);
	}
	printf("\n\n");

#endif
	endtime=d2mce_Mtime();
	printf("\n\nprocessing time:%f", endtime-starttime);
	printf("\ncheck the matrix correct:");
	if (check)
		printf("success!");
	else
		printf("failure!");

	printf("\n");
	d2mce_Exit();

	return 0;

}
