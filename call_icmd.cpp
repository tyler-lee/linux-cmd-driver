#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "cmd_user.h"

bool icmd_open(int* pfd) {
	int fd = -1;
	*pfd = -1;

    fd = open("/dev/icmd", O_RDWR);
    if (-1 == fd) {
        printf("open icmd device failed\n");
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
	struct cmd_params params = {0};
	printf("\nTry %s: params: {%lld}\n", __FUNCTION__, params.addr);
    ret = ioctl(fd, CMD_IOC_DISABLE_IRQ, &params);
    if(ret) {
        printf("CMD_IOC_DISABLE_IRQ failed: errno = %d\n", errno);
	}
	else {
        printf("CMD_IOC_DISABLE_IRQ success: params: {%lld}\n", params.addr);
	}
}

void icmd_enable_irq(int fd) {
	int ret = 0;
	struct cmd_params params = {0};
	printf("\nTry %s: params: {%lld}\n", __FUNCTION__, params.addr);
    ret = ioctl(fd, CMD_IOC_ENABLE_IRQ, &params);
    if(ret) {
        printf("CMD_IOC_ENABLE_IRQ failed: errno = %d\n", errno);
	}
	else {
        printf("CMD_IOC_ENABLE_IRQ success: params: {%lld}\n", params.addr);
	}
}

int main() {
	int fd = -1;
	if(icmd_open(&fd)) {
		printf("open success\n");

		icmd_disable_irq(fd);
		icmd_enable_irq(fd);

		icmd_close(&fd);
	}
}
