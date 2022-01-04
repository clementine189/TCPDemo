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
    int port = 1234;
    const char * ip = "127.0.0.1";
    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET,ip,&server_address.sin_addr);
    server_address.sin_port = htons(port);
    //1、建立一个socket
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    //2、bind用于接收客户端的端口号
    if(bind(sockfd, (struct sockaddr *)&server_address, sizeof(sockaddr_in)) < 0)
    {
        printf("错误，绑定失败！\n");
    }
    else
    {
        printf("绑定成功！\n");
    }
    //3、listen
    if(listen(sockfd, 5) < 0)
    {
        printf("错误，监听失败！\n");
    }
    else
    {
        printf("监听成功！\n");
    }
    //4、accept
    struct sockaddr_in clientAddr = {};
    socklen_t nAddrLen = sizeof(sockaddr_in);
    char msgBuf[] = "Hello, I'm Server.";
    while(true)
    {
        int clientsockfd = accept(sockfd, (struct sockaddr *)&clientAddr, &nAddrLen);
        if(clientsockfd < 0)
        {
            printf("错误，接受到无效客户端！\n");
        }
        printf("新客户端加入：IP = %s \n", inet_ntoa(clientAddr.sin_addr));
        //5、send
        send(clientsockfd, msgBuf, strlen(msgBuf)+1, 0);        
    }

    //6、close
    close(sockfd);
    return 0;

}