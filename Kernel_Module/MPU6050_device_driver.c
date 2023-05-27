#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>


static int __init driver_init(void)
{
	printk("Welcome to my driver!\n");
	return 0;
}

static void __exit driver_exit(void)
{
	printk("Leaving my driver!\n");
	return;
}

module_init(driver_init);
module_exit(driver_exit);

MODULE_LICENSE("MIT");
MODULE_AUTHOR("Foxunderground0");
MODULE_DESCRIPTION("MPU6050 I2C Driver");
MODULE_VERSION("1.0");
