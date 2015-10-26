
#include "Client.h"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
using namespace std;

vector<string> split(string line) {
	vector<string> to_return;
	string word;
	for (unsigned int i = 0; i < line.length(); i++) {
		if (line.at(i) == ' ') {
			to_return.push_back(word);
			word = "";
			continue;
		}
		word += line[i];
	}
	to_return.push_back(word);
	return to_return;
}

string build_request(vector<string> vec, string* host_name,
		string* port_number) {
	int vector_size = vec.size();
	string request;
	std::vector<string>::iterator it = vec.begin();
	request += *it;
	++it;
	request += " " + *it;
	++it;
	if (vector_size == 2) {
		request += " " + *host_name + " " + *port_number;
	} else if (vector_size == 3) {
		*host_name = *it;
		request += " " + *it + " " + *port_number;
	} else if (vector_size == 4) {
		*host_name = *it;
		request += " " + *it;
		++it;
		*port_number = *it;
		request += " " + *it;
	}
	return request.c_str();
}

void connect(string file_name, string request, string host_name, string port_number) {
	Client c(file_name, request, host_name, port_number);
	bool success = c.initiate_connection();
	if (!success) {
		cout << "Could not initiate connection. Server will terminate." << endl;
		exit(2);
	}
	success = c.interact();
	if (!success) {
		cout << "Could not interact with server. Server will terminate."
				<< endl;
		exit(3);
	}
}

string modify_request(vector<string> splitted_request) {
	vector<string>::iterator it = splitted_request.begin();
	string get = "get";
	string post = "post";
	string request = "";
	if ((*it).compare(get) == 0) {
		request += "GET /";
		++it;
		request += *it + " HTTP/1.0\r\n\r\n";
	} else {
		request += "POST /";
		++it;
		request += *it + " HTTP/1.0\r\n\r\n";
	}
	return request;
}

void process_requests(string host_name, string port_number) {
	while (1) {
		printf("Type your request or quit to exit client.\n");
		string line;
		getline(cin, line);
		if (line == "quit") {
			break;
		}
		vector<string> splitted_line = split(line);
		string temp_host_name = host_name;
		string temp_port_number = port_number;
		line = build_request(splitted_line, &temp_host_name, &temp_port_number);
		transform(line.begin(), line.end(), line.begin(),
				(int (*)(int))tolower);
		vector<string> splitted_request = split(line);
		vector<string>::iterator it = splitted_request.begin();
		++it;
		string request = modify_request(splitted_request);
		connect(*it, request, temp_host_name, temp_port_number);
	}
}

int main(int argc, char* argv[]) {
	if (argc < 3) {
		cout
				<< "Error: Wrong params: Sever's IP address (or hostname) and port number must be provided"
				<< endl;
		exit(1);
	}
	process_requests(argv[1], argv[2]);
	return 0;

}

