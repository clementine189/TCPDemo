#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <errno.h>
enum  CMD
{
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
    CMD_ERROR
};

//消息头
struct DataHeader
{
    short dataLength;//数据长度
    short cmd;//命令
};
//登录
struct Login:public DataHeader
{
    Login()
    {
        dataLength = sizeof(Login);
        cmd = CMD_LOGIN;
    }
    /* data */
    char userName[32];
    char PassWord[32];
};
//登录结果
struct LoginResult:public DataHeader
{
    LoginResult()
    {
        dataLength = sizeof(LoginResult);
        cmd = CMD_LOGIN_RESULT;
        result = 0;
    }
    /* data */
    int result;
};
//登出
struct Logout:public DataHeader
{
    Logout()
    {
        dataLength = sizeof(Logout);
        cmd = CMD_LOGOUT;
    }
    /* data */
    char userName[32];
};
//登出结果
struct LogoutResult:public DataHeader
{
    LogoutResult()
    {
        dataLength = sizeof(LogoutResult);
        cmd = CMD_LOGOUT_RESULT;
        result = 1;
    }
    /* data */
    int result;
};



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
    
    
        int clientsockfd = accept(sockfd, (struct sockaddr *)&clientAddr, &nAddrLen);
        if(clientsockfd < 0)
        {
            printf("错误，接受到无效客户端！\n");
        }
        printf("新客户端加入：IP = %s \n", inet_ntoa(clientAddr.sin_addr));
    
    while(true)
    {
        //5.recv 
        DataHeader header = {};
        //char recvBuf[128] = {};
        int nLen = recv(clientsockfd, (char *)&header, sizeof(DataHeader), 0);
        if(nLen < 0)
        {
            printf("客户端已退出，任务结束.");
        }
        // printf("收到命令： %d 数据长度： %d\n", header.cmd, header.dataLength);
        switch(header.cmd)
        {
            case CMD_LOGIN:
                {
                    Login login = {};
                    recv(clientsockfd, (char *)&login+sizeof(DataHeader), sizeof(Login)-sizeof(DataHeader), 0);
                    printf("收到命令: CMD_LOGIN 数据长度: %d UerName: %s PassWord: %s\n", login.dataLength, login.userName, login.PassWord);
                    //忽略判断用户信息的正确性
                    LoginResult ret;
                    // send(clientsockfd, (const char *)&header, sizeof(DataHeader), 0);
                    send(clientsockfd, (const char *)&ret, sizeof(LoginResult), 0);             
                    break;
                }         

            case CMD_LOGOUT:            
                    {
                        Logout logout = {};
                        recv(clientsockfd, (char *)&logout+sizeof(DataHeader), sizeof(Logout)-sizeof(DataHeader), 0);
                        printf("收到命令: CMD_LOGOUT 数据长度: %d UserName: %s\n", logout.dataLength, logout.userName);
                        //忽略判断用户信息的正确性
                        LogoutResult ret;
                        //  send(clientsockfd, (const char *)&header, sizeof(DataHeader), 0);
                        send(clientsockfd, (const char *)&ret, sizeof(LogoutResult), 0);            
                        break;
                    }
            default:            
                    {                
                        header.cmd = CMD::CMD_ERROR;
                        header.dataLength = 0;
                        send(clientsockfd, (const char *)&header, sizeof(DataHeader), 0);
                    
                        break;
                    }

        }
        // //6、处理请求

        // if(0 == strcmp(recvBuf, "getInfo"))
        // {
        //     printf("收到命令：%s\n", recvBuf);
        //     DataPackage dp = {27, "clementine"};
            
        //     send(clientsockfd, (const char *)&dp, sizeof(DataPackage), 0);
        // }
        // else
        // {
        //     printf("收到无效命令： %s\n", recvBuf);
        //     char msgBuf[] = "???";
        //     send(clientsockfd, msgBuf, strlen(msgBuf)+1, 0); 
        // }
               
    }

    //7、close
    close(sockfd);
    printf("已退出，任务结束！\n");
    return 0;

}