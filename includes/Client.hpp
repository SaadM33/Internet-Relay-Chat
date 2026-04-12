#pragma once

#include <iostream>
#include <string>
#include <exception>

class Client
{
	public:
		
		int	fd;
		std::string	nickName;
		std::string	c_buffer;

	public:
		Client(int fd) : fd(fd), nickName(""), c_buffer("") {};
		int getFd() { return fd; }; 
		std::string getNickName() { return nickName; };
		std::string getBuff() { return c_buffer; };

		void	setFd(int new_fd) { fd = new_fd; };
		

};

// for debug, remove later
inline std::ostream& operator<<(std::ostream& os, Client& client)
{
	os << "Client fd: " << client.getFd() << ", Nickname: " << client.getNickName() << ", Buffer: " << client.getBuff();
	return os;
}