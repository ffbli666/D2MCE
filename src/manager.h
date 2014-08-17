/*
	main manager
	node join, share memory info table, node exit
	
*/

#ifndef __MANAGER_H_
#define __MANAGER_H_


#include "define.h"
#include "network.h"
#include "sharememory.h"
#include "table.h"

struct main_s2d_req{
	unsigned short msg_type;        //MSG_NEWMAIN_MANAGER
	unsigned short app_len;    		//join application name len
    unsigned short group_len;		//join group name len
    unsigned short port;			//source port
    unsigned int ip;
	unsigned int id;
//    unsigned int group_instance;	//mac address
};


struct imain_reply{
	struct request_header req;
	unsigned int sm_num;	
};

struct main_sm_info{
	unsigned int hash_id;
	unsigned short name_len;
	unsigned int home_node;
	unsigned int size;
	unsigned int count;
	unsigned int users_count;
//users
};



int main_imanager_reply(void *request);
int main_newmanager_reply(void *request);
int main_imanager_req();


#endif
