/*
 * Server.cpp
 *
 *  Created on: Oct 21, 2015
 *      Author: karim
 */

#include "Server.h"

Server::Server(int port_num) {
	// TODO Auto-generated constructor stub
	this->port_num = port_num;
}

void Server::error(string message) {
	perror(message.c_str());
}

bool Server::prepare_server() {
	this->socket_fd = socket(AF_INET,SOCK_STREAM,0);
	if (this->socket_fd < 0) {
		error("Error opening socket");
		return false;
	}
	bzero((char *) &(this->server_addr), sizeof(this->server_addr));
	this->server_addr = AF_INET;

}

Server::~Server() {
	// TODO Auto-generated destructor stub
}

