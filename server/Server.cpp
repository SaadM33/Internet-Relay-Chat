#include "Server.hpp"

Server::Server(int ac, char **av)
{
	if (ac != 3){
		throw std::invalid_argument("Usage: ./ircserv <port> <password>");
	}
	
	char *endptr;
	int port = std::strtod(av[1], &endptr);
	if (port <= 0 || port > 65535 || *endptr != '\0'){
		throw std::invalid_argument("Error: Invalid port number. Must be between 1 and 65535.");
	}

	std::string	passwd = av[2];
	if (passwd.empty()){
		throw std::invalid_argument("Error: Password cannot be empty.");
	}

	this->port = port;
	this->passwd = passwd;
}


void	Server::ascend()
{
	this->core_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->core_fd == -1) {
		throw std::runtime_error("Error: Failed to create socket.");
	}

	int reuse = 1;
	if (setsockopt(this->core_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
		throw std::runtime_error("Error: Failed to set socket option.");
	}
	
	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(this->port);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(this->core_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		throw std::runtime_error("Error: Failed to bind socket.");
	}

	if (listen(this->core_fd, 20) == -1) {
		throw std::runtime_error("Error: Failed to listen on socket.");
	}

	std::cout << "Server is listening on port " << this->port << std::endl;
}


void	Server::ignite()
{
	this->poll_fds.push_back((struct pollfd){core_fd, POLLIN, 0});

	// ctrl c later
	while (true)
	{
		if (poll(poll_fds.data(), poll_fds.size(), -1) == -1) {
			throw std::runtime_error("Error: Failed to poll sockets.");
		}

		for (size_t i = 0; i < poll_fds.size(); i++)
		{
			c_banished = false;
			if (poll_fds[i].revents & POLLIN)  
			{
				if (poll_fds[i].fd == core_fd)
					this->AcceptClient();
				else
					this->ReceiveClient();
			}
			if (poll_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) { 
				this->DisconnectClient();
			}
			
			if (c_banished)
				i--;
		}
	}
}

// accpt new conn, upadte pollfd vec, make new Client with data, update other classes?,  
void	Server::AcceptClient()
{
	struct sockaddr_in cAddr;
	socklen_t cLen = sizeof(cAddr);

	int new_fd = accept(core_fd, (struct sockaddr*)&cAddr, &cLen);
	if (new_fd < 0)
		throw std::logic_error("couldnt accept client");
		
	this->clients[new_fd] = new Client();
	clients[new_fd]->setFd(new_fd);

	this->poll_fds.push_back((struct pollfd){new_fd, POLLIN, 0});
	
	
	
	
	
	
	std::cout << "New client connected: " << clients[new_fd] << std::endl;
}

void	Server::ReceiveClient()
{	
	std::cout << "Client received"<< std::endl;
}
