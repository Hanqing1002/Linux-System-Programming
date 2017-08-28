# remove the extension in filename. Canvas expects an extension so it was added.
CONFIG_MODULE_SIG=n
obj-m := mycdev.o

KERNEL_DIR = /usr/src/linux-headers-$(shell uname -r)

all:
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD) modules

clean:
	rm -rf *.o *.ko *.mod.* *.symvers *.order *~

