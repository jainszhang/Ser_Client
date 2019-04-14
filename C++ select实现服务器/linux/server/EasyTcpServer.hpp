#ifndef _EasyTcp_Sercer_hpp_
#define _EasyTcp_Sercer_hpp_

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#include<WinSock2.h>
	#include<Windows.h>
	#pragma comment(lib,"ws2_32.lib")//静态链接库

#else
	#include<unistd.h>
	#include<arpa/inet.h>
	#include<string.h>
	#define SOCKET int
	#define INVALID_SOCKET	(SOCKET)(~0)
	#define SOCKET_ERROR	(-1)	
#endif // _WIN32

#include<vector>
#include<stdio.h>
#include"MessageH.hpp"
#include<iostream>
using namespace std;
class EasyTcpServer
{
private:
	SOCKET _sock;
	std::vector<SOCKET> g_clients;
public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
	}
	virtual ~EasyTcpServer()
	{
		Close();
	}

	//初始化sock
	SOCKET InitSocket()
	{
		//启动win socket 2.x环境
	#ifdef WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
	#endif

		if (INVALID_SOCKET != _sock)
		{
			printf("<sock = %d> close old connection...\n", _sock);
			Close();
		}

		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (INVALID_SOCKET == _sock)
		{
			printf("ERROR,init sock failed...\n");

		}
		else
		{
			printf("init sock success...\n");

		}
		return _sock;
		
	}

	//绑定IP和端口号
	int Bind(const char * ip,unsigned short port)
	{
		if (INVALID_SOCKET == _sock)
		{
			InitSocket();
		}
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);//主机到网络字节序的实现
#ifdef _WIN32
		if (ip)
		{
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;
		}
		
#else
		if (ip)
		{
			_sin.sin_addr.s_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.s_addr = INADDR_ANY;
		}
#endif // _WIN32
		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
		
		if (ret == SOCKET_ERROR)
		{
			printf("ERROR, bind port <%d> failed...\n",port);
		}
		else
		{
			printf(" bind port <%d> sucess...\n",port);
		}
		return ret;
	}
	//监听端口号
	int Listen(int n)
	{
		int ret = listen(_sock, n);
		if (SOCKET_ERROR ==ret )
		{
			printf("socket = <%d>,ERROR,listen net interface failed...\n",_sock);
		}
		else
		{
			printf("socket = <%d,>,listen net interface sucess...\n",_sock);
		}
		return ret;
	}
	//接受客户端连接
	SOCKET Accept()
	{
		//4.accept 等待接收客户端连接
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET _cSock = INVALID_SOCKET;
#ifdef _WIN32	
		_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		_cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif // _WIN32

		
		if (INVALID_SOCKET == _cSock)
		{

			printf("socket = <%d>错误，接收到无效客户端SOCKET...\n",(int)_cSock);
		}
		else
		{
			NewUserJoin userJoin;
			SendDataToAll(&userJoin);
			g_clients.push_back(_cSock);
			printf("socket = <%d>新客户端加入：socket =%d ,IP = %s\n",(int)_cSock, (int)_cSock, inet_ntoa(clientAddr.sin_addr));
		}
		return _cSock;
	}
	//关闭sock
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{

		#ifdef _WIN32
		
				for (size_t n = g_clients.size() - 1; n >= 0; n--)
				{
					closesocket(g_clients[n]);
				}
				closesocket(_sock);
				WSACleanup();
				printf("server exited，tasks over...\n");
		#else
				for (size_t n = g_clients.size() - 1; n >= 0; n--)
				{
					close(g_clients[n]);
				}
				close(_sock);
		#endif // _WIN32
		}
	}
	//处理网络消息
	bool OnRun()
	{
		if (isRun())
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
			FD_SET(_sock, &fdRead);
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExp);

			SOCKET maxSock = _sock;
			//每次检查fdRead之前，必须把所有的sock都设置到里面去，select检查内部sock是否有IO操作
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				FD_SET(g_clients[n], &fdRead);
				if (maxSock < g_clients[n])
				{
					maxSock = g_clients[n];
				}
			}
		
			timeval t = { 1,0 };
			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t);
			if (0 > ret)
			{
				printf("select task end��\n");
				Close();
				return false;
			}
			//判断那个有IO操作的sock是否在集合中
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				Accept();
			}
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				if (FD_ISSET(g_clients[n], &fdRead))
				{
					if (-1 == RecvData(g_clients[n]))
					{
						auto iter = g_clients.begin() + n;
						if (iter != g_clients.end())
						{
							g_clients.erase(iter);
						}
					}
				}
			}
			//printf("服务器空闲，处理其他任务\n");


			return true;
		}
		return false;
		
	}
	//判断是否工作
	bool isRun()
	{
		return INVALID_SOCKET != _sock;
	}
	//接收数据

	int RecvData(SOCKET _cSock)
	{
		//缓冲区
		char szRecv[1024] = {};

		//5.接收客户端的数据头包
		int nLen = recv(_cSock, (char *)szRecv, sizeof(DataHeader), 0);

		DataHeader* header = (DataHeader*)szRecv;
		if (nLen <= 0)
		{
			printf("客户端已退出，任务结束\n");
			return -1;
		}
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);//之前已经把header读取出来了，不能再从头读取了
		OnNetMsg(_cSock, header);

		return 0;
	}
	//相应网络消息
	virtual void OnNetMsg(SOCKET _cSock,DataHeader *header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			///接收数据体
			
			Login* login = (Login*)header;
			printf("收到header--命令%d，数据长度%d，username=%s,passwd=%s\n", login->cmd, login->dataLength, login->userName, login->PassWord);
			LoginResult ret;
			ret.result = 1;
			send(_cSock, (char *)&ret, sizeof(LoginResult), 0);
		}
		break;
		case CMD_LOGINOUT:
		{

			LoginOut* loginout = (LoginOut*)header;
			printf("收到header--命令%d，数据长度%d，username=%s\n", loginout->cmd, loginout->dataLength, loginout->userName);
			LoginOutResult ret;
			ret.result = 2;
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
	}
	//发送指定sock数据
	int SendData(SOCKET _cSock, DataHeader *header)
	{
		if (isRun() && header) {

			return send(_cSock, (const char *)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}

	void SendDataToAll(DataHeader *header)
	{
		for (int n = (int)g_clients.size() - 1; n >= 0; n--)
		{
			SendData(g_clients[n], header);
		}
	}

};



#endif // _EasyTcp_Sercer_hpp_
