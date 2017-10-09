#include "cmd.h"
#include <asm/mman.h>
#include <linux/delay.h>
#include <linux/file.h>
#include <linux/highmem.h>
#include <linux/ratelimit.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0))
	#include <linux/sched/signal.h>
#else
	#include <linux/signal.h>
#endif
#include <linux/slab.h>
#include <linux/hashtable.h>
#include <linux/shmem_fs.h>

/**
 * cmd_ioc_enclave_create - handler for CMD_IOC_ENCLAVE_CREATE
 * @filep:	open file to /dev/cmd
 * @cmd:	the command value
 * @arg:	pointer to the struct cmd_enclave_create
 *
 * Creates meta-data for an enclave and executes ENCLS(ECREATE)
 */
static long cmd_ioc_enclave_create(struct file *filep, unsigned int cmd,
				   unsigned long arg)
{
	/*struct cmd_enclave_create *createp = (struct cmd_enclave_create *)arg;*/
	return 0;
}

/**
 * cmd_ioc_enclave_add_page - handler for CMD_IOC_ENCLAVE_ADD_PAGE
 *
 * @filep:	open file to /dev/cmd
 * @cmd:	the command value
 * @arg:	pointer to the struct cmd_enclave_add_page
 *
 * Creates meta-data for an enclave page and enqueues ENCLS(EADD) that will
 * be processed by a worker thread later on.
 */
static long cmd_ioc_enclave_add_page(struct file *filep, unsigned int cmd,
				     unsigned long arg)
{
	/*struct cmd_enclave_add_page *addp = (void *)arg;*/
	return 0;
}

/**
 * cmd_ioc_enclave_init - handler for CMD_IOC_ENCLAVE_INIT
 *
 * @filep:	open file to /dev/cmd
 * @cmd:	the command value
 * @arg:	pointer to the struct cmd_enclave_init
 *
 * Flushes the remaining enqueued ENCLS(EADD) operations and executes
 * ENCLS(EINIT). Does a number of retries because EINIT might fail because of an
 * interrupt storm.
 */
static long cmd_ioc_enclave_init(struct file *filep, unsigned int cmd,
				 unsigned long arg)
{
	/*struct cmd_enclave_init *initp = (struct cmd_enclave_init *)arg;*/

	return 0;
}

typedef long (*cmd_ioc_t)(struct file *filep, unsigned int cmd,
			  unsigned long arg);

long cmd_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
	char data[256];
	cmd_ioc_t handler = NULL;
	long ret;

	switch (cmd) {
	case CMD_IOC_ENCLAVE_CREATE:
		handler = cmd_ioc_enclave_create;
		break;
	case CMD_IOC_ENCLAVE_ADD_PAGE:
		handler = cmd_ioc_enclave_add_page;
		break;
	case CMD_IOC_ENCLAVE_INIT:
		handler = cmd_ioc_enclave_init;
		break;
	default:
		return -ENOIOCTLCMD;
	}

	if (copy_from_user(data, (void __user *)arg, _IOC_SIZE(cmd)))
		return -EFAULT;

	ret = handler(filep, cmd, (unsigned long)((void *)data));
	if (!ret && (cmd & IOC_OUT)) {
		if (copy_to_user((void __user *)arg, data, _IOC_SIZE(cmd)))
			return -EFAULT;
	}

	return ret;
}
