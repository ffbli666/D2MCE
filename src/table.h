#ifndef __TABLE_H_
#define __TABLE_H_

//#include "define.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct table{
	unsigned short index;			//now index of table
	unsigned short table_size;		//number of rows
	unsigned short data_size;		//size of  row
	unsigned short use;
	void **row;			//table
};


struct table* tableCreate(int table_size, int data_size);
int tableDestroy(struct table *table);
int tableGetEmpty(struct table *table);
__inline__ void* tableGetRow(struct table *table, int index);
int tableAdd(struct table *table, void* data, int index);
int tableRemove(struct table *table, int index);
__inline__ void** getTable(struct table *table);
__inline__ int tableIsFull(struct table *table);
void* tableFat(struct table *table, unsigned int increase_size);

#endif

