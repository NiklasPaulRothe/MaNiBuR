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
	// used to store the udp socket
	int udp_socket;
	// err variable for several uses
	int err;
	// from holds the address of the receiver if a file is requested
	struct sockaddr_in from;
	unsigned int flen;



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
	printf("Filename: %s\n", directory);



/* 
	creating a socket in udp_socket
*/
	struct sockaddr_in addr;

	udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (udp_socket < 0) { 
		printf("Error by sender socket creation\n");
		exit(0);	
	}




/* 
	binding the socket to the port given by the user
*/
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	err = bind(udp_socket, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));
	if (err < 0) {
		printf("Error by sender bind\n");
		close(udp_socket);
		exit(0);
	}

/*
	compress the file
*/
	char tar[22 + strlen(directory)];
	sprintf(tar, "tar -czf %s %s", zip_filename, directory);
	system(tar);

/* 
	receiving request (1)
*/
	char request_msg[64];
	int len;

	flen = sizeof(struct sockaddr_in);
	len = recvfrom(udp_socket, request_msg, sizeof(request_msg), 0, (struct sockaddr*) &from, &flen);

	if (len < 0) {
		printf("Error by sender receiving request\n");
		close(udp_socket);
		exit(0);
	}

/*
	Setting the timeout timer to 10 seconds
*/
		struct timeval timer;
		timer.tv_sec = 10;
		timer.tv_usec = 0;
		err = setsockopt(udp_socket, SOL_SOCKET, SO_RCVTIMEO, &timer, sizeof(timer));


/*
	sending archive (3)
*/
	if (*request_msg == REQUEST_T) {
		/*
			sending header (2)
		*/
		char *msg;
		msg = malloc(1492 * sizeof(char));
		struct sockaddr_in destination;
		msg[0] = HEADER_T;
		
		char *msg_tmp;
		msg_tmp = msg + 1; 

		create_header_msg(msg_tmp, directory, zip_filename);
		

		// creating the destination information from the received address
		destination.sin_family = AF_INET;
		destination.sin_port = from.sin_port;
		destination.sin_addr.s_addr = inet_addr(inet_ntoa(from.sin_addr));


		// length of this message is build from one char for the type-id, one short for the length of the name
		// the length of the string which holds the file name and a the int for the size of the file
		int msg_len = sizeof(unsigned char) + sizeof(unsigned short) + strlen(directory) + sizeof(unsigned int);
		err = sendto(udp_socket, msg, msg_len, 0, (struct sockaddr*) &destination, sizeof(struct sockaddr_in));
		if (err < 0) {
			printf("Error by sender sendto\n");
			close(udp_socket);
			exit(0);
		}	
		free(msg);



	/*
		sending file
	*/	
		// open the already compressed file
		FILE *f;
		f = fopen(zip_filename, "r");
		
		// getting the file size to calculate the amount of packages that will be needed
		int filesize = file_size(zip_filename);
		int packages = filesize / 1487;
		int tmp = filesize % 1487;
		if (tmp != 0) {
			packages++;
		}
		
		
		// loop to create every package calculated above
		unsigned int count;
		char *sha_string;
		sha_string = malloc(filesize * sizeof(char));
		for (count = 0; count < packages; count++) {

			// allocating memory for the message
			char *msg_data;
			msg_data = malloc(1492 * sizeof(char));

			// setting the header
			msg_data[0] = DATA_T;
			// setting the sequence number
			unsigned int temp_count = count;
			msg_data[4] = temp_count;
			temp_count >>= 8;
			msg_data[3] = temp_count;
			temp_count >>= 8;
			msg_data[2] = temp_count;
			temp_count >>= 8;
			msg_data[1] = temp_count;

			// insert the data
			int i, temp;
			for (i = 0; i < 1487; i++) {
	
				if((temp = fgetc(f)) != EOF){
					msg_data[i + 5] = temp;
					//printf("test\n");
					sha_string[i + (1487 * count)] = temp;
					//printf("sha: %c\n", sha_string[i + (1487 * count)]);
					//printf("sha string: %c\n", sha_string[i + (1487 * count)]);
					//printf("byte %i in package %u is %c\n", i, count, msg_data[i + 5]);					
				} else {
					break;
				}
			}
			// sending the message
			err = sendto(udp_socket, msg_data, filesize + 5, 0, (struct sockaddr*) &destination, sizeof(struct sockaddr_in));
			if (err < 0) {
				printf("Something went wrong with sending the package!\n");
				close(udp_socket);
				exit(0);
			}
			free(sha_string);
			free(msg_data);
		}

		fclose(f);

		
		/*
			sending the sha-512 value (4)
		*/
		unsigned char msg_sha[65];
		// setting the type-id to SHA512_T
		msg_sha[0] = SHA512_T;

		// create sha-512 value
		unsigned char sha512[64];		
		create_sha512(zip_filename, sha512);
		int i;
		for (i = 0; i < 64; i++) {
			msg_sha[i+1] = sha512[i];
		}
		// Sending the SHA Value
		err = sendto(udp_socket, msg_sha, 65 * sizeof(unsigned char), 0, (struct sockaddr*) &destination, sizeof(struct sockaddr_in));
		// catching the error
		if (err < 0) {
			printf("Error by sender sha512\n"); //(1)
			close(udp_socket);
			exit(0);
		}

/*
	Receiving sha anwer
*/	
		char sha_answer[2];
		err = recvfrom(udp_socket, sha_answer, (2 * sizeof(char)), 0, (struct sockaddr*) &from, &flen);
		if (sha_answer[0] == SHA512_CMP_T) {
			if (sha_answer[1] == SHA512_CMP_OK) {
				printf("Übertragung erfolgreich!\n");
			} else {
				printf("Übertragung fehlgeschlagen!\n");
			}
		} else {
			// TODO: FEHLERBEHANDLUNG
			printf("Wrong packet as answer!\n");
			close(udp_socket);
			exit(0);
		}

	}



/* 
	closing the socket
*/
	// removing the .tar.gz file	
	char rm[3 + strlen(zip_filename)];
	strcpy(rm, "rm ");
	strcat(rm, zip_filename);
	system(rm);

	// closing the socket
	err = close(udp_socket);
	if (err < 0) {
		printf("Error by sender socket close\n");
	}
}

//TODO: Error Handling wenn files nicht geöffnet werden können