#include <pthread.h>
#include <vector>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <iostream>
#include <string>

#define PORT 5642

void* session(void* vactiveSockets)
{
	std::cout << "accepted connection" << std::endl;

	std::vector<int> *activeSockets = (std::vector<int> *)vactiveSockets;

	int activeSocket = activeSockets->at(activeSockets->size() - 1);

	bool shouldRun = true;

	std::string name;

	while (shouldRun == true)
	{
		std::string msg;
		char buffer[1024] = {0};

		read(activeSocket, buffer, 1024);

		msg = buffer;

		if (msg.compare(0, 5, "/QUIT") == 0)
		{
			shouldRun = false;
		} else if (msg.compare(0, 5, "/NAME") == 0)
		{
			name = msg;
			name.erase(0, 6);
			name.erase(name.end() - 1, name.end());
			std::cout << name << std::endl;
		} else 
		{
			for (int i = 0; i < activeSockets->size(); i++)
			{
				if (activeSockets->at(i) != activeSocket)
				{
					send(activeSockets->at(i), (name + ": " + msg).c_str(), name.length() + msg.length() + 2, 0);
				}
			}
		}
	}

	char buffer[1024];

	shutdown(activeSocket, SHUT_RDWR);
	int bempty;
	do {
	bempty = read(activeSocket, buffer, 1024);
	} while (bempty != 0);
	close(activeSocket);	


	for (int i = 0; i < activeSockets->size(); i++)
	{
		if (activeSockets->at(i) == activeSocket)
		{
			activeSockets->erase(activeSockets->begin() + i);
		}
	}

	pthread_exit(NULL);
}

int SetupSockets(int &sockfd)
{
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == 0)
	{
		return -1;
	}

	int yes = 1;

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)))
	{
		return -2;
	}

	struct sockaddr_in address;

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( PORT );

	if (bind(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		return -3;
	}

	if (listen(sockfd, 3) < 0)
	{
		return -4;
	}

	return 0;
}

int main()
{
	int sockfd;

	int socerr = SetupSockets(sockfd);

	if (socerr < 0)
	{
		return socerr;
	}

	std::vector<pthread_t> Threads;

	std::vector<int> activeSockets;

	while (true == true)
	{
		int activeSocket = accept(sockfd, 0, 0);

		activeSockets.push_back(activeSocket);

		pthread_t thread;

		pthread_create(&thread, NULL, session, (void *)&activeSockets);
	}

	return 0;
}