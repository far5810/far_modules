#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>

static struct proc_dir_entry *s_far_dir;
static struct proc_dir_entry *s_attr_file1;
static struct proc_dir_entry *s_attr_file2;

struct far_proc_data {
	int index;
	int read;
};

static int far_proc_file_open(struct inode *inode, struct file *file)
{
	struct far_proc_data* pProcData = 
			kzalloc(sizeof(struct far_proc_data), GFP_KERNEL);

	if(pProcData == NULL)
		return -ENOMEM;
		
	pProcData->index = 0x30313835;
	pProcData->read = 0;

	file->private_data = pProcData;
	
	return 0;
}

static int far_proc_file_release(struct inode *inode, struct file *file)
{
	struct far_proc_data *pProcData = file->private_data;
	kfree(pProcData);
	return 0;
}

static ssize_t far_proc_file_read(struct file *file, char __user *buffer,
			       size_t count, loff_t *ppos)
{
	struct far_proc_data* pProcData=file->private_data;
	if(pProcData->read >= 8)
		return 0;
	if(copy_to_user(buffer, &pProcData->index, 4))
		return -EINTR;
		
	if(copy_to_user(buffer + 4, "\n", 1))
		return -EINTR;
	
	pProcData->index += 0x1000000;
	pProcData->read++;
	return 5;

}

static ssize_t far_proc_file_write(struct file *file, const char __user *buffer,
				size_t count, loff_t *off)
{
	return count;
}

static struct file_operations far_proc_file_ops = 
{
	.owner	 = THIS_MODULE,
	.open	 = far_proc_file_open, 
	.release = far_proc_file_release,
	.read	 = far_proc_file_read, 
	.write	 = far_proc_file_write,
};

int far_proc_init(void)
{
	s_far_dir = proc_mkdir("FAR_MODULE", NULL);
	
	s_attr_file1 = proc_create("file1", S_IRUSR | S_IWUSR | S_IRGRP, 
						s_far_dir, &far_proc_file_ops);
	
	s_attr_file2 = proc_create("file2", 0644, 
						s_far_dir, &far_proc_file_ops);

	return 0;
}
EXPORT_SYMBOL_GPL(far_proc_init);

void far_proc_exit(void)
{
	remove_proc_entry("file1", s_far_dir);
	remove_proc_entry("file2", s_far_dir);
	remove_proc_entry("FAR_MODULE", NULL);
}
EXPORT_SYMBOL_GPL(far_proc_exit);

