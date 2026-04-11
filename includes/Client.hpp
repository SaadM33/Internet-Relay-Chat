#pragma once

#include <iostream>
#include <string>
#include <exception>

class Client
{
	public:
		
		int	fd;
		std::string	nickName;
		std::string	buff;

	public:
		Client(int fd) : fd(fd), nickName(""), buff("") {};
		int getFd() { return fd; }; 
		std::string getNickName() { return nickName; };
		std::string getBuff() { return buff; };

		void	setFd(int new_fd) { fd = new_fd; };
		

};

// for debug, remove later
inline std::ostream& operator<<(std::ostream& os, Client& client)
{
	os << "Client fd: " << client.getFd() << ", Nickname: " << client.getNickName() << ", Buffer: " << client.getBuff();
	return os;
}