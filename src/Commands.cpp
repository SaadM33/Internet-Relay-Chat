#include "Server.hpp"

bool	isAvailable(const std::map<int, Client *> &clients, std::string str) {
	for (std::map<int, Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		if (it->second->nickName == str)
			return false;
	}
	return true;
}

bool	isAlnumStr(std::string str) {
	for (size_t i = 0; i < str.size(); i++) {
		if (!isalnum(str[i]))
			return false;
	}
	return true;
}

void	Server::execCap(int fd, Message msg)
{
	// CAP LS
	if (msg.params.size() == 1 && msg.params[0] == "LS") {
		std::string response = "CAP * LS :multi-prefix\r\n";
		send(fd, response.c_str(), response.size(), 0);
	}
}

void	Server::execPass(int fd, Message msg)
{
	if (this->clients[fd]->isRegistered) {
		//send 462, what if we send the wrong code, error or output change in ssi y?
		return ;
	}

	if (msg.params.size() != 1) {
		if (msg.params.empty())
			//send 461
		return ;
	}

	if (msg.params[0] != this->passwd) {
		//send 464 and disconnect
		return ;
	}

	this->clients[fd]->has_pass = true;
}

void	Server::execNick(int fd, Message msg)
{
	if (msg.params.size() != 1) {
		if (msg.params.empty())
			//send 431
		return ;
	}

	if (!isAlnumStr(msg.params[0])) //later underscore
	{
		//send 432
		return ;
	}

	if (!isAvailable(this->clients, msg.params[0]))
	{
		//send 433
		return ;
	}

	if (!this->clients[fd]->isRegistered)
	{
		this->clients[fd]->nickName = msg.params[0];
		this->clients[fd]->has_nick = true;
	}
	else
	{
		this->clients[fd]->nickName = msg.params[0];
		// send ":old_nick!user@host NICK :new_nick\r\n."
	}
}

void	Server::execUser(int fd, Message msg)
{
	// USER <username> <localhost>
	if (this->clients[fd]->isRegistered) {
		//send 462
		return ;
	}


	if (msg.params.size() < 3 && msg.trailing.empty())
	{
		//send 461
		return ;
	}
	this->clients[fd]->userName = msg.params[0];
	this->clients[fd]->realName = msg.trailing;
	this->clients[fd]->has_user = true;
}

void	Server::execJoin(int fd, Message msg) { (void)fd, (void)msg; }

void	Server::execPart(int fd, Message msg) { (void)fd, (void)msg; }

void	Server::execTopic(int fd, Message msg) { (void)fd, (void)msg; }

void	Server::execInvite(int fd, Message msg) { (void)fd, (void)msg; }

void	Server::execKick(int fd, Message msg) { (void)fd, (void)msg; }

void	Server::execMode(int fd, Message msg) { (void)fd, (void)msg; }

void	Server::execPrivmsg(int fd, Message msg) { (void)fd, (void)msg; }


//fct if hadi w hadi w hadi -> isRegistered = true -> send 001;
void	Server::registerClient(int fd) {
	if (clients[fd]->has_nick && clients[fd]->has_user && clients[fd]->has_pass)
		clients[fd]->isRegistered = true;
	//send 001
}