ifneq ($(KERNELRELEASE),)
obj-m := hello_version.o
else
KDIR := $(HOME)/buildroot/output/build/linux-custom
all:
	$(MAKE) -C $(KDIR) M=$$PWD
endif
