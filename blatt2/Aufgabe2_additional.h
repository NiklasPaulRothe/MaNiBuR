#ifndef AUFGABE_2_ADDITIONAL_H_
#define AUFGABE_2_ADDITIONAL_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/sha.h>
#include <sys/stat.h>

// code inspired by Ted Percival from stackoverflow
// https://stackoverflow.com/questions/8236/how-do-you-determine-the-size-of-a-file-in-c
int file_size(const char *filename) {
	struct stat st;

	if (stat(filename, &st) == 0) {
		return st.st_size;
	}

	return -1;
}

/*
	creates the sha512 string
*/
void create_sha512(char* filename, unsigned char* storage)
{
	int filesize = file_size(filename);

	FILE *f;
	f = fopen(filename, "r");
	unsigned char content[filesize];

	int i = 0;
	char temp;
	while ((temp = fgetc(f)) != EOF) {
		content[i] = temp;
		i++;
	}

	unsigned char sha512_value[SHA512_DIGEST_LENGTH];
	SHA512(content, filesize, sha512_value);

	char *string = create_sha512_string(sha512_value);

	printf("\nsha 512 value:\n");
	printf(string);
	printf("\n");

	fclose(f);
}

char handle_sha512(char* filepath, unsigned char* received_sha) {
	unsigned char sha_value[64];
	create_sha512(filepath, sha_value);

	if (memcmp(sha_value, received_sha, 64) == 0) {
		printf("Übertragung erfolgreich\n");		
		return SHA512_CMP_OK;
	} else {
		printf("Übertragung fehlgeschlagen\n");
		return SHA512_CMP_ERROR;
	}
}

void create_header_msg(char *msg, char *directory, char *zip_filename)
{
	/*
		sending length of the file name
	*/
	strcat(directory, ".tar.gz");
	unsigned short temp = strlen(directory);
	msg[1] = temp;
	temp >>= 8;
	msg[0] = temp;

	/*
		sending name of the file
	*/
	for (temp = 0; temp < strlen(directory); temp++) {
		msg[temp+2] = directory[temp];
	}

	/*
		sending fsize of the file
	*/
	unsigned int size = file_size(zip_filename);
	printf("Filesize: %u\n", size);
	msg[2+strlen(directory)+3] = size;
	size >>= 8;
	msg[2+strlen(directory)+2] = size;
	size >>= 8;
	msg[2+strlen(directory)+1] = size;
	size >>= 8;
	msg[2+strlen(directory)] = size;
	size >>= 8;	
}

/*
	returns the name_len stored in the msg
*/
unsigned short extract_header_name_len(char *msg) 
{
	/*
		combining msg[1] and msg[2] to build the unsigned short for the length
		of the name
	*/
	unsigned short name_len = 0; 
	name_len = name_len | msg[0];
	name_len <<= 8;
	name_len = name_len | msg[1];
	return name_len;
}

/*
	saves the name in the given char *name and returns the file size
*/
unsigned int extract_header_name_file_size(char *msg, char *name, unsigned short name_len)
{

	/*
		extracting the filename and create a string
	*/
	unsigned short index;
	for (index = 0; index < name_len; index++) {
		name[index] = msg[index + 2];
	}
	name[index] = '\0';

	/*
		combining the last 4 bytes to get the size of the file
	*/
	index = index + 2;
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
	return file_size;
}

#endif