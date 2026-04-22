#pragma once

#include <iostream>
#include <vector>
#include <map>

class  Client;

typedef struct s_modeBroadcast {
	std::string modes;
	std::string args;
	std::string unknown;
} modeBroadcast;


class Channel
{
	public:

		std::string				name;
		std::string				topic;
		std::string				key;

		bool					isInviteOnly;
		bool					isKeySet;
		bool					topicRestricted;
		int						membersLimit;

		std::map<int, Client *>	members;
		std::map<int, Client *>	operators;
		std::vector<int>		inviteList;

		Channel(std::string name) : name(name), topic(""), key(""), isInviteOnly(false) {}

		std::string	modeI(bool, modeBroadcast&);
		std::string	modeK(bool, std::string, modeBroadcast&);
		std::string	modeL(bool, std::string, size_t&, modeBroadcast&);
		std::string	modeO(bool, std::string, modeBroadcast&);
		std::string	modeT(bool, modeBroadcast&);

		void	addClient(Client *client);
		void	removeClient(Client *client);

		bool	InInviteList(int fd);
		void	broadcast(Client *client, std::string& message);
};

