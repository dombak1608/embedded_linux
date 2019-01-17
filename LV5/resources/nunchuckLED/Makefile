ifneq ($(KERNELRELEASE),)
obj-m := nunchukLED.o
else
KDIR := $(HOME)/buildroot/output/build/linux-custom
all:
	$(MAKE) -C $(KDIR) M=$$PWD
endif
