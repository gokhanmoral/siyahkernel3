ifneq ($(KERNELRELEASE),)
	obj-m := ffu.o
else
	KERNELDIR ?= ../../..
	PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
cscope:
	find *.[ch] > cscope.files
	cscope -b -i cscope.files
clean:
	rm *.o
	rm *.ko
	rm *.mod.c
endif
