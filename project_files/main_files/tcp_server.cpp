#include "..\header_files\tcp_server.h"
#include <iostream>
#include <sstream>

// Checks whether the socket is valid.
bool tcp_server::is_socket_valid(SOCKET& socket)
{
	if (socket == INVALID_SOCKET)
	{
		report_error("Socket is invalid!");
		return false;
	}
	return true;
}

// Reports an error using WSAGetLastError() and the provided error message. Throws an exception.
void tcp_server::report_error(const std::string& error_message)
{
	int last_error = WSAGetLastError();
	std::cerr << "WSAGetLastError: " << last_error << std::endl;

	throw std::runtime_error(error_message);
	
}

// Parameterized Constructor for initializing IP, Port variables and assigning required values.
tcp_server::tcp_server(const std::string& ip_address, unsigned short int port) : m_ip_address(ip_address), m_port(port)
{
	// The "m_socket_address" instance of "sockaddr_in" is assigned protocol specifications.
	m_socket_address.sin_family = AF_INET; // "AF_INET" stands for IPv4 communication protocol.
	m_socket_address.sin_port = htons(m_port); // "m_port" variable is converted to Network Byte Order for proper communication.

	// IP Address is converted from standard dotted notation to an integer value in the format of "unsigned long". 
	int return_value = inet_pton(AF_INET, m_ip_address.c_str(), &m_socket_address.sin_addr.s_addr);
	if ( return_value == -1)
	{
		report_error("Numeric conversion of the IP Address has failed.");
	}
	else if (return_value == 0)
	{
		std::cout << "IP notation is not a valid IPv4 or IPv6 dotted-decimal address string." << std::endl;
	}
	
	// Server message is set by "build_response"
	build_response("Packet is received and here is my package!");

}


// Function for changing the IP Address and Port value after object initialization.
void tcp_server::change_ip_port(const std::string& ip_address, unsigned short int port)
{
	if (!(ip_address == m_ip_address && port == m_port))
	{
		// Closes the old socket and assigns IP and Port values provided by user.
		if (m_socket != INVALID_SOCKET)
		{
			if (closesocket(m_socket) == SOCKET_ERROR)
			{
				report_error("Socket is not properly closed.");
			}
			m_socket = INVALID_SOCKET;
		}

		m_ip_address = ip_address;
		m_port = port;

		m_socket_address.sin_port = htons(m_port);
			
		int return_value = inet_pton(AF_INET, m_ip_address.c_str(), &m_socket_address.sin_addr.s_addr);
		if (return_value == -1)
		{
			report_error("Numeric conversion of the IP Address has failed.");
		}
		else if (return_value == 0)
		{
			std::cout << "IP notation is not a valid IPv4 or IPv6 dotted-decimal address string." << std::endl;
		}



	}
}

// Updates the "m_server_message" variable with the provided argument.
void tcp_server::build_response(const std::string& message)
{
	m_server_message = message;
	
}

// Closes the socket.
tcp_server::~tcp_server()
{
	if (m_socket != INVALID_SOCKET)
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}
	if (m_new_socket != INVALID_SOCKET)
	{
		closesocket(m_new_socket);
		m_new_socket = INVALID_SOCKET;
	}

	WSACleanup();
}

// Starts server with IPv4 and TCP protocols.
void tcp_server::start_server()
{
	// Winsock library is started with version of 2.0, and the process information is written to "m_wsa_data".
	if (WSAStartup(MAKEWORD(2, 0), &m_wsa_data) != 0)
	{
		report_error("WSAStartup failed!");
	}
	
	else 
	{	// Attempts to assign a socket to "m_socket" with IPv4 and TCP protocol specifications.
		m_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (is_socket_valid(m_socket))
		{	
			// It is safe to cast "sockadrr_in*" to "sockaddr*"(the opposite is also true).
			if (bind(m_socket, (sockaddr*)&m_socket_address, m_socket_address_length) == SOCKET_ERROR)
			{
				report_error("Socket binding failed!");
			}
		}
	}
}

// Closes the bound sockets.
void tcp_server::close_server()
{
	if (m_socket != INVALID_SOCKET) {
		closesocket(m_socket);
		m_socket = INVALID_SOCKET; 
	}

	if (m_new_socket != INVALID_SOCKET) {
		closesocket(m_new_socket);
		m_new_socket = INVALID_SOCKET;
	}

}

// Listens on the specific socket and sends message if communication is properly established.
void tcp_server::start_listen()
{
	if (is_socket_valid(m_socket))
	{
		// Listens on the socket and accepts up to 20 connections at maximum capacity.
		if (listen(m_socket, 20) == SOCKET_ERROR)
		{
			report_error("Socket listening is failed!");
		}

		std::cout << "\n*** Listening on ADDRESS: " << m_ip_address << ", PORT: " << m_port << " ***\n";

		int bytes_received;

		while (true)
		{
			std::cout << "\n====== Waiting for a new connection ======\n";
			// The "m_socket" variable is used only for listening on the specific port, and the "m_new_socket" variable is used when a connection is made on that port. 
			// It manages different communications on the same port using different "Descriptors".
			accept_connection(m_new_socket);
	
			// The received message on the new socket is written to the "buffer" variable, and the size is stored in "bytesReceived".
			char buffer[tcp_server::buffer_size]{ 0 };
			bytes_received = recv(m_new_socket, buffer, tcp_server::buffer_size - 1, 0);
			
			if (bytes_received == 0)
			{
				std::cout << "Client closed the connection." << std::endl;
				if (closesocket(m_new_socket) == SOCKET_ERROR)
				{
					report_error("Socket is not properly closed.");
				}
				m_new_socket = INVALID_SOCKET;
				continue;
			}
			else if (bytes_received == SOCKET_ERROR)
			{
				std::cout << "Failed to receive bytes from client socket connection." << std::endl;
				if (closesocket(m_new_socket) == SOCKET_ERROR)
				{
					report_error("Socket is not properly closed.");
				}
				m_new_socket = INVALID_SOCKET;
				continue;
			}

			buffer[bytes_received] = '\0'; // A null termination character is added to the end of the array.

			// If a message is received, it will be printed.
			std::cout << "=-------------- Received request from client --------------=\n";
			std::cout << "The request of the Client is:" << buffer << std::endl;

			// After receiving the client's response, the server transmits a response.
			send_response();

			// After one transmit and receive operation, the socket is closed (specific to that communication).
			if (closesocket(m_new_socket) == SOCKET_ERROR) 
			{
				report_error("Socket is not properly closed.");
			}
			m_new_socket = INVALID_SOCKET;
		}
	}

	else 
	{
		std::cout << "Socket is invalid. Please create a proper socket with 'start_server()'." << std::endl;
	}

}

// Accepts connection by creating a new socket variable.
void tcp_server::accept_connection(SOCKET& new_socket)
{
	// The communication is established on a different socket variable if a connection request is made on the listening socket("m_socket" variable).
	new_socket = accept(m_socket, (sockaddr*)&m_socket_address, &m_socket_address_length);
	if (new_socket == INVALID_SOCKET)
	{
		std::cout << "Server failed to accept the incoming connection from ADDRESS: " << m_ip_address << ", PORT: " << m_port << "\n";
		std::cout << "WSAGetLastError(): " << WSAGetLastError() << std::endl;
	}
}

// Sends response to the client.
void tcp_server::send_response()
{
	size_t total_bytes_sent = 0;
	size_t message_length = m_server_message.size();
	const char* message_buffer = m_server_message.c_str();

	// The send() function may not always send the all the data. This while loop continues until all the data is sent.
	while (total_bytes_sent < message_length)
	{
		int bytes_sent = send(m_new_socket, message_buffer + total_bytes_sent, message_length - total_bytes_sent, 0);

		if (bytes_sent == SOCKET_ERROR)
		{
			std::cerr << "Failed to send response to client. WSAGetLastError: " 
				<< WSAGetLastError() << std::endl;
			return;
		}
		else
		{
			total_bytes_sent += bytes_sent;
		}
		
	}

	if (total_bytes_sent == m_server_message.size())
	{
		std::cout << ("=------ Server response sent to client successfully -------=\n") << std::endl;
	}
}


