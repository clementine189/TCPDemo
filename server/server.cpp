#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <vector>
#include <algorithm>

enum  CMD
{
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
    CMD_NEW_USER_JION,
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
//新用户加入
struct NewUserJion: public DataHeader
{
    /* data */
    NewUserJion()
    {
        dataLength = sizeof(NewUserJion);
        cmd = CMD_NEW_USER_JION;
        sock = 0;
    }
    int sock;
};

//客户端socket
std::vector<int> g_clients;

int processor(int clientsockfd)
{
    //5.recv 
    char recvBuf[4096] = {};
    
    int nLen = recv(clientsockfd, recvBuf, sizeof(DataHeader), 0);
    DataHeader* header = (DataHeader*)recvBuf;
    if(nLen <= 0)
    {
        printf("客户端<socket: %d>已退出，任务结束.\n", clientsockfd);
        //有客户端退出，返回-1，交给主函数处理
        return -1;
        
    }
    switch(header->cmd)
    {
        case CMD_LOGIN:
            {
                
                recv(clientsockfd, recvBuf+sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
                Login *login = (Login*)recvBuf;
                printf("收到客户端<socket: %d>命令: CMD_LOGIN 数据长度: %d UserName: %s PassWord: %s\n", clientsockfd, login->dataLength, login->userName, login->PassWord);
                //忽略判断用户信息的正确性
                LoginResult ret;
                send(clientsockfd, (const char *)&ret, sizeof(LoginResult), 0);             
                break;
            }         

        case CMD_LOGOUT:            
            {
                recv(clientsockfd, recvBuf+sizeof(DataHeader), header->dataLength-sizeof(DataHeader), 0);
                Logout *logout = (Logout*)recvBuf;
                printf("收到客户端<socket: %d>命令: CMD_LOGOUT 数据长度: %d UserName: %s\n", clientsockfd, logout->dataLength, logout->userName);
                //忽略判断用户信息的正确性
                LogoutResult ret;
                send(clientsockfd, (const char *)&ret, sizeof(LogoutResult), 0);            
                break;
            }
        default:            
            {                
                header->cmd = CMD_ERROR;
                header->dataLength = 0;
                send(clientsockfd, (const char *)&header, sizeof(DataHeader), 0);
                
                break;
            }
    }
    return 0;               
}

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
    int maxfd = sockfd;
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

    
    while(true)
    {
        fd_set fdRead;
        fd_set fdWrite;
        fd_set fdExp;

        FD_ZERO(&fdRead);
        FD_ZERO(&fdWrite);
        FD_ZERO(&fdExp);

        FD_SET(sockfd, &fdRead);
        FD_SET(sockfd, &fdWrite);
        FD_SET(sockfd, &fdExp);

        for(int i = 0; i < g_clients.size(); ++i)
        {
            //把这些客户端放入可读事件查询集中
            FD_SET(g_clients[i], &fdRead);
        }

        //非阻塞模式
        timeval t = {10, 0};
        //nfds是一个整数值，是指fd_set集合中所有描述符
        int ret = select(maxfd + 1, &fdRead, &fdWrite, &fdExp, &t);
        if(ret < 0)
        {
            printf("select任务结束。 \n");
            break;
        }
        //可读事件,判断描述符是否在集合中
        if(FD_ISSET(sockfd, &fdRead))
        {
            FD_CLR(sockfd, &fdRead);
            //4、accept
            struct sockaddr_in clientAddr = {};
            socklen_t nAddrLen = sizeof(sockaddr_in);
            int clientsockfd = accept(sockfd, (struct sockaddr *)&clientAddr, &nAddrLen);
            if(maxfd < clientsockfd)
            {
                maxfd = clientsockfd;
            }
            if(clientsockfd < 0)
            {
                printf("错误，接受到无效客户端！\n");
            }
            else
            {
                //向所有其他客户端发送新加入客户端的信息
                for(int i = 0; i < g_clients.size(); ++i)
                {
                    
                    NewUserJion userJion;
                    userJion.sock = clientsockfd;
                    send(g_clients[i], (const char*)&userJion, sizeof(NewUserJion), 0);
                }

                g_clients.push_back(clientsockfd);
                printf("新客户端加入：IP = %s \n", inet_ntoa(clientAddr.sin_addr));                  
            }        
        }
        for(int i = 0; i <= maxfd; ++i)
        {

            if(FD_ISSET(i, &fdRead)&&(-1 == processor(i)))
            {
                auto iter = std::find(g_clients.begin(), g_clients.end(), i);
                if(iter != g_clients.end())
                {
                    g_clients.erase(iter);
                }
            }
        }

        printf("空闲时间处理其他业务...\n");
    }

    for(int i = 0; i < g_clients.size(); ++i)
    {
        close(g_clients[i]);
    }

    //7、close
    close(sockfd);
    printf("已退出，任务结束！\n");
    return 0;

}