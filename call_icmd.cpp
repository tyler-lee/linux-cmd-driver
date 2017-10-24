#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <signal.h>
#include <sched.h>
#include <pthread.h>
#include <thread>
#include <iostream>

#include "cmd_user.h"

using namespace std;

const int core_num = get_nprocs();

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

void icmd_disable_irq(int fd, int tid) {
	int ret = 0;
	struct cmd_params params = {0, 0};
	params.core = sched_getcpu();
	printf("(%d) Try %s: params= {%lld, %lld}\n", tid, __FUNCTION__, params.core, params.flags);
    ret = ioctl(fd, CMD_IOC_DISABLE_IRQ, &params);
    if(ret) {
        printf("%s failed: errno= %d, %s\n", __FUNCTION__, errno, strerror(errno));
	}
	else {
        printf("(%d) %s success: params= {%lld, %lld}\n", tid, __FUNCTION__, params.core, params.flags);
	}
}

void icmd_enable_irq(int fd, int tid) {
	int ret = 0;
	struct cmd_params params = {0, 0};
	params.core = sched_getcpu();
	printf("(%d) Try %s: params= {%lld, %lld}\n", tid, __FUNCTION__, params.core, params.flags);
    ret = ioctl(fd, CMD_IOC_ENABLE_IRQ, &params);
    if(ret) {
        printf("%s failed: errno= %d, %s\n", __FUNCTION__, errno, strerror(errno));
	}
	else {
        printf("(%d) %s success: params= {%lld, %lld}\n", tid, __FUNCTION__, params.core, params.flags);
	}
}

void icmd_set_interrupt(int fd, int tid) {
	int ret = 0;
	struct cmd_params params = {0, 0};
	params.core = sched_getcpu();
	printf("(%d) Try %s: params= {%lld, %lld}\n", tid, __FUNCTION__, params.core, params.flags);
    ret = ioctl(fd, CMD_IOC_SET_INTERRUPT, &params);
    if(ret) {
        printf("%s failed: errno= %d, %s\n", __FUNCTION__, errno, strerror(errno));
	}
	else {
        printf("(%d) %s success: params= {%lld, %lld}\n", tid, __FUNCTION__, params.core, params.flags);
	}
}

void print_policy_string(int policy) {
	switch (policy)
	{
		case SCHED_FIFO:
			printf ("policy= SCHED_FIFO");
			break;
		case SCHED_RR:
			printf ("policy= SCHED_RR");
			break;
		case SCHED_OTHER:
			printf ("policy= SCHED_OTHER");
			break;
		default:
			printf ("policy= UNKNOWN");
			break;
	}
}
void show_thread_policy_and_priority() {
	int policy;
	sched_param sched;

	int ret = pthread_getschedparam(pthread_self(), &policy, &sched);
	if(ret != 0) printf("%s\n", strerror(errno));
	assert(ret == 0);

	printf("Thread %ld: ", pthread_self());
	print_policy_string(policy);
	printf (", priority= %d\n", sched.sched_priority);
}
void set_thread_policy_and_priority(int policy, int priority) {
	sched_param sched;
	sched.sched_priority = priority;
	int ret = pthread_setschedparam(pthread_self(), policy, &sched);
	if(ret != 0) printf("%s\n", strerror(errno));
	assert(ret == 0);

	printf ("Set thread %ld priority to %d\n", pthread_self(), priority);
}
void show_thread_policy_and_priority(pthread_attr_t *attr) {
	int policy;
	sched_param sched;

	int ret = pthread_attr_getschedparam(attr, &sched);
	assert(ret == 0);
	ret = pthread_attr_getschedpolicy(attr, &policy);
	assert(ret == 0);

	printf("Thread %ld: ", pthread_self());
	print_policy_string(policy);
	printf (", priority= %d\n", sched.sched_priority);
}
void set_thread_policy_and_priority(pthread_attr_t *attr, int policy, int priority) {
	sched_param sched;
	sched.sched_priority = priority;
	int ret = pthread_attr_setschedpolicy(attr, policy);
	assert(ret == 0);
	ret = pthread_attr_setschedparam(attr, &sched);
	assert(ret == 0);
}

void* test_pthread_priority(void *) {
	show_thread_policy_and_priority();
}

void test_stl_thread() {
	//printf("Process (%d) %s\n", getpid(), __FUNCTION__);
	//for(size_t i = 0; i < 800000000; i++);
	//icmd_disable_irq(fd, tid);
	//printf("%ld, %s\n", sched_getcpu(), strerror(errno));

	for(size_t i = 0; i < 5000000000; i++);
	show_thread_policy_and_priority();
	//icmd_set_interrupt(fd);
	//sleep(1);
	//printf("Process (%d) %s success\n", getpid(), __FUNCTION__);

	//icmd_enable_irq(fd, tid);
	//test_me_location();
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
		printf("%s success\n", __FUNCTION__);
	}
}

int fd = -1;
//STL thread
//int main_stl_thread() {
int main() {
	cout << endl << "Begin" << endl << endl;
	if(!icmd_open(&fd)) {
		return -1;
	}

	printf("In %s:\n", __FUNCTION__);
	set_thread_policy_and_priority(SCHED_FIFO, sched_get_priority_max(SCHED_FIFO));
	show_thread_policy_and_priority();
	printf("\n\n");

	icmd_set_apic_timer(fd, 50000);
	//sleep(1);
	//thread threads[core_num];
	//for(int i = 0; i < core_num; i++) {
		//threads[i] = thread(test_stl_thread);
	//}
	//for(int i = 0; i < core_num; i++) {
		//threads[i].join();
	//}

	icmd_close(&fd);
	cout << endl << "Done" << endl << endl;
	return 0;
}
//pthread
int main_pthread() {
//int main() {
	cout << endl << "Begin" << endl << endl;
	if(!icmd_open(&fd)) {
		return -1;
	}

	pthread_t threads[core_num];
	pthread_attr_t attr;
	int ret = 0;

	ret = pthread_attr_init(&attr);
	assert(ret == 0);

	printf("In %s:\n", __FUNCTION__);
	set_thread_policy_and_priority(&attr, SCHED_FIFO, sched_get_priority_max(SCHED_FIFO));
	show_thread_policy_and_priority(&attr);
	printf("\n\n");

	for(int i = 0; i < core_num; i++) {
		ret = pthread_create(&threads[i], &attr, &test_pthread_priority, NULL);
		assert(ret == 0);
	}

	ret = pthread_attr_destroy(&attr);
	assert(ret == 0);

	for(int i = 0; i < core_num; i++) {
		ret = pthread_join(threads[i], NULL);
		assert(ret == 0);
	}

	icmd_close(&fd);
	cout << endl << "Done" << endl << endl;
	return 0;
}
