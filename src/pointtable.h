
#ifndef __POINTTABLE_H_
#define __POINTTABLE_H_

//#include "define.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct pointtable{
	unsigned short table_size;		//number of table row
	unsigned short index;
	unsigned short use;
	void **row;			//table
};

struct pointtable* pointTableCreate(int table_size);
__inline__ void** getPointTable(struct pointtable *pointtable);
int pointTableDestroy(struct pointtable *pointtable);
int pointTableGetEmpty(struct pointtable *pointtable);
int pointTableAttach(struct pointtable *pointtable, void *data, int index);
void* pointTableDetach(struct pointtable *pointtable, int index);
void* pointTableFat(struct pointtable *pointtable, unsigned int increase_size);
#endif

