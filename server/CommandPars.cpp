#include "Server.hpp"

static bool	isNumericCmd(std::string command) {
	if (command.size() != 3)
		return false;

	for (int i = 0; i < 3; i++) {
		if (!isdigit(command[i]))
			return false;
	}

	return true;
}

static bool	isAlphaCmd(std::string command) {
	for (size_t i = 0; i < command.length(); i++) {
		if (!isalpha(command[i]))
			return false;
	}
	return true;
}

bool	isValidCommand(std::string command) {
	if (isNumericCmd(command))
		return true;
	
	if (isAlphaCmd(command))
		return true;
	
	return false;
}

static bool	needsRegistration(std::string command) {
	return ((command != "PASS") && (command != "NICK") && (command != "USER"));
}

void	Server::processCmd(int fd, Message msg) {
	if (!isValidCommand(msg.command))
		return ;
	if (needsRegistration(msg.command) && !this->clients[fd]->isRegistered)
		return ;
}

