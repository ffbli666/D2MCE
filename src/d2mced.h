/*
# Description:
#   d2mced.h
#
# Copyright (C) 2006- by EPS(Embedded and Parallel Systems Lab) @ NTUT CSIE
#
# Date: $Date: 2007/12/20 13:08:14 $
# Version: $Revision: 1.2 $
#
# History:
#
# $Log: d2mced.h,v $
# Revision 1.2  2007/12/20 13:08:14  ffbli
# add log
#
#
*/

#ifndef __D2MCED_H_
#define __D2MCED_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <syslog.h>
#include <unistd.h>
#include <error.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <net/if.h>
#include <sys/stat.h>

#include "define.h"
#include "socket.h"
#include "common.h"
#include "join.h"
#include "table.h"
#include "finalize.h"
#include "manager.h"

#define D2MCED_GROUP_NUMBER 10
#define D2MCED_BUFF_SIZE 256

struct group{
    char app_name[D2MCE_APP_NAME_LEN];		//daemon application name
    char group_name[D2MCE_GROUP_NAME_LEN];	//daemon group name
    unsigned int id;						//coordinate id
    unsigned int ip;						//coordinate ip
    unsigned short int port;				//coordinate computing(main) thread port
	int group_instance;						//hardware address
//	unsigned short int group_id;			//the table index
};

void demon_init(const char *pname, int facility);
void  demon_receiver(int sockfd);					//listen the require
int find_group(char *app_name, char *group_name);	//return -1 then failed
int find_group_useID(unsigned int group_id);

int* find_groups(char *app_name, char *group_name, int *group_num);


int join_request(void *recvbuf, int sockfd, struct sockaddr_in recv_addr);
int finalize_request(void *recvbuf, int sockfd, struct sockaddr_in recv_addr);
int probe_request(void *recvbuf, int sockfd, struct sockaddr_in recv_addr);
int new_manager_request(void *recvbuf, int sockfd, struct sockaddr_in recv_addr);

#endif //__D2MCED_H_
