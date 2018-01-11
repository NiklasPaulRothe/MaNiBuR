
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
static short msg_size = 0;
static struct mutex lock;
static wait_queue_head_t read_queue;

//Decrypts the given Number
static int decrypt(int c)
{	
	// a*(p-2)
	int min_one = secret*(order-2);
	// Formel (2) -   					B ^ a*(p-2) mod p
	int b_min_one = mod_exp(openkey_sender, min_one, order);
	// B⁻1*c
	c = b_min_one * c;
	// Formel (1)
	c = mod_exp(c, 1, order);	

	return c;
}


static ssize_t brpa3_959042_959218_read(struct file *file, char __user * out,
			    size_t size, loff_t * off)
{	
	int err = 0;

	if (mutex_lock_interruptible(&lock)) {
        err = -ERESTARTSYS;
        goto out;
	}

	printk("msg_size: %hu", msg_size);
	while (msg_size == 0) {
		mutex_unlock(&lock);	
		if (wait_event_interruptible
		    (read_queue, msg_size != 0)) {
			err = -ERESTARTSYS;
			goto out;
		}
		if (mutex_lock_interruptible(&lock)) {
			err = -ERESTARTSYS;
			goto out;
		}
	}

	if (copy_to_user(out, msg, size)) {
		err = -EFAULT;
		goto out_unlock;
	}

	memset(msg, 0, 100);
	msg_size = 0;

	out_unlock:
		mutex_unlock(&lock);
	out:
		return err;
}


static ssize_t brpa3_959042_959218_write(struct file *file, const char __user * in,
			     size_t size, loff_t * off)
{
	int err = 0;
	int c = 0;

	if (mutex_lock_interruptible(&lock)) {
		err = -ERESTARTSYS;
		goto out;
	}

	memset(msg, 0, 100);
	readPos = 0;
	if (copy_from_user(msg, in, size)) {
		err = -EFAULT;
		goto out_unlock;
	}

	err = kstrtoint(msg, 10, &c);
	if (err == 0) {
		c = decrypt(c);
		sprintf(msg, "%i", c);
		msg_size = size;
	}

	wake_up_interruptible(&read_queue);

	out_unlock:
		mutex_unlock(&lock);
	out:
		return err;
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
            break;
        case BRPA3_SET_OPENKEY:
        	if (copy_from_user(&variable, (brpa3_args *)arg, sizeof(brpa3_args)))
            {
                return -EACCES;
            }
            openkey_sender = variable.value;
            break;
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
    mutex_init(&lock);
    init_waitqueue_head(&read_queue);

    return 0;
}

static void __exit brpa3_959042_959218_cleanup(void)
{
	misc_deregister(&brpa3_959042_959218_misc_device);
    printk(KERN_INFO "Cleaning up module.\n");
}

module_init(brpa3_959042_959218_init);
module_exit(brpa3_959042_959218_cleanup);
