### Build CMD Driver
To build CMD driver, change the directory to the driver path and enter the following command:
```
$ make
```
You can find the driver *icmd.ko* generated in the same directory.

### Install the CMD Driver
To install the CMD driver, enter the following command with root privilege:
```
$ sudo mkdir -p "/lib/modules/"`uname -r`"/kernel/drivers/thirdparty"
$ sudo cp icmd.ko "/lib/modules/"`uname -r`"/kernel/drivers/thirdparty"
$ sudo sh -c "cat /etc/modules | grep -Fxq icmd || echo icmd >> /etc/modules"
$ sudo /sbin/depmod
$ sudo /sbin/modprobe icmd
```
On Red Hat Enterprise Linux Server or CentOS, need to run below command on each reboot
```
$ sudo /sbin/modprobe icmd
```

### Uninstall the CMD Driver
To uninstall the CMD driver, enter the following commands:
```
$ sudo /sbin/modprobe -r icmd
$ sudo rm -rf "/lib/modules/"`uname -r`"/kernel/drivers/thirdparty/icmd"
$ sudo /sbin/depmod
$ sudo /bin/sed -i '/^icmd$/d' /etc/modules
```
