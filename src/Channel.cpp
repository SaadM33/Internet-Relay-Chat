#include "Channel.hpp"
#include "Server.hpp"

void	Channel::broadcast(Client *client, std::string& message, bool skipSender)
{
	std::string							str;
	std::map<int,Client *>::iterator	it;

	std::string prefix = client->getPrefix();
	for (it = members.begin(); it != members.end(); it++)
	{
		if (skipSender && it->first == client->fd)
            continue;
		str = prefix + " " + message;
		send(it->first, str.c_str(), str.size(), 0);
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

	std::string message = "JOIN " + name + "\r\n";
	broadcast(client, message);

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

int	getFdFromNick(const std::map<int, Client *> &clients, std::string str);

std::string	Channel::modeI(bool modeSwitch, modeBroadcast& brdcst)
{
	if (modeSwitch && !this->isInviteOnly) {
		this->isInviteOnly = true;
		brdcst.modes += "i";
	}
	else if (!modeSwitch && this->isInviteOnly) {
		this->isInviteOnly = false;
		brdcst.modes += ("i");
	}

	return "";
}

std::string	Channel::modeK(bool modeSwitch, std::string newKey, modeBroadcast& brdcst)
{
	if (newKey.empty())
		return "";

	if (modeSwitch) {
		this->key = newKey;
		if (!this->isKeySet) {
			this->isKeySet = true;
			brdcst.modes += "k";
			brdcst.args += " " + newKey;
		}
	}
	else {
		this->key = "";
		if (this->isKeySet) {
			this->isKeySet = false;
			brdcst.modes += "k";
			brdcst.args += " *";
		}
	}

	return "";
}

bool	isPositiveInt(const std::string &str)
{
	size_t i = 0;

	if (str.empty())
		return false;

	if (str[i] == '+')
		i++;

	while (i < str.size()){
		if (!isdigit(str[i]))
			return false;
		i++;
	}

	return true;
}

std::string	Channel::modeL(bool modeSwitch, std::string newLim, size_t &paramIndex, modeBroadcast& brdcst)
{
	if (modeSwitch) {
		if (isPositiveInt(newLim)) {
			std::stringstream ss(newLim);
			ss >> this->membersLimit;
			brdcst.modes += "l";
			brdcst.args += " " + newLim;
		}
		paramIndex++;
	}
	else {
		if (this->membersLimit != -1) {
			this->membersLimit = -1;
			brdcst.modes += "l";
		}
	}
	
	return "";
}

std::string	Channel::modeO(bool modeSwitch, std::string targetNick, modeBroadcast& brdcst)
{
	if (targetNick.empty())
		return "";
	
	int targetFd = getFdFromNick(this->members, targetNick);
	if (members.find(targetFd) == members.end())
		return ERR_USERNOTINCHANNEL;

	if (modeSwitch) {
		if (operators.find(targetFd) != operators.end())
			return "";
		operators[targetFd] = members[targetFd];
		brdcst.modes += "o";
		brdcst.args += " " + targetNick;
	}
	else {
		if (operators.find(targetFd) == operators.end())
			return "";
		operators.erase(targetFd);
		brdcst.modes += "o";
		brdcst.args += " " + targetNick;
	}
	return "";
}

std::string	Channel::modeT(bool modeSwitch, modeBroadcast& brdcst)
{
	if (modeSwitch && !topicRestricted) {
		topicRestricted = true;
		brdcst.modes += "t";
	}
	else if (!modeSwitch && topicRestricted) {
		topicRestricted = false;
		brdcst.modes += "t";
	}

	return "";
}
