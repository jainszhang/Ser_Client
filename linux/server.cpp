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
////#pragma comment(lib,"ws2_32.lib")//��̬���ӿ�
//ctrl + k ctrl + cע�ʹ��룬ctrl+uȡ��ע�ʹ���



enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGINOUT,
	CMD_LOGINOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};
///��Ϣͷ
struct DataHeader
{
	short dataLength;//��ʾ���ݳ���
	short cmd;//����
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
	//������
	char szRecv[1024] = {};

	//5.���տͻ��˵�����ͷ��
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
		///����������
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);//֮ǰ�Ѿ���header��ȡ�����ˣ������ٴ�ͷ��ȡ��
		Login* login = (Login*)szRecv;
		printf("receive header--CMD%d��data length%d��username=%s,passwd=%s\n", login->cmd, login->dataLength, login->userName, login->PassWord);
		LoginResult ret;

		send(_cSock, (char *)&ret, sizeof(LoginResult), 0);

	}
	break;
	case CMD_LOGINOUT:
	{

		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LoginOut* loginout = (LoginOut*)szRecv;
		printf("receive header--CMD%d��data length%d��username=%s\n", loginout->cmd, loginout->dataLength, loginout->userName);
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
	//����windows socket2.x����
#ifdef WIN_32
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
#endif
	////
	//1.����һ���׽���
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//2.�����ڽ��տͻ������ӵ�����˿�
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);//�����������ֽ����ʵ��

	_sin.sin_addr.s_addr = INADDR_ANY;
	if (bind(_sock, (sockaddr*)&_sin, sizeof(_sin)) == SOCKET_ERROR)
	{
		printf("ERROR,bind interface failed...\n");
	}
	else
	{
		printf("bind interface success...\n");
	}

	//3.listen��������˿�
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
		//�������׽��� BSD socket
		fd_set fdRead;//��������socket������
		fd_set fdWrite;
		fd_set fdExp;

		//����������
		FD_ZERO(&fdRead);//��ռ�������
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);

		//����������socket�����뵽������
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
		

		//ÿ�μ��fdRead֮ǰ����������е�sock�����õ�����ȥ��select����ڲ�sock�Ƿ���IO����
		for (int n = (int)g_clients.size() - 1; n >= 0; n--)
		{
			FD_SET(g_clients[n], &fdRead);
		}


		//nfds��һ������ֵ����ָfd_set������������������socket���ķ�Χ������������
		///���������ļ����������ֵ����windows�в�������дΪ0
		timeval t = {1,0};

		///һ��sock��IO������select�ͻ�Ѹ�sockѡ�����������Щû��IO������sockɾ��
		int ret = select(maxSock+1,&fdRead,&fdWrite,&fdExp,&t);
		if (0 > ret)
		{
			printf("select task end��\n");
			break;
		}
		
		//�ж��Ǹ���IO������sock�Ƿ��ڼ�����
		if (FD_ISSET(_sock, &fdRead))
		{
			FD_CLR(_sock, &fdRead);

			//4.accept �ȴ����տͻ�������
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
				//���û�����֪ͨ������
				for (int n = (int)g_clients.size() - 1; n >= 0; n--)
				{
					NewUserJoin userJoin;
					send(g_clients[n],(const char*)&userJoin,sizeof(NewUserJoin),0);
				}
				g_clients.push_back(_cSock);
				printf("new client added��socket =%d ,IP = %s\n", (int)_cSock, inet_ntoa(clientAddr.sin_addr));
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
	//7. �ر��׽���
	
#ifdef WIN_32
	WSACleanup();
#endif
	printf("server exited��tasks over\n");
	getchar();
	return 0;
}
