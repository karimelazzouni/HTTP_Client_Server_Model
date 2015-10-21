#include "Client.h"

int main(int argc, char* argv[]) {
	if (argc < 3) {
		cout << "Error: Wrong params: Sever's IP address (or hostname) and port number must be provided" << endl;
		exit(1);
	}
	Client c(argv[1],argv[2]);
	bool success = c.initiate_connection();
	if (!success) {
		cout << "Could not initiate connection. Server will terminate." << endl;
		exit(2);
	}
	success = c.interact();
	if(!success) {
		cout << "Could not interact with server. Server will terminate." << endl;
		exit(3);
	}

	return 0;

}
