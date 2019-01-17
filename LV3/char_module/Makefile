ifneq ($(KERNELRELEASE),)
obj-m := char_module.o
else
KDIR := $(HOME)/buildroot/output/build/linux-custom
all:
	$(MAKE) -C $(KDIR) M=$$PWD
endif
