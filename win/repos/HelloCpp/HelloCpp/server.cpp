#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include<WinSock2.h>
#include<Windows.h>
#include<stdio.h>

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

	
	_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
	if (INVALID_SOCKET == _cSock)
	{
		printf("���󣬽��յ���Ч�ͻ���SOCKET...\n");
	}
	printf("�¿ͻ��˼��룺socket =%d ,IP = %s\n",(int)_cSock,inet_ntoa(clientAddr.sin_addr));





	while (true)
	{

		DataHeader header = {};
		//5.���տͻ��˵�����c
		int nLen = recv(_cSock, (char *)&header, sizeof(DataHeader), 0);

		if (nLen <= 0)
		{
			printf("�ͻ������˳����������\n");
			break;
		}

//		

		switch (header.cmd)
		{
			case CMD_LOGIN:
				{

					Login login = {};
					recv(_cSock, (char *)&login+sizeof(DataHeader), sizeof(Login)- sizeof(DataHeader), 0);//֮ǰ�Ѿ���header��ȡ�����ˣ������ٴ�ͷ��ȡ��
					printf("�յ�header--����%d�����ݳ���%d��username=%s,passwd=%s\n", login.cmd, login.dataLength,login.userName,login.PassWord);
					LoginResult ret;
					
					send(_cSock, (char *)&ret, sizeof(LoginResult), 0);
					
				}
				break;
			case CMD_LOGINOUT:
				{
					LoginOut loginout = {};
					recv(_cSock, (char *)&loginout+sizeof(DataHeader), sizeof(LoginOut)- sizeof(DataHeader), 0);
					printf("�յ�header--����%d�����ݳ���%d��username=%s\n", loginout.cmd, loginout.dataLength, loginout.userName);
					LoginOutResult ret;
					send(_cSock, (char *)&ret, sizeof(LoginOutResult), 0);
				
				}
				break;
		

			default:
				{	
					header.cmd = CMD_ERROR;
					header.dataLength = 0;
					send(_cSock, (char*)&header, sizeof(DataHeader), 0);
				}
				break;
		}
		
	}


	//7. �ر��׽���
	closesocket(_sock);
	
	WSACleanup();
	printf("�������˳����������\n");
	getchar();
	return 0;
}