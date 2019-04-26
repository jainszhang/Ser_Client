//
//  udpServer.hpp
//  HelloMac
//
//  Created by jains on 2019/4/15.
//  Copyright © 2019年 jains. All rights reserved.
//

#ifndef udpServer_hpp
#define udpServer_hpp

#include <stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include <netdb.h>

#define PORT 1234
#define MAXDATASIZE 100

void server();
void client(int argc,char *argv[]);
#endif /* udpServer_hpp */
