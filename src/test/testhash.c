#include <stdio.h>
#include <stdlib.h>
#include "../hashtable.h"

struct abc{
	int a;
	int b;
	struct abc *next;
};

int main(int argc, char* argv[]){
	struct hashtable *a;
	struct abc **tb;
	struct abc *abc;
	int i;
	a = hashTableCreate(10);
	tb = (struct abc**)getHashTable(a);
	for(i=0;i<10;i++){
		if(tb[i] == NULL)
			printf("%d:null\n",i);
	}
	for(i=0;i<5;i++){
		abc=malloc(sizeof(struct abc));
		abc->a=i+1;
		abc->b=i+2;
		tb[i]=abc;
	}
    for(i=0;i<10;i++){
        if(tb[i] == NULL)
            printf("%d:null\n",i);
		else
			printf("%d:a:%d,b:%d\n",i, tb[i]->a, tb[i]->b);
    }

	return 1;
}
