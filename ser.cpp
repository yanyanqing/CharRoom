#include "utility.h"

class Server
{
public:
	Server(string ip = "127.0.0.1", int port = 6500):_listenfd(-1),_epollfd(-1)
	{
		_sockfd = socket(AF_INET, SOCK_STREAM, 0);
		assert(_sockfd!= -1);
		
		struct sockaddr_in seraddr;
		memset(&seraddr, 0, sizeof(seraddr));
		seraddr.sin_family = AF_INET;
		seraddr.sin_port = htons(port);
		seraddr.sin_addr.s_addr = inet_addr(ip.c_str());
		
		assert(bind(_sockfd, (struct sockaddr*)&seraddr, sizeof(seraddr)) != -1);
		
		assert(listen(_sockfd, 5) != -1);
		
		_epollfd = epoll_create(EPOLL_MAX);
		addfd(_epollfd, _sockfd);

		bool reUse = true;
		setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&reUse, sizeof(bool));
			
	}
	
	void run()
	{
		static struct epoll_event events[EPOLL_MAX];
		char msg[BUFF_SIZE];
		while(1)
		{
			int num = epoll_wait(_epollfd, events, EPOLL_MAX, -1);
			for(int i = 0; i < num; ++i)
			{
				int sockfd = events[i].data.fd;
				if(sockfd == _sockfd)
				{
					struct sockaddr_in cliaddr;
					socklen_t len = sizeof(cliaddr);
					memset(&cliaddr, 0, len);
					_listenfd = accept(sockfd, (struct sockaddr*)&cliaddr, &len);
					if(_listenfd < 0)
					{
						perror("accept error");
						continue;
					}
					else
					{
                        cli_list.push_back(_listenfd);
						cout << "one client join char room ip:" << inet_ntoa(cliaddr.sin_addr)
						<< "port: " << ntohs(cliaddr.sin_port) << endl;
						printf("There are %d person in char room\n", cli_list.size());
						
						memset(msg, 0, BUFF_SIZE);
						sprintf(msg, SERVER_WELCOME, sockfd);
						send(_listenfd, msg, strlen(msg), 0);
						addfd(_epollfd, _listenfd);
					}
				}
				else
				{
					int ret = sendBroadcastMsg(sockfd);
					if(ret < 0)
					{
						perror("recv error");
						exit(0);
					}
				}
			}
		}
	}
	
private:
	int _sockfd;
	int _listenfd;
	int _epollfd;
};



int main()
{
    Server ser;

    ser.run();


    return 0;
}
