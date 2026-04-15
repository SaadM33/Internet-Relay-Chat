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
typedef void (Server::*cmdHandler)(int, Message);

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

		std::map<std::string, cmdHandler>	cmdMap;

	public:
		
		Server(int, char **);

		void	instantiateCmds();
		void	ascend();
		void	ignite();
		
		void	acceptClient();
		void	processClient(int);
		void	processCmd(int, Message);

		void	handleInput(int);

		void	disconnectClient(int, int);
		void	registerClient(int);

		void	execCap(int, Message);
		void	execPass(int, Message);
		void	execNick(int, Message);
		void	execUser(int, Message);
		void	execPing(int, Message);
		void	execPong(int, Message);
		void	execQuit(int, Message);
		void	execJoin(int, Message);
		void	execPart(int, Message);
		void	execTopic(int, Message);
		void	execNames(int, Message);
		void	execInvite(int, Message);
		void	execKick(int, Message);
		void	execMode(int, Message);
		void	execPrivmsg(int, Message);
		void	execNotice(int, Message);
		void	execWho(int, Message);
		void	execWhois(int, Message);
};
