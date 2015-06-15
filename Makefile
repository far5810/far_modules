# Makefile2.6
ifneq ($(KERNELRELEASE),)
#kbuild syntax. dependency relationshsip of files and target modules are listed here.

Hello-objs 			:= hello.o
FAR_MODULE-objs 	:= far_init.o far_proc.o

obj-m := Hello.o
obj-m += FAR_MODULE.o

else
#============= driver build vars
PWD  	:= $(shell pwd)
KVER 	?= $(shell uname -r)
KDIR 	:= /lib/modules/$(KVER)/build

#============= all target to build
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	@rm -rf .*.cmd *.o *.mod.c *.ko .tmp_versions *.order *.symvers

endif