/*
 * Client.cpp
 *
 *  Created on: Oct 21, 2015
 *      Author: karim
 */

#include "Client.h"

Client::Client(string file_name, vector<string> splitted_request,
		string server_ip, string port_num) {
	// TODO Auto-generated constructor stub
	this->servip = server_ip;
	this->portno = port_num;
	this->splitted_request = splitted_request;
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

	vector<string>::iterator it = splitted_request.begin();
	string get = "get";
	string post = "post";
	string request = "";
	string file_name = "";
	if ((*it).compare(get) == 0) {
		++it;
		string temp = (*it).c_str();
		if ((*it)[0] == '/') {
			request += "GET ";
			file_name = temp;

		} else {
			request += "GET /";
			file_name =  '/' + temp;
		}
		request += file_name + " HTTP/1.0\r\n";
		if (send(sockfd, request.c_str(), strlen(request.c_str()), 0) == -1)
			error("send");
		request = "\r\n";
		if (send(sockfd, request.c_str(), strlen(request.c_str()), 0) == -1)
			error("send");

		//////////////

		char buf_req[MAXREQUESTSIZE];
		char buf_rest[MAXREQUESTSIZE];
		while (true) { // response line detection
			char received_chunk[MAXREQUESTSIZE];
			memset(received_chunk, 0, MAXREQUESTSIZE);
			receive_data(received_chunk, MAXREQUESTSIZE); // receive request line
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
		cout << "" << endl;
		cout << "###################################" << endl << buf_req;

		vector<string> v;
		FileHandler::split(buf_req, ' ', v);
		//Response is OK

		cout << v[1] << endl;
		if (v[1].compare("200") == 0) {
			char file[MAXREQUESTSIZE];

			//Getting headers
			while (true) {
				string s(buf_rest);
				//					cout << "\"" << s << "\"" << endl;
				int pos = s.find("\r\n\r\n", 0);
				if (pos != string::npos) { // we reached the end of the GET request, discard the rest
					string headers;
					headers = s.substr(0, pos);
					//Begining of file
					string temp = "";
					temp = s.substr(pos + 4);
					strcpy(file, temp.c_str());
					strcat(buf_rest, headers.c_str());
					cout << s << "###################################" << endl;

					FileHandler::create_file_from_buf(WORKINGDIRECTORY+file_name, file,
							strlen(file));

					break;
				}

				char received_chunk[MAXREQUESTSIZE];
				memset(received_chunk, 0, MAXREQUESTSIZE);

				receive_data(received_chunk, MAXREQUESTSIZE); // receive the rest of the request
				strcat(buf_rest, received_chunk);

			}

			receive_file(WORKINGDIRECTORY+file_name);

		} else if (v[1].compare("404") == 0) {
			// file not found

		} else if (v[1].compare("400") == 0) {
			// bad request

		}

	} else if ((*it).compare(post) == 0) {
		++it;

		string temp = (*it).c_str();
		if ((*it)[0] == '/') {
			request += "POST ";
			file_name = temp;

		} else {
			request += "POST /";
			file_name = '/' + temp;
		}
		request += file_name + " HTTP/1.0\r\n";
		ifstream fs;
		FileHandler::open_file_to_read(&fs,WORKINGDIRECTORY+file_name);
		int file_size = FileHandler::get_file_size(&fs);

		string size_str;//string which will contain the result
		stringstream convert; // stringstream used for the conversion
		convert << file_size;//add the value of Number to the characters in the stream
		size_str = convert.str();

		request += "Content-length: " + size_str + "\r\n";
		request += "\r\n";
		send_data(request.c_str());
		send_file(file_size,&fs);
	}
	else {
		cout << "client: Undefined request, please use the following formats" << endl;
		cout << "client:\tget <file-name> <host-name> (<port-number>)" << endl;
		cout << "client:\tpost <file-name> <host-name> (<port-number>)" << endl;
		cout << "client:\twhere <host-name> and <port-number> are optional parameters" << endl;
	}

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

bool Client::receive_file(string file_name) {
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

bool Client::send_data(const char* buf) {
	cout << "sending data..." << endl;
	int numbytes = 0;
	if ((numbytes = send(sockfd, buf, strlen(buf), 0)) == -1) {
		error("send: could not send data");
		return false;
	}
	return true;

}

bool Client::send_file(int file_size, ifstream* file_stream) {
	cout << "sending chunks..." << endl;
	int sent_bytes = 0;
	char chunk[MAXDATASIZE];
	while (sent_bytes < file_size) {
		FileHandler::read_chunk_in_memory(file_stream, chunk,
				min(file_size - sent_bytes, MAXDATASIZE));
		int numbytes = 0;
		if ((numbytes = send(sockfd, chunk,
				min(file_size - sent_bytes, MAXDATASIZE), 0)) == -1) {
			error("send");
			return false;
		}
		int next_batch_of_bytes = 0;
		while (numbytes < MAXDATASIZE && (sent_bytes + numbytes) < file_size) {
			cout << "\t\tonly " << numbytes << " B were sent" << endl;
			cout << "\t\tattempting to send the rest of the chunk" << endl;
			int remaining_unsent_bytes = MAXDATASIZE - numbytes;
			if ((next_batch_of_bytes = send(sockfd, chunk + numbytes,
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

Client::~Client() {
	// TODO Auto-generated destructor stub
}

