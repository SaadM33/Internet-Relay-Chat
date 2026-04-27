#include "Server.hpp"

void    Server::execBot(int fd, std::string name)
{
    int roll = std::rand() % 6;

    std::string rabbit[] = {
        "--------rabbit----------",
        "  (\\(\\",
        "  ( -.-)  ",
        "  o_(\")(\")",
        ""
    };

    std::string cat[] = {
        "---------cat------------",
        "   /\\_/\\  ",
        "  ( o.o ) ",
        "   > ^ <  ",
        ""
    };

    std::string computer[] = {
        "-------computer---------",
        "          .........",
        "        .'-------.' |",
        "       | .-----. | |",
        "       | |     | | |",
        "     __| |     | | |;",
        "    /  |*`-----'.|.' `",
        "   /   `---------' ;",
        "   /  .'''///////' ",
        "  ... ######### /",
        "  /  ######### /",
        "  `-----------'",
        " /______________/|",
        " `--------------' |",
        "  : |_____| :     |",
        "  n |       n     |",
        "  M |       M     |",
        "   `\"\"'     `\"\"'  ",
        ""
    };

    std::string leet[] = {
        "--------1337------------",
        " __ _____ _____ ____",
        "|  |___ /|___ /|___  |",
        "|  | |_ \\  |_ \\   / /",
        "|  |___) ||__) | / /",
        "|__|____/|____/ /_/",
        ""
    };

    std::string sword[] = {
        "--------sword-----------",
        "               )           ",
        "             (             ",
        "           '    }          ",
        "         (    '            ",
        "        '      (           ",
        "         )  |    )         ",
        "       '   /|\\    `        ",
        "      )   / | \\  ` )       ",
        "     {    | | |  {         ",
        "    }     | | |  .         ",
        "     '    | | |    )       ",
        "    (    /| | |\\    .      ",
        "     .  / | | | \\  (       ",
        "   }    \\ \\ | / /  .       ",
        "    (    \\ `-' /    }      ",
        "    '    / ,-. \\    '      ",
        "     }  / / | \\ \\  }       ",
        "    '   \\ | | | /   }      ",
        "     (   \\| | |/  (        ",
        "       )  | | |  )         ",
        "       .  | | |  '         ",
        "          J | L            ",
        "    /|    J_|_L    |\\      ",
        "    \\ \\___/ o \\___/ /      ",
        "     \\_____ _ _____/       ",
        "           |-|             ",
        "           |-|             ",
        "           |-|             ",
        "          ,'-'.            ",
        "          '---'            ",
        ""
    };

    std::string flower[] = {
        "--------flower----------",
        "   _ _ _",
        "  ((@)(@))",
        " (@)(***)(@ )",
        "  ((@)(@))",
        "     |",
        "     |",
        "    /|\\",
        ""
    };

    std::string *arts[] = { rabbit, cat, computer, leet, sword, flower };
    for (int i = 0; arts[roll][i] != ""; i++)
    {
		std::string message = ":artbot!artbot@localhost PRIVMSG " + name + " :" + arts[roll][i] + "\r\n";
		send(fd, message.c_str(), message.size(), 0);
    }
}

void	Server::execPrivmsg(int fd, Message msg)
{
	if (msg.params.empty())
	{
		 sendReply(fd, ERR_NORECIPIENT, msg.command);
		return ;
	}
	else if (msg.trailing.empty())
	{
		sendReply(fd, ERR_NOTEXTTOSEND, msg.command);
		return ;
	}

	std::stringstream ss(msg.params[0]);
	std::string params;

	while (std::getline(ss, params, ','))
	{
		if (params.empty())
			continue ;
		if (params[0] == '#')
		{
			std::string channelName(params);
			if (this->channels.find(channelName) == this->channels.end())
			{
				this->sendReply(fd, ERR_NOSUCHNICK, channelName);
				continue;
			}
			if (this->channels[channelName]->members.find(fd) == this->channels[channelName]->members.end())
			{
				std::string reply = ":localhost 404 " + clients[fd]->nickName + " " + channelName + " :Cannot send to channel\r\n";
				send(fd, reply.c_str(), reply.size(), 0);
				continue ;
			}
			std::string botTrigger = "!artbot";
			std::string keyword(msg.trailing, 0, botTrigger.size());
			char next = msg.trailing[botTrigger.size()];
			if (keyword == botTrigger && (next == ' ' || next == '\0'))
				this->execBot(fd, channelName);
			else
			{
				std::string message = "PRIVMSG " + params + " :" + msg.trailing + "\r\n";
				this->channels[channelName]->broadcast(this->clients[fd], message, true);
			}
		}
		else if (params[0] == '@')
		{
			std::string channelName(params, 1);
			if (this->channels.find(channelName) == this->channels.end())
			{
				this->sendReply(fd, ERR_NOSUCHNICK, channelName);
				continue;
			}
			if (this->channels[channelName]->operators.find(fd) == this->channels[channelName]->operators.end())
			{
				std::string reply = ":localhost 404 " + clients[fd]->nickName + " " + channelName + " :Cannot send to channel\r\n";
				send(fd, reply.c_str(), reply.size(), 0);
				continue ;
			}
			std::string message = "PRIVMSG " + params + " :" + msg.trailing + "\r\n";
			this->channels[channelName]->operatorsBroadcast(this->clients[fd], message);
		}
		else
		{
			bool found = false;
			for (std::map<int, Client *>::iterator it = this->clients.begin(); it != this->clients.end(); it++)
			{
				if ((*it).second->nickName == params)
				{
					std::string reply = this->clients[fd]->getPrefix() + " PRIVMSG "
					+ params + " :" + msg.trailing + "\r\n";
					send((*it).second->fd, reply.c_str(), reply.size(), 0);
					found = true;
					break ;
				}
			}
			if (!found)
				this->sendReply(fd, ERR_NOSUCHNICK, params);
		}
	}
}
