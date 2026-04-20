#include "Server.hpp"

static bool	needsRegistration(std::string command)
{
	return ((command != "PASS") && (command != "NICK") && (command != "USER") && (command != "CAP"));
}

void	Server::processCmd(int fd, Message msg)
{
	//removed temporarily for a simpler testing flow
	if (needsRegistration(msg.command) && !this->clients[fd]->isRegistered)
	{
		sendReply(fd, ERR_NOTREGISTERED);
		return ;
	}

	std::map<std::string, cmdHandler>::iterator it = cmdMap.find(msg.command);
	if (it != cmdMap.end())
		(this->*(it->second))(fd, msg);
	else
		sendReply(fd, ERR_UNKNOWNCOMMAND);
}
