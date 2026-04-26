#include "Server.hpp"

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
			reply = " +" + reply;

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
