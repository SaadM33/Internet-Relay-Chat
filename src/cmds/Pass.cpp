#include "Server.hpp"

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
		for (size_t i = 0; i < poll_fds.size(); i++)
		{
			if (fd == poll_fds[i].fd)
			{
				this->disconnectClient(i, fd);
				break ;
			}
		}
		return ;
	}
	this->clients[fd]->has_pass = true;
}
