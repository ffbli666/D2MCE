#include "pointtable.h"

#define MAX_POINTTABLE_SIZE 60000

struct pointtable* pointTableCreate(int table_size){
	struct pointtable *pointtable;
	pointtable = (struct pointtable*)malloc(sizeof(struct pointtable));
	pointtable->table_size = table_size;
	pointtable->index = 0;
	pointtable->use = 0;
	pointtable->row = (void**)malloc(sizeof(void*)*table_size);
	return pointtable;
}
__inline__ void** getPointTable(struct pointtable *pointtable){
	return pointtable->row;
}
int pointTableDestroy(struct pointtable *pointtable){
	free(pointtable->row);
	free(pointtable);
	return 1;
}

int pointTableGetEmpty(struct pointtable *pointtable){
	int count = 0;
	while(1){	
		if( ++count >= pointtable->table_size)
			break;
		if(pointtable->index >= pointtable->table_size)
			pointtable->index = 0;
		if( pointtable->row[pointtable->index] == NULL)
			return pointtable->index++;	
		++pointtable->index;
	}
	return -1;
}

int pointTableAttach(struct pointtable *pointtable, void *data, int index){
	pointtable->row[index] = data;
	pointtable->use++;
	return 1;
}
void* pointTableDetach(struct pointtable *pointtable, int index){
	void *ptr;
	ptr = pointtable->row[index];
	pointtable->row[index] = NULL;	
	pointtable->use--;
	pointtable->index = index;
	return ptr;
}

void* pointTableFat(struct pointtable *pointtable, unsigned int increase_size){
	int i;
	struct pointtable *new_table;
	if(pointtable->table_size>=MAX_POINTTABLE_SIZE)
		return pointtable;
	new_table = (struct pointtable*)malloc(sizeof(struct pointtable));
	if(new_table == NULL)
		return NULL;
	new_table->use = pointtable->use;
	new_table->index = pointtable->index;
	new_table->table_size = pointtable->table_size + increase_size;
	if(new_table->table_size > MAX_POINTTABLE_SIZE)
		new_table->table_size = MAX_POINTTABLE_SIZE;
	new_table->row= (void**)malloc(sizeof(void*)*new_table->table_size);
	if(new_table->row == NULL)
		return NULL;
	for(i=0; i<pointtable->table_size; i++){
		if(pointtable->row[i]!=NULL)
			new_table->row[i] = pointtable->row[i];
	}
	if(pointtable->row!=NULL)
		free(pointtable->row);
	if(pointtable!=NULL)
		free(pointtable);
	return new_table;
}


