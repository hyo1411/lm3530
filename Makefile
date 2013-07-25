KERNEL_DIR=/work/kernel/
MAKE=make

obj-m += lm3530_bl_remap.o
lm3530_bl_remap-objs := lm3530_remap.o lm3530_bl.o
PWD = $(shell pwd)
default:
	make   ARCH=arm CROSS_COMPILE=/work/toolchains/arm-eabi-4.4.3/bin/arm-eabi- -C $(KERNEL_DIR) M=$(PWD) modules
clean:
        $(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD) clean
