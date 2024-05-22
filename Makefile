obj-m += chardemo.o

.PHONY: all install clean

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

install:
	sudo make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules_install
	sudo depmod -A

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
