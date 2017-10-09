ifneq ($(KERNELRELEASE),)
	icmd-y := \
		cmd_main.o \
		cmd_ioctl.o 
	obj-m += icmd.o
else
KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

install: default
	$(MAKE) INSTALL_MOD_DIR=kernel/drivers/thirdparty -C $(KDIR) M=$(PWD) modules_install
	sh -c "cat /etc/modules | grep -Fxq icmd || echo icmd >> /etc/modules"

endif

clean:
	rm -vrf *.o *.ko *.order *.symvers *.mod.c .tmp_versions .*o.cmd
