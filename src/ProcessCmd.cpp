#include "Server.hpp"

static bool	needsRegistration(std::string command) {
	return ((command != "PASS") && (command != "NICK") && (command != "USER") && (command != "CAP"));
}

void	Server::processCmd(int fd, Message msg) {

	if (needsRegistration(msg.command) && !this->clients[fd]->isRegistered)
	{
		send(fd, ":localhost 451 * :You have not registered\r\n", sizeof(":localhost 451 * :You have not registered\r\n") - 1, 0);
		return ;
	}

	std::map<std::string, cmdHandler>::iterator it = cmdMap.find(msg.command);
	if (it != cmdMap.end())
	{
		(this->*(it->second))(fd, msg);
	}
	else
		sendReply(fd, "421", msg.command + " :Unknown command");
}