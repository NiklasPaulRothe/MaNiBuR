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
		exit(0);
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
		close(udp_socket);
		exit(0);
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
		close(udp_socket);
		exit(0);
	}


 	/*
 		extracting header
 	*/
	unsigned char head = msg[0];


	/*
		extracting name_len, name and size
	*/
	char *msg_tmp;
	msg_tmp = msg + 1;
	unsigned short name_len = 0;
	name_len = extract_header_name_len(msg_tmp);
	char *name = malloc((name_len + 1) * sizeof(char));
	unsigned int file_size = extract_header_name_file_size(msg_tmp, name, name_len);
	printf("Filename: %s\n", name);
	printf("Filesize: %hu\n", file_size);


/*
	receiving file (4)
*/
	struct timeval timer;
	timer.tv_sec = 10;
	timer.tv_usec = 0;
	err = setsockopt(udp_socket, SOL_SOCKET, SO_RCVTIMEO, &timer, sizeof(timer));

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
			close(udp_socket);
			exit(0);
		}


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
		for (i = 5; i < packet_len; i++) {
			fputc(msg_data[i], f);
			file_position++;			
			if (file_position >= file_size) {
				break;
			}
		}
		

		free(msg_data);
	}	
	fclose(f);

/*
	handle sha512 value
*/
	unsigned char sha_value[64];

	char sha_answer[2];
	sha_answer[0] = SHA512_CMP_T;
	sha_answer[1] = handle_sha512(filepath, sha_value);

	err = sendto(udp_socket, sha_answer, 2, 0, (struct sockaddr*) &destination, sizeof(struct sockaddr_in));


/* 
	closing the socket
*/
	err = close(udp_socket);
	if (err < 0) {
		printf("Error by receiver socket close\n");
	}

}

// TODO: 
// (1) Exits bei error mit socketclose.