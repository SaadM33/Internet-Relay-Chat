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
		sendReply(fd, ERR_NICKNAMEINUSE, msg.params[0]);
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
		std::string brdcst = "NICK :" + msg.params[0] + "\r\n";

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
		sendReply(fd, ERR_NEEDMOREPARAMS, msg.command);
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
			sendReply(fd, ERR_NOSUCHCHANNEL, name);
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
				sendReply(fd, ERR_INVITEONLYCHAN, name);
				continue ;
			}
			if (channels[name]->isKeySet && (i >= keys.size() || keys[i].empty() || keys[i] != channels[name]->key))
			{
				sendReply(fd, ERR_BADCHANNELKEY, name);
				continue ;
			}
			if (channels[name]->members.size() == (size_t)channels[name]->membersLimit)
			{
				sendReply(fd, ERR_CHANNELISFULL, name);
				continue ;
			}
			channels[name]->addClient(clients[fd]);
			// the user needs to be removed from the invitelist once joined to the channel
		}
	}

	
}

void	cleanUpModes(std::string& modes)
{
	bool	modeSwitch = false;

	for (size_t i = 0; i < modes.size(); i++) {
		if ((modeSwitch == true && modes[i] == '+') || (modeSwitch == false && modes[i] == '-'))
			modes.erase(i, 1);
		else if (modeSwitch == true && modes[i] == '-')
			modeSwitch = false;
		else if (modeSwitch == false && modes[i] == '+')
			modeSwitch = true;
	}
}

void	Server::execMode(int fd, Message msg)
{
	if (msg.params.size() < 1) {
		sendReply(fd, ERR_NEEDMOREPARAMS, msg.command);
		return ;
	}

	std::string		target = msg.params[0];
	std::string		senderNick = this->clients[fd]->nickName;

	if (target[0] != '#')
	{
		if (target != senderNick) {
			sendReply(fd, ERR_USERSDONTMATCH);
			return ;
		}
		if (msg.params.size() == 1) {
			sendReply(fd, RPL_UMODEIS);
			return ;
		}
		std::string reply = ":" + senderNick + " MODE " + senderNick + " :" + msg.params[1] + "\r\n";
		send(fd, reply.c_str(), reply.size(), 0);

		return ;
	}

	if (channels.find(target) == channels.end()) {
		sendReply(fd, ERR_NOSUCHCHANNEL, target);
		return ;
	}

	if (channels[target]->members.find(fd) == channels[target]->members.end()) {
		sendReply(fd, ERR_NOTONCHANNEL, target);
		return ;
	}

	if (channels[target]->operators.find(fd) == channels[target]->operators.end()) {
		sendReply(fd, ERR_CHANOPRIVSNEEDED, target);
		return ;
	}

	if (msg.params.size() == 1) {
		std::string reply;

		if (channels[target]->isInviteOnly)
			reply += "i";
		if (channels[target]->topicRestricted)
			reply += "t";
		if (channels[target]->isKeySet)
			reply += "k";
		if (channels[target]->membersLimit != -1)
			reply += "l";
		
		if (channels[target]->isKeySet)
			reply += " " + channels[target]->key;

		if (channels[target]->membersLimit != -1)
		{
			std::ostringstream oss;
			oss << channels[target]->membersLimit;
			reply += " " + oss.str();
		}

		if (!reply.empty())
			reply = "+" + reply;

		reply = target + reply;

		sendReply(fd, RPL_CHANNELMODEIS, reply);
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
				if (modeSwitch == false || brdcst.modes.empty())
					modeSwitch = true;
				break ;
	
			case '-':
				if (modeSwitch == true || brdcst.modes.empty())
					modeSwitch = false;
				break ;
	
			case 'i':
				replyCode = channels[target]->modeI(modeSwitch, brdcst);
				break ;

			case 'k':
				replyCode = channels[target]->modeK(modeSwitch, arg, brdcst);
				paramIndex++;
				break ;

			case 'l':
				replyCode = channels[target]->modeL(modeSwitch, arg, paramIndex, brdcst);
				break ;

			case 'o':
				replyCode = channels[target]->modeO(modeSwitch, arg, brdcst);
				paramIndex++;
				break ;

			case 't':
				replyCode = channels[target]->modeT(modeSwitch, brdcst);
				break ;

			default :
				brdcst.unknown += modeStr[i];
		}

		if (!replyCode.empty())
			sendReply(fd, replyCode);
	}

	if (!brdcst.modes.empty()) {
		cleanUpModes(brdcst.modes);
		std::string finalMsg = "MODE " + target + " " + brdcst.modes + brdcst.args + "\r\n";
		channels[target]->broadcast(clients[fd], finalMsg);
	}
	if (!brdcst.unknown.empty()) {
		std::string reply = ":" + senderNick + " MODE " + target + " '" + brdcst.unknown + "' :Unknown mode characters !\r\n";
		send(fd, reply.c_str(), reply.size(), 0);
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
			sendReply(fd, ERR_NOSUCHCHANNEL, item);
			continue ;
		}
		if (clients[fd]->channels.find(item) == clients[fd]->channels.end()) {
			sendReply(fd, ERR_NOTONCHANNEL, item);
			continue ;
		}

		std::string partMsg = "PART " + item;
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
		sendReply(fd, ERR_NEEDMOREPARAMS, msg.command);
		return;
	}
	std::string	name_ch = msg.params[0];

	if (channels.find(name_ch) == channels.end()) {
		sendReply(fd, ERR_NOSUCHCHANNEL, name_ch);
		return ;
	}
	if (clients[fd]->channels.find(name_ch) == clients[fd]->channels.end()) {
		sendReply(fd, ERR_NOTONCHANNEL, name_ch);
		return ;
	}

	Channel	*chan = channels[name_ch];

	if (msg.trailing.empty())
	{
		if (chan->topic.empty())
			sendReply(fd, RPL_NOTOPIC, name_ch);
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
			sendReply(fd, ERR_CHANOPRIVSNEEDED, name_ch);
		else
		{
			chan->topic = msg.trailing;
			std::string	reply = "TOPIC " + name_ch + " :" + msg.trailing + "\r\n";
			chan->broadcast(clients[fd], reply);
		}
	}
}
void    Server::execInvite(int fd, Message msg)
{
	// Check if there are NOT two params
	if (msg.params.empty() || msg.params.size() < 2)
	{
		sendReply(fd, ERR_NEEDMOREPARAMS, msg.command);
		return ;
	}

	// Check if the target nick does NOT exist
	bool found = false;
	int	targetFd;
	for (std::map<int, Client *>::iterator it = this->clients.begin(); it != this->clients.end(); it++)
	{
		if ((*it).second->nickName == msg.params[0])
		{
			found = true;
			targetFd = (*it).second->fd;
			break ;
		}
	}
	if (!found)
	{
		sendReply(fd, ERR_NOSUCHNICK, msg.params[0]);
		return ;
	}

	// Check if the target channel does NOT exist
	if (this->channels.find(msg.params[1]) == this->channels.end())
	{
		sendReply(fd, ERR_NOSUCHCHANNEL, msg.params[1]);
		return ;
	}

	// Check if the invoker is NOT a channel member
	if (this->channels[msg.params[1]]->members.find(fd) == this->channels[msg.params[1]]->members.end())
	{
		sendReply(fd, ERR_NOTONCHANNEL, msg.params[1]);
		return ;
	}

	// Check if the target channel is on invite-only mode (+i)
	if (this->channels[msg.params[1]]->isInviteOnly)
	{
		// Check if the invoker is NOT an operator
		if (this->channels[msg.params[1]]->operators.find(fd) == this->channels[msg.params[1]]->operators.end())
		{
			sendReply(fd, ERR_CHANOPRIVSNEEDED, msg.params[1]);
			return ;
		}
	}

	// Check if the target user is already in the target channel
	if (this->channels[msg.params[1]]->members.find(targetFd) != this->channels[msg.params[1]]->members.end())
	{
		sendReply(fd, ERR_USERONCHANNEL, msg.params[0] + " " + msg.params[1]);
		return ;
	}

	// On success
		// Send RPL_INVITING to the invoker
	std::string reply = ":localhost 341 " + this->clients[fd]->nickName + " " + msg.params[0] + " " + this->channels[msg.params[1]]->name + "\r\n";
	send(fd, reply.c_str(), reply.size(), 0);
		// Send INVITE message to the target user
		// :<invokerNick>!<invokerUser>@<invokerHost> INVITE <targetNick> <channel>
	reply = this->clients[fd]->getPrefix() + " " + msg.command + " " + msg.params[0] + " " + this->channels[msg.params[1]]->name + "\r\n";
	send(targetFd, reply.c_str(), reply.size(), 0);
		// Add the target user to the inviteList
	this->channels[msg.params[1]]->inviteList.push_back(targetFd);
}

void    Server::execKick(int fd, Message msg)
{
	// Check if there are NOT two params
	if (msg.params.empty() || msg.params.size() < 2)
	{
		sendReply(fd, ERR_NEEDMOREPARAMS, msg.command);
		return ;
	}
	// Check if the target channel does NOT exist
	if (this->channels.find(msg.params[0]) == this->channels.end())
	{
		sendReply(fd, ERR_NOSUCHCHANNEL, msg.params[0]);
		return ;
	}

	// Check if the invoker is NOT a channel member
	if (this->channels[msg.params[0]]->members.find(fd) == this->channels[msg.params[0]]->members.end())
	{
		sendReply(fd, ERR_NOTONCHANNEL, msg.params[0]);
		return ;
	}

	// Check if the invoker is NOT an operator
	if (this->channels[msg.params[0]]->operators.find(fd) == this->channels[msg.params[0]]->operators.end())
	{
		sendReply(fd, ERR_CHANOPRIVSNEEDED, msg.params[0]);
		return ;
	}
	
	// Determine comment (default to invoker nickname if not provided)
	std::string comment;
	if (msg.trailing.empty())
		comment = this->clients[fd]->nickName;
	else
		comment = msg.trailing;

	// Loop over comma-separated users in msg.params[1]
	std::stringstream ss(msg.params[1]);
	std::string targetNick;
	while (std::getline(ss, targetNick, ','))
	{
		// Check if the target nick does NOT exist
		bool found = false;
		int	targetFd;
		for (std::map<int, Client *>::iterator it = this->clients.begin(); it != this->clients.end(); it++)
		{
			if ((*it).second->nickName == targetNick)
			{
				found = true;
				targetFd = (*it).second->fd;
				break ;
			}
		}
		if (!found)
		{
			sendReply(fd, ERR_NOSUCHNICK, targetNick);
			continue ;
		}

		// Check if the target user is NOT in the channel
		if (this->channels[msg.params[0]]->members.find(targetFd) == this->channels[msg.params[0]]->members.end())
		{
			sendReply(fd, ERR_USERNOTINCHANNEL, targetNick + " " + msg.params[0]); // sends: :localhost 441 <invokerNick> <targetNick> <channel> :They aren't on that channel\r\n
			continue ;
		}

		// Send KICK message to all channel members
		std::string brdcst = "KICK " + msg.params[0] + " " + targetNick + " :" + comment + "\r\n";
		this->channels[msg.params[0]]->broadcast(this->clients[fd], brdcst);
		// Remove target from channel members
		this->channels[msg.params[0]]->removeClient(this->clients[targetFd]);
	}
}

void	Server::execPrivmsg(int fd, Message msg)
{
	if (msg.params.empty())
	{
		 sendReply(fd, ERR_NORECIPIENT, msg.command);
		return ;
	}
	else if (msg.trailing.empty())
	{
		sendReply(fd, ERR_NOTEXTTOSEND, msg.command);
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
				this->sendReply(fd, ERR_NOSUCHNICK, channelName);
				continue;
			}
			if (this->channels[channelName]->members.find(fd) == this->channels[channelName]->members.end())
			{
				std::string reply = ":localhost 404 " + clients[fd]->nickName + " " + channelName + " :Cannot send to channel\r\n";
				send(fd, reply.c_str(), reply.size(), 0);
				continue ;
			}
			std::string message = "PRIVMSG " + params + " :" + msg.trailing + "\r\n";
			this->channels[channelName]->broadcast(this->clients[fd], message, true);
		}
		else if (params[0] == '@') // handle operators left!
			//might change mind
			;
		else // handle users
		{
			bool found = false;
			for (std::map<int, Client *>::iterator it = this->clients.begin(); it != this->clients.end(); it++)
			{
				if ((*it).second->nickName == params)
				{
					std::string reply = this->clients[fd]->getPrefix() + " PRIVMSG "
					+ params + " :" + msg.trailing + "\r\n";
					send((*it).second->fd, reply.c_str(), reply.size(), 0);
					found = true;
					break ;
				}
			}
			if (!found)
				this->sendReply(fd, ERR_NOSUCHNICK, params);
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
