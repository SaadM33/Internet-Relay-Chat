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

void extractTraling(std::string &trail, Message &msg)
{
	msg.trailing = trail;
}

Message	splitMessage(std::string unprocessed)
{
	Message	msg;
	size_t	trailIndex = unprocessed.find(" :");
	if (trailIndex != std::string::npos)
	{
		std::string param = unprocessed.substr(0, trailIndex);
		std::stringstream cmdAndParams(param);
		// handle the command and parameters as a stream to automatically skip spaces
		extractCommand(cmdAndParams, msg);
		extractParams(cmdAndParams, msg);
		// handle the trailing as a string to conserve the spaces
		std::string trailing = unprocessed.substr(trailIndex + 2); // +2 to skip the : character
		extractTraling(trailing, msg);
	}
	else
	{
		std::stringstream cmdAndParams(unprocessed);
		extractCommand(cmdAndParams, msg);
		extractParams(cmdAndParams, msg);
	}
	return msg;
}


void	Server::handleInput(int fd)
{
	std::string &input = this->clients[fd]->r_buffer;
	size_t	lineIndex;

	while ((lineIndex = input.find("\r\n")) != std::string::npos)
	{
		std::cout << "-----Buffer before processing: " << input << "-----" << std::endl;
		std::string unprocessedLine = input.substr(0, lineIndex);

		input.erase(0, lineIndex + 2);
		

		Message msg = splitMessage(unprocessedLine);
		
		std::cout << "Received command: " << msg.command << " with parameters: ";
		for (size_t i = 0; i < msg.params.size(); ++i)
			std::cout << msg.params[i] << " ";
		std::cout << "trailing: " << msg.trailing << std::endl;

		this->processCmd(fd, msg);
		std::cout << "-----Buffer after processing: " << input << "-----" << std::endl;
	}
}