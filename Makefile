TARGET_MODULE:=icmd
# If we are running by kernel building system
ifneq ($(KERNELRELEASE),)
    $(TARGET_MODULE)-objs := cmd_main.o cmd_ioctl.o
    obj-m := $(TARGET_MODULE).o
# If we running without kernel build system
else
    BUILDSYSTEM_DIR:=/lib/modules/$(shell uname -r)/build
    PWD:=$(shell pwd)
    DEST_DIR:=/lib/modules/$(shell uname -r)/kernel/drivers/thirdparty

all:
# run kernel build system to make module
	$(MAKE) -C $(BUILDSYSTEM_DIR) M=$(PWD) modules

clean:
# run kernel build system to cleanup in current directory
	$(MAKE) -C $(BUILDSYSTEM_DIR) M=$(PWD) clean

install:
	sudo mkdir -p "$(DEST_DIR)"
	#if [ ! -f "$(DEST_DIR)/$(TARGET_MODULE).ko" ]; then
		sudo cp $(TARGET_MODULE).ko "$(DEST_DIR)"
	#fi
	#sudo sh -c "cat /etc/modules | grep -Fxq $(TARGET_MODULE)) || echo $(TARGET_MODULE)) >> /etc/modules"
	sudo /sbin/depmod
	sudo /sbin/modprobe $(TARGET_MODULE)

uninstall:
	sudo /sbin/modprobe -r $(TARGET_MODULE)
	#if [ -f "$(DEST_DIR)/$(TARGET_MODULE).ko" ]; then
		sudo rm -rf "$(DEST_DIR)/$(TARGET_MODULE).ko"
	#fi
	sudo /sbin/depmod
	#sudo /bin/sed -i '/^$(TARGET_MODULE)$/d' /etc/modules
endif
