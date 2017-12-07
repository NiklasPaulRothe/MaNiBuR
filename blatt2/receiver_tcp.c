#include "Aufgabe2.h"
#include "Aufgabe2_additional.h"

int main(int argc, char **argv)
{	
/*
	often used variable
*/
	// used to store the udp_socket
	int tcp_socket;
	// temporary store for errors
	int err;
	// folder in which the data will be stored
	char *name_prefix = "received/";
	// used to store the sha value
	char sha_input[64];


/*
	getting params
*/
	char *adress;
	int port;
	if (argc == 3) {	
		adress = argv[1];
		port = atoi(argv[2]);
	} else {
		printf("Not enough arguments given or to many arguments\n");
		printf("Please use the program like this: \n");
		printf("./receiver_udp <adress for the server> <port of the server> \n");
		printf("The server adress should be given in dotted decimal(e.g.: 127.0.0.1)");
	}

/* 
	creating a socket in tcp_socket
*/
	struct sockaddr_in addr;

	tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_socket < 0) { 
		printf("Error by sender socket creation\n");	
	} else {
		printf("socket creation successfull!\n");
	}
/*
	connecting to sender
*/
	// Building destination address
	struct sockaddr_in destination;

	destination.sin_family = AF_INET;
	destination.sin_addr.s_addr = inet_addr(adress);
	destination.sin_port = htons(port);

	err = connect(tcp_socket, (struct sockaddr*) &destination, sizeof(struct sockaddr_in));
	if (err < 0) {
		printf("connecting error!\n");
	} else {
		printf("connecting successfull!\n");
	}

/*
	Waiting for message
*/
	unsigned int len;
	char msg[64];
	len = read(tcp_socket, msg, 64);

	/*
		extracting name_len, name and size
	*/

	unsigned short name_len = 0;
	name_len = extract_header_name_len(msg);
	char *name = malloc((name_len + 1) * sizeof(char));
	unsigned int file_size = extract_header_name_file_size(msg, name, name_len);

	printf("Received %u bytes: %s\n", len, name);
	printf("Name-length: %hu, File_size: %u \n", name_len, file_size);



/* 
	closing the socket
*/

	// closing the socket
	err = close(tcp_socket);
	if (err < 0) {
		printf("Error by sender socket close\n");
	} 

	printf("receiver_tcp finished!\n");

}