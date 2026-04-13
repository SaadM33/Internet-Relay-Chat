#include "Server.hpp"

void	Server::execPass(int fd, Message msg) {
	if (this->clients[fd]) {
		//send 462
		return ;
	}

	if (msg.params.size() == 0) { // OR empty string ???
		//send 461
		return ;
	} 
	
	if (msg.params[0] != this->passwd) {
		//send 464 and disconnect
		return ;
	}

	this->clients[fd]->has_password = true;
}

void	Server::execUser(int fd, Message msg) {
	// implementation
	this->clients[fd]->has_user = true;
}

void	Server::execNick(int fd, Message msg) {
	// implementation
	this->clients[fd]->has_nick = true;
}

//fct if hadi w hadi w hadi -> isRegistered = true -> send 001;