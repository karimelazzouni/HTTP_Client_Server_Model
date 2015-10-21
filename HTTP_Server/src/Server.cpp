/*
 * Server.cpp
 *
 *  Created on: Oct 21, 2015
 *      Author: karim
 */

#include "Server.h"

Server::Server(string portno) {
	// TODO Auto-generated constructor stub
	this->portno = portno;
}

void* Server::get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET){
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void /*Server::*/sigchld_handler(int s)
{
	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;
	while(waitpid(-1, NULL, WNOHANG) > 0);
	errno = saved_errno;
}

void Server::error(const char* message) {
	perror(message);
}

bool Server::prepare_server() {
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP
	if ((rv = getaddrinfo(NULL, this->portno.c_str(), &hints, &servinfo)) != 0) {
		error("getaddrinfo: %s\n");
		return false;
	}
	// loop through all the results and bind to the first we can
		for(this->p = servinfo; this->p != NULL; this->p = this->p->ai_next) {
			if ((sockfd = socket(this->p->ai_family, this->p->ai_socktype,this->p->ai_protocol)) == -1) {
				error("server: socket");
				continue;
			}
			if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) {
				error("setsockopt");
				return false;
			}
			if (bind(sockfd, this->p->ai_addr, this->p->ai_addrlen) == -1) {
				close(sockfd);
				error("server: bind");
				continue;
			}
			break;
		}
		freeaddrinfo(servinfo); // all done with this structure
		if (this->p == NULL) {
			error("server: failed to bind\n");
			return false;
		}
		return true;
}

bool Server::listening() {
	if (listen(sockfd, BACKLOG) == -1) {
			error("listen");
			return false;
	}
	return true;
}

bool Server::reap_dead_children() {
	sa.sa_handler = /*Server::*/sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		error("sigaction");
		return false;
	}
	return true;
}

bool Server::accept_connection() {
	while(1) { // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			error("accept");
			return false;
		}
		inet_ntop(their_addr.ss_family,
		get_in_addr((struct sockaddr *)&their_addr),s, sizeof s);
		cout << "server: got connection from " << s << endl;
		if (!fork()) { // this is the child process
				close(sockfd); // child doesn't need the listener
				if (send(new_fd, "Hello, world!", 13, 0) == -1)
					error("send");
				close(new_fd);
				return false;
		}
		close(new_fd); // parent doesn't need this

	}
}

Server::~Server() {
	// TODO Auto-generated destructor stub
}

