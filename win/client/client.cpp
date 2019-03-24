
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
//客户端数量
const int cCount = 10000;
//发送线程数量
const int tCount = 4;
EasyTcpClient* client[cCount];

void sendThread(int id)//4个线程ID为1~4
{
	int c = (cCount) / tCount;
	int begin = (id - 1) *c;
	int end = id * c;
	for (int n = begin; n < end; n++)
	{
		client[n] = new EasyTcpClient();
	}

	for (int n = begin; n < end; n++)
	{
		client[n]->Connect("192.168.236.128", 4567);
		//client[n]->Connect("10.201.20.112", 4567);
		cout <<"thread id :"<<id<< ",Connect= " << n<<endl;
	}

	///等待
	std::chrono::milliseconds t(5000);
	std::this_thread::sleep_for(t);

	Login login[10];
	for (size_t i = 0; i < 10; i++)
	{
		strcpy(login[i].userName, "zhangjing");
		strcpy(login[i].PassWord, "jains");
	}
	/*Login login;
	strcpy(login.userName, "zhangjing");
	strcpy(login.PassWord, "jains");*/
	while (g_bRun)
	{

		for (int n = begin; n < end; n++)
		{

			client[n]->SendData(login);
			//client[n]->onRun();
			
		}
	}
	for (int n = begin; n < end; n++)
	{
		client[n]->Close();
	}
}
int main()
{	//启动UI线程
	std::thread t1(cmdThread);
	t1.detach();//和主线程分离

	//启动发送线程
	for (int n = 0; n < tCount; n++)
	{
		std::thread t1(sendThread,n+1);
		t1.detach();//和主线程分离
	}
	
	while (g_bRun)
	{
		Sleep(100);
	}



	
	printf("exited\n");
	getchar();

	return 0;
}
