#include "Server.hpp"

void	Server::execQuit(int fd, Message msg)
{
	std::map<std::string, Channel *>::iterator it;
	for (it = clients[fd]->channels.begin(); it != clients[fd]->channels.end(); it++)
	{
		std::string message = "QUIT :";
		if (!msg.trailing.empty())
			message += msg.trailing;
		message += "\r\n";
		it->second->broadcast(clients[fd], message, true);
	}
	std::string reply = "ERROR :Closing connection\r\n";
	send(fd, reply.c_str(), reply.size(), 0);
	for (size_t i = 0; i < poll_fds.size(); i++)
	{
		if (fd == poll_fds[i].fd)
		{
			this->disconnectClient(i, fd);
			break ;
		}
	}
}
