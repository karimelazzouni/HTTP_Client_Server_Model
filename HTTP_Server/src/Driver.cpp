#include "Server.h"

int main(int argc, char* argv[]) {

	if (argc < 2) {
		cout << "Error: Wrong params: Port number must be provided" << endl;
		exit(1);
	}

	string server_port = argv[1];
	Server s (server_port);
	bool success = s.prepare_server();
	if (!success) {
		cerr << "Failed to initiate. Server will terminate." << endl;
		exit(1); // failed to initiate server
	}
	success = s.listening();
	if (!success) {
		cerr << "Failed to listen to connections. Server will terminate." << endl;
		exit(2); // failed to listen to connections
	}
	success = s.reap_dead_children();
	if (!success) {
		cerr << "Could not reap zombie processes. Server will terminate." << endl;
		exit(3); // failed to reap zombie children
	}

	cout << "Server: waiting for connections..." << endl;
	success = s.accept_connection();
	if (!success) {
		cerr << "Could not accept connection. Server will terminate." << endl;
		exit(3); // failed to accept connection
	}

	return 0;
}
