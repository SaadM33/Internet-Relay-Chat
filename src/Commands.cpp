#include "Server.hpp"

// std::string c_buffer;

// void	Server::sendReply(int fd, std::string code, std::string message) 
// {
// 	std::string reply = ":localhost " + code + " " + this->clients[fd]->nickName + " :" + message + "\r\n";
// 	send(fd, reply.c_str(), reply.size(), 0);
// }

void	Server::sendReply(int fd, int code, bool clientExist) // just a prototype for standardisation accross all sends
{
	// these if-conditions make this function in DIRE need for change!!
	std::string reply;
	if (clientExist)
	{
		reply = ":localhost " + replyMap[code].first + " " + this->clients[fd]->nickName + " :" + replyMap[code].second + "\r\n";
		send(fd, reply.c_str(), reply.size(), 0);
	}
	else
	{
		if (code == RPL_CAP)
			send(fd, ":localhost CAP * LS :\r\n", 23, 0);
		else if (code == ERR_NEEDMOREPARAMS)
		{
			reply = ":localhost " + replyMap[code].first + " PASS :" + replyMap[code].second + "\r\n";
			send(fd, reply.c_str(), reply.size(), 0);
		}
		else if (code == ERR_PASSWDMISMATCH || code == ERR_NOTREGISTERED)
		{
			reply = ":localhost " + replyMap[code].first + " * :" + replyMap[code].second + "\r\n";
			send(fd, reply.c_str(), reply.size(), 0);
		}
		else
		{
			reply = ":localhost " + replyMap[code].first + " :" + replyMap[code].second + "\r\n";
			send(fd, reply.c_str(), reply.size(), 0);
		}
	}
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
	if (msg.params.size() == 1 && msg.params[0] == "LS")
		// send(fd, ":localhost CAP * LS :\r\n", 23, 0);
		sendReply(fd, RPL_CAP, false);
}

void	Server::execPass(int fd, Message msg)
{
	if (this->clients[fd]->isRegistered) {
		// send(fd, ":localhost 462 :You may not reregister\r\n", 40, 0);
		sendReply(fd, ERR_ALREADYREGISTERED, false);
		return ;
	}

	if (msg.params.empty()) {
		// send(fd, ":localhost 461 PASS :Not enough parameters\r\n", 44, 0);
		sendReply(fd, ERR_NEEDMOREPARAMS, false);
		return ;
	}

	if (msg.params[0] != this->passwd) {
		// send(fd, ":localhost 464 * :Password incorrect\r\n", 38, 0);
		sendReply(fd, ERR_PASSWDMISMATCH, false);
		return ;
	}
	this->clients[fd]->has_pass = true;
}

void	Server::execNick(int fd, Message msg)
{
	if (msg.params.empty()) {
		// sendReply(fd, "431", "No nickname given");
		sendReply(fd, ERR_NONICKNAMEGIVEN, true);
		return ;
	}

	if (!isAlnumStr(msg.params[0])) {
		// sendReply(fd, "432", "Erroneous nickname");
		sendReply(fd, ERR_ERRONEUSNICKNAME, true);
		return ;
	}

	if (!isAvailable(this->clients, msg.params[0])) {
		// sendReply(fd, "433", "Nickname is already in use");
		sendReply(fd, ERR_NICKNAMEINUSE, true);
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
		// sendReply(fd, "462", "You may not reregister");
		sendReply(fd, ERR_ALREADYREGISTERED, true);
		return ;
	}

	if (msg.params.size() < 3 || msg.trailing.empty()){
		// sendReply(fd, "461", "USER :Not enough parameters");
		sendReply(fd, ERR_NEEDMOREPARAMS, true);
		return ;
	}
	this->clients[fd]->userName = msg.params[0];
	this->clients[fd]->realName = msg.trailing;
	this->clients[fd]->has_user = true;

	registerClient(fd);
}

void	Server::registerClient(int fd) {
	if (clients[fd]->has_nick && clients[fd]->has_user && clients[fd]->has_pass)
	{
		clients[fd]->isRegistered = true;
		// sendReply(fd, "001", "Welcome to the IRC server Mortal!");
		sendReply(fd, RPL_WELCOME, false);
	}
}

void	Server::execJoin(int fd, Message msg) { (void)fd, (void)msg; }

void	Server::execPart(int fd, Message msg) { (void)fd, (void)msg; }

void	Server::execTopic(int fd, Message msg) { (void)fd, (void)msg; }

void	Server::execInvite(int fd, Message msg) { (void)fd, (void)msg; }

void	Server::execKick(int fd, Message msg) { (void)fd, (void)msg; }

void	Server::execMode(int fd, Message msg) { (void)fd, (void)msg; }

void	Server::execPrivmsg(int fd, Message msg) { (void)fd, (void)msg; }

