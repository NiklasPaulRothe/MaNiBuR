
#include <linux/init.h>		/* __init and __exit macroses */
#include <linux/kernel.h>	/* KERN_INFO macros */
#include <linux/module.h>	/* required for all kernel modules */
#include <linux/moduleparam.h>	/* module_param() and MODULE_PARM_DESC() */

#include <linux/fs.h>		/* struct file_operations, struct file */
#include <linux/miscdevice.h>	/* struct miscdevice and misc_[de]register() */
#include <linux/mutex.h>	/* mutexes */
#include <linux/string.h>	/* memchr() function */
#include <linux/slab.h>		/* kzalloc() function */
#include <linux/sched.h>	/* wait queues */
#include <linux/uaccess.h>	/* copy_{to,from}_user() */

#include "mod_exp.h"
#include "brpa3_959042_959218_header.h"


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Matthias Thien & Niklas Rothe");
MODULE_DESCRIPTION("Module to decrypt texts");

// Ordung p
static unsigned short order = 59;
module_param(order, ushort, (S_IRUSR | S_IRGRP | S_IROTH));
// Erzeuger g
static unsigned short generator = 2;
module_param(generator, ushort, (S_IRUSR | S_IRGRP | S_IROTH));
// Privater Schlüssel Character Device a 
static unsigned short secret = 5;
module_param(secret, ushort, (S_IRUSR | S_IRGRP | S_IROTH));
// Öffentlicher Schlüssel Character Device A
static unsigned short openkey = 32;
module_param(openkey, ushort, (S_IRUSR | S_IRGRP | S_IROTH));
// Öffentlicher Schlüssel Verschlüssler B
static unsigned short openkey_sender = 16;
module_param(openkey_sender, ushort, (S_IRUSR | S_IRGRP | S_IROTH));

static char msg[100] = {0};
static short readPos = 0;

//Decrypts the given Number
static int decrypt(char *msg_in) 
{

	int c = 0;
	int err = kstrtoint(msg_in, 10, &c);
	if (err == 0) {
		// a*(p-2)
		int min_one = secret*(order-2);
		// Formel (2) -   					B ^ a*(p-2) mod p
		int b_min_one = mod_exp(openkey_sender, min_one, order);

		// B⁻1*c
		c = b_min_one * c;
		// Formel (1) 
		c = mod_exp(c, 1, order);

		sprintf(msg_in, "%i", c);

	} else {
		return -EINVAL;
	}

	return 0;

}


static ssize_t brpa3_959042_959218_read(struct file *file, char __user * out,
			    size_t size, loff_t * off)
{
	short count = 0;
	while(size&&(msg[readPos]!=0)){
		put_user(msg[readPos],out++);
		count++;
		size--;
		readPos++;
	}

	return count;
}


static ssize_t brpa3_959042_959218_write(struct file *file, const char __user * in,
			     size_t size, loff_t * off)
{
	short ind = 0;
	short count = 0;
	memset(msg, 0, 100);
	readPos = 0;
	while(size > 0){
		msg[count++] = in[ind++];
		size--;
	}

	decrypt(msg);

	return count;
}

// updates the secret and openkey
static int update_keys(unsigned short new_secret) {

	if (new_secret < 1 || new_secret > order - 1) {
		return -EINVAL;
	} else {
		//secret
		secret = new_secret;
		//openkey
		openkey = mod_exp(generator, secret, order);

	}
	return 0;

}

// I/O Control
long brpa3_959042_959218_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{	
	brpa3_args variable;
	switch(cmd)
	{
		case BRPA3_SET_SECRET:
			if (copy_from_user(&variable, (brpa3_args *)arg, sizeof(brpa3_args)))
            {
                return -EACCES;
            }
            update_keys(variable.value);
        case BRPA3_SET_OPENKEY:
        	if (copy_from_user(&variable, (brpa3_args *)arg, sizeof(brpa3_args)))
            {
                return -EACCES;
            }
            openkey_sender = variable.value;
        case BRPA3_GET_OPENKEY:
			variable.value = openkey;
			if (copy_to_user((brpa3_args *)arg, &variable, sizeof(brpa3_args)))
			{
				return -EACCES;
			}
			break;
		default:
			return -EINVAL;
	}

	return 0;

}

static struct file_operations brpa3_959042_959218_fops = {
    .owner = THIS_MODULE,
    .write = brpa3_959042_959218_write,
    .read = brpa3_959042_959218_read,
    .unlocked_ioctl = brpa3_959042_959218_ioctl,
    .llseek = noop_llseek
};

static struct miscdevice brpa3_959042_959218_misc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "brpa3_959042_959218",
    .fops = &brpa3_959042_959218_fops
};

static int __init brpa3_959042_959218_init(void)
{	

    misc_register(&brpa3_959042_959218_misc_device);
    printk(KERN_INFO
        "brpa3_959_042 device has been registered");

    return 0;
}

static void __exit brpa3_959042_959218_cleanup(void)
{	
	misc_deregister(&brpa3_959042_959218_misc_device);
    printk(KERN_INFO "Cleaning up module.\n");
}

module_init(brpa3_959042_959218_init);
module_exit(brpa3_959042_959218_cleanup);