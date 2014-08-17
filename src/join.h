/*
# Description:
#   join.h
#
# Copyright (C) 2006- by EPS(Embedded and Parallel Systems Lab) @ NTUT CSIE
#
# Date: $Date: 2007/12/24 02:44:39 $
# Version: $Revision: 1.3 $
#
# History:
#
# $Log: join.h,v $
# Revision 1.3  2007/12/24 02:44:39  ffbli
# dos2unix
#
# Revision 1.2  2007/12/20 13:08:14  ffbli
# add log
#
#
*/
#ifndef __JOIN_H_
#define __JOIN_H_

#include "define.h"
#include "network.h"
#include "table.h"
#include "memory.h"

/* 
	header name
	
	2 = to
	d = demon
	s = src
	c = coordinator

	ex: s2c = src to coordinator


	group_id = group_instance
*/

struct join_s2d_req{    	        //join request to demon packet header
	unsigned short msg_type;        //MSG_JOIN_REQ
	unsigned short app_len;    		//join application name len
    unsigned short group_len;		//join group name len
    unsigned short port;			//source port
    unsigned int group_instance;	//mac address
};
//req.src_node_ip use source ip
struct join_s2c_req{
	struct request_header req;
	unsigned int ip;
	unsigned short port;			//source port
};

struct join_d2s_ok{
	unsigned short msg_type;     		//MSG_JOIN_OK
	unsigned int group_id;				//group_id
};

struct join_d2s_reply{				//daemon join acknowledge packet header
    unsigned short msg_type;			//enum d2mce_msg_join ack
    unsigned int group_id;				//group_instance
    unsigned int id;
    unsigned int ip;					//coordinator ip
    unsigned short port;				//coordinator's computing(main) port
};

struct join_c2s_ok{					//coordinator reply the node join ok
	struct request_header req;
	unsigned int group_id;
    unsigned int  node_id;				//the node index
    unsigned short node_num;			//node_num
 	unsigned int mutex_id;
	unsigned int sem_id;
	unsigned int barrier_id;
};

struct join_node_info{
	unsigned int  id;			//node id
	unsigned int ip;			//node ip
	unsigned short int port;	//node port
};


struct join_c2c_newNode{
	struct request_header req;
	struct join_node_info node_info;
};


/*
struct exit_s2d_req{
	unsigned int msg_type;				//enum d2mce_msg_join ack
   	int group_id;						//the table index
//    unsigned short int app_len;    		//join application name len
//    unsigned short int group_len;		//join group name len
};
*/
struct probe_s2d_req{                	
	unsigned short msg_type;     	//D2MCE_MSG_JOIN_REQ
    unsigned short app_len;    		//join application name len
    unsigned short group_len;		//join group name len
    int group_num;
//	int group_instance;
};

struct probe_d2s_reply{
	unsigned short msg_type;
	char app_name[D2MCE_APP_NAME_LEN];
	char group_name[D2MCE_GROUP_NAME_LEN];
	unsigned int group_instance;
	unsigned int ip;                    //coordinator ip
	unsigned short port;				//coordinator's computing(main) port
	unsigned int id;
};

int join_grant_req(unsigned int group_id, unsigned int id, unsigned int ip, unsigned short port);
int join_grant_reply(void *request);
int join_newNodeAdd(struct join_c2c_newNode *request);


#endif //JOIN_H_

