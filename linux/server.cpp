#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS


#include<stdio.h>

#ifdef _WIN32
	#include<WinSock2.h>
	#include<Windows.h>
#else
	#include<unistd.h>//uni std
	#include<arpa/inet.h>
	#include<string.h>
	#define SOCKET int
	#define INVALID_SOCKET (SOCKET)(~0)
	#define SOCKET_ERROR	(-1)
#endif


#include<vector>
using namespace std;
////#pragma comment(lib,"ws2_32.lib")//静态链接库
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
struct Login :public DataHeader
{
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char PassWord[32];
};

struct LoginResult :public DataHeader
{
	LoginResult()
	{
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int result;

};

struct LoginOut :public DataHeader
{
	LoginOut()
	{
		dataLength = sizeof(LoginOut);
		cmd = CMD_LOGINOUT;
	}
	char userName[32];
};

struct LoginOutResult :public DataHeader
{
	LoginOutResult()
	{
		dataLength = sizeof(LoginOutResult);
		cmd = CMD_LOGINOUT_RESULT;
		result = 0;
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


std::vector<SOCKET> g_clients;

int processor(SOCKET _cSock)
{
	//缓冲区
	char szRecv[1024] = {};

	//5.接收客户端的数据头包
	int nLen = (int)recv(_cSock, (char *)szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;
	if (nLen <= 0)
	{
		printf("client exited ,task end\n");
		return -1;
	}

	switch (header->cmd)
	{
	case CMD_LOGIN:
	{
		///接收数据体
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);//之前已经把header读取出来了，不能再从头读取了
		Login* login = (Login*)szRecv;
		printf("receive header--CMD%d，data length%d，username=%s,passwd=%s\n", login->cmd, login->dataLength, login->userName, login->PassWord);
		LoginResult ret;

		send(_cSock, (char *)&ret, sizeof(LoginResult), 0);

	}
	break;
	case CMD_LOGINOUT:
	{

		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LoginOut* loginout = (LoginOut*)szRecv;
		printf("receive header--CMD%d，data length%d，username=%s\n", loginout->cmd, loginout->dataLength, loginout->userName);
		LoginOutResult ret;
		send(_cSock, (char *)&ret, sizeof(LoginOutResult), 0);

	}
	break;


	default:
	{
		DataHeader header = { 0,CMD_ERROR };
		send(_cSock, (char*)&header, sizeof(DataHeader), 0);
	}
	break;
	}
	return 0;
}

int main()
{
	//启动windows socket2.x环境
#ifdef WIN_32
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
#endif
	////
	//1.建立一个套接字
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//2.绑定用于接收客户端连接的网络端口
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);//主机到网络字节序的实现

	_sin.sin_addr.s_addr = INADDR_ANY;
	if (bind(_sock, (sockaddr*)&_sin, sizeof(_sin)) == SOCKET_ERROR)
	{
		printf("ERROR,bind interface failed...\n");
	}
	else
	{
		printf("bind interface success...\n");
	}

	//3.listen监听网络端口
	if(SOCKET_ERROR == listen(_sock,5))
	{
		printf("ERROR,listen interface failed...\n");
	}
	else
	{
		printf("listen success...\n");
	}
	



	while (true)
	{

		//DataHeader header = {};
		//伯克利套接字 BSD socket
		fd_set fdRead;//描述符（socket）集合
		fd_set fdWrite;
		fd_set fdExp;

		//清理集合内容
		FD_ZERO(&fdRead);//清空集合内容
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);

		//将描述符（socket）加入到集合中
		FD_SET(_sock,&fdRead);
		FD_SET(_sock, &fdWrite);
		FD_SET(_sock, &fdExp);

		SOCKET maxSock = _sock;
		for (int n = (int)g_clients.size() - 1; n >= 0; n--)
		{
			FD_SET(g_clients[n],&fdRead);
			if(maxSock < g_clients[n])
			{
				maxSock = g_clients[n];
			}
		}
		

		//每次检查fdRead之前，必须把所有的sock都设置到里面去，select检查内部sock是否有IO操作
		for (int n = (int)g_clients.size() - 1; n >= 0; n--)
		{
			FD_SET(g_clients[n], &fdRead);
		}


		//nfds是一个整数值，是指fd_set集合中所有描述符（socket）的范围，而不是数量
		///既是所有文件描述符最大值，在windows中参数可以写为0
		timeval t = {1,0};

		///一旦sock有IO操作，select就会把该sock选择出来，把那些没有IO操作的sock删除
		int ret = select(maxSock+1,&fdRead,&fdWrite,&fdExp,&t);
		if (0 > ret)
		{
			printf("select task end、\n");
			break;
		}
		
		//判断那个有IO操作的sock是否在集合中
		if (FD_ISSET(_sock, &fdRead))
		{
			FD_CLR(_sock, &fdRead);

			//4.accept 等待接收客户端连接
			sockaddr_in clientAddr = {};
			int nAddrLen = sizeof(sockaddr_in);
			SOCKET _cSock = INVALID_SOCKET;
			_cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
			if (INVALID_SOCKET == _cSock)
			{

				printf("error , receive invalid client SOCKET...\n");
			}
			else
			{
				//新用户加入通知其他人
				for (int n = (int)g_clients.size() - 1; n >= 0; n--)
				{
					NewUserJoin userJoin;
					send(g_clients[n],(const char*)&userJoin,sizeof(NewUserJoin),0);
				}
				g_clients.push_back(_cSock);
				printf("new client added：socket =%d ,IP = %s\n", (int)_cSock, inet_ntoa(clientAddr.sin_addr));
			}
		}




		for (int n = (int)g_clients.size() - 1; n >= 0; n--)
		{
			if (FD_ISSET(g_clients[n], &fdRead))
			{
				if (-1 == processor(g_clients[n]))
					{
						auto iter = g_clients.begin();
						if(iter != g_clients.end())
						{
							g_clients.erase(iter);
						}
					}
			}
		}


		printf("server idle, handling other tasks\n");
	}

	for (size_t n = g_clients.size() - 1; n >= 0; n--)
	{
		close(g_clients[n]);
	}
	//7. 关闭套接字
	
#ifdef WIN_32
	WSACleanup();
#endif
	printf("server exited，tasks over\n");
	getchar();
	return 0;
}
