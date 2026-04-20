
#include "Client.hpp"
#include <iostream>
#include <vector>

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

		Channel(std::string name, std::string key = "", std::string topic = "") : name(name), topic(topic), key(key), isInviteOnly(false) {}

		std::string	modeI(bool, modeBroadcast&);
		std::string	modeK(bool, std::string, modeBroadcast&);
		std::string	modeL(bool, std::string, size_t&, modeBroadcast&);
		std::string	modeO(bool, std::string, modeBroadcast&);
		std::string	modeT(bool, modeBroadcast&);

		void broadcast(int fd, std::string& message, int exile_fd = -1);
};

typedef struct s_modeBroadcast {
	std::string modes;
	std::string args;
	std::string unknown;
} modeBroadcast;