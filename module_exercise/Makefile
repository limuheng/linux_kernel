PWD := $(shell pwd) 
KERNEL_DIR = /home/hank/Workspace/embedded/buildroot/output/build/linux-5.15.18
# /home/hank/Workspace/embedded/kernel/linux-5.15.37
 
MODULE_NAME = hank
obj-m := $(MODULE_NAME).o
 
all: 
	make ARCH=arm CROSS_COMPILE=arm-buildroot-linux-uclibcgnueabihf- \
		-C $(KERNEL_DIR) M=$(PWD) modules
clean: 
	make -C $(KERNEL_DIR) M=$(PWD) clean