#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

/* This function is called when the module is loaded. */
int simpleInit(void)
{
    printk(KERN_INFO "Loading Module\n");
    return 0;
}
/* This function is called when the module is removed. */
void simpleExit(void)
{
    printk(KERN_INFO "Removing Module\n");
}
/* Macros for registering module entry and exit points. */

module_init(simpleInit);
module_exit(simpleExit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Module");
MODULE_AUTHOR("SGG");