
#ifndef TCP_SERVER
#define TCP_SERVER

#include <string>
#include <WS2tcpip.h>
#include <fstream>


// The server-side of the TCP communication is defined as the "tcp_server" class.
class tcp_server
{
private:
	// "sockadrr_in" is a struct that contains:
	// 
	// short            sin_family;    e.g. AF_INET -> IPv4 address family(IPv6 address family is AF_INET6).
	// unsigned short   sin_port;      e.g. htons(49152) -> Port number is converted to Network Byte Order (big-endian).
	// struct in_addr   sin_addr;      e.g. inet_aton("127.0.0.1") -> IP address is converted to binary in Network Byte Order.
	// char             sin_zero[8];   -> Reserved for system use. A WSK application should set the contents of this array to zero.
	//  
	// and the format of "sin_addr" is:
	// 
	// struct in_addr {
	//	unsigned long s_addr;  // Must be set with "inet_pton()". 
	//						   // "inet_pton()" converts the IPv4 or IPv6 IP Address format to binary form.
	//                         
	//};
	sockaddr_in m_socket_address;

	// You can change the buffer size as you want (max is 64k). The possibility of packet loss increases with buffer size (for non-localhost applications).
	static const unsigned int buffer_size{1024};
	
	std::string m_ip_address{};
	unsigned short m_port{};
	std::string m_server_message{};
	int m_socket_address_length{sizeof(m_socket_address)};
	

	// Main socket variable for binding and listening.
	SOCKET m_socket{INVALID_SOCKET};
	// Temporary socket variable used after accepting a connection from any host.
	SOCKET m_new_socket{INVALID_SOCKET};
	long m_incoming_message{};
	// WSAData holder. The data is written when the "start_server()" function is called.
	WSADATA m_wsa_data{};

	// Terminates the program with an error message specified by an argument and WSAGetLastError().
	static void report_error(const std::string& message);

public:
	// The parameters of the constructor are used to initialize the "m_socket_address" variable.
	tcp_server(const std::string& ip_address, unsigned short int port);
	~tcp_server();

	// Necessary functions for communication are declared.
	void start_server();
	void close_server();
	void accept_connection(SOCKET &new_socket);
	void build_response(const std::string& message);
	void send_response();
	void start_listen();

	bool is_socket_valid(SOCKET& socket);

	// This function is used to change the IP address and port after initializing an object.
	void change_ip_port(const std::string& ip_address, unsigned short int port);

	
};




#endif 