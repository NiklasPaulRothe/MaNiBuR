#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "Aufgabe2.h"
#include "Aufgabe2_additional.h"

int main(int argc, char **argv)
{	
/*
	often used variable
*/
	// used to store the udp_socket
	int udp_socket;
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
	creating a socket in udp_socket
*/
	udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (udp_socket < 0) { 
		printf("Error by receiver socket creation\n"); //(1)
	}


/*
	sending request (1)
*/
	// Creating the destination with address and port given by the user
	struct sockaddr_in destination;

	destination.sin_family = AF_INET;
	destination.sin_addr.s_addr = inet_addr(adress);
	destination.sin_port = htons(port);
	
	// The request message is the REQUEST_T value
	unsigned char request_msg = REQUEST_T;	

	// Sending the request
	err = sendto(udp_socket, &request_msg, sizeof(unsigned char)+1, 0, (struct sockaddr*) &destination, sizeof(struct sockaddr_in));
	// Error handler if it fails
	if (err < 0) {
		printf("Error by receiver sendto\n"); //(1)
	}




/* 
	receiving header (2)
*/
	char msg[32]; //TODO: variable länge der message?
	int len;
	unsigned int flen;
	struct sockaddr_in from;

	flen = sizeof(struct sockaddr_in);


	len = recvfrom(udp_socket, msg, sizeof(msg)+1, 0, (struct sockaddr*) &from, &flen);

	if (len < 0) {
		printf("Error by receiver receiving"); //(1)
	}


 	/*
 		extracting header
 	*/
	unsigned char head = msg[0];


	/*
		combining msg[1] and msg[2] to build the unsigned short for the length
		of the name
	*/
	unsigned short name_len = 0;
	name_len = name_len | msg[1];
	name_len <<= 8;
	name_len = name_len | msg[2];
	
	/*
		extracting the filename and create a string
	*/
	char *name = malloc((name_len+1) * sizeof(char));
	unsigned short index;
	for (index = 0; index < name_len; index++) {
		name[index] = msg[index+3];
	}
	name[index] = '\0';

	/*
		combining the last 4 bytes to get the size of the file
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
	receiving file (4)
*/
	printf("receiving data start\n");

	// the packet number which is expectet next
	unsigned int needed_number = 0;
	// a position in the new file which is needed if multiple packets were send
	int file_position = 0;
	// a temporary file_size to save the original
	int file_size_temp = file_size;

	// the filepath to the created file
	char filepath[name_len + strlen(name_prefix)];
	sprintf(filepath, "%s%s", name_prefix, name);
	// free memory of name variable
	free(name);

	// open the file
	FILE *f;
	f = fopen(filepath, "w");

	// while loop is running until no package with DATA_T is received anymore
	while (1) {
		/*
			receiving a packet
		*/
		printf("package %u is needed now\n", needed_number);
		
		// holds the length of the actual packet
		int packet_len;
		// holds the data of the actual packet
		char *msg_data;

		// allocating memory for the expected data and receiving it
		if (file_size_temp > 1487) {
			// if packet is at max size (probably more coming)
			msg_data = malloc(1492 * sizeof(char)); 
			file_size_temp = file_size_temp - 1487;
			packet_len = recvfrom(udp_socket, msg_data, (1492 * sizeof(char)), 0, (struct sockaddr*) &from, &flen);
		} else {
			// if packet isnt at max size
			msg_data = malloc((file_size_temp + 5) * sizeof(char));
			packet_len = recvfrom(udp_socket, msg_data, ((file_size_temp + 5) * sizeof(char)), 0, (struct sockaddr*) &from, &flen);
		}

		// if the packet_len is negative, smth went wrong by receiving the packet
		if (packet_len < 0) {
			printf("Error by receiver receiving\n"); //(1)
		}

		printf("packet_len: %d\n", packet_len);


		// checks if packet is a DATA_T packet or not

		if (msg_data[0] == SHA512_T) {
			// packet is the sha value packet and the value will be stored
			int i;
			for	(i = 1; i < 64; i++) {				
				sha_input[i-1] = msg_data[i];
			}	
			break;
		} else if (msg_data[0] != DATA_T) {
			free(msg_data);		
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

		// increasing the counter for the next expected packet
		needed_number++;
		
		/*
			putting data into file
		*/
		int i;
		printf("start writing\n");		
		for (i = 5; i < packet_len; i++) {
			fputc(msg_data[i], f);
			file_position++;			
			if (file_position >= file_size) {
				break;
			}
		}
		printf("end writing\n");
		

		free(msg_data);
	}	
	fclose(f);

/*
	handle sha512 value
*/
	unsigned char sha_value[64];
	create_sha512(filepath, sha_value);

	char sha_answer[2];
	sha_answer[0] = SHA512_CMP_T;

	if (strcmp(sha_value, sha_input) == 0) {		
		sha_answer[1] = SHA512_CMP_OK;
		printf("gleich\n");
	} else {
		sha_answer[1] = SHA512_CMP_ERROR;
		printf("nicht gleich\n");
	}

	err = sendto(udp_socket, sha_answer, 2, 0, (struct sockaddr*) &destination, sizeof(struct sockaddr_in));


/* 
	closing the socket
*/
	err = close(udp_socket);
	if (err < 0) {
		printf("Error by receiver socket close\n");
	}


	printf("receiver_udp finished!\n");

}

// TODO: 
// (1) Exits bei error mit socketclose.