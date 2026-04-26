#include "Server.hpp"

void extractCommand(std::stringstream &cmd, Message &msg)
{
	std::string command;

	cmd >> command;
	for (size_t i = 0 ; i < command.size() ; i++)
		command.at(i) = toupper(command.at(i));
	msg.command = command;
}

void extractParams(std::stringstream &prm, Message &msg)
{
	std::string param;

	while (prm >> param)
		msg.params.push_back(param);
}

void extractTrailing(std::string &trail, Message &msg)
{
	if (!trail.empty())
		msg.trailing = trail;
}

Message	splitMessage(std::string unprocessed)
{
	Message	msg;
	std::string trailing;
	size_t	trailIndex = unprocessed.find(" :");
	
	if (trailIndex != std::string::npos)
	{
		trailing = unprocessed.substr(trailIndex + 2);
		unprocessed.erase(trailIndex);
	}
	
	std::stringstream cmdAndParams(unprocessed);

	extractCommand(cmdAndParams, msg);
	extractParams(cmdAndParams, msg);
	extractTrailing(trailing, msg);

	return msg;
}

void	Server::handleInput(int fd)
{
	std::string &input = this->clients[fd]->r_buffer;
	size_t	lineIndex;

	while ((lineIndex = input.find("\r\n")) != std::string::npos)
	{
		std::string unprocessedLine = input.substr(0, lineIndex);

		input.erase(0, lineIndex + 2);

		Message msg = splitMessage(unprocessedLine);
		if (msg.command.empty())
    		continue;

		this->processCmd(fd, msg);
		size_t i;
		for (i = 0; i < poll_fds.size(); i++)
		{
			if (fd == poll_fds[i].fd)
				break ;
		}
		if (i >= poll_fds.size())
			break ;
	}
}
