#include "Server.hpp"

bool	isAvailable(const std::map<int, Client *> &clients, std::string str)
{
	for (std::map<int, Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		if (it->second->nickName == str)
			return false;
	}
	return true;
}

bool	isValidNick(std::string str)
{
	if (str[0] == '#' || str[0] == '@')
		return false;

	for (size_t i = 0; i < str.size(); i++) {
		if (!isalnum(str[i]) && str[i] != '-' && str[i] != '_' && str[i] != '[' && str[i] != ']' && str[i] != '\\'
			&& str[i] != '`' && str[i] != '_' && str[i] != '^' && str[i] != '{' && str[i] != '|' && str[i] != '}')
			return false;
	}
	return true;
}

int	getFdFromNick(const std::map<int, Client *> &clients, std::string str)
{
	for (std::map<int, Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		if (it->second->nickName == str)
			return it->first;
	}
	return -1;
}
