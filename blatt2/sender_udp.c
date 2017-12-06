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
	if (argc == 3) {
		port = atoi(argv[1]);	
		directory = argv[2];
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
	char tar[22 + strlen(directory)];
	sprintf(tar, "tar -czf hallo.tar.gz %s", directory);

	system(tar);

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
	sending archive (3)
*/
	if (*request_msg == REQUEST_T) {
		printf("REQUEST_T by receiver\n");
		/*
			sending header (2)
		*/
		char *msg_start, *msg;
		msg_start = malloc(1492 * sizeof(char));
		msg = msg_start;
		struct sockaddr_in destination;
		msg[0] = HEADER_T;
		

		/*
			sending name length
		*/
		strcat(directory, ".tar.gz");
		unsigned short temp = strlen(directory);
		msg[2] = temp;
		temp >>= 8;
		msg[1] = temp;

		/*
			sending name
		*/
		
		printf("dir: %s\n", directory);
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
		printf("size in msg: %hhu %hhu %hhu %hhu\n", msg[3+strlen(directory)], msg[3+strlen(directory)+1], msg[3+strlen(directory)+2], msg[3+strlen(directory)+3]);
		


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

		free(msg_start);
	/*
		sending file
	*/	
		FILE *f;
		f = fopen("hallo.tar.gz", "r");
		
		int filesize = file_size("hallo.tar.gz");
		int packages = filesize / 1487;
		int tmp = filesize % 1487;

		if (tmp != 0) {
			printf("one package will not be full\n");
			(int)packages++;
		}
		
		printf("number of packages: %d\n", (int)packages);
		//exit(EXIT_FAILURE);
		
		unsigned int count;
		for (count = 0; count < packages; count++) {
			printf("package %u of %i building and sending\n", count, (packages-1));
			char *msg_data, *msg_data_start;
			msg_data_start = malloc(1492 * sizeof(char));
			msg_data = msg_data_start;
			unsigned int temp_count = count;
			msg_data[0] = DATA_T;
			msg_data[4] = temp_count;
			temp_count >>= 8;
			msg_data[3] = temp_count;
			temp_count >>= 8;
			msg_data[2] = temp_count;
			temp_count >>= 8;
			msg_data[1] = temp_count;
			printf("number of packages in msg_data: %hhu\n", msg_data[4]);
			printf("Header + package count for data package \n");
			//exit(EXIT_FAILURE);
			//int temp = fgetc(f);
			//printf("%d\n", temp);
			
			int i, temp;
			for (i = 0; i < 1487; i++) {
				printf("test\n");
				//exit(EXIT_FAILURE);
				if((temp = fgetc(f)) != EOF){
					msg_data[i + 5] = temp;
					printf("byte %i in package %u is %c\n", i, count, msg_data_start[i + 5]);					
				} else {
					printf("EOF reached!\n");
					break;
				}
			}
			//printf("msg: ");
			//for (i = 5; i < filesize + 5; i++) {
			//	printf("%c", msg_data[i]);
			//}
			//printf("\n");
			//printf("msg: %s\n", msg_data_start);
			err = sendto(udp_socket, msg_data_start, filesize + 5, 0, (struct sockaddr*) &destination, sizeof(struct sockaddr_in));
			free(msg_data_start);
		}
		printf("closing file\n");
		fclose(f);
		err = sendto(udp_socket, "Ende", (strlen("Ende")*sizeof(char)), 0, (struct sockaddr*) &destination, sizeof(struct sockaddr_in));

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
