#include <linux/module.h>
#include <linux/kernel.h> /* for using printk */
MODULE_LICENSE("GPL");

int iValue=0;
char *strValue;
int iArray[4];
MODULE_PARM(iValue, "i");
MODULE_PARM(strValue, "s");
MODULE_PARM(iArray, "4i");

int init_module(void)
{
	printk("Hello, World\n");
	return 0;
}


void cleanup_module(void)
{	
	printk("Goodbye cruel world\n");
}