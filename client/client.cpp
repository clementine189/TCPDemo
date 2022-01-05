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
    char cmdBuf[128] = {};
    while(true)
    {
        //3、输入请求
        scanf("%s",cmdBuf);

        //4、处理请求命令
        if(0 == strcmp(cmdBuf,"exit"))
        {
            break;
        }
        else if(0 == strcmp(cmdBuf,"login"))
        {
            //5、向服务器发送登入请求命令
            
            Login login;// = {"clementine", "123456"};
            strcpy(login.userName, "clementine");
            strcpy(login.PassWord, "123456");
            send(sockfd, (const char*)&login, sizeof(Login), 0);

            //6、接收服务器返回数据
            LoginResult loginret = {};
            recv(sockfd, (char*)&loginret, sizeof(LoginResult), 0);
            printf("LoginResult: %d\n", loginret.result);

        }
        else if(0 == strcmp(cmdBuf,"logout"))
        {   //5、向服务器发送登出请求命令
            Logout logout;// = {"clementine"};
            strcpy(logout.userName, "clementine");
            send(sockfd, (const char*)&logout, sizeof(Logout), 0);
            //6、接收服务器返回数据
            LogoutResult logoutret = {};
            recv(sockfd, (char*)&logoutret, sizeof(LogoutResult), 0);
            printf("LogoutResult: %d\n", logoutret.result);
        }
        else
        {
            printf("不支持的命令，请重新输入。 \n");
        }
    
        //6、recv
        
        // int nrecv = recv(sockfd, (char *)&header, sizeof(DataHeader), 0);        
        // if(nrecv > 0)
        // {
        //     DataPackage* dp = (DataPackage*)recvBuf; 
        //     printf("接收到数据：年龄： %d ,姓名： %s \n", dp->age, dp->name);
        // }

    }
    //7、close
    close(sockfd);
    printf("已退出，任务结束！\n");
    return 0;

}