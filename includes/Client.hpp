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

		Client(int fd) : fd(fd), nickName(""), isRegistered(false), r_buffer("") {};
		

};
