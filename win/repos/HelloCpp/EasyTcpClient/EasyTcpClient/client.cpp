#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include<WinSock2.h>
#include<Windows.h>
#include<stdio.h>

#pragma comment(lib,"ws2_32.lib")//静态链接库
//ctrl + k ctrl + c注释代码，ctrl+u取消注释代码



enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGINOUT,
	CMD_LOGINOUT_RESULT,
	CMD_ERROR
};
///消息头
struct DataHeader
{
	short dataLength;//表示数据长度
	short cmd;//命令
};

//DataPackage
struct Login:public DataHeader
{
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char PassWord[32];
};

struct LoginResult:public DataHeader
{
	LoginResult()
	{
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
	}
	int result;

};

struct LoginOut:public DataHeader
{
	LoginOut()
	{
		dataLength = sizeof(LoginOut);
		cmd = CMD_LOGINOUT;
	}
	char userName[32];
};
struct LoginOutResult:public DataHeader
{
	LoginOutResult()
	{
		dataLength = sizeof(LoginOutResult);
		cmd = CMD_LOGINOUT_RESULT;
	}
	int result;

};

int main()
{
	//启动windows socket2.x环境
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
	////

	SOCKET _sock = socket(AF_INET,SOCK_STREAM,0);
	
	if (INVALID_SOCKET == _sock)
	{
		printf("ERROR,建立失败...\n");
	}
	else
	{
		printf("建立成功...\n");
	}
	//2.连接服务器
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	if(SOCKET_ERROR == connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in)))
	{
		printf("ERROR，建立socket失败...\n");
	}
	else
	{
		printf("建立socket成功...\n");
	}




	char cmdBuf[128] = {};
	while (true)
	{
		//3.输入请求命令
		scanf("%s", cmdBuf);

		//4.处理请求
		if (0 == strcmp(cmdBuf, "exit"))
		{
			printf("收到exit，任务结束\n");
			break;
		}

		else if (0 == strcmp(cmdBuf, "login"))
		{
			//向服务器发送请求
			Login login;
			strcpy(login.userName, "zj");
			strcpy(login.PassWord, "jainss");
			send(_sock, (const char *)&login, sizeof(login), 0);

			//接收服务器返回数据
			LoginResult loginRet = {};
			recv(_sock, (char*)&loginRet, sizeof(loginRet), 0);

			printf("LoginResult:%d\n",loginRet.result);
		}
		else if (0 == strcmp(cmdBuf, "loginout"))
		{
			LoginOut logout;
			strcpy(logout.userName, "zj");
			send(_sock, (const char *)&logout, sizeof(logout), 0);

			//接收服务器返回数据
			LoginOutResult loginoutRet = {};
			recv(_sock, (char*)&loginoutRet, sizeof(loginoutRet), 0);
			printf("LoginOutResult:%d\n", loginoutRet.result);
		}
		else
		{
			printf("不支持命令，请重新输入\n");
		}

	}

	

	closesocket(_sock);

	printf("客户端退出，任务结束\n");
	getchar();
	WSACleanup();
	return 0;
}