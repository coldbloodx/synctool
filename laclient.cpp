#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>

#include <iostream>
using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "netutils.h"
#include "command.h"
#include "configparser.h"

const int STR_MAX_LEN = 1024;

void sighandler(int signo)
{
    CommandExecuter::runningflag = 0;
}

int main()
{
    gparser->initkvmap("conf/lasync.conf");

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd == -1)
    {
        perror("socket");
        exit(1);
    }
    string port;
    int ret = gparser->getvalue("port", port);

    sockaddr_in myaddr;
    sockaddr_in peeraddr;

    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(atoi(port.c_str()));
    myaddr.sin_addr.s_addr = INADDR_ANY;

    socklen_t len = sizeof(struct sockaddr);

    int res = bind(sockfd, (struct sockaddr*)&myaddr, len);

    if(res == -1)
    {
        perror("bind");
        exit(1);
    }

    char buff[64];
    gethostname(buff, 64);
    string myname(buff);

    pthread_t tid;

    pthread_create(&tid, NULL, CommandExecuter::executecmd, NULL);

    while(1)
    {
        char recvbuf[STR_MAX_LEN];
        int msglen = recvfrom(sockfd, recvbuf, STR_MAX_LEN, 0, (struct sockaddr*)&peeraddr, &len);
        CommandExecuter::extractmsg((char*)recvbuf, msglen, peeraddr, myname);
    }

    return 0;
}

