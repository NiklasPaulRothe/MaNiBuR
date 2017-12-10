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

	tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_socket < 0) { 
		printf("Error by sender socket creation\n");
		exit(0);	
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
		close(tcp_socket);
		exit(0);
	}

/*
	Waiting for message (2)
*/
	unsigned int len;
	char *msg;
	msg = malloc(1492 * sizeof(char));
	len = read(tcp_socket, msg, 1492);
	if (len < 0) {
		printf("Error receiving message\n");
		close(tcp_socket);
		exit(0);
	}

	/*
		extracting name_len, name and size (2)
	*/

	unsigned short name_len = 0;
	name_len = extract_header_name_len(msg);
	char *name = malloc((name_len + 1) * sizeof(char));
	unsigned int file_size = extract_header_name_file_size(msg, name, name_len);

	free(msg);

/*
	receiving file (4)
*/
	struct timeval timer;
	timer.tv_sec = 10;
	timer.tv_usec = 0;
	err = setsockopt(tcp_socket, SOL_SOCKET, SO_RCVTIMEO, &timer, sizeof(timer));

	// the filepath to the created file
	char filepath[name_len + strlen(name_prefix)];
	printf("File: %s\n", name);
	sprintf(filepath, "%s%s", name_prefix, name);
	// free memory of name variable
	free(name);

	// open the file
	FILE *f;
	f = fopen(filepath, "w");
	printf("Filesize: %hu\n", file_size);

	int bytesread = 0;
	char *msg_data;
	int packet_len;
	msg_data = malloc(file_size * sizeof(char));
	while (bytesread < file_size) {
		packet_len = read(tcp_socket, msg_data + bytesread, file_size - bytesread);
		if (packet_len < 0) {
			printf("Error by receiver receiving\n"); //(1)
			free(msg_data);
			fclose(f);
			close(tcp_socket);
			exit(0);
		}
		bytesread += packet_len;

	}
	//write message in new file
	int i;	
	for (i = 0; i < file_size; i++) {
		fputc(msg_data[i], f);		
	}
	free(msg_data);
	fclose(f);

/*
	receiving sha512 value
*/

	unsigned char *sha512;
	sha512 = malloc(64* sizeof(unsigned char));
	err = read(tcp_socket, sha512, 64 * sizeof(unsigned char));
	if (err < 0){
		printf("Error by receiver receiving sha512 value\n");
		close(tcp_socket);
		exit(0);
	}

/*
	compare SHA512 value and send compare value
*/
	char* cmp;
	cmp = malloc(sizeof(char));
	cmp[0] = handle_sha512(filepath, sha512);
	err = write(tcp_socket, cmp , 1);
	if (err < 0) {
		printf("error sending compare value\n");
		close(tcp_socket);
		exit(0);
	}
	free(sha512);
	free(cmp);


/* 
	closing the socket
*/

	// closing the socket
	err = close(tcp_socket);
	if (err < 0) {
		printf("Error by sender socket close\n");
	} 

}