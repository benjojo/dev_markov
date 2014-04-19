obj-m = devMkov.o

all:
		make -c /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
		make -c /lib/modules/$(shell uname -r)/build M=$(PWD) clean

