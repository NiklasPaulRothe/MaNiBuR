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
		printf("Error by receiver socket creation\n"); //(1)
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
		printf("Error by receiver sendto\n"); //(1)
	}




/* 
	receiving header (2)
*/
	char msg[32];
	int len, flen;
	struct sockaddr_in from;

	flen = sizeof(struct sockaddr_in);

	len = recvfrom(udp_socket, msg, sizeof(msg)+1, 0, (struct sockaddr*) &from, &flen);

	if (len < 0) {
		printf("Error by receiver receiving"); //(1)
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
	char *name, *name_start;
	name_start = malloc((name_len+1) * sizeof(char));
	name = name_start;
	unsigned short index;
	for (index = 0; index < name_len; index++) {
		name[index] = msg[index+3];
	}
	name[index] = '\0';

	/*
		get file size
	*/
	index = index + 3;
	unsigned int file_size = 0;
	file_size = file_size | (unsigned char)msg[index];
	file_size <<= 8;
	index++;
	file_size = file_size | (unsigned char)msg[index];
	file_size <<= 8;
	index++;
	file_size = file_size | (unsigned char)msg[index];
	file_size <<= 8;
	index++;
	file_size = file_size | (unsigned char)msg[index];

	printf("Received %d bytes from host %s port %d: %s\n", len, inet_ntoa(from.sin_addr), ntohs(from.sin_port), name);
	printf("Header: %hhu, Name-length: %hu, File_size: %u \n", head, name_len, file_size);

/*
	create file
*/
	char truncate[name_len+22];
	sprintf(truncate, "truncate -s 0 received/%s", name);

	system(truncate);


/*
	receiving file (4)
*/
	printf("receiving data start\n");
	unsigned int needed_number = 0;
	unsigned int file_pointer = 0;
	while (1) {
		/*
			receiving a packet
		*/
		printf("package %u is needed now\n", needed_number);
		int data_len;
		char *msg_data, *msg_data_start;
		msg_data_start = malloc(1492 * sizeof(char)); //TODO: richtige size
		msg_data = msg_data_start;
		data_len = recvfrom(udp_socket, msg_data, sizeof(msg_data)+1, 0, (struct sockaddr*) &from, &flen);
		if (data_len < 0) {
			printf("Error by receiver receiving\n"); //(1)
		}
		/*
			checks if packet is a Data packet or not
		*/
		if (msg_data[0] != DATA_T) {
			free(msg_data_start);		
			printf("not a data package\n"); //(1)
			break;			
		}
		/*
			checks if its the right packet (in order)
		*/
		unsigned int package_number = 0;
		package_number = package_number | (unsigned char)msg_data[1];
		package_number <<= 8;
		package_number = package_number | (unsigned char)msg_data[2];
		package_number <<= 8;
		package_number = package_number | (unsigned char)msg_data[3];
		package_number <<= 8;
		package_number = package_number | (unsigned char)msg_data[4];
		if (package_number != needed_number) {
			printf(order_error, package_number, needed_number);			
			exit(EXIT_FAILURE); //(1)
		}
		needed_number++;
		
		/*
			putting data into file
		*/

		char filepath[name_len+9];
		sprintf(filepath, "received/%s", name);
		FILE *f;
		f = fopen(filepath, "a");

		int i;
		printf("start writing\n");		
		printf("msg_data_start is %i long\n", strlen(msg_data_start));
		for (i = 5; i < strlen(msg_data_start); i++) {
			char temp = fputc(msg_data_start[i], f);
			printf("%c written to file\n", temp);
		}
		printf("end writing\n");
		fclose(f);

		free(msg_data_start);
	}	



/* 
	closing the socket
*/
	err = close(udp_socket);
	if (err < 0) {
		printf("Error by receiver socket close\n");
	}
 
	free(name_start); // TODO: richtigen platz finden


	printf("receiver_udp finished!\n");

}

// TODO: 
// (1) Exits bei error mit socketclose.