ifneq ($(KERNELRELEASE),)
obj-m := nunchuk.o
else
KDIR := $(HOME)/buildroot/output/build/linux-custom
all:
	$(MAKE) -C $(KDIR) M=$$PWD
endif
