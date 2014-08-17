/* 
# Description:
#   d2mced.c
#
# Copyright (C) 2006- by EPS(Embedded and Parallel Systems Lab) @ NTUT CSIE
#
# Date: $Date: 2007/12/24 02:44:39 $
# Version: $Revision: 1.3 $
#
# History:
#
# $Log: d2mced.c,v $
# Revision 1.3  2007/12/24 02:44:39  ffbli
# dos2unix
#
# Revision 1.2  2007/12/20 13:08:14  ffbli
# add log
#
#
*/
#include "d2mced.h"

#define MAXFD 64

//int g_GroupIndex = 0;	//Group index
//int g_GroupCount = 0;	//Group count, number app and group 

struct table *g_GroupTable;

int main(int argc, char *argv[]){
	int listenfd;
//	daemon_init(argv[0], 0);

	g_GroupTable = tableCreate(D2MCED_GROUP_NUMBER, sizeof(struct group));

	if(read_config()<0){
        perror("Error: read_config()");
		return -1;
	}
	//UDP_init(unsigned int ip,unsigned short int port,struct sockaddr_in * sockaddr)
	listenfd = UDP_bind(INADDR_ANY, g_system_conf.demon_port);
    demon_receiver(listenfd);
	tableDestroy(g_GroupTable);
//	closelog();
    return 1;
}

void demon_receiver(int sockfd){
    ssize_t recv_len;
    struct sockaddr_in recv_addr;
	socklen_t len=sizeof(recv_addr);
	unsigned short *msg_type;
	void *recvbuf = malloc(D2MCED_BUFF_SIZE);	//recv buf
	
	while(1) {
      	recv_len = recvfrom(sockfd, recvbuf, D2MCED_BUFF_SIZE, 0, (struct sockaddr *) &recv_addr, &len);
		msg_type = (unsigned short*)recvbuf;
		//check the require, and data length to process
		if( *msg_type == MSG_JOIN_REQ && recv_len == (sizeof(struct join_s2d_req)
			+((struct join_s2d_req*)recvbuf)->app_len+((struct join_s2d_req*)recvbuf)->group_len)) {

			join_request(recvbuf, sockfd, recv_addr);
		}
		else if(*msg_type == MSG_FINALIZE && recv_len == sizeof(struct finalize_s2d_req)) {
			finalize_request(recvbuf, sockfd, recv_addr);
		}
		else if(*msg_type == MSG_PROBE_REQ 	&& recv_len == (sizeof(struct probe_s2d_req)
				+((struct probe_s2d_req*)recvbuf)->app_len+((struct probe_s2d_req*)recvbuf)->group_len)) {	
			probe_request(recvbuf, sockfd, recv_addr);				
		}
		else if(*msg_type == MSG_NEWMAIN_MANAGER && recv_len == (sizeof(struct main_s2d_req)
			+((struct main_s2d_req*)recvbuf)->app_len+((struct main_s2d_req*)recvbuf)->group_len)) {
			new_manager_request(recvbuf, sockfd, recv_addr);
		}
		//init data
		msg_type = 0;
	}
	free(recvbuf);
    close(sockfd);
}

int join_request(void *recvbuf, int sockfd, struct sockaddr_in recv_addr){
	unsigned short msg_type;
	int find_index;
	int empty_index;
	char app_name[D2MCE_APP_NAME_LEN+1] = {0};
	char group_name[D2MCE_GROUP_NAME_LEN+1] = {0};
	struct join_d2s_reply join_reply;
	struct join_d2s_ok join_ok;
	struct group **groupTable;
	groupTable = (struct group**)getTable(g_GroupTable);
	memcpy(app_name, recvbuf + sizeof(struct join_s2d_req), ((struct join_s2d_req*)recvbuf)->app_len);
	memcpy(group_name, recvbuf + sizeof(struct join_s2d_req) + ((struct join_s2d_req*)recvbuf)->app_len , ((struct join_s2d_req*)recvbuf)->group_len);
//   no one in the table,send the ack to the require node, it's the coordinator
	if( (find_index = find_group(app_name, group_name)) == -1) {
		if(tableIsFull(g_GroupTable)){	
			msg_type = MSG_JOIN_FAILED;
			sendto(sockfd, (void*) &msg_type, sizeof(int), 0, (struct sockaddr*) &recv_addr, sizeof(recv_addr));
			return 1;
		}
		else {//find the empty index
			empty_index = tableGetEmpty(g_GroupTable);
			strncpy( groupTable[empty_index]->app_name, app_name, D2MCE_APP_NAME_LEN);
//			memcpy(groupTable[empty_index]->app_name, app_name, strlen(app_name));
			strncpy( groupTable[empty_index]->group_name, group_name, D2MCE_GROUP_NAME_LEN);
//			memcpy(groupTable[empty_index]->group_name, group_name, strlen(group_name));
			groupTable[empty_index]->port = ((struct join_s2d_req*)recvbuf)->port;
			groupTable[empty_index]->id = 0;
			groupTable[empty_index]->ip = ntohl(recv_addr.sin_addr.s_addr);
			groupTable[empty_index]->group_instance = ((struct join_s2d_req*)recvbuf)->group_instance;
			g_GroupTable->use++;
		
			join_ok.msg_type = MSG_JOIN_OK;
			join_ok.group_id = ((struct join_s2d_req*)recvbuf)->group_instance;
				sendto(sockfd, (void*) &join_ok, sizeof(struct join_d2s_ok),	
					0, (struct sockaddr*) &recv_addr, sizeof(recv_addr));
			return 1;
		}				
	}
	else {	
		//find coordinator, send the coordinator to the require node
		
		join_reply.msg_type = MSG_JOIN_REPLY;
		join_reply.port = groupTable[find_index]->port;
		join_reply.id = groupTable[find_index]->id;
		join_reply.ip = groupTable[find_index]->ip;
		join_reply.group_id = groupTable[find_index]->group_instance;
		sendto(sockfd, (void*) &join_reply, sizeof(struct join_d2s_reply),
				 0, (struct sockaddr*) &recv_addr, sizeof(recv_addr));	
		return 1;
	}
	return -1;
}


int finalize_request(void *recvbuf, int sockfd, struct sockaddr_in recv_addr){
	int find_index;
	unsigned short msg_type;
	if( (find_index = find_group_useID(((struct finalize_s2d_req*)recvbuf)->group_id)) >= 0 ) {
		tableRemove(g_GroupTable, find_index);
		msg_type=MSG_FINALIZE_OK;
		sendto(sockfd, (void*) &msg_type, sizeof(int), 0, (struct sockaddr*) &recv_addr, sizeof(recv_addr));
		return 1;
	}
	return -1;
}

int probe_request(void *recvbuf, int sockfd, struct sockaddr_in recv_addr){
	int *getIndex;
	int i;
	struct probe_d2s_reply probe_reply;
	struct group **groupTable;
	char app_name[D2MCE_APP_NAME_LEN+1] = {0};
	char group_name[D2MCE_GROUP_NAME_LEN+1] = {0};
	
	groupTable = (struct group**)getTable(g_GroupTable);
	memcpy(app_name, recvbuf + sizeof(struct probe_s2d_req), ((struct probe_s2d_req*)recvbuf)->app_len);
	memcpy(group_name, recvbuf + sizeof(struct probe_s2d_req) + ((struct probe_s2d_req*)recvbuf)->app_len , ((struct probe_s2d_req*)recvbuf)->group_len);

	getIndex = find_groups(app_name, group_name, &((struct probe_s2d_req*)recvbuf)->group_num);

	for(i=0; i<((struct probe_s2d_req*)recvbuf)->group_num ; i++){
		probe_reply.msg_type = MSG_PROBE_REPLY;
		strncpy(probe_reply.app_name, groupTable[getIndex[i]]->app_name, D2MCE_APP_NAME_LEN);
//		memcpy(probe_reply.app_name, groupTable[getIndex[i]]->app_name, strlen(groupTable[getIndex[i]]->app_name));
		strncpy(probe_reply.group_name, groupTable[getIndex[i]]->group_name, D2MCE_GROUP_NAME_LEN);
//		memcpy(probe_reply.group_name, groupTable[getIndex[i]]->group_name, strlen(groupTable[getIndex[i]]->group_name));
		probe_reply.group_instance = groupTable[getIndex[i]]->group_instance;
		probe_reply.ip = groupTable[getIndex[i]]->ip;
		probe_reply.port = groupTable[getIndex[i]]->port;
		probe_reply.id = groupTable[getIndex[i]]->id;
		sendto(sockfd, (void*) &probe_reply, sizeof(struct probe_d2s_reply), 0, (struct sockaddr*) &recv_addr, sizeof(recv_addr));
	}	
	free(getIndex);
	return 1;
}

int new_manager_request(void *recvbuf, int sockfd, struct sockaddr_in recv_addr){
	int find_index;
	char app_name[D2MCE_APP_NAME_LEN+1] = {0};
	char group_name[D2MCE_GROUP_NAME_LEN+1] = {0};
	struct request_header req;
	struct group **groupTable;
	groupTable = (struct group**)getTable(g_GroupTable);
	memcpy(app_name, recvbuf + sizeof(struct main_s2d_req), ((struct main_s2d_req*)recvbuf)->app_len);
	memcpy(group_name, recvbuf + sizeof(struct main_s2d_req) + ((struct main_s2d_req*)recvbuf)->app_len , ((struct main_s2d_req*)recvbuf)->group_len);
	//find, change data and send the ok
	if( (find_index = find_group(app_name, group_name)) >= 0) {
		groupTable[find_index]->id = ((struct main_s2d_req*)recvbuf)->id;
		groupTable[find_index]->port = ((struct main_s2d_req*)recvbuf)->port;
		groupTable[find_index]->ip = ((struct main_s2d_req*)recvbuf)->ip;
//		groupTable[find_index]->group_instance = ((struct main_s2d_req*)recvbuf)->group_instance;
		g_GroupTable->use++;
		req.msg_type = MSG_MAIN_OK;
		sendto(sockfd, (void*) &req, sizeof(struct main_s2d_req),	
				0, (struct sockaddr*) &recv_addr, sizeof(recv_addr));
		return 1;
	}
	//not find send the error
	else {	
		req.msg_type = MSG_MAIN_FAILED;
		sendto(sockfd, (void*) &req, sizeof(struct join_d2s_reply),
				0, (struct sockaddr*) &recv_addr, sizeof(recv_addr));	
		return 1;
	}
	return -1;
}

void demon_init(const char *pname, int facility){
	int i;
	pid_t pid;
	if((pid = fork())!=0)
		exit(0);
	setsid();
	signal(SIGHUP, SIG_IGN);
	if((pid = fork())!=0)
		exit(0);
	chdir("/");
	umask(0);
	for(i=0; i<MAXFD; i++)
		close(i);
	openlog(pname, LOG_PID, facility);
}

int find_group(char *app_name, char *group_name){
	int i;
	struct group **groupTable;
	groupTable = (struct group**)getTable(g_GroupTable);
	for( i=0; i<D2MCED_GROUP_NUMBER; i++) {
		if( strcmp( groupTable[i]->group_name, group_name) == 0){
			if( strcmp( groupTable[i]->app_name, app_name) == 0){
				return i;
			}
		}
	}
	return -1;
}

int find_group_useID(unsigned int group_id){
	int i;
	struct group **groupTable;
	groupTable = (struct group**)getTable(g_GroupTable);
	for( i=0; i<D2MCED_GROUP_NUMBER; i++) {
		if( groupTable[i]->group_instance == group_id){
			return i;
		}
	}
	return -1;
}



//return value: int* it's int arrray. the index array indicate  thr group with the same group & app.
//group_num: number of group will find.
int* find_groups(char *app_name, char *group_name, int *group_num){
	int *getIndex;
	int i;
	int count = 0;
	char search_app_name=0;
	char search_group_name=0;
	struct group **groupTable;
	groupTable = (struct group**)getTable(g_GroupTable);
	getIndex = malloc(sizeof(int)*(*group_num));
	if(strlen(app_name) >0)
		search_app_name=1;
	if(strlen(group_name) >0)
		search_group_name=1;
	for( i=0; i<D2MCED_GROUP_NUMBER; i++){
		if(count >= *group_num)
			break;
		if(search_app_name == 1 && strcmp( groupTable[i]->app_name, app_name) != 0)
			continue;
		if(search_group_name ==1 && strcmp( groupTable[i]->group_name, group_name) != 0)
			continue;
		getIndex[count]=i;
		count++;
	}
	*group_num = count;
	return getIndex;
}


