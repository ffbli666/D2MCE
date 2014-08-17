#include <stdio.h>
#include <stdlib.h>
#include "../d2mce.h"

int main(int argc, char *argv[]){
	d2mce_ginfo_t *group_table[10];
	int get_count = 10;
	int count;
	int i;
	char a;
	char ip[16];
	d2mce_init();
	count = d2mce_probe("matrix", "", group_table, get_count);
	printf("%d %d\n", count, get_count);
	for(i=0;i<count;i++)
		printf("app:%s, group:%s, ip:%s\n", group_table[i]->app_name, group_table[i]->group_name,  inet_ntop(AF_INET, &group_table[i]->ip , ip, 16 ));
	d2mce_join_gid(group_table[0]);
//	d2mce_barrier("wait other node", 2);
//	d2mce_barrier("wait all finish", 2);
	print_info();
	scanf("%c", &a);
	print_info();
	d2mce_finalize();
	return 1;	
}
