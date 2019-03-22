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
	CMD_NEW_USER_JOIN,
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

struct NewUserJoin :public DataHeader
{
	NewUserJoin()
	{
		dataLength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		sock = 0;
	}
	int sock;

};

int processor(SOCKET _cSock)
{
	//缓冲区
	char szRecv[1024] = {};

	//5.接收客户端的数据c
	int nLen = recv(_cSock, (char *)szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;
	if (nLen <= 0)
	{
		printf("与服务器断开连接，任务结束\n");
		return -1;
	}


	switch (header->cmd)
	{
	case CMD_LOGIN_RESULT:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);//之前已经把header读取出来了，不能再从头读取了
		LoginResult* login_Result = (LoginResult*)szRecv;
		printf("服务器数据消息：CMD_LOGIN_RESULT，数据长度%d\n", login_Result->dataLength);
	}
	break;
	case CMD_LOGINOUT:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);//之前已经把header读取出来了，不能再从头读取了
		LoginOutResult* loginout_Result = (LoginOutResult*)szRecv;
		printf("服务器数据消息：CMD_LOGINOUT_RESULT，数据长度%d\n", loginout_Result->dataLength);
	}
	break;

	case CMD_NEW_USER_JOIN:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);//之前已经把header读取出来了，不能再从头读取了
		NewUserJoin* userJoin = (NewUserJoin*)szRecv;
		printf("服务器数据消息：NEW_USER_JOIN，数据长度%d\n", userJoin->dataLength);
	}
	break;
	}
}

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
		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_sock, &fdReads);
		timeval t = { 0,0 };
		int ret = select(_sock, &fdReads, 0, 0, &t);
		if (ret < 0)
		{
			printf("select任务结束\n");
			break;
		}

		if (FD_ISSET(_sock, &fdReads))
		{
			FD_CLR(_sock, &fdReads);
			if (-1 == processor(_sock))
			{
				printf("seletc任务结束2\n");
				break;
			}
		}
		
		printf("客户端空闲，处理其他任务\n");
		Login login;
		strcpy(login.userName, "zj");
		strcpy(login.PassWord, "jainss");

		send(_sock, (const char * )&login, sizeof(Login), 0);
		//Sleep(1000);

	}

	

	closesocket(_sock);

	printf("已退出\n");
	getchar();
	WSACleanup();
	return 0;
}