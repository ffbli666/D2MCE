#include <stdio.h>
#include <stdlib.h>
#include "../../include/d2mce.h"

#define BUCKETS       256         /* number of buckets for each proc */
#define SEED            1
#define NODES			1
unsigned int *a, *b, *flag;
int node_id;

unsigned int pow(int x)
{
	unsigned int i, j;
	i = 0;
	j = 1;
	for (i = 0; i < x; i++)
		j *= 2;
	return j;
}

unsigned int log2(int x)
{
	unsigned int i, j;
	i = 0;
	j = 1;
	while (x >= j * 2) {
		i++;
		j = j * 2;
	}
	return i;
}

main(int argc, char ** argv)
{
	unsigned int count[BUCKETS*16];
	int i, j, k, x, y, counter, z;
	int remainder, temp1, temp2, temp3, temp, start, locked;
	double time1, time2, time3, time4, time5, time6;
	unsigned int KEY, MAX, RANGE, MAGIC, BSIZE, PAGES;
	d2mce_barrier_t b1;
	d2mce_mutex_t m1;

	if (argc >= 2) {
		if (argv[1][0] == 'a') KEY = 262144;
		else if (argv[1][0] == 'b') KEY = 524288;
		else if (argv[1][0] == 'c') KEY = 1048576;
		else if (argv[1][0] == 'd') KEY = 2097152;
		else if (argv[1][0] == 'e') KEY = 4194304;
		else if (argv[1][0] == 'f') KEY = 8388608;
		else KEY = 4194304;
	} else KEY = 4194304;
	d2mce_init();
// jia_init(argc, argv);
	node_id = d2mce_join("bucketsort","eps",0);
// jia_barrier();
	time1 = d2mce_time();
// time1 = jia_clock();

	BSIZE = KEY / NODES / BUCKETS;
	PAGES = BSIZE / 1024 + 1;
	BSIZE = PAGES * 1024;
	d2mce_mutex_init(&m1,"m1");
	d2mce_barrier_init(&b1,"b1");
	a = d2mce_malloc("a",BUCKETS * BSIZE * NODES * sizeof(unsigned int));
	b = d2mce_malloc("b", KEY * sizeof(unsigned int));
	flag = d2mce_malloc("flag",4096);
//   a = (unsigned int *) jia_alloc(BUCKETS * BSIZE *
//                                  jiahosts * sizeof(unsigned int));
//   b = (unsigned int *) jia_alloc(KEY * sizeof(unsigned int));
//   flag = (unsigned int *) jia_alloc(4096);

	d2mce_barrier(&b1, NODES);
// jia_barrier();
	printf("Stage 1: Memory allocation done!\n");
// time2 = jia_clock();
	time2 = d2mce_time();

//   srand(jiapid+SEED);
	srand(node_id+SEED);
	MAGIC = (KEY / NODES) * node_id;

//   if (jiapid == 0)
	if (node_id == 0) {
		flag[0] = 0;
		flag[1] = 0;
	}

//   for (i = 0; i < KEY / jiahosts; i++)
	for (i = 0; i < KEY / NODES; i++) {
		temp1 = rand() % 1024;
		temp2 = rand() % 2048;
		temp3 = rand() % 2048;
		j = MAGIC + i;
		b[j] = (temp1 << 22) + (temp2 << 11) + temp3;
		if (b[j] == 0) b[j] = 1;
	}
	d2mce_barrier(&b1, NODES);
//   jia_barrier();
//   time3 = jia_clock();
	time3 = d2mce_time();
// printf("Stage 2: %d Integers initialized!\n", KEY / jiahosts);
	printf("Stage 2: %d Integers initialized!\n", KEY / NODES);
	for (i = 0; i < BUCKETS * 16; i++)
		count[i] = 0;

	RANGE = pow(32 - log2(BUCKETS));
	printf("range = %d, d2mcehosts = %d\n", RANGE, NODES);

	for (i = 0; i < KEY / NODES; i++) {
		j = MAGIC + i;
		k = (b[j] / RANGE) * NODES + node_id;
		/*
		 *       printf("k at %d\n", k);
		 *             printf("A got at %d, B %d\n", k * BSIZE + count[k], j);
		 *             */
		a[k * BSIZE + count[k]] = b[j];

		count[k]++;
	}
	d2mce_barrier(&b1, NODES);
//   jia_barrier();
//   time4 = jia_clock();
	time4 = d2mce_time();
	printf("Stage 3: Distribution done!\n", KEY);

	RANGE = RANGE / NODES;

	for (i = 0; i < BUCKETS * 16; i++)
		count[i] = 0;

	for (i = 0; i < BUCKETS / NODES; i++) {
		counter = KEY / NODES * node_id;
		for (j = 0; j < NODES; j++) {
			k = (node_id * BUCKETS + i * NODES + j) * BSIZE;
			if (k > 16 * 1048576) printf("ERROR KEY!\n");
			/* printf("a[%d] = %d, \n", k, a[k]); */
			while (a[k] > 0) {
				b[counter] = a[k];
				counter++;
				k++;
				/*   printf("a[%d] = %d\n", k, a[k]); */
			}
		}

		/* printf("RANGE calculated!\n"); */

		for (j = KEY / NODES * node_id; j < counter; j++) {
			k = b[j] / RANGE;
			/* printf("j = %d, b[j] = %d, k = %d, index = %d\n",
			*                  j, b[j], k, k * BSIZE + count[k]); */
			a[k * BSIZE + count[k]] = b[j];
			count[k]++;
		}

		for (j = 0; j < NODES; j++) {
			k = node_id * BUCKETS + i * NODES + j;

			MAGIC = k * BSIZE;
			printf("MAGIC = %d\n", MAGIC);

			/*
			 * 	 if (k == 0)
			 * 	 	    for (x = 0; x < count[k]; x++)
			 * 	 	    	       printf("a[%d] = %d\n", x, a[x]);
			 * 	 	    	       */

			for (x = count[k]; x >= 1; x--)
				for (y = 2; y <= x; y++) {
					z = MAGIC + y - 1;
					if (a[z-1] > a[z]) {
						temp = a[z-1];
						a[z-1] = a[z];
						a[z] = temp;
					}
				}

			/*
			 * 	 if (k == 0)
			 * 	 	    for (x = 0; x < count[k]; x++)
			 * 	 	    	       printf("a[%d] = %d\n", x, a[x]);
			 * 	 	    	       */
		}
	}
	d2mce_barrier(&b1, NODES);
//   jia_barrier();
//   time5 = jia_clock();
	time5 = d2mce_time();
	printf("Stage 4: Local Sorting done!\n", KEY);

	counter = 0;
	for (i = 0; i < BUCKETS*16; i++)
		counter += count[i];
	d2mce_mutex_lock(&m1);
//   jia_lock(0);
	d2mce_mutex_lock(&m1);
	while (flag[0] != node_id) { //jia_unlock(0);
		d2mce_mutex_unlock(&m1);
		for (i = 0; i < rand() * 100; i++);
		d2mce_mutex_lock(&m1);
//      jia_lock(0);
	}

	start = flag[1];
	flag[1] += counter;
	printf("My start is %d, end at %d\n", start, start + counter);
	flag[0]++;
	// jia_unlock(0);
	d2mce_mutex_lock(&m1);

	x = start;
	if (x % BSIZE > 0) {
//    jia_lock(node_id);
		d2mce_mutex_lock(&m1);
				locked = 1;
	} else
		locked = 0;

	for (i = BUCKETS * node_id; i < BUCKETS * (node_id + 1); i++) {
		y = i * BSIZE;
		for (j = 0; j < count[i]; j++) {
			if (locked == 1 && x % BSIZE == 0) {
				//          jia_unlock(node_id);
				d2mce_mutex_unlock(&m1);
				locked = 0;
			} else if (locked == 0 && start + counter - x < BSIZE
			           && x % BSIZE == 0) {
//		 jia_lock(node_id+1);
				d2mce_mutex_lock(&m1);
				locked = 1;
			}
			/*
			 *          if (x == 199 || x == 202)
			 *                      printf("a[%d + %d] = %d\n", y, j, a[y+j]);
			 *                      */
			b[x] = a[y + j];
			x++;
		}
	}

	if (locked == 1)
//		jia_unlock(node_id+1);
		d2mce_mutex_unlock(&m1);

//   jia_barrier();
	d2mce_barrier(&b1,NODES);

//   time6 = jia_clock();
	time6 = d2mce_time();
	printf("Stage 5: Write back to array done!\n");

	if (node_id == 0)
		for (i = 0; i < KEY-1; i++)
			if (b[i] > b[i+1])
				printf("Error in keys %d (%d) and %d (%d)\n", i, b[i], i+1,
				       b[i+1]);

	printf("Time\t%f\t%f\t%f\t%f\t%f\t%f\n", time2-time1, time3-time2,
	       time4-time3, time5-time4, time6-time5, time6-time1);

	//jia_exit();
	d2mce_finalize();

	return 0;
}

