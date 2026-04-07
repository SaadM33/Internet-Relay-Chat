#include "Server.hpp"

int main(int ac, char **av)
{
	Server server;

	try
	{
		server.setArgs(ac, av);

		server.ascend();
		server.ignite();
		// server.nuke();
		
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

}