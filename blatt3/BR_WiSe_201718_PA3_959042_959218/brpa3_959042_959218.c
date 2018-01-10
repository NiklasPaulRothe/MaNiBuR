
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
MODULE_DESCRIPTION("Module to decrypt positive numbers with the elgamal algorithm");

static unsigned long buffer_size = 8192;
module_param(buffer_size, ulong, (S_IRUSR | S_IRGRP | S_IROTH));
MODULE_PARM_DESC(buffer_size, "Internal buffer size");

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

struct buffer {
	wait_queue_head_t read_queue;
	struct mutex lock;
	char *data, *end;
	char *read_ptr;
	unsigned long size;
};

static struct buffer *buffer_alloc(unsigned long size)
{
	struct buffer *buf = NULL;

	buf = kzalloc(sizeof(*buf), GFP_KERNEL);
	if (unlikely(!buf))
		goto out;

	buf->data = kzalloc(size, GFP_KERNEL);
	if (unlikely(!buf->data))
		goto out_free;

	init_waitqueue_head(&buf->read_queue);

	mutex_init(&buf->lock);

	/* It's unused for now, but may appear useful later */
	buf->size = size;

 out:
	return buf;

 out_free:
	kfree(buf);
	return NULL;
}

static void buffer_free(struct buffer *buffer)
{
	kfree(buffer->data);
	kfree(buffer);
}

//Decrypts the given Number
static int decrypt(struct buffer *buf) 
{
	int c = 0;
	int err = kstrtoint(buf->data, 10, &c);

	printk("Secret: %hu", secret);
	printk("Openkey: %hu", openkey);

	printk("Before Decrypted: %i", c);
	if (err == 0) {
		// a*(p-2)
		int min_one = secret*(order-2);
		// Formel (2) -   					B ^ a*(p-2) mod p
		int b_min_one = mod_exp(openkey_sender, min_one, order);

		// B⁻¹*c
		c = b_min_one * c;
		// Formel (1) 
		c = c % order;
		printk("After Decrypted: %i", c);

		sprintf(buf->data, "%i", c);

	} else {
		return -EINVAL;
	}

	return 0;

}

static ssize_t brpa3_959042_959218_read(struct file *file, char __user * out,
			    size_t size, loff_t * off)
{
	struct buffer *buf = file->private_data;
	ssize_t result;	
	//size_t size_store = size;
	//int count = 0;

	if (mutex_lock_interruptible(&buf->lock)) {
		result = -ERESTARTSYS;
		goto out;
	}

	while (buf->read_ptr == buf->end) {
		mutex_unlock(&buf->lock);
		if (file->f_flags & O_NONBLOCK) {
			result = -EAGAIN;
			goto out;
		}
		if (wait_event_interruptible
		    (buf->read_queue, buf->read_ptr != buf->end)) {
			result = -ERESTARTSYS;
			goto out;
		}
		if (mutex_lock_interruptible(&buf->lock)) {
			result = -ERESTARTSYS;
			goto out;
		}
	}

	/*while (size_store > 0) {
		put_user(*buf->read_ptr,out++);
		size_store--;
		buf->read_ptr++;
	}*/


	if (copy_to_user(out, buf->read_ptr, size)) {
		result = -EFAULT;
		goto out_unlock;
	}

	// Um Fragmente dieser Berechnung zu verhindern wird hier der Buffer gecleared
	memset(buf->read_ptr, 0, size);

	buf->read_ptr += size;
	result = size;

 out_unlock:
	printk("out_unlock read");
	mutex_unlock(&buf->lock);
 out:
 	printk("out read");
	return result; 
}

static ssize_t brpa3_959042_959218_write(struct file *file, const char __user * in,
			     size_t size, loff_t * off)
{
	struct buffer *buf = file->private_data;
	ssize_t result;
	//int size_store = size;
	//int x = 0;



	if (size > buffer_size) {
		result = -EFBIG;
		goto out;
	}

	if (mutex_lock_interruptible(&buf->lock)) {
		result = -ERESTARTSYS;
		goto out;
	}

	/*while(size_store > 0) {
		buf->data[x] = in[x];
		x++;
		size_store--;
	}*/


	if (copy_from_user(buf->data, in, size)) {
		result = -EFAULT;
		goto out_unlock;
	}

	buf->end = buf->data + size;
	buf->read_ptr = buf->data;

	if (buf->end > buf->data) {
		//printk("decrypt number");
		decrypt(buf);
	}

	wake_up_interruptible(&buf->read_queue);

	result = size;

 out_unlock:
 	//printk("out_unlock");
	mutex_unlock(&buf->lock);
 out:
 	//printk("out");
	return result;
}

static int brpa3_959042_959218_open(struct inode *inode, struct file *file)
{
	struct buffer *buf;
	int err = 0;

	/*
	 * Real code can use inode to get pointer to the private
	 * device state.
	 */

	buf = buffer_alloc(buffer_size);
	if (unlikely(!buf)) {
		err = -ENOMEM;
		goto out;
	}

	file->private_data = buf;


 out:
	return err;
}

static int brpa3_959042_959218_release(struct inode *inode, struct file *file)
{
	struct buffer *buf = file->private_data;

	buffer_free(buf);

	return 0;
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
    .open = brpa3_959042_959218_open,
    .write = brpa3_959042_959218_write,
    .read = brpa3_959042_959218_read,
    .release = brpa3_959042_959218_release,
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
	if (!buffer_size)
        return -1;
    misc_register(&brpa3_959042_959218_misc_device);
    printk(KERN_INFO
        "brpa3_959_042 device has been registered, buffer size is %lu bytes\n",
        buffer_size);

    return 0;   // Non-zero return means that the module couldn't be loaded.
}

static void __exit brpa3_959042_959218_cleanup(void)
{	
	misc_deregister(&brpa3_959042_959218_misc_device);
    printk(KERN_INFO "Cleaning up module.\n");
}

module_init(brpa3_959042_959218_init);
module_exit(brpa3_959042_959218_cleanup);