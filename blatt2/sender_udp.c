#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/stat.h>

#include "Aufgabe2.h"

//code inspired by Ted Percival from stackoverflow
//https://stackoverflow.com/questions/8236/how-do-you-determine-the-size-of-a-file-in-c
int file_size(const char *filename) {
	struct stat st;

	if (stat(filename, &st) == 0) {
		return st.st_size;
	}

	return -1;
}

int main(int argc, char **argv)
{	

/*
	Getting params
*/
	int port;
	char *directory;
	int package_size;
	if (argc == 3) {
		port = atoi(argv[1]);	
		directory = argv[2];
		package_size = 0;
	} else {
		printf("Not enough arguments given or to many arguments\n");
		printf("Please use the program like this: \n");
		printf("./sender_udp <port for the sender> <path of the file> \n");
		exit(EXIT_FAILURE);
	}



/* 
	socket creation
*/
	int udp_socket, err;
	struct sockaddr_in addr;

	udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (udp_socket < 0) { 
		printf("Error by sender socket creation\n");
		exit(EXIT_FAILURE);
	}



/* 
	binding the socket
*/
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	err = bind(udp_socket, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));
	if (err < 0) {
		printf("Error by sender bind\n");
		close(udp_socket);
		exit(EXIT_FAILURE);
	}

/*
	compress file
*/
	//TODO: vernünftiges zippen!, rm Befehl beim closen beachten
	system("tar -czf hallo.tar.gz hallo.txt");

/* 
	receiving request (1)
*/
	char request_msg[64];
	int len, flen;
	struct sockaddr_in from;

	flen = sizeof(struct sockaddr_in);

	len = recvfrom(udp_socket, request_msg, sizeof(request_msg), 0, (struct sockaddr*) &from, &flen);

	if (len < 0) {
		printf("Error by sender receiving");
		close(udp_socket);
		exit(EXIT_FAILURE);
	}

/*
	sending archive
*/
	if (*request_msg == REQUEST_T) {
		printf("REQUEST_T by receiver\n");
		/*
			sending header (2)
		*/
		char *msg;
		struct sockaddr_in destination;
		msg[0] = HEADER_T;
		

		/*
			sending name length
		*/
		unsigned short temp = strlen(directory);
		msg[2] = temp;
		temp >>= 8;
		msg[1] = temp;

		/*
			sending name
		*/
		for (temp = 0; temp < strlen(directory); temp++) {
			msg[temp+3] = directory[temp];
		}

		/*
			sending file size
		*/
		unsigned int size = file_size("hallo.tar.gz");
		printf("size: %u\n", size);
		msg[3+strlen(directory)+3] = size;
		size >>= 8;
		msg[3+strlen(directory)+2] = size;
		size >>= 8;
		msg[3+strlen(directory)+1] = size;
		size >>= 8;
		msg[3+strlen(directory)] = size;
		size >>= 8;
		printf("msg: %u\n", msg[11]);

		


		destination.sin_family = AF_INET;
		destination.sin_port = from.sin_port;
		destination.sin_addr.s_addr = inet_addr(inet_ntoa(from.sin_addr));


		
		int msg_len = sizeof(unsigned char) + sizeof(unsigned short) + strlen(directory) + sizeof(unsigned int);
		printf("msg_len %d\n", msg_len);
		err = sendto(udp_socket, msg, msg_len, 0, (struct sockaddr*) &destination, sizeof(struct sockaddr_in));
		if (err < 0) {
			printf("Error by sender sendto\n");
			close(udp_socket);
			exit(EXIT_FAILURE);
		}	

	/*
		sending file
	*/	FILE *f;
		f = fopen("hallo.tar.gz", "r");
		
		int filesize = file_size("hallo.tar.gz");
		int packages = filesize / 1487;
		int tmp = filesize % 1487;

		if (tmp != 0) {
			printf("test\n");
			(int)packages++;
		}
		
		printf("%d\n", (int)packages);
		printf("test\n");
		exit(EXIT_FAILURE);
		
		unsigned int count;
		for (count = 0; count < packages; count++) {
			printf("test\n");
			char *msg;

			msg[0] = DATA_T;
			msg[4] = count;
			count >>= 8;
			msg[3] = count;
			count >>= 8;
			msg[2] = count;
			count >>= 8;
			msg[1] = count;
			int i;
			printf("test %s\n",msg);
			exit(EXIT_FAILURE);
			//int temp = fgetc(f);
			//printf("%d\n", temp);
			
			for (i = 0; i < 1487; i++) {
				printf("test\n");
				exit(EXIT_FAILURE);
				//if(fgetc(f) != EOF){
				//	msg[i + 5] = fgetc(f);
				//} else {
				//	fclose(f);
				//	break;
				//}
			}
			
			//err = sendto(udp_socket, msg, file_size, 0, (struct sockaddr*) &destination, sizeof(struct sockaddr_in));
		}
		


	}



/* 
	closing the socket
*/
	//TODO: an Dateinamen koppeln
	system("rm hallo.tar.gz");
	err = close(udp_socket);
	if (err < 0) {
		printf("Error by sender socket close\n");
	}

	printf("sender_udp finished!\n");

}
