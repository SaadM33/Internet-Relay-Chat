#include "Server.hpp"

void	Server::execNick(int fd, Message msg)
{
	if (msg.params.empty()) {
		sendReply(fd, ERR_NONICKNAMEGIVEN);
		return ;
	}

	if (!isValidNick(msg.params[0])) {
		sendReply(fd, ERR_ERRONEUSNICKNAME);
		return ;
	}

	if (getFdFromNick(this->clients, msg.params[0]) != -1) {
		sendReply(fd, ERR_NICKNAMEINUSE, msg.params[0]);
		return ;
	}

	if (!this->clients[fd]->isRegistered)
	{
		clients[fd]->nickName = msg.params[0];
		this->clients[fd]->has_nick = true;
		registerClient(fd);
	}
	else
	{
		std::string brdcst = clients[fd]->getPrefix() + " NICK :" + msg.params[0] + "\r\n";
		std::set<int> seen;

		seen.insert(fd);
		send(fd, brdcst.data(), brdcst.size(), 0);

		for ( std::map<std::string, Channel *>::iterator it = clients[fd]->channels.begin(); it != clients[fd]->channels.end(); it++)
		{
			std::map<int,Client *>::iterator	it2;
			for (it2 = it->second->members.begin(); it2 != it->second->members.end(); it2++)
			{
				if (seen.find(it2->first) == seen.end())
				{
					seen.insert(it2->first);
					send(it2->first, brdcst.data(), brdcst.size(), 0);
				}
			}
		}
		clients[fd]->nickName = msg.params[0];
	}
}
