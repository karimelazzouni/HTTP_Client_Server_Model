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
			cout << "waiting for HTTP request" << endl;

			char buf_req[MAXREQUESTSIZE];
			char buf_rest[MAXREQUESTSIZE];
			while (true) { // request line detection
				char received_chunk[MAXDATASIZE];
				memset(received_chunk, 0, MAXDATASIZE);
				receive_data(received_chunk, MAXDATASIZE); // receive request line
//				cout << "\treceived 1 : \"" << received_chunk << "\"" << endl;
				string s = received_chunk;

				int pos = s.find("\r\n", 0);
				if (pos != string::npos) {
					string s1, s2;
					s1 = s.substr(0, pos);
					s2 = s.substr(pos);
					strcpy(buf_req, s1.c_str());
					strcpy(buf_rest, s2.c_str());
//					cout << "\tbuf_req: \"" << buf_req << "\"" << endl;
//					cout << "\tbuf_rest: \"" << buf_rest << "\"" << endl;
					break;
				} else {
					strcat(buf_req, received_chunk);
				}
			}
			// find out whether the received request is GET or POST
			vector<string> v;
			FileHandler::split(buf_req, ' ', v);
			if (v[0].compare("GET") == 0) {
				// GET request detected
				cout << "GET request detected" << endl;
				cout << "###################################" << endl
						<< buf_req;
				string file = WORKINGDIRECTORY + v[1];
				while (true) {
					string s(buf_rest);
//					cout << "\"" << s << "\"" << endl;
					int pos = s.find("\r\n\r\n", 0);
					if (pos != string::npos) { // we reached the end of the GET request, discard the rest
						string headers;
						headers = s.substr(0, pos);
						strcat(buf_rest, headers.c_str());
						cout << s << "###################################"
								<< endl;
						break;
					}
					char received_chunk[MAXDATASIZE];
					memset(received_chunk, 0, MAXDATASIZE);

					receive_data(received_chunk, MAXDATASIZE); // receive the rest of the request
					strcat(buf_rest, received_chunk);

				}

				cout << "server: requesting file: " << file << endl;
				//Check for file existence
				//File exists
				if (FileHandler::fexists(file)) {
					cout << "Exists" << file << endl;

					//Send File
					ifstream file_stream;
					FileHandler::open_file_to_read(&file_stream, file);
					if (!file_stream.is_open()) {
						error("Could not open file");
						return false;
					}
					int file_size = FileHandler::get_file_size(&file_stream);
					cout << "sending actual file..." << endl;
					const char* message = construct_message(200).c_str();
					send_data(message);
					send_file(file_size, &file_stream);
					cout << "file was sent successfully" << endl;
				}
				//File doesn't exist
				else {
					cout << "Doesn't Exist" << endl;
					string not_found = construct_message(404);
					if (send(new_fd, not_found.c_str(),
							strlen(not_found.c_str()), 0) == -1)
						error("send");
				}
			} else if (v[0].compare("POST") == 0) {
				// POST request detected
				cout << "POST request detected" << endl;
				cout << "###################################" << endl
						<< buf_req;
				string file_name = WORKINGDIRECTORY+v[1];
				string file_size;
				bool valid_request = false;
				char file[MAXREQUESTSIZE];
				/////////////////////////////////////////////////
				//Getting headers
				while (true) {
					string s(buf_rest);
					cout << s.length()<<"\"" << s << "\"" << endl;
					int pos = s.find("\r\n\r\n", 0);
					if (pos != string::npos) { // we reached the end of the POST request, get the length from the headers and receive the file
						string headers;
						headers = s.substr(0, pos);
						char header_char_arr [MAXREQUESTSIZE];
						strcpy(header_char_arr,headers.c_str());
						// extract Content-length from the headers, else report a bad request
						v.clear();
						cout << "\theaders: " << header_char_arr << endl;
						FileHandler::split_string(header_char_arr, "\r\n", v);
						for (int i = 0; i < v.size();i++) {
							vector<string> v2;
							FileHandler::split(v[i],':',v2);
							if (v2.size() > 0 && v2[0].compare("Content-length") == 0) { // found Content-length header, request is valid
								file_size = v2[1];
								valid_request = true;
								break;
							}

						}
						if (!valid_request) {
							string message = construct_message(400);
							send_data(message.c_str());

						}
						else {
							//Begining of file
							string temp = "";
							cout<<"LL:"<<pos<<" "<<s<<endl;
							temp = s.substr(pos + 4);
							strcpy(file, temp.c_str());
							strcat(buf_rest, headers.c_str());
							int size_int = atoi(file_size.c_str());
							cout << "\tsize: " << size_int << endl;

							cout << s << "###################################" << endl;
							int size_written = FileHandler::create_file_from_buf(file_name, file, strlen(file),size_int);
//							cout<<"FILE: "<<sizeof(file)<<" Written: "<<size_written<<" size int: "<<size_int<<endl;
							cout<<":::"<<file_name<<endl;
							if((receive_file(file_name, size_int-size_written)) == false) {
								cout << "server: An error has occurred and the client may have disconnected" << endl;
							}
							string message = construct_message(200);
							send_data(message.c_str());
						}
						break;
					}

					char received_chunk[MAXREQUESTSIZE];
					memset(received_chunk, 0, MAXREQUESTSIZE);

					receive_data(received_chunk, MAXREQUESTSIZE); // receive the rest of the request
					strcat(buf_rest, received_chunk);

				}
				/////////////////////////////////////////////////


			} else { //neither
					 // return 400 bad request
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

bool Server::send_data(const char* buf) {
	cout << "sending data..." << endl;
	int numbytes = 0;
	if ((numbytes = send(new_fd, buf, strlen(buf), 0)) == -1) {
		error("send: could not send data");
		return false;
	}
	return true;

}

bool Server::send_file(int file_size, ifstream* file_stream) {
	cout << "sending chunks..." << endl;
	int sent_bytes = 0;
	char chunk[MAXDATASIZE];
	while (sent_bytes < file_size) {
		FileHandler::read_chunk_in_memory(file_stream, chunk,
				min(file_size - sent_bytes, MAXDATASIZE));
		int numbytes = 0;
		if ((numbytes = send(new_fd, chunk,
				min(file_size - sent_bytes, MAXDATASIZE), 0)) == -1) {
			error("send");
			return false;
		}
		int next_batch_of_bytes = 0;
		while (numbytes < MAXDATASIZE && (sent_bytes + numbytes) < file_size) {
			cout << "\t\tonly " << numbytes << " B were sent" << endl;
			cout << "\t\tattempting to send the rest of the chunk" << endl;
			int remaining_unsent_bytes = MAXDATASIZE - numbytes;
			if ((next_batch_of_bytes = send(new_fd, chunk + numbytes,
					remaining_unsent_bytes, 0)) == -1) {
				error("send");
				return false;
			}
			numbytes += next_batch_of_bytes;
			cout << "\t\tyet another " << next_batch_of_bytes
					<< " B of data were sent" << endl;
		}
		sent_bytes += numbytes;
		cout << "\tchunk sent successully: " << numbytes
				<< " B sent; total sent bytes: " << sent_bytes << " B;"
				<< file_size - sent_bytes << " B remaining" << endl;
	}

	return true;
}

bool Server::receive_data(char* buf_to_write, int bytes_received) {
	int numbytes = 0;
	if ((numbytes = recv(new_fd, buf_to_write, bytes_received, 0)) == -1) {
		error("recv");
		return false;
	}
	//cout<<"THERE "<<buf_to_write<<endl;
	buf_to_write[numbytes] = '\0';
	return true;
}

bool Server::receive_file(string file_name, int file_size) {
	cout << "receiving chunks..." << endl;
	char buf [MAXDATASIZE];
	int numbytes = 0, received_bytes = 0;
	while (received_bytes < file_size) { // receiving data
		if (((numbytes = recv(new_fd, buf,sizeof(buf), 0))) <= 0) { // whether an error has occurred or the client got disconnected
			return false;
		}
		int bytes_to_write = min(numbytes,file_size-received_bytes);
		cout<<"ss:"<<file_name<<endl;
		FileHandler::concat_to_existing_file(file_name, buf, bytes_to_write);
		cout << "\tchunk received successfully: " << numbytes << "B received" << endl;
		received_bytes += numbytes;
	}
	return true;

}

string Server::construct_message(int req_number) {
	string message = "";
	time_t now = time(0);
	string dt = ctime(&now);
	switch (req_number) {
	case 200: {
		// OK message
		message = "HTTP/1.0 200 OK\r\nDate: " + dt
				+ "Server: MAK-Server/1.0\r\n\r\n";
		break;
	}
	case 404: {
		// Not found
		message = "HTTP/1.0 404 Not Found\r\nDate: " + dt
				+ "Server: MAK-Server/1.0\r\n\r\n";
		break;
	}

	case 400: {
		// Bad request
		message = "HTTP/1.0 400 Bad Request\r\nDate: " + dt
				+ "Server: MAK-Server/1.0\r\n\r\n";
		break;

	}
	default:
		break;
	}
	return message;
}

