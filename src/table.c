#include "table.h"

#define MAX_TABLE_SIZE 60000

struct table* tableCreate(int table_size, int data_size){
	struct table *table;
	int i;
	int *row;
	table = (struct table*)malloc(sizeof(struct table));
	if(table==NULL)
		return NULL;
	table->index = 0;
	table->use = 0;
	table->table_size = table_size;
	table->data_size = data_size;
	table->row = (void**)malloc(sizeof(void*)*table_size);
	if(table->row==NULL)
		return NULL;
	for(i=0;i<table_size;i++){
		table->row[i] = (void*)malloc(data_size);
		if(table->row[i]==NULL)
			return NULL;
		row = table->row[i];
		*row = -1;
	}
	return table;
}
int tableGetEmpty(struct table *table){
	int count = 0;
	int *row;
	while(1)
	{	
		if( ++count >= table->table_size)
			break;
		if(table->index >= table->table_size)
			table->index = 0;
		row = table->row[table->index];
		if( *row == -1)
			return table->index++;	
		++table->index;
	}
	return -1;
}
__inline__ void* tableGetRow(struct table *table, int index){
	return table->row[index];
}

int tableAdd(struct table *table, void* data, int index){
	memcpy(table->row[index], data, table->data_size);
	table->use++;
	return 1;
}

int tableRemove(struct table *table, int index){
	int *row;
	bzero(table->row[index], table->data_size);
	row = table->row[index];
	*row = -1;
	table->index = index;
	table->use--;
	return 1;
}
__inline__ void** getTable(struct table *table){
	return table->row;
}
int tableDestroy(struct table *table){
	int i;
	for(i=0; i<table->table_size; i++){
		if(table->row[i]!=NULL)
			free(table->row[i]);
	}
	if(table->row!=NULL)
		free(table->row);
	if(table!=NULL);
		free(table);
	return 1;
}
__inline__ int tableIsFull(struct table *table){
	return table->use == table->table_size;
}

void* tableFat(struct table *table, unsigned int increase_size){
	int i;
	int *row;
	struct table *new_table;
	if(table->table_size>=MAX_TABLE_SIZE)
		return table;
	new_table = (struct table*)malloc(sizeof(struct table));
	if(new_table == NULL)
		return NULL;
	new_table->use = table->use;
	new_table->index = table->index;
	new_table->table_size = table->table_size + increase_size;
	if(new_table->table_size > MAX_TABLE_SIZE)
		new_table->table_size = MAX_TABLE_SIZE;
	new_table->data_size = table->data_size;
	new_table->row= (void**)malloc(sizeof(void*)*new_table->table_size);
	if(new_table->row == NULL)
		return NULL;
	for(i=0; i<new_table->table_size; i++){
		if(i<table->table_size){
			if(table->row[i]!=NULL )
				new_table->row[i] = table->row[i];
			}
		else{
			new_table->row[i] = (void*)malloc(new_table->data_size);	
			row = new_table->row[i];
			*row = -1;
		}
	}
	free(table->row);
	free(table);
	return new_table;
}

