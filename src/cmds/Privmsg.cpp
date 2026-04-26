#include "Server.hpp"

void	Server::execPrivmsg(int fd, Message msg)
{
	if (msg.params.empty())
	{
		 sendReply(fd, ERR_NORECIPIENT, msg.command);
		return ;
	}
	else if (msg.trailing.empty())
	{
		sendReply(fd, ERR_NOTEXTTOSEND, msg.command);
		return ;
	}

	std::stringstream ss(msg.params[0]);
	std::string params;

	while (std::getline(ss, params, ','))
	{
		if (params.empty())
			continue ;
		if (params[0] == '#')
		{
			std::string channelName(params);
			if (this->channels.find(channelName) == this->channels.end())
			{
				this->sendReply(fd, ERR_NOSUCHNICK, channelName);
				continue;
			}
			if (this->channels[channelName]->members.find(fd) == this->channels[channelName]->members.end())
			{
				std::string reply = ":localhost 404 " + clients[fd]->nickName + " " + channelName + " :Cannot send to channel\r\n";
				send(fd, reply.c_str(), reply.size(), 0);
				continue ;
			}
			std::string message = "PRIVMSG " + params + " :" + msg.trailing + "\r\n";
			this->channels[channelName]->broadcast(this->clients[fd], message, true);
		}
		else if (params[0] == '@')
		{
			std::string channelName(params, 1);
			if (this->channels.find(channelName) == this->channels.end())
			{
				this->sendReply(fd, ERR_NOSUCHNICK, channelName);
				continue;
			}
			if (this->channels[channelName]->operators.find(fd) == this->channels[channelName]->operators.end())
			{
				std::string reply = ":localhost 404 " + clients[fd]->nickName + " " + channelName + " :Cannot send to channel\r\n";
				send(fd, reply.c_str(), reply.size(), 0);
				continue ;
			}
			std::string message = "PRIVMSG " + params + " :" + msg.trailing + "\r\n";
			this->channels[channelName]->operatorsBroadcast(this->clients[fd], message);
		}
		else
		{
			bool found = false;
			for (std::map<int, Client *>::iterator it = this->clients.begin(); it != this->clients.end(); it++)
			{
				if ((*it).second->nickName == params)
				{
					std::string reply = this->clients[fd]->getPrefix() + " PRIVMSG "
					+ params + " :" + msg.trailing + "\r\n";
					send((*it).second->fd, reply.c_str(), reply.size(), 0);
					found = true;
					break ;
				}
			}
			if (!found)
				this->sendReply(fd, ERR_NOSUCHNICK, params);
		}
	}
}
