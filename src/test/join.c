#include <stdio.h>
#include <stdlib.h>
#include "../d2mce.h"

int main(int argc, char *argv[]){
	char input;
	d2mce_init();
	d2mce_join("matrix","eps", 0);
	print_info();
	scanf("%c", &input);
	print_info();
	d2mce_finalize();
	return 1;	
}
