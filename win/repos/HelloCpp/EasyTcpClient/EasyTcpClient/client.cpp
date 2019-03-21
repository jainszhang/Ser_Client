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
	CMD_LOGINOUT,
	CMD_ERROR
};
///��Ϣͷ
struct DataHeader
{
	short dataLength;//��ʾ���ݳ���
	short cmd;//����
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
		//3.������������
		scanf("%s", cmdBuf);

		//4.��������
		if (0 == strcmp(cmdBuf, "exit"))
		{
			printf("�յ�exit���������\n");
			break;
		}
		else if (0 == strcmp(cmdBuf, "login"))
		{
			Login login = {"zj","jains"};
			DataHeader dh = {sizeof(Login),CMD_LOGIN};

			send(_sock, (const char *)&dh, sizeof(dh), 0);
			send(_sock, (const char *)&login, sizeof(login), 0);

			//���շ�������������
			DataHeader retHeader = {};
			LoginResult loginRet = {};
			recv(_sock,(char*)&retHeader,sizeof(retHeader),0);
			recv(_sock, (char*)&loginRet, sizeof(loginRet), 0);

			printf("LoginResult:%d\n",loginRet.result);
		}
		else if (0 == strcmp(cmdBuf, "loginout"))
		{
			LoginOut logout = {};
			DataHeader dh = { sizeof(logout), CMD_LOGINOUT};
			send(_sock, (const char *)&dh, sizeof(dh), 0);
			send(_sock, (const char *)&logout, sizeof(logout), 0);

			//���շ�������������
			DataHeader retHeader = {};
			LoginOutResult loginoutRet = {};
			recv(_sock, (char*)&retHeader, sizeof(retHeader), 0);
			recv(_sock, (char*)&loginoutRet, sizeof(loginoutRet), 0);
			printf("LoginOutResult:%d\n", loginoutRet.result);
		}
		else
		{
			printf("��֧���������������\n");
		}

	}

	

	closesocket(_sock);

	printf("�ͻ����˳����������\n");
	getchar();
	WSACleanup();
	return 0;
}