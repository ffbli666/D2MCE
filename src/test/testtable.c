#include <stdio.h>
#include <stdlib.h>
#include "../table.h"

struct abc{
	int a;
	int b;
	int c;
};
int main(int argc, char *argv[]){
	struct table *table;
	int row;
	struct abc **tb;
	struct abc r;
	struct abc *get;
	int i,j;
	table = tableCreate(10, sizeof(struct abc));
	tb = (struct abc**)getTable(table);
//	tb = getTable(table);
	for(i=0;i<12;i++){
		row = tableGetEmpty(table);
		if(row >= 0){
/*			tb[row]->a=i+1;
			tb[row]->b=i+2;
			tb[row]->c=i+3;
*/
			r.a=i+1;
			r.b=i+2;
			r.c=i+3;
			tableAdd(table, (void*)&r, row);
		}
		else
			printf("table is full\n");
	}
	for(i=0;i<10;i++){
		get = tableGetRow(table, i);
		printf("%d %d %d\n", get->a, get->b, get->c);
//		printf("%d %d %d\n", tb[i]->a, tb[i]->b, tb[i]->c);
	}
	printf("use %d\n", table->use);
	printf("is full %d\n\n", tableIsFull(table));
	tableRemove(table, 5);
	tableRemove(table, 1);
	row=tableGetEmpty(table);
/*	tb[row]->a=10;
	tb[row]->b=10;
	tb[row]->c=10;
*/
	r.a=10;
	r.b=10;
	r.c=10;
	tableAdd(table, (void*)&r, row);
    for(i=0;i<10;i++){
        printf("%d %d %d\n", tb[i]->a, tb[i]->b, tb[i]->c);
    }
	printf("use %d\n", table->use);	
	printf("is full %d\n", tableIsFull(table));
	
	tableDestroy(table);
	return 1;
}
