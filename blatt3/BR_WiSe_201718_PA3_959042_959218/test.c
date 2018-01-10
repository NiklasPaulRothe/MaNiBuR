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



int normal(int argc, char **argv) {
	int fd = open("/dev/brpa3_959042_959218", O_RDWR);	
	//printf("inserted: %s\n", argv[1]);
	//write(fd, argv[1], strlen(argv[1]));
	//read(fd, argv[1], strlen(argv[1]));
	//printf("output: %s\n", argv[1]);
	

	brpa3_args v;
	//v.value = 8;
	//if (ioctl(fd, BRPA3_SET_OPENKEY, &v) == -1){
	//	perror("Error Set_Openkey 1");
	//}

	//argv[1] = temp;


	brpa3_args v_key;
	if (ioctl(fd, BRPA3_GET_OPENKEY, &v_key) == -1){
		perror("Error Get_Openkey");
	}
	printf("openkey: %hu\n", v_key.value);

	//v.value = 16;
	//if (ioctl(fd, BRPA3_SET_OPENKEY, &v) == -1){
	//	perror("Error Set_Openkey 2");
	//}

	v.value = 5;
	if (ioctl(fd, BRPA3_SET_SECRET, &v) == -1){
		perror("ERROR SET_SECRET");
	}

	printf("inserted: %s\n", argv[1]);
	write(fd, argv[1], strlen(argv[1]));
	read(fd, argv[1], strlen(argv[1]));
	printf("output: %s\n", argv[1]);




}

int multi(int argc, char **argv) {
	int fd = open("/dev/brpa3_959042_959218", O_RDWR);
	if (fork()) {
    	/* Parent is the writer */
	    while (1)
	        write(fd, argv[1], strlen(argv[1]));
	} else {
    	/* child is the reader */
	    while (1) {
	        read(fd, argv[1], strlen(argv[1]));
	        printf("Read: %s\n", argv[1]);
	    }
	}
}

int main(int argc, char **argv) {
	normal(argc, argv);
	//multi(argc, argv);
}