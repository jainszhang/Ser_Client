
#include<thread>
#include"EasyTcpClient.hpp"
#pragma comment(lib,"ws2_32.lib")//静态链接库
//ctrl + k ctrl + c注释代码，ctrl+u取消注释代码



void cmdThread(EasyTcpClient *client)
{
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			client->Close();
			printf("退出\n");
			break;
		}
		else if (0 == strcmp(cmdBuf, "login"))
		{
			Login login;
			strcpy(login.userName, "zj");
			strcpy(login.PassWord, "jainss");
			client->SendData(&login);


		}
		else if (0 == strcmp(cmdBuf, "loginout"))
		{
			LoginOut loginout;
			strcpy(loginout.userName, "zj");
			client->SendData(&loginout);
		}
		else
		{
			printf("not support cmd\n");
		}

	}
	
}

int main()
{
	EasyTcpClient client;
	int ret = client.InitSocket();
	if (!ret)
	{
		client.Connect("192.168.236.128", 4567);
	}

	//启动UI线程
	std::thread t1(cmdThread,&client);
	t1.detach();//和主线程分离

	while (client.isRun())
	{
		if (!client.onRun())
			break;
		/*printf("client idle，processing other tasks...\n");
		Sleep(1000);*/
	}

	client.Close();
	printf("exited\n");
	getchar();

	return 0;
}
