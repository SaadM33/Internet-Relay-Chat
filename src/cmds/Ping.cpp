#include "Server.hpp"

void	Server::execPing(int fd, Message msg)
{
	if (msg.params.empty())
	{
		sendReply(fd, ERR_NOORIGIN);
		return ;
	}

	std::string reply = "PONG :" + msg.params[0] + "\r\n";
	send(fd, reply.c_str(), reply.size(), 0);
}
