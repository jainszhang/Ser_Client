#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include<WinSock2.h>
#include<Windows.h>
#include<stdio.h>

#pragma comment(lib,"ws2_32.lib")//��̬���ӿ�
//ctrl + k ctrl + cע�ʹ��룬ctrl+uȡ��ע�ʹ���
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

	//3.���շ�������Ϣ
	char recvBuf[256] = {};
	int nlen = recv(_sock, recvBuf, 256, 0);
	if (nlen > 0)
	{
		printf("���յ����ݣ�%s\n", recvBuf);
	}
	getchar();
	closesocket(_sock);


	WSACleanup();
	return 0;
}