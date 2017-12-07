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
	Waiting for message (2)
*/
	unsigned int len;
	char msg[64];
	len = read(tcp_socket, msg, 64);

	/*
		extracting name_len, name and size (2)
	*/

	unsigned short name_len = 0;
	name_len = extract_header_name_len(msg);
	char *name = malloc((name_len + 1) * sizeof(char));
	unsigned int file_size = extract_header_name_file_size(msg, name, name_len);

	printf("Received %u bytes: %s\n", len, name);
	printf("Name-length: %hu, File_size: %u \n", name_len, file_size);

/*
	receiving file (4)
*/
	printf("receiving data start\n");

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

	int end_of_file = 0;
	while(end_of_file == 0) {
		
		// holds the length of the actual packet
		int packet_len;
		// holds the data of the actual packet
		char *msg_data;

		// allocating memory for the expected data and receiving it
		if (file_size_temp > 1492) {
			// if packet is at max size (probably more coming)
			msg_data = malloc(1492 * sizeof(char)); 
			file_size_temp = file_size_temp - 1492;
			packet_len = read(tcp_socket, msg_data, (1492 * sizeof(char)));
		} else {
			// if packet isnt at max size
			msg_data = malloc((file_size_temp) * sizeof(char));
			packet_len = read(tcp_socket, msg_data, ((file_size_temp) * sizeof(char)));
		}

		// if the packet_len is negative, smth went wrong by receiving the packet
		if (packet_len < 0) {
			printf("Error by receiver receiving\n"); //(1)
		}

		printf("packet_len: %d\n", packet_len);

	/*
		putting data into file
	*/
		int i;
		printf("start writing\n");		
		for (i = 0; i < packet_len; i++) {
			printf("Byte #: %d - %c \n", i, msg_data[i]);
			fputc(msg_data[i], f);
			file_position++;			
			if (file_position >= file_size) {
				printf("EOF \n");
				end_of_file = 1;
				break;
			}
		}
		printf("end writing\n");
		
		free(msg_data);

	}
	fclose(f);


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