/*
 * Client.h
 *
 *  Created on: Oct 21, 2015
 *      Author: karim
 */

#ifndef CLIENT_H_
#define CLIENT_H_

#define MAXDATASIZE 512 // max number of bytes we can get at once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include "FileHandler.h"

using namespace std;

class Client {
public:
	Client(string file_name, string request, string server_ip, string port_num);
	void *get_in_addr(struct sockaddr *sa);
	void error(const char* message);
	bool initiate_connection();
	bool interact();
	bool receive_data(char* buf_to_write, int bytes_received);
	bool receive_file(string file_name, int sock_fd/*, int file_size*/);
	virtual ~Client();
private:
	int sockfd, numbytes;
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	string servip;
	string portno;
	string request;
	string file_name;

};

#endif /* CLIENT_H_ */
