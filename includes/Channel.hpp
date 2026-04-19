
#include "Client.hpp"
#include <iostream>
#include <vector>

class Channel
{

	public:
		std::string				name;
		std::string				topic;
		std::string				key;

		std::vector<Client *>	clients;
		std::vector<Client *>	operators;

		Channel(std::string name, std::string key = "", std::string topic = "") : name(name), topic(topic), key(key) {}


};

