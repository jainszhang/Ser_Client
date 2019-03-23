
#include<thread>
#include"EasyTcpClient.hpp"
#pragma comment(lib,"ws2_32.lib")//静态链接库
//ctrl + k ctrl + c注释代码，ctrl+u取消注释代码


bool g_bRun = true;
void cmdThread()
{
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			g_bRun = false;
			printf("退出\n");
			break;
		}
		/*else if (0 == strcmp(cmdBuf, "login"))
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
		}*/
		else
		{
			printf("not support cmd\n");
		}

	}
	
}

int main()
{
	const int cCount = 2;

	EasyTcpClient* client[cCount];
	for (int n = 0; n < cCount; n++)
	{
		client[n] = new EasyTcpClient();
		client[n]->Connect("192.168.236.128", 4567);
	}


	//启动UI线程
	std::thread t1(cmdThread);
	t1.detach();//和主线程分离

	Login login;
	strcpy(login.userName, "zhangjing");
	strcpy(login.PassWord, "jains");
	while (g_bRun)
	{
		
		for (int n = 0; n < cCount; n++)
		{
			
			client[n]->SendData(&login);
			client[n]->onRun();
		}
	}
	for (int n = 0; n < cCount; n++)
	{
		client[n]->Close();
	}
	
	printf("exited\n");
	getchar();

	return 0;
}
