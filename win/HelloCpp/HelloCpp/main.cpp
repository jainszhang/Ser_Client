#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include<WinSock2.h>
#include<Windows.h>
#include<stdio.h>

////#pragma comment(lib,"ws2_32.lib")//��̬���ӿ�
//ctrl + k ctrl + cע�ʹ��룬ctrl+uȡ��ע�ʹ���
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
	//4.accept �ȴ����տͻ�������
	sockaddr_in clientAddr = {};
	int nAddrLen = sizeof(sockaddr_in);
	SOCKET _cSock = INVALID_SOCKET;

	char msgBuf[] = "Hello ,I am server.";

	while (true)
	{
		_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
		if (INVALID_SOCKET == _cSock)
		{
			printf("���󣬽��յ���Ч�ͻ���SOCKET...\n");
		}

		printf("�¿ͻ��˼��룺IP = %s\n",inet_ntoa(clientAddr.sin_addr));
		//5. send ��ͻ��˷�������
		
		send(_cSock, msgBuf, strlen(msgBuf) + 1, 0);
	}


	

	//6. �ر��׽���
	closesocket(_sock);
	
	WSACleanup();
	return 0;
}