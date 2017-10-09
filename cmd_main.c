#include "cmd.h"
#include <linux/acpi.h>
#include <linux/file.h>
#include <linux/highmem.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/suspend.h>
#include <linux/hashtable.h>
#include <linux/kthread.h>
#include <linux/platform_device.h>

#define DRV_DESCRIPTION "LHR Cmd Driver"
#define DRV_VERSION "0.10"

MODULE_DESCRIPTION(DRV_DESCRIPTION);
MODULE_AUTHOR("Huorong Li <lihuorong@iie.ac.cn>");
MODULE_VERSION(DRV_VERSION);
MODULE_LICENSE("Dual BSD/GPL");

#ifdef CONFIG_COMPAT
long cmd_compat_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
	return cmd_ioctl(filep, cmd, arg);
}
#endif

static const struct file_operations cmd_fops = {
	.owner			= THIS_MODULE,
	.unlocked_ioctl		= cmd_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl		= cmd_compat_ioctl,
#endif
	/*.mmap			= cmd_mmap,*/
	/*.get_unmapped_area	= cmd_get_unmapped_area,*/
};

static struct miscdevice cmd_dev = {
	.name	= "icmd",
	.fops	= &cmd_fops,
	.mode   = 0666,
};

/*
static int cmd_pm_suspend(struct device *dev)
{
	struct cmd_tgid_ctx *ctx;
	struct cmd_encl *encl;

	kthread_stop(kcmdswapd_tsk);
	kcmdswapd_tsk = NULL;

	list_for_each_entry(ctx, &cmd_tgid_ctx_list, list) {
		list_for_each_entry(encl, &ctx->encl_list, encl_list) {
			cmd_invalidate(encl, false);
			encl->flags |= CMD_ENCL_SUSPEND;
			flush_work(&encl->add_page_work);
		}
	}

	return 0;
}

static int cmd_pm_resume(struct device *dev)
{
	kcmdswapd_tsk = kthread_run(kcmdswapd, NULL, "kswapd");
	return 0;
}

static SIMPLE_DEV_PM_OPS(cmd_drv_pm, cmd_pm_suspend, cmd_pm_resume);
*/

static int cmd_dev_init(struct device *dev)
{
	int ret = 0;
	pr_info("linux_cmd: " DRV_DESCRIPTION " v" DRV_VERSION "\n");

	cmd_dev.parent = dev;
	ret = misc_register(&cmd_dev);
	if (ret) {
		pr_err("linux_cmd: misc_register() failed\n");
	}

	return ret;
}

static atomic_t cmd_init_flag = ATOMIC_INIT(0);

static int cmd_drv_probe(struct platform_device *pdev)
{
	if (atomic_cmpxchg(&cmd_init_flag, 0, 1)) {
		pr_warn("linux_cmd: second initialization call skipped\n");
		return 0;
	}

	return cmd_dev_init(&pdev->dev);
}

static int cmd_drv_remove(struct platform_device *pdev)
{
	if (!atomic_cmpxchg(&cmd_init_flag, 1, 0)) {
		pr_warn("linux_cmd: second release call skipped\n");
		return 0;
	}

	misc_deregister(&cmd_dev);

	return 0;
}

//#ifdef CONFIG_ACPI
//static struct acpi_device_id cmd_device_ids[] = {
//	{"INT0E0C", 0},
//	{"", 0},
//};
//MODULE_DEVICE_TABLE(acpi, cmd_device_ids);
//#endif

static struct platform_driver cmd_drv = {
	.probe = cmd_drv_probe,
	.remove = cmd_drv_remove,
	.driver = {
		.name			= "linux_cmd",
		/*.pm			= &cmd_drv_pm,*/
		/*.acpi_match_table	= ACPI_PTR(cmd_device_ids),*/
	},
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0))
module_platform_driver(cmd_drv);
#else
static struct platform_device *pdev;
int init_cmd_module(void)
{
	platform_driver_register(&cmd_drv);
	pdev = platform_device_register_simple("linux_cmd", 0, NULL, 0);
	if (IS_ERR(pdev))
		pr_err("platform_device_register_simple failed\n");
	return 0;
}

void cleanup_cmd_module(void)
{
	dev_set_uevent_suppress(&pdev->dev, true);
	platform_device_unregister(pdev);
	platform_driver_unregister(&cmd_drv);
}

module_init(init_cmd_module);
module_exit(cleanup_cmd_module);
#endif

