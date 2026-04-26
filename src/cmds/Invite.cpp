#include "Server.hpp"

void    Server::execInvite(int fd, Message msg)
{
	if (msg.params.empty() || msg.params.size() < 2)
	{
		sendReply(fd, ERR_NEEDMOREPARAMS, msg.command);
		return ;
	}

	bool found = false;
	int	targetFd;
	for (std::map<int, Client *>::iterator it = this->clients.begin(); it != this->clients.end(); it++)
	{
		if ((*it).second->nickName == msg.params[0])
		{
			found = true;
			targetFd = (*it).second->fd;
			break ;
		}
	}
	if (!found)
	{
		sendReply(fd, ERR_NOSUCHNICK, msg.params[0]);
		return ;
	}

	if (this->channels.find(msg.params[1]) == this->channels.end())
	{
		sendReply(fd, ERR_NOSUCHCHANNEL, msg.params[1]);
		return ;
	}

	if (this->channels[msg.params[1]]->members.find(fd) == this->channels[msg.params[1]]->members.end())
	{
		sendReply(fd, ERR_NOTONCHANNEL, msg.params[1]);
		return ;
	}

	if (this->channels[msg.params[1]]->isInviteOnly)
	{
		if (this->channels[msg.params[1]]->operators.find(fd) == this->channels[msg.params[1]]->operators.end())
		{
			sendReply(fd, ERR_CHANOPRIVSNEEDED, msg.params[1]);
			return ;
		}
	}

	if (this->channels[msg.params[1]]->members.find(targetFd) != this->channels[msg.params[1]]->members.end())
	{
		sendReply(fd, ERR_USERONCHANNEL, msg.params[0] + " " + msg.params[1]);
		return ;
	}

	std::string reply = ":localhost 341 " + this->clients[fd]->nickName + " " + msg.params[0] + " " + this->channels[msg.params[1]]->name + "\r\n";
	send(fd, reply.c_str(), reply.size(), 0);

	reply = this->clients[fd]->getPrefix() + " " + msg.command + " " + msg.params[0] + " " + this->channels[msg.params[1]]->name + "\r\n";
	send(targetFd, reply.c_str(), reply.size(), 0);

	this->channels[msg.params[1]]->inviteList.push_back(targetFd);
}
