//
//  udpServer.cpp
//  HelloMac
//
//  Created by jains on 2019/4/15.
//  Copyright © 2019年 jains. All rights reserved.
//

#include "udpTest.hpp"

/*Linux下UDP服务器套接字程序，服务器接收客户端发送的信息并显示，同时显示客户的IP地址、端口号，并向客户端发送信息。如果服务器接收的客户信息为“bye”，则退出循环，并关闭套接字。*/
void server()
{
    int _sockfd;
    struct sockaddr_in _server;
    struct sockaddr_in _client;
    socklen_t _addrLen;
    int _num;
    char buf[MAXDATASIZE];
    if((_sockfd = socket(AF_INET, SOCK_DGRAM, 0))==-1)
    {
        perror("create socket failed");
        exit(1);
    }
    
    bzero(&_server, sizeof(_server));
    _server.sin_family=AF_INET;
    _server.sin_port=htons(PORT);
    _server.sin_addr.s_addr=htonl(INADDR_ANY);
    if(bind(_sockfd, (struct sockaddr*)&_server, sizeof(_server))==-1)
    {
        perror("Bind () error");
        exit(1);
    }
    printf("绑定成功\n");
    _addrLen = sizeof(_client);
    printf("等待接收客户端消息\n");
    while (1) {
        _num = int(recvfrom(_sockfd, buf, MAXDATASIZE, 0, (struct sockaddr*)&_client, &_addrLen));
        if(_num<0)
        {
            perror("recvfrom() error\n");
            exit(1);
        }
        buf[_num] = '\0';
        printf("You got a message %s from client.\nIt's ip is%s, port is %d\n",buf,
               inet_ntoa(_client.sin_addr),htons(_client.sin_port));
        sendto(_sockfd,"Welcometo my server.\n",22,0,(struct sockaddr *)&_client,_addrLen);
        if(!strcmp(buf,"bye"))
            break;

    }
    close(_sockfd);
}


/*1、客户根据用户提供的IP地址将用户从终端输入的信息发送给服务器，然后等待服务器的回应。
 2、服务器接收客户端发送的信息并显示，同时显示客户的IP地址、端口号，并向客户端发送信息。如果服务器接收的客户信息为“bye”，则退出循环，并关闭套接字。
 3、客户接收、显示服务器发回的信息，并关闭套接字
 */
void client(int argc,char *argv[])
{
    int sockfd, num;
    char buf[MAXDATASIZE];
    
    struct hostent *he;
    struct sockaddr_in server,peer;
    
    if (argc !=3)
    {
        printf("Usage: %s <IP Address><message>\n",argv[0]);
        exit(1);
    }
    
    if ((he=gethostbyname(argv[1]))==nullptr)
    {
        printf("gethostbyname()error\n");
        exit(1);
    }
    
    if ((sockfd=socket(AF_INET, SOCK_DGRAM,0))==-1)
    {
        printf("socket() error\n");
        exit(1);
    }
    
    bzero(&server,sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr= *((struct in_addr *)he->h_addr);
    sendto(sockfd, argv[2],strlen(argv[2]),0,(struct sockaddr *)&server,sizeof(server));
    socklen_t  addrlen;
    addrlen=sizeof(server);
    while (1)
    {
        if((num=int(recvfrom(sockfd,buf,MAXDATASIZE,0,(struct sockaddr *)&peer,&addrlen)))== -1)
        {
            printf("recvfrom() error\n");
            exit(1);
        }
        if (addrlen != sizeof(server) ||memcmp((const void *)&server, (const void *)&peer,addrlen) != 0)
        {
            printf("Receive message from otherserver.\n");
            continue;
        }
        
        buf[num]='\0';
        printf("Server Message:%s\n",buf);
        break;
    }
    close(sockfd);
}
