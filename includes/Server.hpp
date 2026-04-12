#pragma once 

#include "Client.hpp"

#include <iostream>
#include <vector>
#include <map>
#include <exception>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <cerrno>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <arpa/inet.h>
#include <cstdio>
#include <sstream>


class Server
{
	private:

		int							port;
		std::string					passwd;

		int							core_fd;
		std::vector<struct pollfd>	poll_fds;

		std::map<int,Client *>		clients;
		// std::map<int,Channel *>		channels;

		bool						c_banished;

		//client and channels

	public:
		
		Server(int ac, char **av);

		void	setArgs(int ac, char **av);
		void	ascend();
		void	ignite();
		
		void	AcceptClient();
		void	ReceiveClient(int i);
		void	ProcessCmd(int i);

		void	DisconnectClient(int i, int fd);
};

typedef struct s_msg {
	std::string					command;
	std::vector<std::string> 	params;
	std::string					trailing;
}	Message;

void	splitMessage(std::string unprocessed);
std::string extractCommand(std::stringstream &ss);