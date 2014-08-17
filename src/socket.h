#ifndef __SOCKET_H_
#define __SOCKET_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "define.h"

// get local address (ip) , give the device name
in_addr_t get_local_addr(char *eth);
// get local broadcast address (ip) , give the device name
in_addr_t get_local_broadaddr(char *eth);
// get local hardware address(mac)
int get_local_hwaddr(char *dev, unsigned char hwaddr[6]);

//get a non use port
unsigned short get_nonuse_port();
unsigned short get_listen_port(int listen_fd);

//set the fd timer , if timeout the fd not have data will return 0
int readable_timer(int fd, int sec,int usec);

//TCP network
int TCP_init(unsigned int ip, unsigned short  port, struct sockaddr_in *sockaddr);
int TCP_listen(unsigned int listen_ip, unsigned short  listen_port);
int TCP_connect(unsigned int dest_ip, unsigned short  dest_port);

//UDP network
int UDP_init(unsigned int ip, unsigned short  port, struct sockaddr_in *sockaddr);
int UDP_bind(unsigned int listen_ip, unsigned short  listen_port);
int UDP_setBroadcast(int sockfd);	
int UDP_setTTL(int sockfd, int ttl);


#endif

