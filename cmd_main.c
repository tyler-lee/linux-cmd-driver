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

static const char module_info[] = "Successfully install icmd: " DRV_DESCRIPTION " v" DRV_VERSION "\n";
static const ssize_t module_info_size = sizeof(module_info);
static ssize_t device_file_read(struct file *file_ptr, char __user *user_buffer, size_t count, loff_t *position)
{
    pr_info("icmd: Device file is read at offset = %i, read bytes count = %u", (int)*position, (unsigned int)count );
    /* If position is behind the end of a file we have nothing to read */
    if( *position >= module_info_size )
        return 0;
    /* If a user tries to read more than we have, read only as many bytes as we have */
    if( *position + count > module_info_size )
        count = module_info_size - *position;
    if( copy_to_user(user_buffer, module_info + *position, count) != 0 )
        return -EFAULT;
    /* Move reading position */
    *position += count;

    return count;
}

static const struct file_operations cmd_fops = {
	.owner			= THIS_MODULE,
	.unlocked_ioctl		= cmd_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl		= cmd_compat_ioctl,
#endif
	.read    = device_file_read,
};

/*
	static struct miscdevice cmd_dev = {
		.name	= "icmd",
		.fops	= &cmd_fops,
		.mode   = 0666,
	};

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

	static int cmd_dev_init(struct device *dev)
	{
		int ret = 0;
		pr_info("Register linux_cmd: " DRV_DESCRIPTION " v" DRV_VERSION "\n");

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
		int ret = 0;
		if (atomic_cmpxchg(&cmd_init_flag, 0, 1)) {
			pr_warn("linux_cmd: second initialization call skipped\n");
			return 0;
		}
		pr_info("Register linux_cmd: " DRV_DESCRIPTION " v" DRV_VERSION "\n");

		cmd_dev.parent = &pdev->dev;
		ret = misc_register(&cmd_dev);
		if (ret) {
			pr_err("linux_cmd: misc_register() failed\n");
		}

		return ret;
		//return cmd_dev_init(&pdev->dev);
	}

	static int cmd_drv_remove(struct platform_device *pdev)
	{
		pr_info("Deregister linux_cmd: " DRV_DESCRIPTION " v" DRV_VERSION "\n");
		if (!atomic_cmpxchg(&cmd_init_flag, 1, 0)) {
			pr_warn("linux_cmd: second release call skipped\n");
			return 0;
		}

		misc_deregister(&cmd_dev);

		return 0;
	}

#ifdef CONFIG_ACPI
	static struct acpi_device_id cmd_device_ids[] = {
		{"INT0E0C", 0},
		{"", 0},
	};
	MODULE_DEVICE_TABLE(acpi, cmd_device_ids);
#endif

	static struct platform_driver cmd_drv = {
		.probe = cmd_drv_probe,
		.remove = cmd_drv_remove,
		.driver = {
			.name			= "linux_cmd",
			//.pm			= &cmd_drv_pm,
#ifdef CONFIG_ACPI
			.acpi_match_table	= ACPI_PTR(cmd_device_ids),
#endif
		},
	};
*/

/*#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0))*/
/*module_platform_driver(cmd_drv);*/
/*#else*/
/*static struct platform_device *pdev;*/
static int icmd_major_number = 0;
static const char device_name[] = "icmd";
int init_cmd_module(void)
{
	int result;
	pr_info("Register linux_cmd: " DRV_DESCRIPTION " v" DRV_VERSION "\n");
	/*platform_driver_register(&cmd_drv);*/
	/*pdev = platform_device_register_simple("linux_cmd", 0, NULL, 0);*/
	/*if (IS_ERR(pdev))*/
		/*pr_err("platform_device_register_simple failed\n");*/

	/* Registering device */
	result = register_chrdev( icmd_major_number, device_name, &cmd_fops );
	if( result < 0 )
	{
		pr_warn("icmd: can\'t register character device with errorcode = %i\n", result );
		return result;
	}

	icmd_major_number = result;
	pr_info("icmd: registered character device with major number = %i and minor numbers 0...255\n" , icmd_major_number );

	return 0;
}

void cleanup_cmd_module(void)
{
	pr_info("Deregister linux_cmd: " DRV_DESCRIPTION " v" DRV_VERSION "\n");
	/*dev_set_uevent_suppress(&pdev->dev, true);*/
	/*platform_device_unregister(pdev);*/
	/*platform_driver_unregister(&cmd_drv);*/
	if(icmd_major_number != 0)
	{
		unregister_chrdev(icmd_major_number, device_name);
	}
}

module_init(init_cmd_module);
module_exit(cleanup_cmd_module);
/*#endif*/

