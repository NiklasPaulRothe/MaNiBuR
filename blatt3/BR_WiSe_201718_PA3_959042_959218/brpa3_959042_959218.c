
#include <linux/init.h>     /* __init and __exit macroses */
#include <linux/kernel.h>   /* KERN_INFO macros */
#include <linux/module.h>   /* required for all kernel modules */
#include <linux/moduleparam.h>  /* module_param() and MODULE_PARM_DESC() */

#include <linux/fs.h>       /* struct file_operations, struct file */
#include <linux/miscdevice.h>   /* struct miscdevice and misc_[de]register() */
#include <linux/mutex.h>    /* mutexes */
#include <linux/string.h>   /* memchr() function */
#include <linux/slab.h>     /* kzalloc() function */
#include <linux/sched.h>    /* wait queues */
#include <linux/uaccess.h>  /* copy_{to,from}_user() */

#include "mod_exp.h"
#include "brpa3_959042_959218_header.h"


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Matthias Thien & Niklas Rothe");
MODULE_DESCRIPTION("Module to decrypt positive numbers with the elgamal algorithm");


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

struct locks {
    wait_queue_head_t read_queue;
    struct mutex lock;
};

char msg[100] = {0};
int msg_size = 0;



static struct locks* queue_lock_init(void)
{	

	struct locks *locks = NULL;
	locks = kzalloc(sizeof(*locks), GFP_KERNEL);
    if (unlikely(!locks))
        goto out;

    init_waitqueue_head(&locks->read_queue);

    mutex_init(&locks->lock);

    out:
    	return locks;
}

static void buffer_free(struct locks *locks)
{
    kfree(locks);
}

//Decrypts the given Number
static int decrypt(int c)
{

    printk("Secret: %hu", secret);
    printk("Openkey: %hu", openkey);

    printk("Before Decrypted: %i", c);
    // a*(p-2)
    int min_one = secret*(order-2);
    // Formel (2) -                     B ^ a*(p-2) mod p
    int b_min_one = mod_exp(openkey_sender, min_one, order);
    // B⁻¹*c
    c = b_min_one * c;
    // Formel (1)
    c = c % order;
    printk("After Decrypted: %i", c);

    return c;

}

static ssize_t brpa3_959042_959218_read(struct file *file, char __user * out,
                size_t size, loff_t * off)
{
    ssize_t result = 0;
    struct locks *locks = file->private_data;
    
    printk("Hello from read function");
    printk("Inhalt File in Read: %s", msg);
    printk("size: %i", size);
    //Warten bis Mutex frei wird
    if (mutex_lock_interruptible(&locks->lock)) {
        result = -ERESTARTSYS;
        goto out;
    }
 	printk("msg_size: %i",msg_size);
    if (msg_size == 0) {
        printk("test1");
        mutex_unlock(&locks->lock);
        if (file->f_flags & O_NONBLOCK) {
            printk("test2");
            result = -EAGAIN;
            goto out;
        }
        if (wait_event_interruptible
            (locks->read_queue, msg_size != 0)) {
            printk("test3");
            result = -ERESTARTSYS;
            goto out;
        }
        if (mutex_lock_interruptible(&locks->lock)) {
            printk("test4");
            result = -ERESTARTSYS;
            goto out;
        }
    } else if (msg_size < 0) {
    	printk("test");
    	result = 0;
    	goto out_unlock;
    }

    printk("test5");
    if (size > 100) {
	   	size = msg_size;
    }
    //Schreiben des Buffer vom Kernel in den UserSpace
    if (copy_to_user(out, msg, size)) {
        result = -EFAULT;
        goto out_unlock;
    }

    //Um Fragmente der hier ausgelesenen Berechnung zu verhindern wird der Buffer gecleared
    memset(msg, 0, msg_size);
    result = msg_size;
    msg_size = -1;
    printk("size: %i", size);

 out_unlock:
    mutex_unlock(&locks->lock);
 out:
    printk("Bye from read function");
    return result;
}

static ssize_t brpa3_959042_959218_write(struct file *file, const char __user * in,
                 size_t size, loff_t * off)
{
    struct locks *locks = file->private_data;
    ssize_t result = 0;
    int c = 0;
    printk("Hello from write function");

    //Eingaben die Länger als 100Byte sind kann das Programm nicht verabeiten.
    if (size > 100) {
        result = -EFBIG;
        goto out;
    }

    //Anfordern des Mutex
    if (mutex_lock_interruptible(&locks->lock)) {
        result = -ERESTARTSYS;
        goto out;
    }

    //Kopieren der Eingabe aus dem UserSpace
    if (copy_from_user(msg, in, size)) {
        result = -EFAULT;
        goto out_unlock;
    }
    //Auswerten der Eingabe
    msg_size = size;
    if (msg_size > 0) {
        //Umwandeln in einen int
        result = kstrtoint(msg, 10, &c);
        if (result == 0) {
            //Entschlüsseln
            c = decrypt(c);
            //Löschen des Speichers um zu verhindern das Relikte der Eingabe stehen bleiben
            memset(msg, 0, 100);
            //Die entschlüsselte Zahl wieder in den Speicher schreiben
            sprintf(msg, "%i", c);
            msg_size = strlen(msg);
            result = msg_size;
            wake_up_interruptible(&locks->read_queue);
        } else {
            result = -EINVAL;
        }
    }

 out_unlock:
    mutex_unlock(&locks->lock);
 out:
    printk("Bye from write function");
    return result;
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
static int brpa3_959042_959218_open(struct inode *inode, struct file *file)
{
	struct locks *locks;
	int err = 0;
	locks = queue_lock_init();
	if(unlikely(!locks)){
		err = -ENOMEM;
		goto out;
	}

	file->private_data = locks;
	out:
		return err;
}

static int brpa3_959042_959218_release(struct inode *inode, struct file *file)
{
    struct locks *locks = file->private_data;
    printk("Hello from release function");

    buffer_free(locks);

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
    misc_register(&brpa3_959042_959218_misc_device);
    printk(KERN_INFO
        "brpa3_959_042 device has been registered\n");
    return 0;   // Non-zero return means that the module couldn't be loaded.
}

static void __exit brpa3_959042_959218_cleanup(void)
{
    misc_deregister(&brpa3_959042_959218_misc_device);
    printk(KERN_INFO "Cleaning up module.\n");
}

module_init(brpa3_959042_959218_init);
module_exit(brpa3_959042_959218_cleanup);
