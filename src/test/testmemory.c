#include <stdio.h>
#include <stdlib.h>
#include "../memory.h"
struct a{
	int a;
	int b;
};

double mtime(){
    double gettime;
    struct timeval now;
    gettimeofday(&now, 0);
    gettime = 1000000*now.tv_sec + now.tv_usec;
    gettime /= 1000000;
    return gettime;
}


int main(int argc, char* argv[]){
	int i;
	double time;
	void *buf;
//	void *buf2;
	int times = atoi(argv[1]);
	int sizes = atoi(argv[2]);
	//buf2 = malloc(51200000000);
	mem_init();
	time = -mtime();
	for(i=0;i<times;i++){
		buf = malloc(sizes);
		((struct a*)buf)->a=i;
		((struct a*)buf)->b=i+1;
		free(buf);
	}
	time += mtime();
	printf("malloc time %f\n",time);
	
    time = -mtime();
    for(i=0;i<times;i++){
        buf = mem_malloc(sizes);
        ((struct a*)buf)->a=i;
        ((struct a*)buf)->b=i+1;
		mem_free(buf);
    }
    time += mtime();
    printf("mem_malloc time %f\n",time);

	mem_destroy();
	return 1;
}
