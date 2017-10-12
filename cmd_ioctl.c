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

//define per cpu irq flags
DEFINE_PER_CPU(unsigned long, flags);
/**
 * @filep:	open file to /dev/icmd
 * @cmd:	the command value
 * @arg:	pointer to the struct cmd_params
 */
static long cmd_ioc_disable_irq(struct file *filep, unsigned int cmd, unsigned long arg) {
	//disable preempt: preempt_disable is nestable.
	preempt_disable();
	//save interrupt flags
	unsigned long tmp = 0;
	local_irq_save(tmp);

	get_cpu_var(flags) = tmp;
	struct cmd_params *params = (struct cmd_params *)arg;
	params->core = smp_processor_id();
	params->flags = tmp;
	put_cpu_var(flags);
	/*local_bh_disable();*/

	pr_info("icmd: core %d disable preempt and irq, flags=%ld\n", smp_processor_id(), tmp);
	return CMD_SUCCESS;
}

/**
 * @filep:	open file to /dev/icmd
 * @cmd:	the command value
 * @arg:	pointer to the struct cmd_params
 */
static long cmd_ioc_enable_irq(struct file *filep, unsigned int cmd, unsigned long arg) {
	/*local_bh_enable();*/
	unsigned long tmp = get_cpu_var(flags);
	pr_info("icmd: core %d enable preempt and irq, flags=%ld\n", smp_processor_id(), tmp);
	struct cmd_params *params = (struct cmd_params *)arg;
	params->core = smp_processor_id();
	params->flags = tmp;
	put_cpu_var(flags);
	/*local_bh_disable();*/

	//restore interrupt flags
	local_irq_restore(tmp);
	//enable preempt: preempt_enable is nestable.
	preempt_enable();

	return CMD_SUCCESS;
}

static long cmd_ioc_set_interrupt(struct file *filep, unsigned int cmd, unsigned long arg) {
   /* struct cmd_params *params = (struct cmd_params *)arg;*/
	/*params->core = smp_processor_id();*/
	/*[>pit_prepare_sleep(100);<]*/
	/*[>spin_local_irqsave();<]*/
	/*preempt_disable();*/
	/*local_irq_save(get_cpu_var(flags));*/
	/*put_cpu_var(flags);*/
	cmd_ioc_disable_irq(filep, cmd, arg);

	/*msleep(1);*/
	/*ssleep(1);*/
	/*udelay(unsigned long);*/
	/*ndelay(unsigned long);*/
	/*mdelay(unsigned long);*/
	mdelay(2000);
	pr_info("icmd: core %d preempt and irq\n", smp_processor_id());

	/*local_irq_restore(get_cpu_var(flags));*/
	/*put_cpu_var(flags);*/
	/*preempt_enable();*/
	cmd_ioc_enable_irq(filep, cmd, arg);

	return CMD_SUCCESS;
}

typedef long (*cmd_ioc_t)(struct file *filep, unsigned int cmd, unsigned long arg);
//long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
long cmd_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {
	char data[256];
	cmd_ioc_t handler = NULL;
	long ret;

	switch (cmd) {
	case CMD_IOC_DISABLE_IRQ:
		handler = cmd_ioc_disable_irq;
		break;
	case CMD_IOC_ENABLE_IRQ:
		handler = cmd_ioc_enable_irq;
		break;
	case CMD_IOC_SET_INTERRUPT:
		handler = cmd_ioc_set_interrupt;
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

#ifdef CONFIG_COMPAT
//long (*compat_ioctl) (struct file *, unsigned int, unsigned long);
long cmd_compat_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {
	return cmd_ioctl(filep, cmd, arg);
}
#endif

