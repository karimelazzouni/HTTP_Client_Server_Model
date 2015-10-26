/*
 * Client.cpp
 *
 *  Created on: Oct 21, 2015
 *      Author: karim
 */

#include "Client.h"

Client::Client(string file_name, string request, string server_ip,
		string port_num) {
	// TODO Auto-generated constructor stub
	this->servip = server_ip;
	this->portno = port_num;
	this->request = request;
	this->file_name = file_name;
}

void *Client::get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

void Client::error(const char* message) {
	perror(message);
}

bool Client::initiate_connection() {
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(this->servip.c_str(), this->portno.c_str(), &hints,
			&servinfo)) != 0) {
		string str = gai_strerror(rv);
		string str2 = "getaddrinfo: " + str + "\n";
		error(str2.c_str());
		return false;
	}
	// loop through all the results and connect to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol))
				== -1) {
			error("client: socket");
			continue;
		}
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			error("client: connect");
			continue;
		}

		break;
	}
	if (p == NULL) {
		error("client: failed to connect\n");
		return false;
	}
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *) p->ai_addr), s,
			sizeof s);
	return true;
}

bool Client::interact() {
	printf("client: connecting to %s\n", s);
	freeaddrinfo(servinfo); // all done with this structure

	if (send(sockfd, request.c_str(), strlen(request.c_str()), 0) == -1)
		error("send");

	//Keep connection to get response
//	char buf[MAXDATASIZE];
//	int byte_count;

//	if ((byte_count = recv(sockfd, buf, sizeof buf, 0)) == -1) {
//		error("recv");
//		return false;
//	}
//	buf[byte_count] = '\0';

	bool success = receive_file(file_name, sockfd);
	if (!success) {
		error("client: cannot receive file.");
		return false;
	}
//	cout << buf << endl;

	//	close(sockfd);
	return true;
}

bool Client::receive_data(char* buf_to_write, int bytes_received) {
	if ((numbytes = recv(sockfd, buf_to_write, bytes_received, 0)) == -1) {
		error("recv");
		return false;
	}
	buf_to_write[numbytes] = '\0';
	return true;
}

bool Client::receive_file(string file_name, int sock_fd/*, int file_size*/) {
	cout << "receiving chunks..." << endl;
	while (((numbytes = recv(sockfd, buf,/*file_size*/sizeof(buf), 0))) > 0) { // receiving data
		FileHandler::concat_to_existing_file(file_name, buf, numbytes);
		cout << "\tchunk received successfully: " << numbytes << "B received"
				<< endl;
	}
	if (numbytes == 0) { // server has disconnected after sending the whole file
		cout << "file received successfully" << endl;
	} else { // numbytes == -1; some error has occurred
		error("recv");
		return false;
	}
	return true;

}

Client::~Client() {
	// TODO Auto-generated destructor stub
}

