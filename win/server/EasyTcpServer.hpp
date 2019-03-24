#ifndef _EasyTcp_Sercer_hpp_
#define _EasyTcp_Sercer_hpp_

#ifdef _WIN32
	#define FD_SETSIZE      2506//重新定义可连接客户端数量
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
#include<thread>
#include<mutex>
#include<atomic>
#include"CELLTimerstamp.hpp"

#ifndef RECV_BUF_SIZE
#define RECV_BUFF_SIZE 10240
#endif // !RECV_BUF_SIZE

#define _CellServer_THREAD_COUNT 4


//客户端数据类型
class ClientSocket {
public:
	ClientSocket(SOCKET sockfd = INVALID_SOCKET)
	{
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, sizeof(_szMsgBuf));
		_lastPos = 0;
		
	}
	SOCKET sockfd()
	{
		return _sockfd;
	}
	char * msgBuf()
	{
		return _szMsgBuf;
	}

	int getLastPos()
	{
		return _lastPos;
	}
	void setLastPos(int pos)
	{
		_lastPos = pos;
	}


private:
	SOCKET _sockfd;//socket fd_set file desc set

	//第二缓冲区--消息缓冲区
	char _szMsgBuf[RECV_BUFF_SIZE * 10];
	//消息缓冲区尾部位置
	int _lastPos;
};

//网络事件接口
class INetEvent
{
public:
	//客户端离开
	virtual void OnLeave(ClientSocket* pClient)=0;//纯虚函数
	virtual void OnNetMsg(SOCKET cSock, DataHeader* header) = 0;
private:

};


class CellServer
{
public:
	CellServer(SOCKET sock = INVALID_SOCKET)
	{
		_sock = sock;
		_pThread = nullptr;
		_pNetEvent = nullptr;
	}

	~CellServer()
	{
		delete _pThread;
		Close();
		_sock = INVALID_SOCKET;
	}
	void setEventObj(INetEvent* event)
	{
		_pNetEvent = event;
	}
	bool OnRun()
	{
		while (isRun())
		{
			if (_clientsBuff.size() > 0)
			{
				std::lock_guard<std::mutex>lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
					_clients.push_back(pClient);
				}
				_clientsBuff.clear();
			}
			//如果没有需要处理的客户端，就跳过
			if (_clients.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
			//伯克利套接字 BSD socket
			fd_set fdRead;//描述符（socket）集合

			//清理集合内容
			FD_ZERO(&fdRead);//清空集合内容

			SOCKET maxSock = _clients[0]->sockfd();
			//每次检查fdRead之前，必须把所有的sock都设置到里面去，select检查内部sock是否有IO操作
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				FD_SET(_clients[n]->sockfd(), &fdRead);
				if (maxSock < _clients[n]->sockfd())
				{
					maxSock = _clients[n]->sockfd();
				}
			}

			timeval t = { 1,0 };
			int ret = select(maxSock + 1, &fdRead, nullptr, nullptr, &t);
			if (0 > ret)
			{
				printf("select task end..\n");
				Close();
				return false;
			}

			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				if (FD_ISSET(_clients[n]->sockfd(), &fdRead))
				{
					if (-1 == RecvData(_clients[n]))
					{
						auto iter = _clients.begin() + n;
						if (iter != _clients.end())
						{
							if (_pNetEvent)
							{
								_pNetEvent->OnLeave(_clients[n]);
							}
							delete _clients[n];
							_clients.erase(iter);
						}
					}
				}
			}
		}
	}
	//判断是否工作
	bool isRun()
	{
		return INVALID_SOCKET != _sock;
	}
	//关闭sock
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{

#ifdef _WIN32

			for (size_t n = _clients.size() - 1; n >= 0; n--)
			{
				closesocket(_clients[n]->sockfd());
				delete _clients[n];//释放内存
			}
			closesocket(_sock);
			WSACleanup();
			printf("server exited，tasks over...\n");
#else
			for (size_t n = _clients.size() - 1; n >= 0; n--)
			{
				close(_clients[n]->sockfd());
				delete _clients[n];//释放内存
			}
			close(_sock);
#endif // _WIN32
			_clients.clear();
		}
	}
	//接收数据

	int RecvData(ClientSocket* pClient)
	{
		//缓冲区
		char _szRecv[RECV_BUFF_SIZE] = {};

		//5.接收客户端的数据头包
		int nLen = recv(pClient->sockfd(), _szRecv, RECV_BUFF_SIZE, 0);


		if (nLen <= 0)
		{
			printf("客户端已退出，任务结束\n");
			return -1;
		}

		memcpy(pClient->msgBuf() + pClient->getLastPos(), _szRecv, nLen);
		//消息缓冲区的数据尾部位置后移
		pClient->setLastPos(pClient->getLastPos() + nLen);

		//判断消息缓冲区的数据长度大于消息头DataHeader长度，此时可以知道当前消息长度
		while (pClient->getLastPos() >= sizeof(DataHeader))
		{
			DataHeader* header = (DataHeader*)pClient->msgBuf();
			//判断消息缓冲区数据长度大于消息长度
			if (pClient->getLastPos() >= header->dataLength)
			{
				//剩余未处理消息缓冲区数据长度
				int nSize = pClient->getLastPos() - header->dataLength;
				//处理网络消息
				OnNetMsg(pClient->sockfd(), header);
				//将消息缓冲区剩余未处理数据前移
				memcpy(pClient->msgBuf(), pClient->msgBuf() + header->dataLength, nSize);
				//消息缓冲区的数据尾部位置前移
				pClient->setLastPos(nSize);
			}
			else
			{
				//消息缓冲区剩余数据消息不够一条完整消息
				break;

			}
		}
		return 0;


	}
	//相应网络消息
	virtual void OnNetMsg(SOCKET cSock, DataHeader *header)
	{

		_pNetEvent->OnNetMsg(cSock, header);
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			//接收客户端的数据
			//Login* login = (Login*)header;
			//printf("收到header--命令%d，数据长度%d，username=%s,passwd=%s\n", login->cmd, login->dataLength, login->userName, login->PassWord);
			//给客户端回应消息
			//LoginResult ret;/*
			//ret.result = 1;
			//SendData(cSock, &ret);*/
		}
		break;
		case CMD_LOGINOUT:
		{
			//接收到客户端数据
			//LoginOut* loginout = (LoginOut*)header;
			//printf("收到header--命令%d，数据长度%d，username=%s\n", loginout->cmd, loginout->dataLength, loginout->userName);
			//给客户端回应消息
			/*LoginOutResult ret;
			ret.result = 2;
			SendData(cSock,&ret);*/

		}
		break;

		default://出现接收错误情况打印
		{/*
			DataHeader header = { 0,CMD_ERROR };
			send(_sock, (char*)&header, sizeof(DataHeader), 0);*/
			printf("socket<%d> receive undefined data. data length is %d\n", (int)cSock, header->dataLength);
		}
		break;
		}
	}

	void addClient(ClientSocket* pClient)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		//_mutex.lock();

		_clientsBuff.push_back(pClient);
		//_mutex.unlock;
	}
	void Start()
	{
		_pThread = new std::thread (std::mem_fun(&CellServer::OnRun),this);

	}
	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}
private:
	SOCKET _sock;
	//正式客户队列
	std::vector<ClientSocket*> _clients;
	
	//客户缓冲队列
	std::vector<ClientSocket*> _clientsBuff;
	//缓冲队列的锁
	std::mutex _mutex;
	std::thread* _pThread;
	//网络事件对象
	INetEvent* _pNetEvent;

	
};

///new 在heap上
class EasyTcpServer:public INetEvent
{
private:
	SOCKET _sock;
	/*std::vector<ClientSocket*> _clients;*/
	//消息处理对象，内部会创建线程
	std::vector<CellServer*> _cellServers;
	//每秒消息计时
	CELLTimestamp _tTime;
	//收到消息计数
	std::atomic_int _recvCount;
	//客户端计数
	std::atomic_int _clientCount;

public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
		_recvCount = 0;
		_clientCount = 0;

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
		SOCKET cSock = INVALID_SOCKET;
#ifdef _WIN32	
		cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif // _WIN32

		
		if (INVALID_SOCKET == cSock)
		{

			printf("socket = <%d>错误，接收到无效客户端SOCKET...\n",(int)cSock);
		}
		else
		{
			//NewUserJoin userJoin;
			//SendDataToAll(&userJoin);

			addClientToServer(new ClientSocket(cSock));
			//printf("socket = <%d>新客户端加入：socket =%d ,IP = %s\n",(int)cSock, (int)cSock, inet_ntoa(clientAddr.sin_addr));
		}
		return cSock;
	}
	void addClientToServer(ClientSocket* pClient)
	{
		//查找客户数量最少的cellserver消息处理对象
		auto pMinServer = _cellServers[0];
		for (auto pCellServer : _cellServers)
		{
			if (pMinServer->getClientCount() > pCellServer->getClientCount())
			{
				pMinServer = pCellServer;
			}
		}
		pMinServer->addClient(pClient);
		_clientCount++;
	}
	void Start()
	{
		for (int n = 0; n < _CellServer_THREAD_COUNT; n++)
		{
			auto ser = new CellServer(_sock);
			_cellServers.push_back(ser);
			//注册网络事件接受对象
			ser->setEventObj(this);
			//启动消息处理线程
			ser->Start();
		}
	}
	//关闭sock
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{

		#ifdef _WIN32
				closesocket(_sock);
				WSACleanup();
		#else
				close(_sock);
		#endif // _WIN32
		}
	}
	//处理网络消息
	
	//判断是否工作
	bool isRun()
	{
		return INVALID_SOCKET != _sock;
	}


	void time4msg()
	{
	
		auto t1 = _tTime.getElapsedSecond();
		if ( t1> 1.0)
		{
			printf("time<%lf>,socket<%d>,clients<%d>,recvCount<%d>\n",t1,_sock, (int)_clientCount,(int)(_recvCount/t1));
			_recvCount = 0;
			_tTime.update();
		}
		
	}


	//处理网络消息
	bool OnRun()
	{
		if (isRun())
		{

			time4msg();
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

			timeval t = { 1,0 };
			int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, &t);
			if (0 > ret)
			{
				printf("select task end..\n");
				Close();
				return false;
			}
			//判断那个有IO操作的sock是否在集合中
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				Accept();
				return true;
			}
			return true;
		}
		return false;
	}

	
	//发送指定sock数据
	int SendData(SOCKET cSock, DataHeader *header)
	{
		if (isRun() && header) {

			return send(cSock, (const char *)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}

	virtual void OnLeave(ClientSocket* pClient)
	{
		_clientCount--;
	}
	virtual void OnNetMsg(SOCKET cSock, DataHeader* header)
	{
		_recvCount++;
	}
};

#endif // _EasyTcp_Sercer_hpp_
