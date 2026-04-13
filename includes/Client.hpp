#pragma once

#include <iostream>
#include <string>
#include <exception>

class Client
{
	public:
		int			fd;

		std::string	nickName;
		std::string	userName;
		std::string realName;

		bool		isRegistered;
		bool		has_pass;
		bool		has_nick;
		bool		has_user;

		std::string	r_buffer;
		char*		w_buffer;


	public:
		Client(int fd) : fd(fd), nickName(""), isRegistered(false), r_buffer("") {};
		int getFd() { return fd; }; 
		std::string getNickName() { return nickName; };
		std::string getBuff() { return r_buffer; };

		void	setFd(int new_fd) { fd = new_fd; };
		void	setNickName(std::string new_nick) {nickName = new_nick;};
		void	setUserName(std::string new_user) {nickName = new_user;};
		void	setRealName(std::string new_real) {nickName = new_real;};
		

};

// for debug, remove later
inline std::ostream& operator<<(std::ostream& os, Client& client)
{
	os << "Client fd: " << client.getFd() << ", Nickname: " << client.getNickName() << ", Buffer: " << client.getBuff();
	return os;
}