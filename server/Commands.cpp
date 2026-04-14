#include "Server.hpp"

bool	isAvailable(const std::map<int, Client *> &clients, std::string str) {
	for (std::map<int, Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		if (it->second->getNickName() == str)
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
		this->clients[fd]->setNickName(msg.params[0]);
		this->clients[fd]->has_nick = true;
	}
	else
	{
		this->clients[fd]->setNickName(msg.params[0]);
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
	this->clients[fd]->setUserName(msg.params[0]);
	this->clients[fd]->setRealName(msg.trailing);
	this->clients[fd]->has_user = true;
}
//fct if hadi w hadi w hadi -> isRegistered = true -> send 001;
void	Server::registerClient(int fd) {
	if (clients[fd]->has_nick && clients[fd]->has_user && clients[fd]->has_pass)
		clients[fd]->isRegistered = true;
	//send 001
}