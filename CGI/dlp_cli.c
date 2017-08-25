#include 	<stdio.h>  
#include 	<sys/types.h>  
#include	<sys/socket.h>  
#include	<unistd.h>  
#include	<stdlib.h>  
#include	<errno.h>  
#include	<arpa/inet.h>  
#include	<netinet/in.h>  
#include	<string.h>  

#define ERR_EXIT(m) \
do{	\
	perror(m); \
	exit(EXIT_FAILURE); \
}while (0)

int main(void)
{
	int sock;
	pid_t pid;
	unsigned char reg[12] = { 0xFE,0x0C,0x01,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0xFF };
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		//  listenfd = socket(AF_INET, SOCK_STREAM, 0)
		ERR_EXIT("socket error");

	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(7777);
	servaddr.sin_addr.s_addr = inet_addr("120.25.205.94");

	if (connect(sock, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
		ERR_EXIT("connect error");
	struct sockaddr_in localaddr;
	char cli_ip[20];
	socklen_t local_len = sizeof(localaddr);
	memset(&localaddr, 0, sizeof(localaddr));
	if (getsockname(sock, (struct sockaddr *) &localaddr, &local_len) != 0)
		ERR_EXIT("getsockname error");
	inet_ntop(AF_INET, &localaddr.sin_addr, cli_ip, sizeof(cli_ip));
	printf("host %s:%d\n", cli_ip, ntohs(localaddr.sin_port));
	
	write(sock, reg, 12);
//	memset(sendbuf, 0, sizeof(sendbuf));
	fd_set rset;
	int nready,ret;
	FD_ZERO(&rset);
	char recvbuf[1024] = { 0 };
	while (1)
	{
		FD_SET(sock, &rset);
		nready = select(sock + 1, &rset, NULL, NULL, NULL);
		if (nready == -1)
			ERR_EXIT("select error");
		if (nready == 0)
			continue;
		if (FD_ISSET(sock, &rset))
		{
			ret = read(sock, recvbuf, sizeof(recvbuf));
			if (ret == -1)
				ERR_EXIT("read error");
			else if (ret == 0)
			{
				printf("server close\n");
				break;
			}
			else
			{
				if ((pid = fork()) < 0) 
					exit(0);
				else if(pid == 0)
				{
					if(recvbuf[2] == '1')
					{
						printf("play 1!\n");
						execl("/usr/bin/gplay","gplay","/root/1.264",(char *) 0);
					}
					else if(recvbuf[2] == '2')
					{
						printf("play 2!\n");
						execl("/usr/bin/gplay","gplay","/root/2.264",(char *) 0);
					}
					else
						execl("/usr/bin/gplay","gplay","/root/3.264",(char *) 0);
					memset(recvbuf, 0, ret);
				}
			}
		}
	}

	close(sock);
	return 0;
}
