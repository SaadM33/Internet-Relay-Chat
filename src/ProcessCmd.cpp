#include "Server.hpp"

static bool	needsRegistration(std::string command) {
	return ((command != "PASS") && (command != "NICK") && (command != "USER") && (command != "CAP"));
}

void	Server::processCmd(int fd, Message msg) {

	if (needsRegistration(msg.command) && !this->clients[fd]->isRegistered)
	{
		send(fd, "MyServerName 451 :You have not registered\r\n", 44, 0);
		return ;
	}

	std::map<std::string, cmdHandler>::iterator it = cmdMap.find(msg.command); // fill the cmdMap with the command name as key and the corresponding member function pointer as value
	if (it != cmdMap.end())
	{
		(this->*(it->second))(fd, msg);
	}
	else
	{
		std::string errorMsg = "MyServerName 421" + this->clients[fd]->nickName + " " + msg.command + " :Unknown command\r\n";

		send(fd, errorMsg.c_str(), errorMsg.size(), 0); //store the erorMsg later into the write buffer of client[fd]
	}
}