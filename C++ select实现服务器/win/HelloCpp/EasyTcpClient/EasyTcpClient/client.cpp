#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include<WinSock2.h>
#include<Windows.h>
#include<stdio.h>

#pragma comment(lib,"ws2_32.lib")//静态链接库
//ctrl + k ctrl + c注释代码，ctrl+u取消注释代码


struct DataPackage
{
	int age;
	char name[32];
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
			getchar();
			break;
		}
		else
		{
			//5.向服务器发送请求命令
			send(_sock, cmdBuf, strlen(cmdBuf) + 1, 0);
		}

		//6.接收服务器信息
		char recvBuf[128] = {};
		int nlen = recv(_sock, recvBuf, 128, 0);
		if (nlen > 0)
		{
			DataPackage* dp = (DataPackage*)recvBuf;
			printf("接收到数据：年龄：%d, 姓名：%s\n", dp->age,dp->name);
		}
	}

	
	getchar();
	closesocket(_sock);

	printf("客户端退出，任务结束\n");
	getchar();
	WSACleanup();
	return 0;
}