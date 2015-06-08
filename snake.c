/*==================Includes==================*/
#include <linux/module.h>
#include <linux/kernel.h> // for using printk 
#include <linux/errno.h>
#include <asm/semaphore.h>
#include <linux/fs.h> //for struct file_operations
#include <asm/uaccess.h> //for copy_from_user()
//#include <linux/sched.h>
//#include <linux/slab.h>         // for kmalloc
//#include <linux/spinlock.h>
//#include <linux/wait.h>
#include "snake.h"

MODULE_LICENSE("GPL");

/*==================Defines==================*/
#define SNAKE_MODULE "snake"
#define ZERO 0

/*==================Global Variables==================*/
static int major = -1;
struct file_operations fops = {         
        .open=         my_open,
        .release=     my_release,
        .read=          my_read,
        .write=         my_write,
        .llseek=        my_llseek,
        .ioctl=           my_ioctl,
        .owner=       THIS_MODULE
};

/*==================Init==================*/
int init_module(void)
{
	major = register_chrdev(ZERO, SNAKE_MODULE, &fops);
	if(major < 0)
	{
		printk(KERN_ALERT " Registering char device failed with %d\n", major);
        return major;
	}
	
	//TODO: Initialization
	
	return 0;
}

/*==================Clean==================*/
void cleanup_module(void)
{	
	int ret = unregister_chrdev(major, SNAKE_MODULE);
	if(ret < 0)
    { 
		printk(KERN_ALERT "Error in unregister_chrdev: %d\n", ret);
    }
}