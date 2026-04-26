#include "Server.hpp"

void    Server::execKick(int fd, Message msg)
{
	if (msg.params.empty() || msg.params.size() < 2)
	{
		sendReply(fd, ERR_NEEDMOREPARAMS, msg.command);
		return ;
	}

	if (this->channels.find(msg.params[0]) == this->channels.end())
	{
		sendReply(fd, ERR_NOSUCHCHANNEL, msg.params[0]);
		return ;
	}

	if (this->channels[msg.params[0]]->members.find(fd) == this->channels[msg.params[0]]->members.end())
	{
		sendReply(fd, ERR_NOTONCHANNEL, msg.params[0]);
		return ;
	}

	if (this->channels[msg.params[0]]->operators.find(fd) == this->channels[msg.params[0]]->operators.end())
	{
		sendReply(fd, ERR_CHANOPRIVSNEEDED, msg.params[0]);
		return ;
	}

	std::string comment;
	if (msg.trailing.empty())
		comment = this->clients[fd]->nickName;
	else
		comment = msg.trailing;

	std::stringstream ss(msg.params[1]);
	std::string targetNick;
	while (std::getline(ss, targetNick, ','))
	{
		bool found = false;
		int	targetFd;
		for (std::map<int, Client *>::iterator it = this->clients.begin(); it != this->clients.end(); it++)
		{
			if ((*it).second->nickName == targetNick)
			{
				found = true;
				targetFd = (*it).second->fd;
				break ;
			}
		}
		if (!found)
		{
			sendReply(fd, ERR_NOSUCHNICK, targetNick);
			continue ;
		}

		if (this->channels[msg.params[0]]->members.find(targetFd) == this->channels[msg.params[0]]->members.end())
		{
			sendReply(fd, ERR_USERNOTINCHANNEL, targetNick + " " + msg.params[0]);
			continue ;
		}

		std::string brdcst = "KICK " + msg.params[0] + " " + targetNick + " :" + comment + "\r\n";
		this->channels[msg.params[0]]->broadcast(this->clients[fd], brdcst);
		this->channels[msg.params[0]]->removeClient(this->clients[targetFd]);
	}
}
