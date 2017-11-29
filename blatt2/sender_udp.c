#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "Aufgabe2.h"

int main(int argc, char **argv)
{	

/*
	Getting params
*/
	int port;
	char *directory;
	int package_size;
	if (argc == 3) {
		port = atoi(argv[1]);	
		directory = argv[2];
		package_size = 0;
	} else {
		printf("Not enough arguments given or to many arguments\n");
		printf("Please use the program like this: \n");
		printf("./sender_udp <port for the sender> <path of the file> \n");
		exit(EXIT_FAILURE);
	}



/* 
	socket creation
*/
	int udp_socket, err;
	struct sockaddr_in addr;

	udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (udp_socket < 0) { 
		printf("Error by sender socket creation\n");
		exit(EXIT_FAILURE);
	}



/* 
	binding the socket
*/
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	err = bind(udp_socket, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));
	if (err < 0) {
		printf("Error by sender bind\n");
		close(udp_socket);
		exit(EXIT_FAILURE);
	}



/* 
	receiving request (1)
*/
	char request_msg[64];
	int len, flen;
	struct sockaddr_in from;

	flen = sizeof(struct sockaddr_in);

	len = recvfrom(udp_socket, request_msg, sizeof(request_msg), 0, (struct sockaddr*) &from, &flen);

	if (len < 0) {
		printf("Error by sender receiving");
		close(udp_socket);
		exit(EXIT_FAILURE);
	}

/*
	sending archive
*/
	if (*request_msg == REQUEST_T) {
		printf("REQUEST_T by receiver\n");
		/*
			sending header (2)
		*/
		char *msg;
		struct sockaddr_in destination;

		msg[0] = HEADER_T;
		unsigned short temp = strlen(directory);
		msg[1] = 1;
		msg[2] = 1;

		for (temp = 0; temp < strlen(directory); temp++) {
			msg[temp+3] = directory[temp];
		}

		msg[3+strlen(directory)] = 3;
		msg[3+strlen(directory)+1] = 3;
		msg[3+strlen(directory)+2] = 3;
		msg[3+strlen(directory)+3] = 3;


		destination.sin_family = AF_INET;
		destination.sin_port = from.sin_port;
		destination.sin_addr.s_addr = inet_addr(inet_ntoa(from.sin_addr));

		printf("%s\n", msg);

		err = sendto(udp_socket, msg, strlen(msg)+1, 0, (struct sockaddr*) &destination, sizeof(struct sockaddr_in));
		if (err < 0) {
			printf("Error by sender sendto\n");
			close(udp_socket);
			exit(EXIT_FAILURE);
		}	


	}



/* 
	closing the socket
*/
	err = close(udp_socket);
	if (err < 0) {
		printf("Error by sender socket close\n");
	}

	printf("sender_udp finished!\n");

}
