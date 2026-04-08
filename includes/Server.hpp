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
#include <cstdio>

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

		void setArgs(int ac, char **av);
		void ascend();
		void ignite();

		void AcceptClient();
		void ReceiveClient();
		void DisconnectClient(){}


};