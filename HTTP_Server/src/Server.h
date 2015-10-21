/*
 * Server.h
 *
 *  Created on: Oct 21, 2015
 *      Author: karim
 */

#ifndef SERVER_H_
#define SERVER_H_

#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"
#include "sys/types.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include <string>
#include <vector>
#include <iostream>

using namespace std;

class Server {
public:
	Server(int port_num);
	bool prepare_server();
	void error(string message);
	virtual ~Server();
private:
	int port_num = 0; // port number on which server accepts connections, defined as a parameter
	int socket_fd = 0; // socket file descriptor (array subscript into the file descriptor table)
	vector<int> new_socket_fd(); // new sockets file descriptors
	int client_add_len = 0; // size of client's address, needed for system call accept
	int num_chars_read_written = 0; // return value of read and write calls, contains number of
								// characters read or written
	string buffer; // string read from socket
	struct socketaddress_in server_addr();
	struct socketaddress_in cli_addr();
};

#endif /* SERVER_H_ */
