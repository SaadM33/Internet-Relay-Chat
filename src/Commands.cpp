#include "Server.hpp"

void	Server::sendReply(int fd, std::string code)
{
	std::string reply;
	reply = ":localhost " + code + " " + this->clients[fd]->nickName + " :" + replyMap[code] + "\r\n";
	
	send(fd, reply.c_str(), reply.size(), 0);	
}

bool	isAvailable(const std::map<int, Client *> &clients, std::string str)
{
	for (std::map<int, Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		if (it->second->nickName == str)
			return false;
	}
	return true;
}

bool	isAlnumStr(std::string str)
{
	for (size_t i = 0; i < str.size(); i++) {
		if (!isalnum(str[i]) && str[i] != '-' && str[i] != '_')
			return false;
	}
	return true;
}

void	Server::execCap(int fd, Message msg)
{
	if (msg.params.size() >= 1 && msg.params[0] == "LS")
		send(fd, ":localhost CAP * LS :\r\n", 23, 0);
}

void	Server::execPass(int fd, Message msg)
{
	if (msg.params.empty()) {
		sendReply(fd, ERR_NEEDMOREPARAMS);
		return ;
	}

	if (this->clients[fd]->isRegistered) {
		sendReply(fd, ERR_ALREADYREGISTERED);
		return ;
	}

	if (msg.params[0] != this->passwd) {
		sendReply(fd, ERR_PASSWDMISMATCH);
		return ;
	}
	this->clients[fd]->has_pass = true;
}

void	Server::execNick(int fd, Message msg)
{
	if (msg.params.empty()) {
		sendReply(fd, ERR_NONICKNAMEGIVEN);
		return ;
	}

	if (!isAlnumStr(msg.params[0])) {
		sendReply(fd, ERR_ERRONEUSNICKNAME);
		return ;
	}

	if (!isAvailable(this->clients, msg.params[0])) {
		sendReply(fd, ERR_NICKNAMEINUSE);
		return ;
	}

	if (!this->clients[fd]->isRegistered)
	{
		this->clients[fd]->nickName = msg.params[0];
		this->clients[fd]->has_nick = true;
		registerClient(fd);
	}
	else
	{
		std::string brdcst = this->clients[fd]->getPrefix() + " NICK :" + msg.params[0] + "\r\n";

		send(fd, brdcst.c_str(), brdcst.size(), 0); // send broadcast to all clients in the same channels as fd
		this->clients[fd]->nickName = msg.params[0];
	}
}

void	Server::execUser(int fd, Message msg)
{
	if (this->clients[fd]->isRegistered) {
		sendReply(fd, ERR_ALREADYREGISTERED);
		return ;
	}

	if (msg.params.size() < 3 || msg.trailing.empty()){
		sendReply(fd, ERR_NEEDMOREPARAMS);
		return ;
	}
	this->clients[fd]->userName = msg.params[0];
	this->clients[fd]->realName = msg.trailing;
	this->clients[fd]->has_user = true;

	registerClient(fd);
}

void	Server::registerClient(int fd)
{
	if (clients[fd]->has_nick && clients[fd]->has_user && clients[fd]->has_pass)
	{
		clients[fd]->isRegistered = true;
		sendReply(fd, RPL_WELCOME);
	}
}

void	Server::execJoin(int fd, Message msg) { (void)fd, (void)msg; }

void	Server::execPart(int fd, Message msg) { (void)fd, (void)msg; }

void	Server::execTopic(int fd, Message msg) { (void)fd, (void)msg; }

void	Server::execInvite(int fd, Message msg) { (void)fd, (void)msg; }

void	Server::execKick(int fd, Message msg) { (void)fd, (void)msg; }

void	Server::execMode(int fd, Message msg) { (void)fd, (void)msg; }

void	Server::execPrivmsg(int fd, Message msg) { (void)fd, (void)msg; }

