#include "Channel.hpp"

void Channel::broadcast(int fd, std::string& message, int forsake_fd = 0)
{
	std::string							str;
	std::map<int,Client *>::iterator	it;

	for (it = members.begin(); it != members.end(); it++)
	{
		if (forsake_fd && it->first == fd)
			continue;
		
		str = it->second->getPrefix() + message;
		send(it->first, message.c_str(), message.size(), 0);
	}
}