#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <errno.h>

int main()
{
    //1、建立一个socket
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    //2、connect
    const char* ip = "127.0.0.1";
    struct sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(1234);
    inet_pton(AF_INET,ip,&serverAddr.sin_addr);
    socklen_t nAddrLen = sizeof(sockaddr_in);

    if(connect(sockfd,(sockaddr *)&serverAddr, nAddrLen) < 0)
    {
        printf("错误， 连接失败！");
    }
    else
    {
        printf("连接成功！\n");
    }
    //3、recv
    char recvBuf[256];
    int nrecv = recv(sockfd, recvBuf, 256, 0);        
    if(nrecv > 0)
    {
        printf("接收到数据：%s \n", recvBuf);
    }

    //4、close
    close(sockfd);
    return 0;

}