#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include<WinSock2.h>
#include<Windows.h>
#include<stdio.h>

////#pragma comment(lib,"ws2_32.lib")//静态链接库
//ctrl + k ctrl + c注释代码，ctrl+u取消注释代码



enum CMD
{
	CMD_LOGIN,
	CMD_LOGINOUT,
	CMD_ERROR
};
///消息头
struct DataHeader
{
	short dataLength;//表示数据长度
	short cmd;//命令
};

//DataPackage
struct Login
{
	char userName[32];
	char PassWord[32];
};

struct LoginResult
{
	int result;

};

struct LoginOut
{
	char userName[32];
};
struct LoginOutResult
{
	int result;

};
int main()
{
	//启动windows socket2.x环境
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
	////
	//1.建立一个套接字
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//2.绑定用于接收客户端连接的网络端口
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);//主机到网络字节序的实现
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(_sock, (sockaddr*)&_sin, sizeof(_sin)) == SOCKET_ERROR)
	{
		printf("ERROR,绑定接口失败...\n");
	}
	else
	{
		printf("绑定成功...\n");
	}

	//3.listen监听网络端口
	if(SOCKET_ERROR == listen(_sock,5))
	{
		printf("ERROR,监听网络接口失败...\n");
	}
	else
	{
		printf("监听成功...\n");
	}
	//4.accept 等待接收客户端连接
	sockaddr_in clientAddr = {};
	int nAddrLen = sizeof(sockaddr_in);
	SOCKET _cSock = INVALID_SOCKET;

	
	_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
	if (INVALID_SOCKET == _cSock)
	{
		printf("错误，接收到无效客户端SOCKET...\n");
	}
	printf("新客户端加入：socket =%d ,IP = %s\n",(int)_cSock,inet_ntoa(clientAddr.sin_addr));





	while (true)
	{

		DataHeader header = {};
		//5.接收客户端的数据c
		int nLen = recv(_cSock, (char *)&header, sizeof(DataHeader), 0);

		if (nLen <= 0)
		{
			printf("客户端已退出，任务结束\n");
			break;
		}

		printf("收到header--命令%d，数据长度%d\n",header.cmd,header.dataLength);

		switch (header.cmd)
		{
			case CMD_LOGIN:
				{

					Login login = {};
					recv(_cSock, (char *)&login, sizeof(Login), 0);
					//忽略判断用户名和密码
					LoginResult ret = {1};
					send(_cSock, (char *)&header, sizeof(DataHeader), 0);
					send(_cSock, (char *)&ret, sizeof(LoginResult), 0);
					break;
				}
				
			case CMD_LOGINOUT:
			{
				LoginOut loginout = {};
				recv(_cSock, (char *)&loginout, sizeof(LoginOut), 0);
				LoginOutResult ret = { 1 };
				send(_cSock, (char *)&header, sizeof(DataHeader), 0);
				send(_cSock, (char *)&ret, sizeof(LoginOutResult), 0);
				break;
			}
		

			default:
			{	
				header.cmd = CMD_ERROR;
				header.dataLength = 0;
				send(_cSock, (char*)&header, sizeof(DataHeader), 0);
				break;
			}
		}
		
	}


	//7. 关闭套接字
	closesocket(_sock);
	
	WSACleanup();
	printf("服务器退出，任务结束\n");
	getchar();
	return 0;
}