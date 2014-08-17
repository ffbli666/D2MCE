#include <stdio.h>
#include <stdlib.h>

#include "../hashtable.h"

struct aaa{
	struct hashheader hash;
	int a1;
	char name;
	int a;
};

struct bbb{
	struct hashheader hash;
	char name;
	int b;
};


int main(){
	struct hashtable *tb;
	struct aaa *a, *atemp;
	struct bbb *b, *btemp;
	a=malloc(sizeof(struct aaa));
	b=malloc(sizeof(struct bbb));
	a->hash.id =1;
	a->hash.next = NULL;
	a->hash.name = malloc(1);
	strcpy(a->hash.name,"a");
	a->name = 'a';
	a->a1=10;
	a->a =1;
	b->hash.id =11;
	b->hash.next = NULL;
	b->hash.name = malloc(1);
	strcpy(b->hash.name, "b");
	b->name = 'b';
	b->b =2;
	tb = hashTableCreate(10);
	hashTableInsert(tb, (struct hashheader*)a);	
	hashTableInsert(tb, (struct hashheader*)b);
	atemp = hashTableSearch(tb, a->hash.id, a->hash.name);
	btemp = hashTableSearch(tb, b->hash.id, a->hash.name);
	printf("a:%d %c %d %d\n",atemp->hash.id, atemp->name, atemp->a, atemp->a1);
	printf("b:%d %c %d\n",btemp->hash.id, btemp->name, btemp->b);
	hashTableDestroy(tb);
	return 1;
}
