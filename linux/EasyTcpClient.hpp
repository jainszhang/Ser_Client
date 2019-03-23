#ifndef _EasyTcpClient_hpp
#define _EasyTcpClient_hpp

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

#include<stdio.h>
#include"MessageH.hpp"
#include<iostream>
using namespace std;
class EasyTcpClient
{
	
public:
	SOCKET _sock;
	EasyTcpClient() {
		_sock = INVALID_SOCKET;
	}


	//虚析构函数
	virtual ~EasyTcpClient()
	{
		Close();
	}
	//初始化socket
	int InitSocket()
	{
		//启动win socket 2.x环境
#ifdef WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif

		if (INVALID_SOCKET != _sock)
		{
			printf("<sock = %d> close old connection...\n",_sock);
			Close();
		}

		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (INVALID_SOCKET == _sock)
		{
			printf("ERROR,init sock failed...\n");
			return -1;
		}
		else
		{
			printf("init sock success...\n");
			return 0;
		}
		


	}

	//连接服务器
	int Connect(const char *ip,unsigned short port)
	{

		if (INVALID_SOCKET == _sock)
		{
			if (InitSocket())
			{
				printf("second init failed...\n");
				return -1;
			}
		}

		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
		

#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif

		int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret)
		{
			printf("ERROR，connect socket failed...\n");
		}
		else
		{
			printf("connect socket success...\n");
		}
		return ret;
	}

	//关闭socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
		//关闭win sock 2.x环境
#ifdef WIN32
		closesocket(_sock);
		WSACleanup();
#else
		close(_sock);
#endif
		_sock = INVALID_SOCKET;
		}

	}

	

	//处理数据
	bool onRun()
	{
		if (isRun()) {
			fd_set fdReads;
			FD_ZERO(&fdReads);
			FD_SET(_sock, &fdReads);
			timeval t = { 1,0 };
			//检测该sock上有没有IO操作，如果有保留该sock
			int ret = select(_sock+1, &fdReads, 0, 0, &t);
			if (ret < 0)
			{
				printf("<socket = %d>select task over...\n", _sock);
				return false;
			}

			if (FD_ISSET(_sock, &fdReads))
			{
				FD_CLR(_sock, &fdReads);
				if (-1 == RecvData())
				{
					printf("seletc tase over2\n");
					return false;
				}
			}
			return true;
		}
		return false;

//		printf("客户端空闲，处理其他任务\n");

	}

	bool isRun()
	{
		return INVALID_SOCKET != _sock;
	}

	//响应服务器回复的消息
	void OnNetMsg(DataHeader *header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			LoginResult* loginresult = (LoginResult*)header;
			printf("CMD: LOGIN, data length is: %d, result=%d\n", loginresult->dataLength, loginresult->result);

		}
		break;
		case CMD_LOGINOUT_RESULT:
		{

			LoginOutResult* loginoutResult = (LoginOutResult*)header;
			printf("CMD:CMD_LOGINOUT_RESULT , data length is: %d\n", loginoutResult->result);

		}
		break;

		case CMD_NEW_USER_JOIN:
		{
			NewUserJoin* userJoin = (NewUserJoin*)header;
			printf("server returned data：NEW_USER_JOIN，data length%d\n", userJoin->dataLength);
		}
		break;


		default:
		{
			DataHeader header = { 0,CMD_ERROR };
			send(_sock, (char*)&header, sizeof(DataHeader), 0);
		}
		break;
		}
	}
	//接收数据,处理粘包,拆包
	int RecvData()
	{
		char szRecv[4096] = {};
		int nLen = recv(_sock, (char *)szRecv, sizeof(DataHeader), 0);
		
		DataHeader* header = (DataHeader*)szRecv;
		if (nLen <= 0)
		{
			printf("dis connected with server ,tasks over\n");
			return -1;
		}
		recv(_sock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);

		DataHeader* head = header;
		OnNetMsg(head);
		return 0;
	}

	int SendData(DataHeader *header)
	{
		if (isRun() && header) {
			
			return send(_sock, (const char *)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
private:

};



#endif