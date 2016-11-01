#include "utility.h"

class Client
{
public:
	Client(string ip = "127.0.0.1", int port = 6500):_clifd(-1),_epollfd(-1),isCliWork(true)
	{
		_clifd = socket(AF_INET, SOCK_STREAM, 0);
		assert(_clifd != -1);
		
		struct sockaddr_in seraddr;
		memset(&seraddr, 0, sizeof(seraddr));
		seraddr.sin_family = AF_INET;
		seraddr.sin_port = htons(port);
		seraddr.sin_addr.s_addr = inet_addr(ip.c_str());
		
		assert(connect(_clifd, (struct sockaddr*)&seraddr, sizeof(seraddr)) != -1);
		if(pipe(_pipefd) < 0)
		{
			perror("pipe error");
			exit(0);
		}
		
		_epollfd = epoll_create(EPOLL_MAX);
		if(_epollfd < 0)
		{
			perror("epoll_create error");
			exit(-1);
		}
		
//		addfd(_epollfd, _pipefd[0]);
//		addfd(_epollfd, _clifd);
	}
	
	
	
	void run()
	{
		char msg[BUFF_SIZE] = {0};
		int pid = fork();
		if(pid < 0)
		{
			perror("fork error");
			exit(-1);
		}
		if(pid == 0)
		{
			close(_pipefd[0]);
			while(isCliWork)
			{
				memset(msg, 0, BUFF_SIZE);
				cout << "please input exit quit the char room:";
				fgets(msg, BUFF_SIZE, stdin);
				if(strncmp(msg, "exit", 4) == 0)
				{
					isCliWork = false;
				}
				else 
				{
					if(write(_pipefd[1], msg, strlen(msg)) < 0)
                    {
                        perror("send error");
                        exit(0);
                    }
				}
			}
		}
		else 
		{
    		addfd(_epollfd, _pipefd[0]);
	    	addfd(_epollfd, _clifd);
			close(_pipefd[1]);
			struct epoll_event events[EPOLL_MAX];
			while(isCliWork)
			{
				int num = epoll_wait(_epollfd, events, EPOLL_MAX, -1);
				
				for(int i = 0; i < num; ++i)
				{
					memset(msg, 0, BUFF_SIZE);
					int sockfd = events[i].data.fd;
					if(sockfd == _clifd)
					{	
						int ret = recv(sockfd, msg, BUFF_SIZE, 0);
						if(ret <= 0)
						{
							cout << "Server close" << endl;
							close(_clifd);
							isCliWork = false;
						}
						else
							cout << msg << endl;
					}
					else
					{
                        memset(msg, 0, BUFF_SIZE);
						int ret = read(_pipefd[0], msg, BUFF_SIZE);
						if(ret <= 0)
						{
							perror("recv error");
							isCliWork = false;
						}
						else 
							send(_clifd, msg, strlen(msg), 0);
					}
				}
			}
		}
	}
	
private:
	int _clifd;
	int _epollfd;
	int _pipefd[2];
	bool isCliWork;
};



int main()
{
    Client cli;
    cli.run();

    return 0;
}
