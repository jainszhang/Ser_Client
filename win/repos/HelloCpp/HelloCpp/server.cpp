#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include<WinSock2.h>
#include<Windows.h>
#include<stdio.h>

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


std::vector<SOCKET> g_clients;

int processor(SOCKET _cSock)
{
	//������
	char szRecv[1024] = {};

	//5.���տͻ��˵�����c
	int nLen = recv(_cSock, (char *)szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;
	if (nLen <= 0)
	{
		printf("�ͻ������˳����������\n");
		return -1;
	}

	switch (header->cmd)
	{
	case CMD_LOGIN:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);//֮ǰ�Ѿ���header��ȡ�����ˣ������ٴ�ͷ��ȡ��
		Login* login = (Login*)szRecv;
		printf("�յ�header--����%d�����ݳ���%d��username=%s,passwd=%s\n", login->cmd, login->dataLength, login->userName, login->PassWord);
		LoginResult ret;

		send(_cSock, (char *)&ret, sizeof(LoginResult), 0);

	}
	break;
	case CMD_LOGINOUT:
	{

		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LoginOut* loginout = (LoginOut*)szRecv;
		printf("�յ�header--����%d�����ݳ���%d��username=%s\n", loginout->cmd, loginout->dataLength, loginout->userName);
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
}
int main()
{
	//����windows socket2.x����
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
	////
	//1.����һ���׽���
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//2.�����ڽ��տͻ������ӵ�����˿�
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);//�����������ֽ����ʵ��
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(_sock, (sockaddr*)&_sin, sizeof(_sin)) == SOCKET_ERROR)
	{
		printf("ERROR,�󶨽ӿ�ʧ��...\n");
	}
	else
	{
		printf("�󶨳ɹ�...\n");
	}

	//3.listen��������˿�
	if(SOCKET_ERROR == listen(_sock,5))
	{
		printf("ERROR,��������ӿ�ʧ��...\n");
	}
	else
	{
		printf("�����ɹ�...\n");
	}
	



	while (true)
	{

		//DataHeader header = {};
		//������ socket
		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExp;

		FD_ZERO(&fdRead);//��ռ�������
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);


		FD_SET(_sock,&fdRead);
		FD_SET(_sock, &fdWrite);
		FD_SET(_sock, &fdExp);


		for (int n = (int)g_clients.size() - 1; n >= 0; n--)
		{
			FD_SET(g_clients[n], &fdRead);
		}


		//nfds��һ������ֵ����ָfd_set������������������socket���ķ�Χ������������
		///���������ļ����������ֵ����windows�в�������дΪ0
		timeval t = {0,0};

		int ret = select(_sock+1,&fdRead,&fdWrite,&fdExp,&t);
		if (0 > ret)
		{
			printf("select ���������\n");
			break;
		}
		if (FD_ISSET(_sock, &fdRead))
		{
			FD_CLR(_sock, &fdRead);

			//4.accept �ȴ����տͻ�������
			sockaddr_in clientAddr = {};
			int nAddrLen = sizeof(sockaddr_in);
			SOCKET _cSock = INVALID_SOCKET;
			_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
			if (INVALID_SOCKET == _cSock)
			{

				printf("���󣬽��յ���Ч�ͻ���SOCKET...\n");
			}
			g_clients.push_back(_cSock);
			printf("�¿ͻ��˼��룺socket =%d ,IP = %s\n", (int)_cSock, inet_ntoa(clientAddr.sin_addr));
		
		}

		for (size_t n = 0;n<fdRead.fd_count; n++)
		{
			if (-1 == processor(fdRead.fd_array[n]))
			{
				auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[n]);
				if (iter != g_clients.end())
				{
					g_clients.erase(iter);
				}
			}
		}
	}

	for (size_t n = g_clients.size() - 1; n >= 0; n--)
	{
		closesocket(g_clients[n]);
	}
	//7. �ر��׽���
	
	
	WSACleanup();
	printf("�������˳����������\n");
	getchar();
	return 0;
}