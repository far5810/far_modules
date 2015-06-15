#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/fs.h>			// struct file_operations

#include <asm/uaccess.h>
#include <linux/vmalloc.h>		// kzalloc,kfree

#include "far_defines.h"


#define DEVICE_NAME		"FAR_MODULE"

static const char *DATA_TEST = "There are some test data for 'FAR_MODULE'\n";

struct far_dev {
	int 			far_major;
	int				far_sys_enable;
	struct cdev 	far_cdev;
	struct device*	far_dev;
	struct class* 	far_class;
};

static struct far_dev* pdev = NULL;
static int read_flag;

static int far_open(struct inode *, struct file *);
static int far_release(struct inode *, struct file *);
static ssize_t far_read(struct file *, char __user *, size_t, loff_t *);
// static ssize_t far_write(struct file *, char __user *, size_t, loff_t *);
// static int far_ioctl(struct inode *, struct file *, unsigned int, unsigned long);

static ssize_t far_sys_enable_show(struct device *dev, 
	struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", pdev->far_sys_enable);
}

static ssize_t far_sys_enable_store(struct device *dev, 
	struct device_attribute *attr, const char *buf, size_t size)
{
	if (sscanf(buf, "%d", &pdev->far_sys_enable) != 1)
		return -EINVAL;

	return size;
}

static DEVICE_ATTR(enable, S_IRUGO | S_IWUSR, far_sys_enable_show, far_sys_enable_store);

static struct file_operations far_fops =
{
	.owner	= THIS_MODULE,
	.open	= far_open,
	.read	= far_read,
	// .write	= far_write,
	// .fasync	= far_fasync,
	.release= far_release,
	// .poll	= far_poll,
	// .unlocked_ioctl	= far_ioctl,
};

static int far_open(struct inode *inode, struct file *filp)
{
	read_flag = 0;
	printk(KERN_INFO "'%s' opened by user space\n", DEVICE_NAME);
	return 0;
}

static int far_release(struct inode *inode, struct file *filp)
{
	read_flag = 0;
	return 0;
}

static ssize_t far_read(struct file *filp, char __user * buffer, size_t count, loff_t *ppos)
{
	int res = strlen(DATA_TEST);
	
	if(!read_flag) {
		__copy_to_user(buffer, DATA_TEST, res);
		read_flag = 1;
	} else 
		res = 0;
	return res;
}

static int __init far_init(void)
{
	int res;
	dev_t devno;
	
	pdev = (struct far_dev*)kzalloc(sizeof(struct far_dev), GFP_KERNEL);
	if (!pdev) {
		printk( KERN_ERR "failed to kzalloc memery for far driver struct\n");
		return -ENOMEM;
	}
	
	//================================================================
	res = alloc_chrdev_region(&devno, 0, 1, DEVICE_NAME);
	if(res < 0) {
		printk(KERN_ERR " Can't register major number\n");
		goto err1;
	}
	
	pdev->far_major = MAJOR(devno);
	pdev->far_sys_enable = 38;
	
	cdev_init(&pdev->far_cdev, &far_fops);
	pdev->far_cdev.owner = THIS_MODULE;
	cdev_add(&pdev->far_cdev, devno, 1);
	
	//================================================================
	pdev->far_class = class_create(THIS_MODULE, "abc");
	if (IS_ERR(pdev->far_class)) {
		printk(KERN_ERR "Err: failed create irdev class\n");
		goto err2;
	}
	pdev->far_dev = device_create(pdev->far_class, NULL, devno, NULL, 
						DEVICE_NAME);
	if (IS_ERR(pdev->far_dev))
		goto err3;
	
	res = device_create_file(pdev->far_dev, &dev_attr_enable);
	if (res < 0)
		goto err4;
	//================================================================
	
	far_proc_init();
	
	printk(KERN_INFO "init '%s' module\n", DEVICE_NAME);
	return 0;
	
err4:
	device_destroy(pdev->far_class, MKDEV(pdev->far_major, 0));
err3:
	class_destroy(pdev->far_class);
err2:
	cdev_del(&pdev->far_cdev);
	unregister_chrdev_region(devno, 1);
err1:
	kfree(pdev);
	return -1;
}

static void __exit far_exit(void)
{
	//================================================================
	device_remove_file(pdev->far_dev, &dev_attr_enable);
	device_destroy(pdev->far_class, MKDEV(pdev->far_major, 0));
	class_destroy(pdev->far_class);
	//================================================================
	cdev_del(&pdev->far_cdev);
	unregister_chrdev_region(MKDEV(pdev->far_major, 0), 1);
	
	far_proc_exit();
	
	kfree(pdev);
	printk(KERN_INFO "exit '%s' modules\n", DEVICE_NAME);
}

module_init(far_init);
module_exit(far_exit);

MODULE_AUTHOR("Sandy Far <far5810@126.com>");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Simple driver test in linux");
MODULE_ALIAS("FAR MODULE");
