#ifndef __ARCH_INTEL_CMD_H__
#define __ARCH_INTEL_CMD_H__

#include "cmd_user.h"
#include <linux/kref.h>
#include <linux/version.h>
#include <linux/rbtree.h>
#include <linux/rwsem.h>
#include <linux/sched.h>
#include <linux/mmu_notifier.h>
#include <linux/radix-tree.h>

long cmd_ioctl(struct file *filep, unsigned int cmd, unsigned long arg);
#ifdef CONFIG_COMPAT
long cmd_compat_ioctl(struct file *filep, unsigned int cmd, unsigned long arg);
#endif

#endif /* __ARCH_X86_INTEL_CMD_H__ */
