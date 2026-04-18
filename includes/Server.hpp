#pragma once 

#include "Client.hpp"

// I advise creating a new file dedicated to libraries and macros due to the sheer amount of lines
#include <iostream>
#include <vector>
#include <map>
#include <exception>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <cerrno>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <arpa/inet.h>
#include <cstdio>
#include <sstream>

// Macros for server replies:
#define RPL_CAP "000"
#define RPL_WELCOME "001"
#define RPL_UMODEIS "221"
#define RPL_AWAY "301"
#define RPL_CHANNELMODEIS "324"
#define RPL_CREATIONTIME "329"
#define RPL_NOTOPIC "331"
#define RPL_TOPIC "332"
#define RPL_TOPICWHOTIME "333"
#define RPL_INVITING "341"
#define RPL_NAMREPLY "353"
#define RPL_ENDOFNAMES "366"
#define ERR_NOSUCHNICK "401"
#define ERR_NOSUCHSERVER "402"
#define ERR_NOSUCHCHANNEL "403"
#define ERR_CANNOTSENDTOCHAN "404"
#define ERR_TOOMANYCHANNELS "405"
#define ERR_NORECIPIENT "411"
#define ERR_NOTEXTTOSEND "412"
#define ERR_UNKNOWNCOMMAND "421"
#define ERR_NONICKNAMEGIVEN "431"
#define ERR_ERRONEUSNICKNAME "432"
#define ERR_NICKNAMEINUSE "433"
#define ERR_NICKCOLLISION "436"
#define ERR_USERNOTINCHANNEL "441"
#define ERR_NOTONCHANNEL "442"
#define ERR_USERONCHANNEL "443"
#define ERR_NOTREGISTERED "451"
#define ERR_NEEDMOREPARAMS "461"
#define ERR_ALREADYREGISTERED "462"
#define ERR_PASSWDMISMATCH "464"
#define ERR_CHANNELISFULL "471"
#define ERR_INVITEONLYCHAN "473"
#define ERR_BANNEDFROMCHAN "474"
#define ERR_BADCHANNELKEY "475"
#define ERR_BADCHANMASK "476"
#define ERR_CHANOPRIVSNEEDED "482"
#define ERR_UMODEUNKNOWNFLAG "501"
#define ERR_USERSDONTMATCH "502"

typedef struct s_msg {
	std::string					command;
	std::vector<std::string> 	params;
	std::string					trailing;
}	Message;

class Server;
typedef void (Server::*cmdHandler)(int, Message);

class Server
{
	private:
		int									port;
		std::string							passwd;

		int									core_fd;
		std::vector<struct pollfd>			poll_fds;

		std::map<int,Client *>				clients;
		// std::map<int,Channel *>		channels;

		bool								c_banished;

		std::map<std::string, cmdHandler>	cmdMap;
		std::map<std::string, std::string>	replyMap;

	public:
		
		Server(int, char **);

		void	instantiateCmds();
		void	instantiateReplies();
		void	ascend();
		void	ignite();
		
		void	acceptClient();
		void	processClient(int);
		void	processCmd(int, Message);

		void	handleInput(int);

		void	disconnectClient(int, int);
		void	registerClient(int);

		void	execCap(int, Message);
		void	execPass(int, Message);
		void	execNick(int, Message);
		void	execUser(int, Message);
		void	execJoin(int, Message);
		void	execPart(int, Message);
		void	execTopic(int, Message);
		void	execInvite(int, Message);
		void	execKick(int, Message);
		void	execMode(int, Message);
		void	execPrivmsg(int, Message);

		// void	sendReply(int fd, std::string code, std::string message);
		void	sendReply(int fd, std::string code);

};
