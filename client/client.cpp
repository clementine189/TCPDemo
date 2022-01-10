#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <thread>

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

int processor(int sockfd)
{
    //5.recv 
    char recvBuf[4096] = {};
    int nLen = recv(sockfd, recvBuf, sizeof(DataHeader), 0);
    DataHeader* header = (DataHeader*)recvBuf;
    if(nLen <= 0)
    {
        printf("与服务器断开连接，任务结束.\n");
        //有客户端退出，返回-1，交给主函数处理
        return -1;
        
    }
    switch(header->cmd)
    {
        case CMD_LOGIN_RESULT:
        {
            recv(sockfd, recvBuf+sizeof(DataHeader), header->dataLength-sizeof(DataHeader), 0);
            LoginResult *loginret = (LoginResult*) recvBuf;
            printf("收到服务端消息: CMD_LOGIN_RESULT 数据长度: %d\n", loginret->dataLength);
            break;
        }         

        case CMD_LOGOUT_RESULT:            
        {
            recv(sockfd, recvBuf+sizeof(DataHeader), header->dataLength-sizeof(DataHeader), 0);
            LogoutResult *logoutret = (LogoutResult*)recvBuf;
            printf("收到服务端消息: CMD_LOGOUT_RESULT 数据长度: %d \n", logoutret->dataLength);
            break;
        }
        case CMD_NEW_USER_JION:
        {
            recv(sockfd, recvBuf+sizeof(DataHeader), header->dataLength-sizeof(DataHeader), 0);
            NewUserJion *userjion = (NewUserJion*)recvBuf;
            printf("收到服务端消息: CMD_NEW_USER_JION 数据长度: %d \n", userjion->dataLength);
            break;
        }
        
    }
    return 0;               
}

bool g_bRun = true;
void cmdThread(int sockfd)
{
    
    while(1)
    {
        char cmdBuf[256] = {};
        scanf("%s", cmdBuf);
        if(0 == strcmp(cmdBuf, "exit"))
        {
            printf("退出cmdThread线程\n");
            g_bRun = false;
            return ;
        }else if(0 == strcmp(cmdBuf, "login"))
        {
            Login login;
            strcpy(login.userName, "clementine");
            strcpy(login.PassWord, "1234");
            send(sockfd, (const char*)&login, sizeof(Login), 0);
        }else if(0 == strcmp(cmdBuf, "logout"))
        {
            Logout logout;
            strcpy(logout.userName, "clementine");
            send(sockfd, (const char*)&logout, sizeof(logout), 0);
        }        
    }

}

int main()
{
    //1、建立一个socket
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    int maxfd = sockfd;
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
    //启动线程
    std::thread t1(cmdThread, sockfd);
    //考虑线程退出的问题
    t1.detach();
    while(g_bRun)
    {

        fd_set fdRead;

        FD_ZERO(&fdRead);
 
        FD_SET(sockfd, &fdRead);

        timeval t {10, 0};
        int ret = select(maxfd + 1, &fdRead, 0, 0, &t);
        if(ret < 0)
        {
            printf("select任务结束1\n");
            break;
        }
        if(FD_ISSET(sockfd, &fdRead))
        {
            FD_CLR(sockfd, &fdRead);

            if(-1 == processor(sockfd))
            {
                printf("select任务结束2\n");
                break;
            }
        }
        printf("空闲时处理其他问题\n");
    }
    //7、close
    close(sockfd);
    printf("已退出，任务结束！\n");
    return 0;

}