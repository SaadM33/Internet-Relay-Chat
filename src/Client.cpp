#include "Client.hpp"


Client::Client(int fd, sockaddr_in addr)
{
	this->fd = fd;
	this->ip = inet_ntoa(addr.sin_addr);

	this->nickName = "*";
	this->userName = "";
	this->realName = "";

	this->isRegistered = false;
	this->has_pass = false;
	this->has_nick = false;
	this->has_user = false;

	this->r_buffer = "";
}

std::string	Client::getPrefix()
{
	return ":" + this->nickName + "!" + this->userName + "@" + this->ip;
}