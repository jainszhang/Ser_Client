#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include<WinSock2.h>
#include<Windows.h>
#include<stdio.h>

#pragma comment(lib,"ws2_32.lib")//��̬���ӿ�
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
	//������
	char szRecv[1024] = {};

	//5.���տͻ��˵�����c
	int nLen = recv(_cSock, (char *)szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;
	if (nLen <= 0)
	{
		printf("��������Ͽ����ӣ��������\n");
		return -1;
	}


	switch (header->cmd)
	{
	case CMD_LOGIN_RESULT:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);//֮ǰ�Ѿ���header��ȡ�����ˣ������ٴ�ͷ��ȡ��
		LoginResult* login_Result = (LoginResult*)szRecv;
		printf("������������Ϣ��CMD_LOGIN_RESULT�����ݳ���%d\n", login_Result->dataLength);
	}
	break;
	case CMD_LOGINOUT:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);//֮ǰ�Ѿ���header��ȡ�����ˣ������ٴ�ͷ��ȡ��
		LoginOutResult* loginout_Result = (LoginOutResult*)szRecv;
		printf("������������Ϣ��CMD_LOGINOUT_RESULT�����ݳ���%d\n", loginout_Result->dataLength);
	}
	break;

	case CMD_NEW_USER_JOIN:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);//֮ǰ�Ѿ���header��ȡ�����ˣ������ٴ�ͷ��ȡ��
		NewUserJoin* userJoin = (NewUserJoin*)szRecv;
		printf("������������Ϣ��NEW_USER_JOIN�����ݳ���%d\n", userJoin->dataLength);
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

	SOCKET _sock = socket(AF_INET,SOCK_STREAM,0);
	
	if (INVALID_SOCKET == _sock)
	{
		printf("ERROR,����ʧ��...\n");
	}
	else
	{
		printf("�����ɹ�...\n");
	}
	//2.���ӷ�����
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	if(SOCKET_ERROR == connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in)))
	{
		printf("ERROR������socketʧ��...\n");
	}
	else
	{
		printf("����socket�ɹ�...\n");
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
			printf("select�������\n");
			break;
		}

		if (FD_ISSET(_sock, &fdReads))
		{
			FD_CLR(_sock, &fdReads);
			if (-1 == processor(_sock))
			{
				printf("seletc�������2\n");
				break;
			}
		}
		
		printf("�ͻ��˿��У�������������\n");
		Login login;
		strcpy(login.userName, "zj");
		strcpy(login.PassWord, "jainss");

		send(_sock, (const char * )&login, sizeof(Login), 0);
		//Sleep(1000);

	}

	

	closesocket(_sock);

	printf("���˳�\n");
	getchar();
	WSACleanup();
	return 0;
}