#ifndef AUFGABE_2_ADDITIONAL_H_
#define AUFGABE_2_ADDITIONAL_H_


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
int create_sha512(char* filename, unsigned char* storage)
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

	char sha512_value[SHA512_DIGEST_LENGTH];
	SHA512(content, filesize, sha512_value);

	char *string = create_sha512_string(sha512_value);

	printf("\nsha 512 value:\n");
	printf(string);
	printf("\n");

	fclose(f);
}



#endif