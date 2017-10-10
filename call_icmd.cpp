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
        printf("open /dev/icmd failed: errno = %d\n", errno);
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
	//printf("\nTry %s: params: {%lld}\n", __FUNCTION__, params.addr);
    ret = ioctl(fd, CMD_IOC_DISABLE_IRQ, &params);
    if(ret) {
        printf("%s failed: errno= %d\n", __FUNCTION__, errno);
	}
	else {
        printf("Process (%d) %s success: params= {%lld}\n", getpid(), __FUNCTION__, params.addr);
	}
}

void icmd_enable_irq(int fd) {
	int ret = 0;
	struct cmd_params params = {0};
	//printf("\nTry %s: params= {%lld}\n", __FUNCTION__, params.addr);
    ret = ioctl(fd, CMD_IOC_ENABLE_IRQ, &params);
    if(ret) {
        printf("%s failed: errno= %d\n", __FUNCTION__, errno);
	}
	else {
        printf("Process (%d) %s success: params= {%lld}\n", getpid(), __FUNCTION__, params.addr);
	}
}

int main() {
	int fd = -1;
	if(!icmd_open(&fd)) {
		return -1;
	}

	icmd_disable_irq(fd);

	for(size_t i = 0; i < 10000000000; i++);

	icmd_enable_irq(fd);

	icmd_close(&fd);

	return 0;
}
