#include <iostream>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <string>
#include <string.h>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <list>
using namespace std;

#define EPOLL_MAX 1024
#define BUFF_SIZE 1024

#define SERVER_WELCOME "Welcome you join to the chat room! Your chat ID is: Client #%d"
#define SERVER_MESSAGE "ClientID %d say >>>> %s"
#define CAUTION "There is only one int the char room!"

list<int> cli_list;

int setnonblock(int fd)
{
	int old_fd = fcntl(fd, F_GETFL, 0);
	int new_fd = old_fd | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_fd);
	
	return old_fd;
}

void addfd(int epollfd, int fd)
{
	struct epoll_event ev;
	ev.data.fd = fd;
	ev.events = EPOLLIN | EPOLLET;
	
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
	setnonblock(fd);
}

int sendBroadcastMsg(int sockfd)
{
	char msg[BUFF_SIZE] = {0};
	char buf[BUFF_SIZE] = {0};
	int ret = recv(sockfd, msg, BUFF_SIZE, 0);
	if(ret < 0)
	{
		perror("recv error");
		return -1;
	}
	else if(ret == 0)
	{	
		cli_list.remove(sockfd);
		printf("ClientID:%d closed\t There are %d person in the char room\n", sockfd, cli_list.size());
		close(sockfd);
	}
	else 
	{
		if(cli_list.size() == 1)
		{
			send(sockfd, CAUTION, strlen(CAUTION), 0);
			return 1;
		}
		else 
		{
			sprintf(buf, SERVER_MESSAGE, sockfd, msg);
			list<int>::iterator it = cli_list.begin();
			for(; it != cli_list.end(); ++it)
			{
				if(*it != sockfd)
				{
					send(*it, buf, strlen(buf), 0);
				}
			}
		}
	}
	
	return ret;
}
