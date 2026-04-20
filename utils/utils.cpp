#include "Server.hpp"

bool	isAvailable(const std::map<int, Client *> &clients, std::string str)
{
	for (std::map<int, Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		if (it->second->nickName == str)
			return false;
	}
	return true;
}

bool	isAlnumStr(std::string str)
{
	for (size_t i = 0; i < str.size(); i++) {
		if (!isalnum(str[i]) && str[i] != '-' && str[i] != '_')
			return false;
	}
	return true;
}