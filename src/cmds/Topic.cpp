#include "Server.hpp"

void	Server::execTopic(int fd, Message msg)
{
	if (msg.params.size() < 1) {
		sendReply(fd, ERR_NEEDMOREPARAMS, msg.command);
		return;
	}
	std::string	name_ch = msg.params[0];

	if (channels.find(name_ch) == channels.end()) {
		sendReply(fd, ERR_NOSUCHCHANNEL, name_ch);
		return ;
	}
	if (clients[fd]->channels.find(name_ch) == clients[fd]->channels.end()) {
		sendReply(fd, ERR_NOTONCHANNEL, name_ch);
		return ;
	}

	Channel	*chan = channels[name_ch];

	if (msg.trailing.empty())
	{
		if (chan->topic.empty())
			sendReply(fd, RPL_NOTOPIC, name_ch);
		else
		{
			std::string reply = ":localhost 332 " + clients[fd]->nickName + " " + name_ch;
			reply += " :" + chan->topic + "\r\n";
			send(fd, reply.c_str(), reply.size(), 0);
		}
	}
	else
	{
		if (chan->topicRestricted && chan->operators.find(fd) == chan->operators.end())
			sendReply(fd, ERR_CHANOPRIVSNEEDED, name_ch);
		else
		{
			chan->topic = msg.trailing;
			std::string	reply = "TOPIC " + name_ch + " :" + msg.trailing + "\r\n";
			chan->broadcast(clients[fd], reply);
		}
	}
}
