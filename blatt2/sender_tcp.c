#include "Aufgabe2.h"
#include "Aufgabe2_additional.h"

int main(int argc, char **argv)
{
/*
	variables
*/
	// name of the temporary tar.gz file
	char *zip_filename = "temp.tar.gz";
	// port for the socket
	int port;
	// file path to the file which should be delivered
	char *directory;
	// used to store the tcp sockets
	int tcp_socket;
	int tcp_socket2;
	// err variable for several uses
	int err;
	// from holds the address of the receiver if a file is requested
	struct sockaddr_in from;
	socklen_t flen;
	flen = sizeof(struct sockaddr_in);

/*
	getting params
*/
	if (argc == 3) {
		port = atoi(argv[1]);	
		directory = argv[2];
	} else {
		printf("Not enough arguments given or to many arguments\n");
		printf("Please use the program like this: \n");
		printf("./sender_udp <port for the sender> <path of the file> \n");
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
	binding the socket to the port given by the user
*/
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	err = bind(tcp_socket, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));
	if (err < 0) {
		printf("Error by sender bind\n");
		close(tcp_socket);
	} else {
		printf("binding successfull!\n");
	}
/*
	listening for connection
*/

	err = listen(tcp_socket, 1);
	if (err < 0) {
		printf("listen failed!\n");
	} else {
		printf("listen successfull!\n");
	}
/*
	accepting a connection
*/
	tcp_socket2 = accept(tcp_socket, (struct sockaddr *) &from, &flen);
	if (tcp_socket2 < 0) {
		printf("accept failed!\n");
	} else {
		printf("Received connection from %s!\n", inet_ntoa(from.sin_addr));
	} 

/*
	sending first message
*/

	char *msg = "This went very successfull! Congrats!";

	unsigned int len;
	len = write(tcp_socket2, msg, strlen(msg));



/*
	closing file socket
*/
	err = close(tcp_socket2);
	if (err < 0) {
		printf("Error by sender file socket close\n");
	}


/* 
	closing the initial socket
*/
	// removing the .tar.gz file	
	char rm[3 + strlen(zip_filename)];
	strcpy(rm, "rm ");
	strcat(rm, zip_filename);
	//system(rm);

	// closing the socket
	err = close(tcp_socket);
	if (err < 0) {
		printf("Error by initial socket close\n");
	} 

	printf("sender_tcp finished!\n");

}