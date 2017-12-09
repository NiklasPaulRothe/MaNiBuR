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
	char msg[1492];
	len = read(tcp_socket, msg, 1492);

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
	struct timeval timer;
	timer.tv_sec = 10;
	timer.tv_usec = 0;
	err = setsockopt(tcp_socket, SOL_SOCKET, SO_RCVTIMEO, &timer, sizeof(timer));

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
	int count_packages = 0;
	int bytesread = 0;
	char *msg_data;
	int packet_len;
	msg_data = malloc(file_size * sizeof(char));
	while (bytesread < file_size) {
		packet_len = read(tcp_socket, msg_data + bytesread, file_size - bytesread);
		if (packet_len < 0) {
			printf("Error by receiver receiving\n"); //(1)
		}
		bytesread += packet_len;

	}
	int i;
	printf("start writing\n");		
	for (i = 0; i < file_size; i++) {
		printf("Byte #: %d - %c \n", i, msg_data[i]);
		fputc(msg_data[i], f);		
	}
	printf("end writing\n");
	free(msg_data);
	fclose(f);

/*
	receiving sha512 value
*/

	unsigned char *sha512;
	sha512 = malloc(sizeof(char) * 64);
	err = read(tcp_socket, sha512, 64);
	if (err < 0){
		printf("Error by receiver receiving sha512 value\n");
	}

	char* cmp;
	cmp = malloc(sizeof(char));
	cmp[0] = handle_sha512(filepath, sha512);
	err= write(tcp_socket, cmp , 1);
	if (err < 0) {
		printf("error sending compare value\n");
	}
	free(cmp);


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