#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <sys/types.h>          /* See NOTES */
#include    <sys/socket.h>
#include    <errno.h>
#include    <netinet/in.h>
#include    <arpa/inet.h>
char InputBuffer[4096] = {0};

int DecodeAndProcessData(char *input); 	//具体译码和处理数据，该函数代码略

int control(char* context)
{
    int socket_fd;
    char temp[64] = {0};
    struct sockaddr_in server_addr;
    char rBuf[128] = {0};
    if((socket_fd = socket(AF_INET,SOCK_STREAM,0)) < 0 )
    {
        printf("create socket error: %s(errno:%d)\n)",strerror(errno),errno);
        return 0;
    }
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(4444);

    if(inet_pton(AF_INET,"127.0.0.1",&server_addr.sin_addr) <=0)
    {
        printf("inet_pton error!\n");
        return 0;
    }
    if(connect(socket_fd,(struct sockaddr*)&server_addr,sizeof(server_addr))<0)
    {
       // printf("connect error: %s(errno: %d)\n",strerror(errno),errno);
        return 0;
    }
    if(send(socket_fd,context,strlen(context),0 ) < 0)
    {
        printf("send message error\n");
        return 0;
    }
    while(1)
    {
        if(recv(socket_fd, rBuf, sizeof(rBuf), 0) <= 0)
            break;
    }
    sprintf(temp,"{'isSuccess':%d}",rBuf[0]-48);
    printf("%s\n",temp);
    close(socket_fd);
    return 1;
}
int main(int argc, char*argv[])
{
    int ContentLength;	// 数据长度
    int i,x;
    char *p;
    char *pRequestMethod; 	// METHOD属性值
    setvbuf(stdin, NULL, _IONBF, 0); // 关闭stdin的缓冲
    //   printf("Content-type:text/html\n\n"); // 从stdout中输出，告诉Web服务器返回的信息类型
    printf("Content-type: text/html\n\n");
    //    printf("\n"); // 插入一个空行，结束头部信息
    // 从环境变量REQUEST_METHOD中得到METHOD属性值
    pRequestMethod = getenv("REQUEST_METHOD");
    if (pRequestMethod == NULL)
    {
        printf("<p>request = null</p>");
        return 0;
    }
    if (strcasecmp(pRequestMethod, "POST") == 0)
    {
        //      printf("<p>OK the method is POST!\n</p>");
        p = getenv("CONTENT_LENGTH");     //从环境变量CONTENT_LENGTH中得到数据长度
        if (p != NULL)
            ContentLength = atoi(p);
        else
            ContentLength = 0;
        if (ContentLength > sizeof(InputBuffer) - 1)
            ContentLength = sizeof(InputBuffer) - 1;
        i = 0;
        while (i < ContentLength)
        {                         //从stdin中得到Form数据
            x = fgetc(stdin);
            if (x == EOF)
                break;
            InputBuffer[i++] = x;
        }
        InputBuffer[i] = '\0';
        ContentLength = i;
        //parseJson(InputBuffer);
    }
    else if (strcasecmp(pRequestMethod, "GET") == 0)
    {
        p = getenv("QUERY_STRING");     //从环境变量QUERY_STRING中得到Form数据
        if (p != NULL)
        {
            strncpy(InputBuffer, p, sizeof(InputBuffer));
            DecodeAndProcessData(InputBuffer);    //具体译码和处理数据，该函数代码略
        }
    }
    return 0;
}

int DecodeAndProcessData(char *input)    //具体译码和处理数据
{
	char buf[25] = { 0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x33,0x31,0x33,0x32,0x33,0x33,0x33,0x34,0x33,0x35,0x33,0x36,0x33,0x37,0x33,0x38 };
	char *str;
	str = strstr(input,"name");
	if(str != NULL)
	{
		str += 5;
		buf[0] = *str;
    	printf("Success!\n");
    	control(buf);
    }
   	else
   		printf("MSG: not found!\n");
    return 0;
}
