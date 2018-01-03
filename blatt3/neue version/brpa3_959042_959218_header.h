#ifndef BRPA3_H
#define BRPA3_H
#include <linux/ioctl.h>

typedef struct {
	unsigned short value;
} brpa3_args;


#define BRPA3_SET_SECRET _IOW('q', 1, brpa3_args *)
#define BRPA3_SET_OPENKEY _IOW('q', 2, brpa3_args *)
#define BRPA3_GET_OPENKEY _IOR('q', 3, brpa3_args *)

#endif
