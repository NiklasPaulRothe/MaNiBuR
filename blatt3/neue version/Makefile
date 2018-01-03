obj-m += brpa3_959042_959218.o
CFLAGS_brpa3_959042_959218.o += -DDEBUG

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean