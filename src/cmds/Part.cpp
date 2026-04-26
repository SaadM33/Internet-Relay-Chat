#include "Server.hpp"

void	Server::execPart(int fd, Message msg)
{
	if (msg.params.size() < 1) {
		sendReply(fd, ERR_NEEDMOREPARAMS);
		return;
	}

	std::stringstream ss(msg.params[0]);
	std::string item;

	while (std::getline(ss, item, ','))
	{
		if (item.empty())
			continue ;
		if (channels.find(item) == channels.end()) {
			sendReply(fd, ERR_NOSUCHCHANNEL, item);
			continue ;
		}
		if (clients[fd]->channels.find(item) == clients[fd]->channels.end()) {
			sendReply(fd, ERR_NOTONCHANNEL, item);
			continue ;
		}

		std::string partMsg = "PART " + item;
		if (!msg.trailing.empty())
			partMsg += " :" + msg.trailing;
		partMsg += "\r\n";
		channels[item]->broadcast(clients[fd], partMsg);
		
		channels[item]->removeClient(clients[fd]);

		if (channels[item]->members.empty()) {
			delete channels[item];
			channels.erase(item);
		}
	}
}
