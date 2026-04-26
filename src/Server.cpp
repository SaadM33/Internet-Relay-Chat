#include "Server.hpp"

Server::Server(int ac, char **av)
{
	if (ac != 3)
		throw std::invalid_argument("Usage: ./ircserv <port> <password>");

	char *endptr;
	int port = std::strtod(av[1], &endptr);
	if (port <= 0 || port > 65535 || *endptr != '\0'){
		throw std::invalid_argument("Error: Invalid port number. Must be between 1 and 65535.");
	}

	std::string	passwd = av[2];
	if (passwd.empty())
		throw std::invalid_argument("Error: Password cannot be empty.");

	this->port = port;
	this->passwd = passwd;
	this->instantiateCmds();
	this->instantiateReplies();
}

void	Server::instantiateReplies()
{
	this->replyMap[RPL_WELCOME] = "Welcome to the IRC server Mortal!";
	this->replyMap[ERR_NOSUCHNICK] = "No such nick/channel";
	this->replyMap[ERR_UNKNOWNCOMMAND] =  "Command beyond comprehension";  
	this->replyMap[ERR_NONICKNAMEGIVEN] =  "No nickname given";
	this->replyMap[ERR_ERRONEUSNICKNAME] =  "Erroneous nickname";
	this->replyMap[ERR_NICKNAMEINUSE] =  "Nickname is already in use";
	this->replyMap[ERR_NOTREGISTERED] = "You have not registered";
	this->replyMap[ERR_NEEDMOREPARAMS] =  "Not enough parameters";
	this->replyMap[ERR_ALREADYREGISTERED] =  "You may not reregister";
	this->replyMap[ERR_PASSWDMISMATCH] =  "Password incorrect";
	this->replyMap[ERR_CHANNELISFULL] =  "Channel is full";
	this->replyMap[ERR_INVITEONLYCHAN] =  "Cannot join channel (+i)";
	this->replyMap[ERR_BADCHANNELKEY] =  "Cannot join channel (+k)";
	this->replyMap[ERR_CHANOPRIVSNEEDED] =  "You're not channel operator";
	this->replyMap[ERR_NOTEXTTOSEND] = "No text to send";
	this->replyMap[ERR_NORECIPIENT] = "No recipient given (PRIVMSG)";
	this->replyMap[ERR_NOSUCHCHANNEL] = "No such channel";
	this->replyMap[ERR_NOORIGIN] = "No origin specified";
	this->replyMap[RPL_NOTOPIC] = "No topic is set";
	this->replyMap[ERR_NOTONCHANNEL] = "You're not on that channel";
	this->replyMap[ERR_CANNOTSENDTOCHAN] = "Cannot send to channel";
	this->replyMap[ERR_USERNOTINCHANNEL] = "They're not on that channel";
	this->replyMap[ERR_USERONCHANNEL] = "is already on channel";
}

void	Server::instantiateCmds()
{
	this->cmdMap["CAP"] = &Server::execCap;
	this->cmdMap["PASS"] = &Server::execPass;
	this->cmdMap["NICK"] = &Server::execNick;
	this->cmdMap["USER"] = &Server::execUser;
	this->cmdMap["JOIN"] = &Server::execJoin;
	this->cmdMap["PART"] = &Server::execPart;
	this->cmdMap["TOPIC"] = &Server::execTopic;
	this->cmdMap["INVITE"] = &Server::execInvite;
	this->cmdMap["KICK"] = &Server::execKick;
	this->cmdMap["MODE"] = &Server::execMode;
	this->cmdMap["PRIVMSG"] = &Server::execPrivmsg;
	this->cmdMap["PING"] = &Server::execPing;
}

void	Server::ascend()
{
	this->core_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->core_fd == -1) {
		throw std::runtime_error("Error: Failed to create socket.");
	}

	int reuse = 1;
	if (setsockopt(this->core_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
		throw std::runtime_error("Error: Failed to set socket option.");
	}
	
	sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(this->port);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(this->core_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		throw std::runtime_error("Error: Failed to bind socket.");
	}

	if (listen(this->core_fd, 20) == -1) {
		throw std::runtime_error("Error: Failed to listen on socket.");
	}

	std::cout << "Server is listening on port " << this->port << std::endl;
}

void	Server::ignite()
{
	this->poll_fds.push_back((struct pollfd){core_fd, POLLIN, 0});

	// ctrl c later
	while (true)
	{
		if (poll(poll_fds.data(), poll_fds.size(), -1) == -1) {
			throw std::runtime_error("Error: Failed to poll sockets.");
		}
		for (size_t i = 0; i < poll_fds.size(); i++)
		{
			c_banished = false;
			if (poll_fds[i].revents & POLLIN)  
			{
				if (poll_fds[i].fd == core_fd)
					this->acceptClient();
				else
					this->processClient(i);
			}
			else if (poll_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) { 
				this->disconnectClient(i, poll_fds[i].fd);
				std::cout << "Error or hangup detected on client socket, disconnecting client." << std::endl;
			}

			if (c_banished)
				i--;
		}
	}
}

void	Server::acceptClient()
{
	sockaddr_in cAddr;
	socklen_t cLen = sizeof(cAddr);

	int new_fd = accept(core_fd, (struct sockaddr*)&cAddr, &cLen);
	if (new_fd < 0)
		throw std::logic_error("Couldnt accept client");

	fcntl(new_fd, F_SETFL, O_NONBLOCK);

	this->clients[new_fd] = new Client(new_fd, cAddr);

	this->poll_fds.push_back((pollfd){new_fd, POLLIN, 0});

	std::cout << "New client connected with fd: " << new_fd << std::endl;
}

void	Server::processClient(int i)
{
	char 	tmp[1024] = {0};
	int		fd = poll_fds[i].fd;

	int ret = recv(fd, tmp, sizeof(tmp), 0);
	if (ret <= 0)
	{
		disconnectClient(i, fd);
		return ;
	}

	this->clients[fd]->r_buffer.append(tmp, ret);	
	this->handleInput(fd);
}

void	Server::disconnectClient(int i, int fd)
{
	// removing a client from a channel, it should iterate over this->channels and not this->clients[fd]->channels
	for (std::map<std::string, Channel *>::iterator it = this->clients[fd]->channels.begin(); it != this->clients[fd]->channels.end(); it++) 
		it->second->removeClient(this->clients[fd]);

	close(poll_fds[i].fd);
	this->poll_fds.erase(poll_fds.begin() + i);
	delete this->clients[fd];
	this->clients.erase(fd);
	c_banished = true;

	std::cout << "Client " << fd << " disconnected" << std::endl;
}

void	Server::sendReply(int fd, std::string code, std::string middle)
{
	std::string reply;

	reply = ":localhost " + code + " " + this->clients[fd]->nickName;
	
	if (!middle.empty())
		reply += " " + middle;

	reply += " :" + replyMap[code] + "\r\n";

	send(fd, reply.c_str(), reply.size(), 0);	
}