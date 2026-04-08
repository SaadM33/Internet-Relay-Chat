#pragma once

#include <iostream>
#include <string>
#include <exception>

class Client
{
	private:
		
		int	fd;
		int	nickName;
		std::string	buff;

	public:
		Client() : fd(-1), nickName(-1), buff("") {};
		int getFd() { return fd; }; 
		int getNickName() { return nickName; };
		std::string getBuff() { return buff; };

		void	setFd(int new_fd) { fd = new_fd; };
		

};

inline std::ostream& operator<<(std::ostream& os, Client& client)
{
	os << "Client fd: " << client.getFd() << ", Nickname: " << client.getNickName() << ", Buffer: " << client.getBuff();
	return os;
}