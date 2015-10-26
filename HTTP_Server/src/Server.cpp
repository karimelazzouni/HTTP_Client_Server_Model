/*
 * Server.cpp
 *
 *  Created on: Oct 21, 2015
 *      Author: karim
 */

#include "Server.h"
#define MAX_BUF 1024

Server::Server(string portno) {
	// TODO Auto-generated constructor stub
	this->portno = portno;
}

void* Server::get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

void /*Server::*/sigchld_handler(int s) {
	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;
	while (waitpid(-1, NULL, WNOHANG) > 0)
		;
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
	if ((rv = getaddrinfo(NULL, this->portno.c_str(), &hints, &servinfo))
			!= 0) {
		error("getaddrinfo: %s\n");
		return false;
	}
	// loop through all the results and bind to the first we can
	for (this->p = servinfo; this->p != NULL; this->p = this->p->ai_next) {
		if ((sockfd = socket(this->p->ai_family, this->p->ai_socktype,
				this->p->ai_protocol)) == -1) {
			error("server: socket");
			continue;
		}
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))
				== -1) {
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
	while (1) { // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size);
		if (new_fd == -1) {
			error("accept");
			return false;
		}
		inet_ntop(their_addr.ss_family,
				get_in_addr((struct sockaddr *) &their_addr), s, sizeof s);
		cout << "server: got connection from " << s << endl;
		if (!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener
			char buf[MAX_BUF];
			int byte_count;

			if ((byte_count = recv(new_fd, buf, sizeof buf, 0)) == -1) {
				error("recv");
				return false;
			}
			buf[byte_count] = '\0';
			printf("Server: received %d bytes\n\"%s\"\n", byte_count, buf);
			//Parse
			//Split request
			vector<string> v;
			string s = buf;
			FileHandler::split(s, '\r\n', v);
			string initial = v[0];
			vector<string> headers;
			for (int i = 1; i < v.size(); ++i) {
				headers.push_back(v[i]);
			}
			//Split initial
			v.clear();
			FileHandler::split(initial, ' ', v);
			string method = v[0];
			string file = WORKINGDIRECTORY+v[1];


			//GET
			if (method.compare("GET") == 0) {
				//Check for file existence
				//File exists
				if (FileHandler::fexists(file)) {
					cout << "Exists"<<file << endl;

					//Send File
					ifstream file_stream;
					FileHandler::open_file_to_read(&file_stream,
							file);
					if (!file_stream.is_open()) {
						error("Could not open file");
						return false;
					}
					int file_size = FileHandler::get_file_size(&file_stream);
					cout << "sending actual file..." << endl;
					send_data(sockfd,file_size,&file_stream);
//					send_data(sockfd, file_size, &file_stream);
					cout << "file was sent successfully" << endl;
				}
				//File doesn't exist
				else {
					cout << "Doesn't Exists" << endl;
					string not_found = "HTTP/1.0 404 Not Found\r\n";
					if (send(new_fd, not_found.c_str(),
							strlen(not_found.c_str()), 0) == -1)
							error("send");
				}

			} else if (method.compare("POST") == 0) {

				cout << "POST" << endl;
			} else {
				error("Not GET nor POST");
			}
			close(new_fd);
			return true;
		}
		close(new_fd); // parent doesn't need this

	}
}

Server::~Server() {
	// TODO Auto-generated destructor stub
}

bool Server::send_data(int sock_fd,int file_size, ifstream* file_stream) {
	cout << "sending chunks..." << endl;
	int sent_bytes = 0;
	char chunk [MAXDATASIZE];
	while (sent_bytes < file_size) {
		FileHandler::read_chunk_in_memory(file_stream,chunk,min(file_size-sent_bytes,MAXDATASIZE));
		int numbytes = 0;
		if ((numbytes = send(new_fd, chunk, min(file_size-sent_bytes,MAXDATASIZE), 0)) == -1) {
			error("send");
			return false;
		}
		int next_batch_of_bytes = 0;
		while (numbytes < MAXDATASIZE && (sent_bytes + numbytes) < file_size) {
			cout << "\t\tonly " << numbytes << " B were sent" << endl;
			cout << "\t\tattempting to send the rest of the chunk" << endl;
			int remaining_unsent_bytes = MAXDATASIZE - numbytes;
			if ((next_batch_of_bytes = send(new_fd, chunk+numbytes, remaining_unsent_bytes, 0)) == -1) {
				error("send");
				return false;
			}
			numbytes += next_batch_of_bytes;
			cout << "\t\tyet another " << next_batch_of_bytes << " B of data were sent" << endl;
		}
		sent_bytes += numbytes;
		cout << "\tchunk sent successully: " << numbytes << " B sent; total sent bytes: " << sent_bytes << " B;" << file_size-sent_bytes << " B remaining" << endl;
	}

	return true;
}
