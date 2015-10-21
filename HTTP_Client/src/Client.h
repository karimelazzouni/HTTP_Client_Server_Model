/*
 * Client.h
 *
 *  Created on: Oct 21, 2015
 *      Author: karim
 */

#ifndef CLIENT_H_
#define CLIENT_H_

#define MAXDATASIZE 100 // max number of bytes we can get at once

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

using namespace std;

class Client {
public:
	Client(string server_ip, string port_num);
	void *get_in_addr(struct sockaddr *sa);
	void error(const char* message);
	bool initiate_connection();
	bool interact();
	virtual ~Client();
private:
	int sockfd, numbytes;
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	string servip;
	string portno;

};

#endif /* CLIENT_H_ */
