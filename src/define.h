#ifndef __d2mce_define_h
#define __d2mce_define_h
#include <net/if.h>
#include <pthread.h>

//configure
#define CONF_SM_DIFF					//share memory: memory diff
//#define CONF_SM_AUTO_HOME_MIGARATION	//share memory: home migaration : use score board
#define CONF_SM_DISSEMINATE_UPDATE		//share memory: disseminate upate protocol
#define CONF_MANAGER_AUTO_MIGARATION	//manager: manager migration
#define CONF_SM_EVENT_DRIVER			//share memory: event driver

#define MEMORY_POOL_TOTAL_SIZE	102400	//100kb
#define MEMORY_POOL_64_COUNT	30		//3200byte
#define MEMORY_POOL_1024_COUNT	10		//10kb
#define MEMORY_POOL_10240_COUNT 2		//20kb
#define MEMORY_POOL_OTHER_COUNT 32
//share memory auto home migration
#ifdef CONF_SM_AUTO_HOME_MIGARATION
	#define SM_HOME_MIGRATION_THRESHOLD	10
#endif

//common
#define YES		1
#define NO		0

#define TRUE	1
#define FALSE	0

#define ERROR	-1
#define SUCCESS 1

#define ON		1
#define OFF		0


//name length
#define D2MCE_APP_NAME_LEN 		30
#define D2MCE_GROUP_NAME_LEN 	30
#define BARRIER_NAME_LEN		30
#define MUTEX_NAME_LEN			30
#define SEM_NAME_LEN			30
#define SM_NAME_LEN				30



#define node_info join_node_info
//request
#define JOIN_BUF_SIZE	(sizeof(struct join_s2d_req)+D2MCE_APP_NAME_LEN+D2MCE_GROUP_NAME_LEN)
#define JOIN_GRANT_SIZE	(sizeof(struct join_c2s_ok)+MAX_NODE_NUM*sizeof(struct join_node_info))
#define JOIN_PROBE_SIZE	sizeof(struct probe_d2s_reply)
#define JOIN_NEWNODE_SIZE 1024

#define BARRIER_INIT_SIZE	(sizeof(struct barrier_init_req)+BARRIER_NAME_LEN)
#define BARRIER_BUF_SIZE	(sizeof(struct barrier_req)+BARRIER_NAME_LEN)
#define BARRIER_IBAR_SIZE	10240

#define MUTEX_BUF_SIZE	(sizeof(struct mutex_req)+MUTEX_NAME_LEN)
#define MUTEX_IMUTEX_SIZE	10240


#define SEM_INIT_SIZE	(sizeof(struct sem_init_req)+SEM_NAME_LEN)
#define SEM_BUF_SIZE	(sizeof(struct sem_req)+SEM_NAME_LEN)
#define SEM_ISEM_SIZE	10240


#define SM_INIT_SIZE	(sizeof(struct sm_init_req)+SM_NAME_LEN)
#define SM_BUF_SIZE		(sizeof(struct sm_req)+SM_NAME_LEN)
#define SM_IHOME_SIZE	1024

#define MAIN_IMAIN_SIZE	10240
//receiver
#define RECEIVER_BUFSIZE 	1024
#define BUF_STACK_SIZE		50


/* d2mce config  define */
#define MAX_NODE_NUM 33
#define D2MCE_CONF_FILE		"d2mce.conf"
#define D2MCE_CONF_DIR		".d2mce_conf"
#define D2MCE_CONF_NETDEV_LEN     IFNAMSIZ

#define THREAD_NUM 5
#define DATA_THREAD_NUM 1

//table size
#define RECEIVER_TABLE_SIZE 	32
#define SENDER_TABLE_SIZE 		64
#define DATA_WQUEUE_SIZE		MAX_NODE_NUM+1
#define CONTROL_WQUEUE_SIZE		MAX_NODE_NUM+1	//MAX_NODE_NUM not enough
#define SYNC_TABLE_SIZE			32


#define BARRIER_HASH_SIZE	MAX_NODE_NUM
#define MUTEX_HASH_SIZE		MAX_NODE_NUM
#define SEM_HASH_SIZE		MAX_NODE_NUM
#define SM_HASH_SIZE		MAX_NODE_NUM


/* original the 4 it is control thread doing
#define LOCK_WQUEUE_SIZE 		10
#define SEM_WQUEUE_SIZE			10
#define BARRIER_WQUEUE_SIZE		10
#define JOIN_WQUEUE_SIZE			10
*/

//coordinator thread id

struct d2mce_conf_info{
        char network_device[D2MCE_CONF_NETDEV_LEN];     //device name
        unsigned int ip;                                //bind local ip
        unsigned int mask;                              //local subnet mask
        unsigned int broadcast_ip;                      //broadcasting ip
      	unsigned short port;						//record the local main port
        unsigned short demon_port;                  //demon listen port
        unsigned char hwaddr[6];                        //hardware addr
}g_system_conf;

//d2mce node table
struct node{
	unsigned int id;			//node id
	unsigned int ip;			//node ip
	unsigned short port;		//node port
	int recv_fd;				//node recv fd
	int send_fd;				//node send_fd
};
struct node_lnk{
	unsigned int  id;
	struct node_lnk *next;
};

struct group_coordinator{
	unsigned int  main_id;
	unsigned int  mutex_id;
	unsigned int  sem_id;
	unsigned int  barrier_id;
	pthread_mutex_t main_lock;
	pthread_mutex_t mutex_lock;
	pthread_mutex_t sem_lock;
	pthread_mutex_t barrier_lock;
};

struct d2mce_group{
	unsigned int group_id;				//group id =  coordinator's mac
	unsigned int node_id;				//the local node id
	unsigned int node_num;				//the group number of nodes;
	char *app_name;
	char *group_name; 					
	struct group_coordinator coordinator;
	struct table *node_table;
	struct hashtable *sm_table;			//share memory data
	struct hashtable *mutex_table;			//lock
	struct hashtable *barrier_table;		//barrier
	struct hashtable *sem_table;			//sem
}g_group;

struct d2mce_overhead{
	unsigned int msg_count;
	unsigned int msg_size;
}g_overhead;

/*
	Define diff infomation
*/
#define DIFF_THRESHOLD	2000000
#define DIFF_SIZE_RATE	1
#define DIFF_NODE_NUM	3

struct diff_header{
	unsigned int offset;
	unsigned int length;
};

struct diff{
	unsigned int size;
	void *data;
};

/*
  *     Define socket request Message
  */
  
//d2mce join msg define
enum msg_join_req{
    MSG_JOIN_REQ = 1,
    MSG_JOIN_GRANT,
    MSG_JOIN_NEWNODE_ADD
};

enum msg_join_reply{
    MSG_JOIN_OK = 5,	//it is coordinate or connect ok
    MSG_JOIN_REPLY,		//it is not coordinate shoulde send to coordinate
    MSG_JOIN_FAILED,	//connect fail
};

#define MSG_PROBE_REQ 10
#define MSG_PROBE_REPLY 11

//barrier
enum msg_barrier_req{
	MSG_BARRIER_INIT = 20,
	MSG_BARRIER_REQ,
	MSG_IBAR_MANAGER,		//i am barrier manager
	MSG_NEWBAR_MANAGER
};

enum msg_barrier_reply{
	MSG_BARRIER_OK = 30,
	MSG_BARRIER_FAILED,
	MSG_IBAR_REPLY,
	MSG_IBAR_READY
};
//mutex
enum msg_mutex_req{
	MSG_MUTEX_INIT = 40,
	MSG_MUTEX_LOCK,
	MSG_MUTEX_UNLOCK,
	MSG_MUTEX_TRY,
	MSG_IMUTEX_MANAGER,
	MSG_NEWMUTEX_MANAGER
};
enum msg_mutex_reply{
	MSG_MUTEX_OK = 50,
	MSG_MUTEX_FAILED,
	MSG_IMUTEX_REPLY,
	MSG_IMUTEX_READY
};
//semaphore
enum msg_semaphore_req{
	MSG_SEM_INIT = 60,
	MSG_SEM_POST,
	MSG_SEM_WAIT,
	MSG_SEM_TRY,
	MSG_ISEM_MANAGER,
	MSG_NEWSEM_MANAGER
};

enum msg_semaphore_reply{
	MSG_SEM_OK = 70,
	MSG_SEM_FAILED,
	MSG_ISEM_REPLY,
	MSG_ISEM_READY
};
//share memory
enum msg_share_memory_req{
	MSG_SM_INIT = 80,
	MSG_SM_READMISS,
	MSG_SM_WRITEMISS,
	MSG_SM_WRITEMISS_NOREPLY,
	MSG_SM_FETCH,
	MSG_SM_INVALID,
	MSG_SM_INVALID_NOREPLY,
	MSG_SM_MULTIREAD,
	MSG_SM_MULTIWRITE,
	MSG_SM_MULTIWRITE_NOREPLY,
	MSG_SM_UPDATE_REGISTER,
	MSG_SM_UPDATE_UNREGISTER,
	MSG_SM_UPDATE,
	MSG_SM_IAMHOME,
	MSG_SM_HOME_READY,
	MSG_SM_NEWHOME,
	MSG_SM_UHOME
};

enum msg_share_memory_reply{
	MSG_SM_OK = 120,
	MSG_SM_FAILED,
	MSG_SM_INIT_REPLY,
	MSG_SM_INVALID_OK,
	MSG_SM_IAMHOME_REPLY
};

enum msg_main_manager_req{
	MSG_IMAIN_MANAGER = 150,
	MSG_NEWMAIN_MANAGER,
};

enum msg_main_manager_reply{
	MSG_MAIN_OK = 160,
	MSG_MAIN_FAILED,
	MSG_IMAIN_REPLY,
	MSG_IMAIN_READY,
};

enum msg_manager_req{
	MSG_YOU_MANAGER = 170,
	MSG_NODE_EXIT,
};
enum msg_manager_reply{
	MSG_YOU_OK = 180,
};

#define	MSG_FINALIZE	300
#define MSG_FINALIZE_OK 310 

#define MSG_D2MCE_TERMINAL 9999

#endif
