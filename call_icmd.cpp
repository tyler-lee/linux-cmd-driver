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
	struct cmd_params params = {0};
	//printf("\nTry %s: params: {%lld}\n", __FUNCTION__, params.addr);
    ret = ioctl(fd, CMD_IOC_DISABLE_IRQ, &params);
    if(ret) {
        printf("%s failed: errno= %d, %s\n", __FUNCTION__, errno, strerror(errno));
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
        printf("%s failed: errno= %d, %s\n", __FUNCTION__, errno, strerror(errno));
	}
	else {
        printf("Process (%d) %s success: params= {%lld}\n", getpid(), __FUNCTION__, params.addr);
	}
}

void icmd_set_interrupt(int fd) {
	int ret = 0;
	struct cmd_params params = {0};
	//printf("\nTry %s: params= {%lld}\n", __FUNCTION__, params.addr);
    ret = ioctl(fd, CMD_IOC_SET_INTERRUPT, &params);
    if(ret) {
        printf("%s failed: errno= %d, %s\n", __FUNCTION__, errno, strerror(errno));
	}
	else {
        printf("Process (%d) %s success: params= {%lld}\n", getpid(), __FUNCTION__, params.addr);
	}
}

void test_irq(int fd) {
	icmd_disable_irq(fd);

	for(size_t i = 0; i < 1000000000; i++);
	//sleep(1);
	//printf("Process (%d) %s success\n", getpid(), __FUNCTION__);

	icmd_enable_irq(fd);
}

const int core_num = 4;
#include <thread>
#include <iostream>
using namespace std;
int main() {
	cout << endl << "Begin" << endl << endl;
	int fd = -1;
	if(!icmd_open(&fd)) {
		return -1;
	}

	thread threads[core_num];
	for(int i = 0; i < core_num; i++) {
		//threads[i] = thread(icmd_set_interrupt, fd);
		threads[i] = thread(test_irq, fd);
	}
	for(int i = 0; i < core_num; i++) {
		threads[i].join();
	}

	icmd_close(&fd);

	cout << endl << "Done" << endl << endl;
	return 0;
}
