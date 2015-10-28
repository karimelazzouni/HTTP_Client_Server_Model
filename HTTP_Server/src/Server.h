/*
 * Server.h
 *
 *  Created on: Oct 21, 2015
 *      Author: karim
 */

#ifndef SERVER_H_
#define SERVER_H_

#define BACKLOG 10
#define MAXDATASIZE 1000
#define MAXREQUESTSIZE 8192
#define WORKINGDIRECTORY "/tmp/server"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <string>
#include <iostream>
#include <ctime>

#include "FileHandler.h"

using namespace std;

class Server {
public:
	Server(string port_num);
	bool prepare_server();
	bool listening();
	bool reap_dead_children();
	bool accept_connection();
	void error(const char* message);
	void* get_in_addr(struct sockaddr *sa);
	bool send_file(int file_size, ifstream* file_stream);
	bool send_data(const char* buf);
	bool receive_data(char* buf_to_write, int bytes_received);
	bool receive_file(string file_name, int file_size);
	string construct_message(int req_number);
	virtual ~Server();
private:
	string portno;
	int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;
};

#endif /* SERVER_H_ */
