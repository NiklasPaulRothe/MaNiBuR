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

	//display file name
	printf("File: %s\n", directory);

/* 
	creating a socket in tcp_socket
*/
	struct sockaddr_in addr;

	tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_socket < 0) { 
		printf("Error by sender socket creation\n");
		exit(0);	
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
		exit(0);
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
		close(tcp_socket);
		exit(0);
	}
/*
	accepting a connection
*/
	tcp_socket2 = accept(tcp_socket, (struct sockaddr *) &from, &flen);
	if (tcp_socket2 < 0) {
		printf("accept failed!\n");
		close(tcp_socket);
		exit(0);
	}

	//set waiting time for socket
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
		
	// loop to create every package calculated above
	unsigned int count;
	for (count = 0; count < packages; count++) {
		// allocating memory for the message
		char *msg_data;
		msg_data = malloc(1492 * sizeof(char));
		// insert the data
		int byte_count, temp;
		for (byte_count = 0; byte_count < 1492; byte_count++) {
			if((temp = fgetc(f)) != EOF){
				msg_data[byte_count] = temp;					
			} else {
				break;
			}
		}
		// sending the message
		err = write(tcp_socket2, msg_data, byte_count * sizeof(char));
		if (err < 0) {
			printf("Something went wrong with sending the package!\n");
			close(tcp_socket2);
			close(tcp_socket);
			exit(0);
		}
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
			close(tcp_socket2);
			close(tcp_socket);
			exit(0);
	}

/*
	receive SHA512 compare value and evaluate data transfer
*/
	char* cmp;
	cmp = malloc(sizeof(char));
	err = read(tcp_socket2, cmp, sizeof(char));
	if (cmp[0] == SHA512_CMP_OK) {
		printf("Übertragung erfolgreich!\n");
	} else {
		printf("Übertragung fehlgeschlagen!\n");
	}
	free(cmp);
	//wait until socket is empty before closing
	char* temp;
	temp = malloc(sizeof(char));
	while ((err = read(tcp_socket2, temp, sizeof(char))) > 0) {
		sleep(3);
	}
	free(temp);
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
	system(rm);

	// closing the socket
	err = close(tcp_socket);
	if (err < 0) {
		printf("Error by initial socket close\n");
	} 


}