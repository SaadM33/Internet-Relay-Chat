#include "Channel.hpp"
#include "Client.hpp"

// void	Channel::broadcast(Client *client, std::string& message)
// {
// 	std::string							str;
// 	std::map<int,Client *>::iterator	it;

// 	std::string prefix = client->getPrefix();
// 	int i = 0;
// 	std::cout << "will be sent to: " << members.size() << " members!" << std::endl;
// 	for (it = members.begin(); it != members.end(); it++)
// 	{
// 		std::cout << "time: " << i++ << " trying to send to " << it->second->nickName << std::endl;
// 		str = prefix + " " + message;
// 		send(it->first, str.c_str(), str.size(), 0);
// 	}
// }

void Channel::broadcast(Client *client, std::string& message, bool skipSender)
{
	std::string							str;
	std::map<int,Client *>::iterator	it;

	for (it = members.begin(); it != members.end(); it++)
    {
        if (skipSender && it->first == client->fd)
            continue;
        send(it->first, message.c_str(), message.size(), 0);
    }
}

bool	Channel::InInviteList(int fd)
{
	for (size_t i = 0; i < inviteList.size(); i++)
	{
		if (inviteList[i] == fd)
			return true;
	}
	return false;
}	

void	Channel::addClient(Client *client)
{
	if (members.size() == 0)
		operators[client->fd] = client;

	members[client->fd] = client; // add client to channel's members list
	client->channels[name] = this; // add channel to client's channels list wa mii ya rasi derni

	// In addClient (JOIN):
	std::string message = client->getPrefix() + " JOIN " + name + "\r\n";
	broadcast(client, message); // sends to all including joiner
	// std::string message = "JOIN " + name + "\r\n";
	// broadcast(client, message);

	if (!topic.empty())
	{
		message = ":localhost 332 " + client->nickName + " " + name + " :" + topic + "\r\n";
		send(client->fd, message.c_str(), message.size(), 0);
	}

	message = ":localhost 353 " + client->nickName + " = " + name + " :";
	for (std::map<int, Client *>::iterator it = members.begin(); it != members.end(); it++)
	{
		if (operators.find(it->first) != operators.end())
			message += "@" + it->second->nickName + " ";
		else
			message += it->second->nickName + " ";
	}
	message += "\r\n";
	send(client->fd, message.c_str(), message.size(), 0);

	message = ":localhost 366 " + client->nickName + " " + name + " :End of NAMES list\r\n";
	send(client->fd, message.c_str(), message.size(), 0);		
}

void	Channel::removeClient(Client *client)
{
	operators.erase(client->fd);
	members.erase(client->fd);
	
	client->channels.erase(name);
}