#include"EasyTcpServer.hpp"
int main()
{
	EasyTcpServer server;
	//server.InitSocket();
	server.Bind(nullptr,4568);
	server.Listen(5);
	
	while (server.isRun())
	{
		server.OnRun();

	}
	server.Close();
	getchar();
	return 0;
}