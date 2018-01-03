#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>

#include "brpa3_959042_959218_header.h"
#include "mod_exp.h"

int main(int argc, char **argv) {
	char* temp = malloc(strlen(argv[1]) * sizeof(char));
	int i;
	for (i=0;i<strlen(argv[1]);i++){
		temp[i] = argv[1][i];	
	}

	
	int fd = open("/dev/brpa3_959042_959218", O_RDWR);	
	printf("inserted: %s\n", argv[1]);
	write(fd, argv[1], strlen(argv[1]));
	read(fd, argv[1], strlen(argv[1]));
	printf("output: %s\n", argv[1]);
	

	brpa3_args v;
	v.value = 8;
	if (ioctl(fd, BRPA3_SET_OPENKEY, &v) == -1){
		perror("Error Set_Openkey 1");
	}

	//argv[1] = temp;

	printf("inserted 2: %s\n", temp);
	write(fd, temp, strlen(temp));
	read(fd, temp, strlen(temp));
	printf("output 2: %s\n", temp);


	brpa3_args v_key;
	if (ioctl(fd, BRPA3_GET_OPENKEY, &v_key) == -1){
		perror("Error Get_Openkey");
	}
	printf("openkey: %hu\n", v_key.value);

	v.value = 16;
	if (ioctl(fd, BRPA3_SET_OPENKEY, &v) == -1){
		perror("Error Set_Openkey 2");
	}

	v.value = 10;
	if (ioctl(fd, BRPA3_SET_SECRET, &v) == -1){
		perror("ERROR SET_SECRET");
	}
	

}