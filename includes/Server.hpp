#pragma once 

#include "Channel.hpp"
#include "Client.hpp"
#include "utils.hpp"


class Server
{
	private:

		int									port;
		std::string							passwd;

		int									core_fd;
		std::vector<struct pollfd>			poll_fds;

		std::map<int, Client *>				clients;
		std::map<std::string, Channel *>	channels;

		bool								c_banished;

		std::map<std::string, cmdHandler>	cmdMap;
		std::map<std::string, std::string>	replyMap;

	public:
		
		Server(int, char **);

		void	instantiateCmds();
		void	instantiateReplies();

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
		void	execJoin(int, Message);
		void	execPart(int, Message);
		void	execTopic(int, Message);
		void	execInvite(int, Message);
		void	execKick(int, Message);
		void	execMode(int, Message);
		void	execPrivmsg(int, Message);
		void	execPing(int, Message);

		void	sendReply(int fd, std::string code, std::string middle = "");
};
