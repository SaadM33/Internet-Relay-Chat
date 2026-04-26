#include "Server.hpp"

void	Server::execNick(int fd, Message msg)
{
	if (msg.params.empty()) {
		sendReply(fd, ERR_NONICKNAMEGIVEN);
		return ;
	}

	if (!isAlnumStr(msg.params[0])) {
		sendReply(fd, ERR_ERRONEUSNICKNAME);
		return ;
	}

	if (getFdFromNick(this->clients, msg.params[0]) != -1) {
		sendReply(fd, ERR_NICKNAMEINUSE, msg.params[0]);
		return ;
	}

	if (!this->clients[fd]->isRegistered)
	{
		this->clients[fd]->nickName = msg.params[0];
		this->clients[fd]->has_nick = true;
		registerClient(fd);
	}
	else
	{
		std::string brdcst = "NICK :" + msg.params[0] + "\r\n";

		std::map<std::string, Channel *>::iterator it;
		for (it = clients[fd]->channels.begin(); it != clients[fd]->channels.end(); it++)
			it->second->broadcast(clients[fd], brdcst);
		
		clients[fd]->nickName = msg.params[0];
	}
}
