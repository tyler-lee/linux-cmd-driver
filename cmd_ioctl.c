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
#include <asm/apic.h>

//define per cpu irq flags
DEFINE_PER_CPU(unsigned long, flags);
/**
 * @filep:	open file to /dev/icmd
 * @cmd:	the command value
 * @arg:	pointer to the struct cmd_params
 */
static long cmd_ioc_disable_irq(struct file *filep, unsigned int cmd, unsigned long arg) {
	unsigned long tmp = 0;
	struct cmd_params *params = (struct cmd_params *)arg;
	//disable preempt: preempt_disable is nestable.
	preempt_disable();
	//save interrupt flags
	local_irq_save(tmp);

	get_cpu_var(flags) = tmp;
	params->core = smp_processor_id();
	params->flags = tmp;
	put_cpu_var(flags);
	/*local_bh_disable();*/

	/*pr_info("icmd: core %d disable preempt and irq, flags=%ld\n", smp_processor_id(), tmp);*/
	return CMD_SUCCESS;
}

/**
 * @filep:	open file to /dev/icmd
 * @cmd:	the command value
 * @arg:	pointer to the struct cmd_params
 */
static long cmd_ioc_enable_irq(struct file *filep, unsigned int cmd, unsigned long arg) {
	struct cmd_params *params = (struct cmd_params *)arg;
	unsigned long tmp = get_cpu_var(flags);
	/*pr_info("icmd: core %d enable preempt and irq, flags=%ld\n", smp_processor_id(), tmp);*/
	params->core = smp_processor_id();
	params->flags = tmp;
	put_cpu_var(flags);

	//restore interrupt flags
	local_irq_restore(tmp);
	//enable preempt: preempt_enable is nestable.
	preempt_enable();

	return CMD_SUCCESS;
}

static long cmd_ioc_set_interrupt(struct file *filep, unsigned int cmd, unsigned long arg) {
	preempt_disable();
	local_irq_disable();
	/*pr_info("icmd: core %d preempt and irq\n", smp_processor_id());*/

	/*cmd_ioc_disable_irq(filep, cmd, arg);*/
	/*msleep(1);*/
	/*ssleep(1);*/
	/*udelay(unsigned long);*/
	/*ndelay(unsigned long);*/
	/*mdelay(unsigned long);*/
	/*mdelay(2000);*/
	/*cmd_ioc_enable_irq(filep, cmd, arg);*/
	//TODO: do jobs

	preempt_enable();
	local_irq_enable();

	return CMD_SUCCESS;
}

static inline int lapic_is_integrated(void)
{
#ifdef CONFIG_X86_64
	return 1;
#else
	return APIC_INTEGRATED(lapic_get_version());
#endif
}
/* Clock divisor */
#define APIC_DIVISOR 16
#define TSC_DIVISOR  8
static void __setup_APIC_LVTT(unsigned int clocks, int oneshot, int irqen)
{
	unsigned int lvtt_value, tmp_value;

	lvtt_value = LOCAL_TIMER_VECTOR;
	if (!oneshot)
		lvtt_value |= APIC_LVT_TIMER_PERIODIC;
	else if (boot_cpu_has(X86_FEATURE_TSC_DEADLINE_TIMER)) {
		printk_once(KERN_DEBUG "boot_cpu_has(X86_FEATURE_TSC_DEADLINE_TIMER)\n");
		lvtt_value |= APIC_LVT_TIMER_TSCDEADLINE;
	}

	if (!lapic_is_integrated())
		lvtt_value |= SET_APIC_TIMER_BASE(APIC_TIMER_BASE_DIV);

	if (!irqen)
		lvtt_value |= APIC_LVT_MASKED;

	apic_write(APIC_LVTT, lvtt_value);

	if (lvtt_value & APIC_LVT_TIMER_TSCDEADLINE) {
		/*
		 * See Intel SDM: TSC-Deadline Mode chapter. In xAPIC mode,
		 * writing to the APIC LVTT and TSC_DEADLINE MSR isn't serialized.
		 * According to Intel, MFENCE can do the serialization here.
		 */
		asm volatile("mfence" : : : "memory");

		printk_once(KERN_DEBUG "TSC deadline timer enabled\n");
		return;
	}

	/*
	 * Divide PICLK by 16
	 */
	tmp_value = apic_read(APIC_TDCR);
	apic_write(APIC_TDCR,
		(tmp_value & ~(APIC_TDR_DIV_1 | APIC_TDR_DIV_TMBASE)) |
		APIC_TDR_DIV_16);

	if (!oneshot)
		apic_write(APIC_TMICT, clocks / APIC_DIVISOR);
}
static long cmd_ioc_set_apic_timer(struct file *filep, unsigned int cmd, unsigned long arg) {
	struct cmd_params *params = (struct cmd_params *)arg;

	preempt_disable();
	local_irq_disable();

	__setup_APIC_LVTT(params->clocks, 1, 1);

	preempt_enable();
	local_irq_enable();

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
	case CMD_IOC_EMPTY_CALL:
		//this is an empty call
		return CMD_SUCCESS;
	case CMD_IOC_SET_APIC_TIMER:
		handler = cmd_ioc_set_apic_timer;
		return CMD_SUCCESS;
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

