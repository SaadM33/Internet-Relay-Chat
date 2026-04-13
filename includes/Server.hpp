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

typedef struct s_msg {
	std::string					command;
	std::vector<std::string> 	params;
	std::string					trailing;
}	Message;

class Server;

typedef void (Server::*CmdHandler)(Message msg);

class Server
{
	private:
		int									port;
		std::string							passwd;

		int									core_fd;
		std::vector<struct pollfd>			poll_fds;

		std::map<int,Client *>				clients;
		// std::map<int,Channel *>		channels;

		bool								c_banished;

		std::map<std::string, CmdHandler>	cmdMap;

	public:
		
		Server(int, char **);

		void	setArgs(int, char **);
		void	ascend();
		void	ignite();
		
		void	acceptClient();
		bool	receiveClient(int);
		void	processCmd(int, Message);

		void	handleInput(int);

		void	disconnectClient(int, int);
};

