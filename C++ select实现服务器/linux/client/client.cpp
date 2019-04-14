
#include<thread>
#include"EasyTcpClient.hpp"
#pragma comment(lib,"ws2_32.lib")//?????
//ctrl + k ctrl + c?????ctrl+u??????



void cmdThread(EasyTcpClient *client)
{
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			client->Close();
			printf("??\n");
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
		//client.Connect("192.168.236.128", 4567);
		client.Connect("10.201.17.179", 4568);
	}

	//??UI??
	std::thread t1(cmdThread,&client);
	t1.detach();//??????

	while (client.isRun())
	{
		if (!client.onRun())
			break;
		/*printf("client idle?processing other tasks...\n");
		Sleep(1000);*/
	}

	client.Close();
	printf("exited\n");
	getchar();

	return 0;
}
