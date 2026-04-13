#include "Server.hpp"

int main(int ac, char **av)
{
	Server Srv(ac, av);

	try
	{
		Srv.ascend();
		Srv.ignite();
		// Srv.nuke();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}