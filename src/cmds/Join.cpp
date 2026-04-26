#include "Server.hpp"

void	CommasBegone(std::vector<std::string> &chs, std::vector<std::string> &keys, std::vector<std::string> &params)
{
	std::stringstream ss(params[0]);
	std::string item;

	while (std::getline(ss, item, ','))
		chs.push_back(item);
	
	if (params.size() > 1)
	{
		std::stringstream ss2(params[1]);
		while (std::getline(ss2, item, ','))
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
		}
		else
		{
			if (channels[name]->isInviteOnly && channels[name]->findInviteList(fd) == channels[name]->inviteList.end())
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
		}

		channels[name]->addClient(clients[fd]);

		if (channels[name]->findInviteList(fd) != channels[name]->inviteList.end())
			channels[name]->inviteList.erase(channels[name]->findInviteList(fd));
	}
}
