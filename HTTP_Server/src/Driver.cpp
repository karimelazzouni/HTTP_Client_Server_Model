#include "Server.h"

int main(int argc, char* argv[]) {

	if (argc < 2) {
		cout << "Error: Wrong params: Port number must be provided" << endl;
		exit(1);
	}

	int server_port = atoi(argv[1]);
	Server s (server_port);

	return 0;
}
