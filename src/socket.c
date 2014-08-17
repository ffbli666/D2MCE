#include "socket.h"


//get ipv4 ip
in_addr_t get_local_addr(char *dev){
	struct ifreq ifr;
	int sockfd;
	struct sockaddr_in *sock_ptr;
	if(dev ==NULL)
		return -1;
	if( strlen(dev) > IFNAMSIZ)
		return -1;
	if((sockfd = socket( PF_INET, SOCK_DGRAM, 0 )) < 0)
		return -1;
	strncpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name));
	//get ip
	if(ioctl(sockfd, SIOCGIFADDR, &ifr) < 0)
		return -1;
	sock_ptr=(struct sockaddr_in*) &ifr.ifr_addr;
	close(sockfd);
	return ntohl(sock_ptr->sin_addr.s_addr);
}
//get ipv4 broadcast
in_addr_t get_local_broadaddr(char *dev){
	struct ifreq ifr;
	int sockfd;
	struct sockaddr_in *sock_ptr;
	if(dev ==NULL)
		return -1;
	if( strlen(dev) > IFNAMSIZ)
		return -1;
	if((sockfd = socket( PF_INET, SOCK_DGRAM, 0 )) < 0)
		return -1;
	strncpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name));
	//get broadcast ip
	if(ioctl(sockfd, SIOCGIFBRDADDR, &ifr) < 0)
		return -1;
	sock_ptr=(struct sockaddr_in*) &ifr.ifr_broadaddr;
	close(sockfd);
	return ntohl(sock_ptr->sin_addr.s_addr);
}
//get device MAC
int get_local_hwaddr(char *dev, unsigned char hwaddr[6]){
    struct ifreq ifr;
    int sockfd;
    if(dev ==NULL)
		return -1;
    //struct sockaddr_in *sock_ptr;
    if( strlen(dev) > IFNAMSIZ)
        return -1;
    if((sockfd = socket( PF_INET, SOCK_DGRAM, 0 )) < 0)
        return -1;
    strncpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name));
    //get device MAC
    if(ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0)
        return -1;
    memcpy(hwaddr, ifr.ifr_hwaddr.sa_data, 6);
    close(sockfd);
    return 1;
}

unsigned short get_nonuse_port(){
    struct sockaddr_in recv_addr, bind_addr;
    int sockfd;
    socklen_t socklen;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&recv_addr,sizeof(recv_addr));
    recv_addr.sin_family = AF_INET;
    recv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    recv_addr.sin_port = 0;
    if (bind(sockfd, (struct sockaddr *)&recv_addr, sizeof(recv_addr))) {
        perror("bind:");
		return -1;
    }

    socklen = sizeof(bind_addr);
    if (getsockname(sockfd,(struct sockaddr *) &bind_addr, &socklen)) {
        perror("getsockname:");
		return -1;
    }

   // printf("Allocated port number = %u\n", ntohs(sin_read.sin_port));
    close(sockfd);
    return ntohs(bind_addr.sin_port);
}

unsigned short get_listen_port(int listen_fd){
	 socklen_t len;
	struct sockaddr_in sin;
    len = sizeof(sin);
    if (getsockname(listen_fd, (struct sockaddr *)&sin, &len)){
        perror("get_listen_port:");
        return -1;
    }
	return ntohs(sin.sin_port);
}


int readable_timer(int fd, int sec,int usec){
    fd_set fset;
    struct timeval time;

    FD_ZERO(&fset);
    FD_SET(fd, &fset);
    time.tv_sec = sec;
    time.tv_usec = usec;
    return (select(fd+1, &fset, NULL, NULL, &time));
}



int TCP_init(unsigned int ip, unsigned short port, struct sockaddr_in *sockaddr){
	int sockfd;
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))<0){
		perror("Error:TCP socket");
		exit(-1);
   	}
    sockaddr->sin_family = AF_INET;
    sockaddr->sin_addr.s_addr = htonl(ip);
	if(port == 0)
		sockaddr->sin_port = 0;		
	else
	    sockaddr->sin_port = htons(port);	
	return sockfd;
}

int TCP_listen(unsigned int listen_ip, unsigned short listen_port){
    int listenfd;
    struct sockaddr_in server_addr;
	listenfd = TCP_init(listen_ip, listen_port,&server_addr);
    if( bind(listenfd, (struct sockaddr *) &server_addr, sizeof(server_addr))<0 ){
        perror("Error: TCP bind()");
        exit(-1);
    }

    if( listen(listenfd, MAX_NODE_NUM+1)<0) {
        perror("Error: TCP listen()");
        exit(-1);
    }
	return listenfd;

}

int TCP_connect(unsigned int dest_ip,unsigned short dest_port){
    int connfd;
    struct sockaddr_in server_addr;
	connfd = TCP_init(dest_ip,dest_port,&server_addr);
    if( connect(connfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
        perror("Error: TCP connect()");
        exit(-1);
    }
	return connfd;
}

int UDP_init(unsigned int ip, unsigned short port, struct sockaddr_in *sockaddr){
    int sockfd;
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0){
		perror("Error: UDP socket");
		exit(-1);
    }
	sockaddr->sin_family = AF_INET;
	sockaddr->sin_addr.s_addr = htonl(ip);
   	if(port == 0)
		sockaddr->sin_port = 0;		
	else
	    sockaddr->sin_port = htons(port);
	return sockfd;
}

int UDP_bind(unsigned int listen_ip, unsigned short listen_port){
	int listenfd; 
	struct sockaddr_in server_addr;
	listenfd = UDP_init(listen_ip,listen_port, &server_addr);
	if( bind(listenfd, (struct sockaddr *) &server_addr, sizeof(server_addr))<0 ){
    	perror("Error: UDP bind()");
    	exit(-1);
    }
	return listenfd;

}

int UDP_setBroadcast(int sockfd){	
    int broadcast = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast))<0){
        perror("Error: UDP setBroadcast()\n");
		exit(-1);
    }
	return 1;
}

int UDP_setTTL(int sockfd, int ttl){
	if(ttl<1)
		return -1;
	if(setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl))<0){
		perror("Error: UDP setTTL()\n");
		exit(-1);
	}
	return 1;
}


