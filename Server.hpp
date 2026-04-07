#include <iostream>
#include <vector>
#include <exception>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>

// class singleton Server
class Server
{
	private:

		int							port;
		std::string					passwd;

		int							core_fd;
		std::vector<struct pollfd>	poll_fds;

		bool						c_banished;
		
		//client and channels

	public:
		
		Server();
		~Server();


		void setArgs(int ac, char **av);
		void ascend();
		void ignite();


};
