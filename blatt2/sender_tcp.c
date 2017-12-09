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
	compress the file
*/
	char tar[22 + strlen(directory)];
	sprintf(tar, "tar -czf %s %s", zip_filename, directory);
	system(tar);


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
	struct timeval timer;
	timer.tv_sec = 10;
	timer.tv_usec = 0;
	err = setsockopt(tcp_socket2, SOL_SOCKET, SO_RCVTIMEO, &timer, sizeof(timer));

/*
	sending first message (2)
*/

	char *msg;
	msg = malloc(1492 * sizeof(char));
	create_header_msg(msg, directory, zip_filename);

	int len;
	len = write(tcp_socket2, msg, 1492 * sizeof(char));
	printf("Send bytes: %d\n", len);

	free(msg);


/*
	sending data (3)
*/

	// open the already compressed file
	FILE *f;
	f = fopen(zip_filename, "r");
	
	// getting the file size to calculate the amount of packages that will be needed
	int filesize = file_size(zip_filename);
	int packages = filesize / 1492;
	int tmp = filesize % 1492;
	if (tmp != 0) {
		packages++;
	}

	printf("number of packages: %d\n", packages);
		
	// loop to create every package calculated above
	unsigned int count;
	for (count = 0; count < packages; count++) {
		//printf("package %u of %i building and sending\n", count, (packages-1));
			// allocating memory for the message
		char *msg_data;
		msg_data = malloc(1492 * sizeof(char));
		// insert the data
		int byte_count, temp;
		for (byte_count = 0; byte_count < 1492; byte_count++) {
			if((temp = fgetc(f)) != EOF){
				msg_data[byte_count] = temp;
				printf("byte %i in package %u is %c\n", byte_count, count, msg_data[byte_count]);					
			} else {
				printf("EOF reached!\n");
				break;
			}
		}
		// sending the message
		err = write(tcp_socket2, msg_data, byte_count * sizeof(char));
		if (err < 0) {
			printf("Something went wrong with sending the package!\n");
		}
		printf("Bytes send: %d\n", err);
		free(msg_data);
	}
	fclose(f);


/*
	create and send SHA512 value
*/
	unsigned char sha512[64];
	create_sha512(zip_filename, sha512);

	err = write(tcp_socket2, sha512, 64 * sizeof(char));
	if (err < 0) {
			printf("Error by sender sha512\n");
	}

	char* cmp;
	cmp = malloc(sizeof(char));
	err = read(tcp_socket2, cmp, sizeof(char));
	if (cmp[0] == SHA512_CMP_OK) {
		printf("Übertragung erfolgreich!\n");
	} else {
		printf("Übertragung fehlgeschlagen!\n");
	}
	free(cmp);
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