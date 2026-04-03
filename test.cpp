#include <iostream>          // For std::cout, std::cerr
#include <cstring>            // For memset, strlen
#include <sys/socket.h>       // For socket(), accept(), send(), recv(), close()
#include <netinet/in.h>       // For struct sockaddr_in, INADDR_ANY, htons()
#include <unistd.h>           // For close(), read(), write()
#include <arpa/inet.h>        // For inet_ntop() (convert IP address to string)

/*
 * ============================================================================
 * SIMPLE ECHO SERVER - UNDERSTANDING SOCKET PROGRAMMING FUNDAMENTALS
 * ============================================================================
 * 
 * This program demonstrates the basic steps to create a TCP server that:
 * 1. Creates a listening socket
 * 2. Binds it to a port
 * 3. Listens for one client connection
 * 4. Accepts the connection
 * 5. Receives a message from the client
 * 6. Echoes the message back to the client
 * 7. Closes the connection
 * 
 * Each step is explained with detailed comments to understand what's happening
 * at the system level.
 */

int main(void)
{
	// =========================================================================
	// STEP 1: CREATE A SERVER SOCKET
	// =========================================================================
	// 
	// socket() creates a file descriptor for network communication.
	// 
	// Parameters:
	//   - AF_INET:     Address Family - use IPv4 (AF_INET6 for IPv6)
	//   - SOCK_STREAM: Socket Type - use TCP (reliable, ordered delivery)
	//                  (SOCK_DGRAM would be UDP - unreliable, unordered)
	//   - 0:           Protocol - 0 means auto-select (TCP for SOCK_STREAM)
	//
	// Return value: a file descriptor (integer) representing the socket
	//   - On success: returns positive integer (e.g., 3, 4, 5...)
	//   - On error: returns -1
	//
	// Why it works: The kernel allocates resources for this socket and tracks it
	// with a file descriptor, just like regular files.

	std::cout << "=== STEP 1: Creating server socket ===" << std::endl;
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	
	if (server_fd == -1)
	{
		std::cerr << "Error: Failed to create socket" << std::endl;
		return (1);
	}
	std::cout << "✓ Server socket created (fd: " << server_fd << ")" << std::endl << std::endl;

	// =========================================================================
	// STEP 2: SET SOCKET OPTIONS - ALLOW ADDRESS REUSE
	// =========================================================================
	//
	// After closing a socket, the port stays in TIME_WAIT state for a while.
	// This prevents us from immediately binding the same port again.
	// 
	// SO_REUSEADDR allows us to bind to the same port after restart.
	//
	// setsockopt() modifies socket behavior:
	//   - server_fd:       the socket to modify
	//   - SOL_SOCKET:      option level (socket level options)
	//   - SO_REUSEADDR:    the specific option to set
	//   - &reuse:          pointer to the option value (1 = enabled)
	//   - sizeof(reuse):   size of the option value

	std::cout << "=== STEP 2: Setting socket option SO_REUSEADDR ===" << std::endl;
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
	{
		std::cerr << "Error: Failed to set socket option" << std::endl;
		close(server_fd);
		return (1);
	}
	std::cout << "✓ Socket option set (allows port reuse)" << std::endl << std::endl;

	// =========================================================================
	// STEP 3: PREPARE ADDRESS STRUCTURE FOR BINDING
	// =========================================================================
	//
	// We need to tell the socket what port to listen on and what interface
	// to listen on. This is done using a struct sockaddr_in.
	//
	// struct sockaddr_in {
	//     short sin_family;           // AF_INET (IPv4)
	//     unsigned short sin_port;    // Port number (in network byte order)
	//     struct in_addr sin_addr;    // IP address (in network byte order)
	//     char sin_zero[8];           // Padding (usually ignored)
	// };
	//
	// Network byte order: on some systems, numbers are stored differently
	// (big-endian vs little-endian). Network byte order is always big-endian.
	// - htons(): converts host byte order to network byte order for 16-bit (short)
	// - htonl(): converts host byte order to network byte order for 32-bit (long)

	std::cout << "=== STEP 3: Creating address structure ===" << std::endl;
	struct sockaddr_in server_addr;
	
	// Clear the structure to avoid garbage values
	memset(&server_addr, 0, sizeof(server_addr));
	
	server_addr.sin_family = AF_INET;           // IPv4
	server_addr.sin_port = htons(9999);         // Port 9999 (converted to network byte order)
	server_addr.sin_addr.s_addr = INADDR_ANY;   // Listen on all available interfaces (0.0.0.0)
	
	std::cout << "✓ Address structure ready:" << std::endl;
	std::cout << "  - Family: AF_INET (IPv4)" << std::endl;
	std::cout << "  - Port: 9999" << std::endl;
	std::cout << "  - Address: INADDR_ANY (0.0.0.0)" << std::endl << std::endl;

	// =========================================================================
	// STEP 4: BIND SOCKET TO ADDRESS AND PORT
	// =========================================================================
	//
	// bind() associates the socket with a specific port.
	// This tells the kernel: "incoming connections to this port on this interface
	// should be routed to this socket".
	//
	// Parameters:
	//   - server_fd:              the socket file descriptor
	//   - (struct sockaddr*)...:  pointer to address structure (cast required)
	//   - sizeof(server_addr):    size of the address structure
	//
	// Why cast to sockaddr*? For historical reasons, bind() takes a generic
	// sockaddr pointer. We use sockaddr_in internally but cast it to sockaddr.
	//
	// Return value:
	//   - On success: 0
	//   - On error: -1 (usually means port already in use, permission denied)

	std::cout << "=== STEP 4: Binding socket to port 9999 ===" << std::endl;
	if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
	{
		std::cerr << "Error: Failed to bind socket" << std::endl;
		close(server_fd);
		return (1);
	}
	std::cout << "✓ Socket bound to port 9999" << std::endl << std::endl;

	// =========================================================================
	// STEP 5: MARK SOCKET AS LISTENING
	// =========================================================================
	//
	// listen() marks the socket as a passive socket that will accept
	// incoming connections.
	//
	// Parameters:
	//   - server_fd:  the socket to mark as listening
	//   - 1:          the backlog (max number of pending connections)
	//                 When the backlog is full, new connection requests are rejected
	//                 This is usually a small number (1-128)
	//
	// After listen(), the socket enters LISTEN state and can accept connections.
	// This doesn't actually wait for connections yet - it just prepares the socket.

	std::cout << "=== STEP 5: Marking socket as listening ===" << std::endl;
	if (listen(server_fd, 1) == -1)
	{
		std::cerr << "Error: Failed to listen on socket" << std::endl;
		close(server_fd);
		return (1);
	}
	std::cout << "✓ Socket marked as listening" << std::endl;
	std::cout << "  Server is now ready to accept connections!" << std::endl;
	std::cout << "  Waiting for a client to connect..." << std::endl << std::endl;

	// =========================================================================
	// STEP 6: PREPARE STRUCTURES TO ACCEPT A CLIENT CONNECTION
	// =========================================================================
	//
	// When accept() is called, it will provide us with:
	// 1. A new socket file descriptor for communicating with the client
	// 2. The client's address information (IP, port)
	//
	// We need to prepare a sockaddr_in structure to receive this information.
	// The client_len variable will be updated by accept() to the actual size.

	std::cout << "=== STEP 6: Preparing to accept client connection ===" << std::endl;
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	
	std::cout << "✓ Client address structure prepared" << std::endl << std::endl;

	// =========================================================================
	// STEP 7: ACCEPT AN INCOMING CLIENT CONNECTION
	// =========================================================================
	//
	// accept() is a BLOCKING call. It pauses execution and waits until:
	// - A client connects to this socket, OR
	// - The call is interrupted by a signal
	//
	// What happens at the OS level:
	// 1. Client connects to port 9999
	// 2. The OS creates a socket pair (one on server, one on client)
	// 3. A connection request is placed in the backlog queue
	// 4. accept() removes this request from the queue and returns
	//
	// Return value:
	//   - On success: a new file descriptor for communicating with this client
	//   - On error: -1
	//
	// Important: The server_fd socket is still used for listening for more clients.
	// The client_fd socket is used to communicate with this specific client.

	std::cout << "=== STEP 7: Accepting client connection (BLOCKING CALL) ===" << std::endl;
	std::cout << "⏳ Waiting for client connection..." << std::endl;
	std::cout << "   (Try connecting with: telnet localhost 9999)" << std::endl;
	
	int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
	
	if (client_fd == -1)
	{
		std::cerr << "Error: Failed to accept connection" << std::endl;
		close(server_fd);
		return (1);
	}
	
	std::cout << "✓ Client connected!" << std::endl;
	std::cout << "  Client fd: " << client_fd << std::endl;
	std::cout << "  Client IP: " << inet_ntoa(client_addr.sin_addr) << std::endl;
	std::cout << "  Client Port: " << ntohs(client_addr.sin_port) << std::endl << std::endl;

	// =========================================================================
	// STEP 8: RECEIVE A MESSAGE FROM THE CLIENT
	// =========================================================================
	//
	// recv() reads data from the connected client socket.
	// Like accept(), recv() can be BLOCKING (waits for data to arrive).
	//
	// Parameters:
	//   - client_fd:        socket to receive from
	//   - buffer:           where to store the received data
	//   - sizeof(buffer):   max bytes to receive
	//   - 0:                flags (0 = no special flags, MSG_DONTWAIT = non-blocking)
	//
	// Return value:
	//   - On success: number of bytes received (1 to sizeof(buffer))
	//   - If client closes: 0
	//   - On error: -1
	//
	// Important: recv() does NOT null-terminate the buffer!
	// We need to handle this manually if we treat it as a string.

	std::cout << "=== STEP 8: Receiving message from client ===" << std::endl;
	std::cout << "⏳ Waiting for client to send data..." << std::endl;
	
	char buffer[1024];
	memset(buffer, 0, sizeof(buffer));  // Clear the buffer first
	
	int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
	
	if (bytes_received == -1)
	{
		std::cerr << "Error: Failed to receive data" << std::endl;
		close(client_fd);
		close(server_fd);
		return (1);
	}
	else if (bytes_received == 0)
	{
		std::cout << "Client closed the connection" << std::endl;
		close(client_fd);
		close(server_fd);
		return (0);
	}
	
	// Null-terminate the buffer to safely use it as a string
	buffer[bytes_received] = '\0';
	
	std::cout << "✓ Message received!" << std::endl;
	std::cout << "  Bytes received: " << bytes_received << std::endl;
	std::cout << "  Message: [" << buffer << "]" << std::endl << std::endl;

	// =========================================================================
	// STEP 9: SEND THE MESSAGE BACK TO THE CLIENT (ECHO)
	// =========================================================================
	//
	// send() transmits data over a connected socket.
	// It's the counterpart to recv().
	//
	// Parameters:
	//   - client_fd:      socket to send to
	//   - buffer:         data to send
	//   - bytes_received: number of bytes to send
	//   - 0:              flags (0 = no special flags)
	//
	// Return value:
	//   - On success: number of bytes sent
	//   - On error: -1
	//
	// Note: send() might not send all bytes at once if the buffer is full.
	// In production code, you should keep sending until all bytes are transmitted.

	std::cout << "=== STEP 9: Sending message back to client ===" << std::endl;
	
	int bytes_sent = send(client_fd, buffer, bytes_received, 0);
	
	if (bytes_sent == -1)
	{
		std::cerr << "Error: Failed to send data" << std::endl;
		close(client_fd);
		close(server_fd);
		return (1);
	}
	
	std::cout << "✓ Message echoed back!" << std::endl;
	std::cout << "  Bytes sent: " << bytes_sent << std::endl << std::endl;

	// =========================================================================
	// STEP 10: CLOSE THE CONNECTION AND CLEANUP
	// =========================================================================
	//
	// close() terminates a socket and frees its resources.
	//
	// Two sockets need to be closed:
	// 1. client_fd: the socket for this specific client connection
	// 2. server_fd: the listening socket itself
	//
	// If we only close client_fd:
	//   - We disconnect from this client
	//   - But the server is still listening (would accept more clients)
	//
	// For this simple program, we close both and exit.

	std::cout << "=== STEP 10: Closing connections ===" << std::endl;
	close(client_fd);
	std::cout << "✓ Client socket closed" << std::endl;
	
	close(server_fd);
	std::cout << "✓ Server socket closed" << std::endl;
	std::cout << std::endl;

	std::cout << "=== SERVER SHUTDOWN ===" << std::endl;
	std::cout << "Program completed successfully!" << std::endl;
	
	return (0);
}
