#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "cmd_user.h"

bool icmd_open(int* pfd) {
	int fd = -1;
	*pfd = -1;

    fd = open("/dev/icmd", O_RDWR);
    if (-1 == fd) {
        printf("open /dev/icmd failed: errno= %d, %s\n", errno, strerror(errno));
		return false;
    }

	*pfd = fd;
	return true;
}
void icmd_close(int* pfd) {
	if (*pfd != -1)
	{
		close(*pfd);
		*pfd = -1;
	}
}

void icmd_disable_irq(int fd) {
	int ret = 0;
	struct cmd_params params;
	memset(&params, 0, sizeof(cmd_params));
    ret = ioctl(fd, CMD_IOC_DISABLE_IRQ, &params);
    if(ret) {
        printf("%s failed: errno= %d, %s\n", __FUNCTION__, errno, strerror(errno));
	}
}

void icmd_enable_irq(int fd) {
	int ret = 0;
	struct cmd_params params;
	memset(&params, 0, sizeof(cmd_params));
    ret = ioctl(fd, CMD_IOC_ENABLE_IRQ, &params);
    if(ret) {
        printf("%s failed: errno= %d, %s\n", __FUNCTION__, errno, strerror(errno));
	}
}

void icmd_set_apic_timer(int fd, size_t clocks) {
	int ret = 0;
	struct cmd_params params;
	memset(&params, 0, sizeof(cmd_params));
	params.clocks = clocks;
    ret = ioctl(fd, CMD_IOC_SET_APIC_TIMER, &params);
    if(ret) {
        printf("%s failed: errno= %d, %s\n", __FUNCTION__, errno, strerror(errno));
	}
	else {
        //printf("%s success\n", __FUNCTION__);
	}
}
