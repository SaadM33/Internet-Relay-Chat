#include "Server.hpp"

int	getFdFromNick(const std::map<int, Client *> &clients, std::string str)
{
	for (std::map<int, Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		if (it->second->nickName == str)
			return it->first;
	}
	return -1;
}

void	Server::execCap(int fd, Message msg)
{
	if (msg.params.size() >= 1 && msg.params[0] == "LS")
		send(fd, ":localhost CAP * LS :\r\n", 23, 0);
}

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
		return ;
	}
	this->clients[fd]->has_pass = true;
}

void	Server::execNick(int fd, Message msg)
{
	if (msg.params.empty()) {
		sendReply(fd, ERR_NONICKNAMEGIVEN);
		return ;
	}

	if (!isAlnumStr(msg.params[0])) {
		sendReply(fd, ERR_ERRONEUSNICKNAME);
		return ;
	}

	if (getFdFromNick(this->clients, msg.params[0]) != -1) {
		sendReply(fd, ERR_NICKNAMEINUSE);
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
		// std::string brdcst = "NICK :" + msg.params[0] + "\r\n";
		std::string brdcst = clients[fd]->getPrefix() + " NICK :" + msg.params[0] + "\r\n";
		std::map<std::string, Channel *>::iterator it;
		for (it = clients[fd]->channels.begin(); it != clients[fd]->channels.end(); it++)
			it->second->broadcast(clients[fd], brdcst);
		
		clients[fd]->nickName = msg.params[0];
	}
}

void	Server::execUser(int fd, Message msg)
{
	if (this->clients[fd]->isRegistered) {
		sendReply(fd, ERR_ALREADYREGISTERED);
		return ;
	}

	if (msg.params.size() < 3 || msg.trailing.empty()){
		sendReply(fd, ERR_NEEDMOREPARAMS);
		return ;
	}
	this->clients[fd]->userName = msg.params[0];
	this->clients[fd]->realName = msg.trailing;
	this->clients[fd]->has_user = true;

	registerClient(fd);
}

void	Server::registerClient(int fd)
{
	if (clients[fd]->has_nick && clients[fd]->has_user && clients[fd]->has_pass)
	{
		clients[fd]->isRegistered = true;
		sendReply(fd, RPL_WELCOME);
	}
}

void	CommasBegone(std::vector<std::string> &chs, std::vector<std::string> &keys, std::vector<std::string> &params)
{
	std::stringstream ss(params[0]);
	std::string item;

	while (std::getline(ss, item, ','))
		chs.push_back(item);
	
	if (params.size() > 1)
	{
		std::stringstream ss2(params[1]);
		while (std::getline(ss2, item, ',')) // if we have "key1,key2" and 3 channels, we will have "key1,key2,key2" 
			keys.push_back(item);
	}
}

void	Server::execJoin(int fd, Message msg)
{
	if (msg.params.size() < 1)
	{
		sendReply(fd, ERR_NEEDMOREPARAMS);
		return ;
	}

	std::vector<std::string>	ch_names;
	std::vector<std::string>	keys;
	CommasBegone(ch_names, keys, msg.params);

	for (size_t i = 0; i < ch_names.size(); i++)
	{
		std::string name = ch_names[i];

		if (name.empty() || name[0] != '#')
		{
			sendReply(fd, ERR_NOSUCHCHANNEL);
			continue ;
		}
		if (channels.find(name) == channels.end())
		{
			Channel *new_Ch = new Channel(name);
			channels[name] = new_Ch;

			new_Ch->addClient(clients[fd]);
		}
		else
		{
			if (channels[name]->isInviteOnly && !channels[name]->InInviteList(fd))
			{
				sendReply(fd, ERR_INVITEONLYCHAN);
				continue ;
			}
			if (channels[name]->isKeySet && (i >= keys.size() || keys[i].empty() || keys[i] != channels[name]->key))
			{
				sendReply(fd, ERR_BADCHANNELKEY);
				continue ;
			}
			if (channels[name]->members.size() == (size_t)channels[name]->membersLimit)
			{
				sendReply(fd, ERR_CHANNELISFULL);
				continue ;
			}
			channels[name]->addClient(clients[fd]);
		}
	}

	
}

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

void	Server::execMode(int fd, Message msg)
{
	if (msg.params.size() < 1) {
		sendReply(fd, ERR_NEEDMOREPARAMS);
		return ;
	}

	std::string		name = msg.params[0];

	if (channels.find(name) == channels.end()) {
		sendReply(fd, ERR_NOSUCHCHANNEL);
		return ;
	}

	if (channels[name]->members.find(fd) == channels[name]->members.end()) {
		sendReply(fd, ERR_NOTONCHANNEL); 	
		return ;
	}

	if (channels[name]->operators.find(fd) == channels[name]->operators.end()) {
		sendReply(fd, ERR_CHANOPRIVSNEEDED);
		return ;
	}

	if (msg.params.size() == 1) {
		sendReply(fd, RPL_CHANNELMODEIS);
		return ;
	}

	if (msg.params[1].empty() || (msg.params[1][0] != '+' && msg.params[1][0] != '-'))
		return ;

	std::string		modeStr = msg.params[1];
	modeBroadcast	brdcst;
	bool			modeSwitch = true;
	size_t			paramIndex = 2;

	for (size_t i = 0; i < modeStr.size(); i++)
	{
		std::string	replyCode = "";
		std::string arg = "";
		if (paramIndex < msg.params.size())
			arg = msg.params[paramIndex];
		
		switch (modeStr[i]) {
			case '+':
				if (modeSwitch == false || brdcst.modes.empty()) {
					modeSwitch = true;
					brdcst.modes += "+";
				}
				break ;
	
			case '-':
				if (modeSwitch == true || brdcst.modes.empty()) {
					modeSwitch = false;
					brdcst.modes += "-";
				}
				break ;
	
			case 'i':
				replyCode = channels[name]->modeI(modeSwitch, brdcst);
				break ;

			case 'k':
				replyCode = channels[name]->modeK(modeSwitch, arg, brdcst);
				paramIndex++;
				break ;

			case 'l':
				replyCode = channels[name]->modeL(modeSwitch, arg, paramIndex, brdcst);
				break ;

			case 'o':
				replyCode = channels[name]->modeO(modeSwitch, arg, brdcst);
				paramIndex++;
				break ;

			case 't':
				replyCode = channels[name]->modeT(modeSwitch, brdcst);
				break ;

			default :
				brdcst.unknown += modeStr[i];
		}

		if (!replyCode.empty())
			sendReply(fd, replyCode);
	}
}

void	Server::execPart(int fd, Message msg)
{
	if (msg.params.size() < 1) {
		sendReply(fd, ERR_NEEDMOREPARAMS);
		return;
	}

	std::stringstream ss(msg.params[0]);
	std::string item;

	while (std::getline(ss, item, ','))
	{
		if (item.empty())
			continue ;
		if (channels.find(item) == channels.end()) {
			sendReply(fd, ERR_NOSUCHCHANNEL);
			continue ;
		}
		if (clients[fd]->channels.find(item) == clients[fd]->channels.end()) {
			sendReply(fd, ERR_NOTONCHANNEL);
			continue ;
		}

		std::string partMsg = clients[fd]->getPrefix() + " PART " + item;		
		// std::string partMsg = "PART " + item;
		if (!msg.trailing.empty())
			partMsg += " :" + msg.trailing;
		partMsg += "\r\n";
		channels[item]->broadcast(clients[fd], partMsg);
		
		channels[item]->removeClient(clients[fd]);

		if (channels[item]->members.empty()) {
			delete channels[item];
			channels.erase(item);
		}
	}
}

void	Server::execTopic(int fd, Message msg)
{
	if (msg.params.size() < 1) {
		sendReply(fd, ERR_NEEDMOREPARAMS);
		return;
	}
	std::string	name_ch = msg.params[0];

	if (channels.find(name_ch) == channels.end()) {
		sendReply(fd, ERR_NOSUCHCHANNEL);
		return ;
	}
	if (clients[fd]->channels.find(name_ch) == clients[fd]->channels.end()) {
		sendReply(fd, ERR_NOTONCHANNEL);
		return ;
	}

	Channel	*chan = channels[name_ch];

	if (msg.trailing.empty())
	{
		if (chan->topic.empty())
			sendReply(fd, RPL_NOTOPIC);
		else
		{
			std::string reply = ":localhost 332 " + clients[fd]->nickName + " " + name_ch;
			reply += " :" + chan->topic + "\r\n";
			send(fd, reply.c_str(), reply.size(), 0);
		}
	}
	else
	{
		if (chan->topicRestricted && chan->operators.find(fd) == chan->operators.end())
			sendReply(fd, ERR_CHANOPRIVSNEEDED);
		else
		{
			chan->topic = msg.trailing;
			std::string reply = clients[fd]->getPrefix() + " TOPIC " + name_ch + " :" + msg.trailing + "\r\n";
			// std::string	reply = "TOPIC " + name_ch + " :" + msg.trailing + "\r\n";
			chan->broadcast(clients[fd], reply);
		}
	}
}
void	Server::execInvite(int fd, Message msg) { (void)fd, (void)msg; }

void	Server::execKick(int fd, Message msg) { (void)fd, (void)msg; }

void	Server::execPrivmsg(int fd, Message msg)
{
	if (msg.params.empty())
	{
		this->sendReply(fd, ERR_NORECIPIENT);
		return ;
	}
	else if (msg.trailing.empty())
	{
		sendReply(fd, ERR_NOTEXTTOSEND);
		return ;
	}

	std::stringstream ss(msg.params[0]);
	std::string params;

	while (std::getline(ss, params, ','))
	{
		if (params.empty())
			continue ;
		if (params[0] == '#') // handle channels 
		{
			std::string channelName(params);
			if (this->channels.find(channelName) == this->channels.end())
			{
				this->sendReply(fd, ERR_NOSUCHCHANNEL);
				continue;
			}
			if (this->channels[channelName]->members.find(fd) == this->channels[channelName]->members.end())
			{
				std::string reply = ":localhost 404 " + clients[fd]->nickName + " " + channelName + " :Cannot send to channel\r\n";
				send(fd, reply.c_str(), reply.size(), 0);
				continue ;
			}
			std::string message = this->clients[fd]->getPrefix() + " PRIVMSG " + params + " :" + msg.trailing + "\r\n";
			this->channels[channelName]->broadcast(this->clients[fd], message, true);
		}
		else if (params[0] == '@') // handle operators left!
		{
			std::string channelName(params, 1);
			if (this->channels.find(channelName) == this->channels.end())
			{
				this->sendReply(fd, ERR_NOSUCHCHANNEL);
				continue;
			}
			if (this->channels[channelName]->members.find(fd) == this->channels[channelName]->members.end())
			{
				this->sendReply(fd, ERR_CANNOTSENDTOCHAN);
				continue ;
			}
			this->channels[channelName]->broadcast(this->clients[fd], msg.trailing);
		}
		else // handle users
		{
			bool found = false;
			for (std::map<int, Client *>::iterator it = this->clients.begin(); it != this->clients.end(); it++)
			{
				if ((*it).second->nickName == params)
				{
					std::cout << "oops" << std::endl;
					std::string reply = this->clients[fd]->getPrefix() + " PRIVMSG "
					+ params + " :" + msg.trailing + "\r\n";
					send((*it).second->fd, reply.c_str(), reply.size(), 0);
					found = true;
					break ;
				}
			}
			if (!found)
				this->sendReply(fd, ERR_NOSUCHNICK);
		}
	}
}

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
