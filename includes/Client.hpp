#pragma once

#include "Placeholder.hpp"
#include <iostream>
#include <string>
#include <exception>
#include <netinet/in.h>


class Client
{
	public:

		int			fd;
		std::string	ip;

		std::string	nickName;
		std::string	userName;
		std::string realName;

		bool		isRegistered;
		bool		has_pass;
		bool		has_nick;
		bool		has_user;

		std::string	r_buffer;

		Client(int fd, sockaddr_in addr);

		std::string	getPrefix();
		

};
