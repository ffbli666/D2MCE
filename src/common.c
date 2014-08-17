/*
# Description:
#   common.c
#
# Copyright (C) 2006- by EPS(Embedded and Parallel Systems Lab) @ NTUT CSIE
#
# Date: $Date: $
# Version: $Revision:  $
#
# History:
#
# $Log: $
#
#
*/

#include "common.h"

//!!need check more
//use a global variable "d2mec_conf"
int read_config(){
	
	FILE *file;
	char ch;
	char *value[2];
	char line[256]={0};
	int get_len=0;
	int value_len=0;
	snprintf(line, 255,"./%s",D2MCE_CONF_FILE);
	file = fopen(line, "r");
	if(file == NULL){
		snprintf(line, 255,"%s/%s/%s", getenv("HOME"), D2MCE_CONF_DIR, D2MCE_CONF_FILE);
		file = fopen(line, "r");
		if(file == NULL){
			printf("can't not open %s\n", line);
			return -1;
		}
	}
	while(!feof(file)){
		ch = getc(file);
		if(ch == '#'){
			while(getc(file) != '\n')
				if(feof(file))
					break;
			continue;
		}
		else if( ch=='\n')
			continue;
		else{
			if( feof(file) )
				break;
			fseek(file, -1, SEEK_CUR);
			get_len = fgetline(file, line, 255);
			split(' ', line, value, 2);
			value_len=strlen(value[1]);
			// read conf value. can add new variable and value ,if need.
			if( strcmp(value[0], "network_device")==0 ){
				if(value_len<=0 || value_len > D2MCE_CONF_NETDEV_LEN-1)
					return -1;
			snprintf(g_system_conf.network_device, D2MCE_CONF_NETDEV_LEN-1, "%s", value[1]);
			}
			else if( strcmp(value[0],"ip")==0 ){
				if(value_len>15)
					return -1;
				inet_pton(AF_INET, value[1], &g_system_conf.ip);
			}
			else if( strcmp(value[0], "subnet_mast")==0 ){
				if(value_len>15)
					return -1;
				inet_pton(AF_INET, value[1], &g_system_conf.mask);
			}
			else if( strcmp(value[0], "port")==0 ){
				if( atoi(value[1])<0 || atoi(value[1])>65536)
					return -1;
				g_system_conf.demon_port = atoi(value[1]);
			}
			bzero(&line, get_len);
		}
	}
	//check rule. add new , if need.
	if(strlen(g_system_conf.network_device)!=0){
		if((g_system_conf.ip = get_local_addr(g_system_conf.network_device))<0)
			return -1;
		if(get_local_hwaddr(g_system_conf.network_device, g_system_conf.hwaddr)<0)
			return -1;
			
	}
	if(g_system_conf.ip == 0)
		return -1;
	if(g_system_conf.broadcast_ip ==0){
		if(g_system_conf.network_device != NULL){
			if((g_system_conf.broadcast_ip = get_local_broadaddr(g_system_conf.network_device))<0)
				return -1;
		}
		else
			return -1;
	}
	if(g_system_conf.mask != 0)
		g_system_conf.broadcast_ip = g_system_conf.ip | ~g_system_conf.mask ;
	if(g_system_conf.demon_port == 0)
		return -1;
	fclose(file);
	return 1;
}


int fgetline(FILE *fp, char *line, int lim) {
    char *start;
    int c;

    start=line;
    while (--lim>1 && (c=getc(fp)) != EOF && c != '\n')
        *line++ = c;
    if (c == '\0')
        *line++ = c;
    else if (lim == 1) {
        *line++ = '\0';
        fprintf(stderr, "WARNING. fgetline: Line too long, splitted.\n");
    }
    *line = '\0';
    return  line - start;
}

int split(char symbol, char *line, char **lst, int lst_len)
{
	int i;
	int assign=0;
	lst[0] = line;
	for (i=1; *line; line++) {
		if (*line == symbol) {
			*line = '\0';
			if (i >= lst_len)
				break;
			lst[i++] = line + 1;
			assign++;
		}
	}
	assign++;
	if(assign <lst_len){
		for(i=assign; i<lst_len; i++)
			lst[i]=line;
	}
	return i;
}


char *str_malloc(char *src_str, int maxlen, int add_len){
    char *str;
    if(strlen(src_str)<maxlen){
        str = malloc(strlen(src_str)+ add_len);
	}
    else{
        str = malloc(maxlen + add_len);
	}
	
    return str;
}

unsigned int hash_str(char *str){
	int i;
	int len;
	unsigned int key;
	unsigned int number;
	key = 0;
	len = strlen(str);
    for(i=0; i<len; i+=sizeof(unsigned int)){
		number = 0;
		if((len-i) < sizeof(unsigned int))
			memcpy((void*)&number, str+i, len-i);
		else
	        memcpy((void*)&number, str+i, sizeof(unsigned int));
        key+=number;
    }
	return key;
}

