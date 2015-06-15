#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

static int __init hello_init(void)
{
	printk(KERN_INFO "init hello module\n");
	return 0;
}
static void __exit hello_exit(void)
{
	printk(KERN_INFO "exit hello modules\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_AUTHOR("Sandy Far <far5810@126.com>");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Simple driver test in linux");
MODULE_ALIAS("Hello test");
