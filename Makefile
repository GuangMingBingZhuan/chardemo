obj-m += chardemo.o

.PHONY: all install clean

all: test chardemo.ko

test: test.o

chardemo.ko: chardemo.c chardemo.h
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

install:
	sudo make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules_install
	sudo depmod -A

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm test test.o
