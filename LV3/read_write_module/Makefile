ifneq ($(KERNELRELEASE),)
obj-m := read_write.o
else
KDIR := $(HOME)/buildroot/output/build/linux-custom
all:
	$(MAKE) -C $(KDIR) M=$$PWD
endif
