ifneq ($(KERNELRELEASE),)
obj-m := nunchuk.o
else
KDIR := /opt/faks/buildroot/output/build/linux-custom
all:
	$(MAKE) -C $(KDIR) M=$$PWD
endif
