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
	char *adresse;
	if (argc == 3) {	
		adresse = argv[1];
		port = atoi(argv[2]);
	} else {
		printf("Not enough arguments given or to many arguments\n");
		printf("Please use the program like this: \n");
		printf("./receiver_udp <adress for the server> <port of the server> \n");
		printf("The server adress should be given in dotted decimal(e.g.: 127.0.0.1)");
	}


/* 
	socket creation
*/
	int udp_socket, err;
	struct sockaddr_in addr;
	udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (udp_socket < 0) { 
		printf("Error by receiver socket creation\n");
	}


/*
	sending request (1)
*/
	struct sockaddr_in destination;

	destination.sin_family = AF_INET;
	destination.sin_port = htons(port);
	destination.sin_addr.s_addr = inet_addr(adresse);
	unsigned char request_msg;
	request_msg = REQUEST_T;

	err = sendto(udp_socket, &request_msg, sizeof(unsigned char)+1, 0, (struct sockaddr*) &destination, sizeof(struct sockaddr_in));
	if (err < 0) {
		printf("Error by receiver sendto\n");
	}




/* 
	receiving header (2)
*/
	char msg[32];
	int len, flen;
	struct sockaddr_in from;

	flen = sizeof(struct sockaddr_in);

	len = recvfrom(udp_socket, msg, sizeof(msg), 0, (struct sockaddr*) &from, &flen);

	if (len < 0) {
		printf("Error by receiver receiving");
	}

	/*
		get header
	*/
	unsigned char head = msg[0];

	/*
		get name length
	*/
	unsigned short name_len = 0;
	name_len = name_len | msg[1];
	name_len <<= 8;
	name_len = name_len | msg[2];
	
	/*
		get file name
	*/
	char *name;
	unsigned short index;
	for (index = 0; index < name_len; index++) {
		name[index] = msg[index+3];
		printf(" ");
	}
	name[index] = '\0';

	/*
		get file size
	*/
	index = index + 3;
	unsigned int file_size = 0;
	file_size = file_size | msg[index];
	file_size <<= 8;
	file_size = file_size | msg[index+1];
	file_size <<= 8;
	file_size = file_size | msg[index+2];
	file_size <<= 8;
	file_size = file_size | msg[index+3];

	printf("Received %d bytes from host %s port %d: %s\n", len, inet_ntoa(from.sin_addr), ntohs(from.sin_port), name);
	printf("Header: %hhu, Name-length: %hu, File_size: %u \n", head, name_len, file_size);

/*
	receiving file
*/
	



/* 
	closing the socket
*/
	err = close(udp_socket);
	if (err < 0) {
		printf("Error by receiver socket close\n");
	}
 
	printf("receiver_udp finished!\n");

}
