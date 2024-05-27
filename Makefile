obj-m += chardemo.o

.PHONY: all install clean module_install rules_install

all: test chardemo.ko

install: module_install rules_install

clean:
	rm test test.o
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

test: test.o chardemo.h

chardemo.ko: chardemo.c chardemo.h
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

module_install: chardemo.ko
	sudo make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules_install
	sudo depmod -A

rules_install: 99-chardemo.rules
	sudo cp 99-chardemo.rules /lib/udev/rules.d/
