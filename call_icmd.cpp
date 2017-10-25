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

int fd = -1;

using namespace std;

const int core_num = get_nprocs();

uint64_t rdtscp() {
#ifdef __linux__
	uint64_t a, d;
	//asm volatile ("xor %%rax, %%rax\n" "cpuid"::: "rax", "rbx", "rcx", "rdx");
	asm volatile ("rdtscp" : "=a" (a), "=d" (d) : : "rcx");
	return (d << 32) | a;
#else
	unsigned int tsc;

	return __rdtscp(&tsc);
#endif
}
#define clflush(p) asm volatile("clflush (%0)" : : "r" (p) : "memory")

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
void set_thread_affinity(int cpu) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu, &mask);

	//printf("Thread %lu is running on cpu %d\n", pthread_self(), cpu);
	int ret = pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask);
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

//测量空的系统调用
void measurement_empty_ioctl() {
	size_t count = 1000000;
	cout << __FUNCTION__ << " (loops): " << count << endl;

	int ret = 0;
	struct cmd_params params;
	memset(&params, 0, sizeof(cmd_params));

	uint64_t cycles = rdtscp();
	for(int i = 0; i < count; i++) {
		ret = ioctl(fd, CMD_IOC_EMPTY_CALL, &params);
		if (ret != CMD_SUCCESS) abort();
	}
	cycles = rdtscp()-cycles;

	cout << "Result (cycles per inout): " << cycles / count << endl << endl;
}

void sleep_for_cycles(size_t cycles) {
	uint64_t end = rdtscp() + cycles;
	uint64_t clocks = 0;
	while ((clocks = rdtscp()) < end);
}
void _irq_cost_using_apic(int cpu, size_t count) {
	set_thread_affinity(cpu);
	printf("Enter core: %d\n", cpu);

	size_t clocks = 600000;
	uint64_t cycles = rdtscp();
	for(int i = 0; i < count; i++) {
		//make sure while apic timer is running when enclave is alive.
		icmd_set_apic_timer(fd, clocks / 3);
		sleep_for_cycles(clocks);
	}
	cycles = rdtscp()-cycles;
	cycles = cycles / count;

	if(cycles < clocks) cout << "negetive" << endl;
	cout << "Result (cycles per mix): " << cycles << endl;
	cout << "Result (cycles per inout): " << (cycles - clocks) << endl << endl;
}
void measure_irq_cost_using_apic() {
	size_t count = 10000;
	cout << __FUNCTION__ << " (loops): " << count << endl
		<< "============ Application Mode =============" << endl;

	_irq_cost_using_apic(sched_getcpu(), count);
	//thread threads[CORES_PER_CPU];
	//for(int i = 0; i < CORES_PER_CPU; i++) {
		//threads[i] = thread(_irq_cost_using_apic, i, count);
	//}
	//for(int i = 0; i < CORES_PER_CPU; i++) {
		//threads[i].join();
	//}
}

//STL thread
//int main_stl_thread() {
int main() {
	system("clear");
    fflush( stdout );

	cout << endl << "Begin" << endl << endl;
	if(!icmd_open(&fd)) {
		return -1;
	}

	printf("In %s:\n", __FUNCTION__);
	set_thread_policy_and_priority(SCHED_FIFO, sched_get_priority_max(SCHED_FIFO));
	show_thread_policy_and_priority();
	printf("\n\n");

	icmd_set_apic_timer(fd, 50000);
	//measurement_empty_ioctl();
	//measure_irq_cost_using_apic();
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
