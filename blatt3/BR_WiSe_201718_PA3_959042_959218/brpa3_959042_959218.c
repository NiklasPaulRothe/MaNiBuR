
#include <linux/init.h>     /* __init and __exit macroses */
#include <linux/kernel.h>   /* KERN_INFO macros */
#include <linux/module.h>   /* required for all kernel modules */
#include <linux/moduleparam.h>  /* module_param() and MODULE_PARM_DESC() */

#include <linux/fs.h>       /* struct file_operations, struct file */
#include <linux/miscdevice.h>   /* struct miscdevice and misc_[de]register() */
#include <linux/mutex.h>    /* mutexes */
#include <linux/slab.h>     /* kzalloc() function */
#include <linux/sched.h>    /* wait queues */
#include <linux/uaccess.h>  /* copy_{to,from}_user() */

#include "mod_exp.h"
#include "brpa3_959042_959218_header.h"


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Matthias Thien & Niklas Rothe");
MODULE_DESCRIPTION("Module to decrypt positive numbers with the elgamal algorithm");

//Parameter für den Elgamal Algorithmus können beim Starten des Kernel frei gesetzt werden
//Ein Aufruf würde wir folgt aussehen:
//	insmod brpa3_959042_959218.ko buffer_size=50 secret = 9
//usw.

//Größe des Speichers für die Zahl
static unsigned short buffer_size = 16;
module_param(buffer_size, ushort, (S_IRUSR | S_IRGRP | S_IROTH));
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

//locks-Struct zum Speichern des Mutex und der waitqueue
struct locks {
    wait_queue_head_t read_queue;
    struct mutex lock;
};

//msg dient als Speicher für die Eingehende Zahl
char *msg;
//msg_size gibt an wie lang (wie viele Stellen) sie hat
int msg_size = 0;


//Allokiert Speicher für die waitqueue und den mutex lock und initialisiert diese
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

//Free'd den Speicher der Locks wieder
static void queue_lock_exit(struct locks *locks)
{
    kfree(locks);
}

//Read Methode wird aufgerufen sobald jemand vom Device (/dev/brpa3_959042_959218) lesen möchte
static ssize_t elgamal_read(struct file *file, char __user * out,
                size_t size, loff_t * off)
{
    ssize_t result = 0;
    struct locks *locks = file->private_data;
    
    //Warten bis Mutex frei wird
    if (mutex_lock_interruptible(&locks->lock)) {
        result = -ERESTARTSYS;
        goto out;
    }

    //Schleife falls nichts zum Lesen vorhanden ist
    while (msg_size <= 0) {
        mutex_unlock(&locks->lock);
        //Sollte msg_size negativ sein wurde diese vom read selbst gesetzt
        //Hiermit bricht der cat Kommando ab, wenn er gelesen hat.
        if (msg_size < 0) {
	    	result = 0;
	    	goto out_unlock;
    	}
    	if (file->f_flags & O_NONBLOCK) {
            result = -EAGAIN;
            goto out;
        }
        //Wenn der Speicher (msg) nichtmehr leer ist läuft das Programm weiter
        //Wenn ein Interrupt von außen kommt(zB durch Strg+C) wird der Teil im 
        //if-Statement ausgeführt
        if (wait_event_interruptible
            (locks->read_queue, msg_size != 0)) {
            result = -ERESTARTSYS;
            goto out;
        }
        //Sollte was im Speicher stehen muss wieder ein Mutex angeforder werden.
        if (mutex_lock_interruptible(&locks->lock)) {
            result = -ERESTARTSYS;
            goto out;
        }
    } 

    //Durch die Konstante Array Größe von msg darf size nicht größer buffer_size sein
    //(cat benutzt zB eine größere size)
    if (size > buffer_size) {
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
    //-1 Wird gesetzt, damit zB cat nicht endlos weiter ließt kann (Abgefangen weiter oben)
    msg_size = -1;

 out_unlock:
    mutex_unlock(&locks->lock);
 out:
    return result;
}

//Entschlüsselt eine gegebene Zahl anhand des Elgamal Algorithmus
//Dazu werden die Globalen Parameter genutzt
static int decrypt(int c)
{
    // a*(p-2)
    int min_one = secret*(order-2);
    // Formel (2) -                     B ^ a*(p-2) mod p
    int b_min_one = mod_exp(openkey_sender, min_one, order);
    // B⁻¹*c
    c = b_min_one * c;
    // Formel (1)
    c = c % order;

    return c;
}

//Die write Methode wird aufgerufen wenn ein Programm in das Device schreiben 
//möchte (/dev/brpa3_959042_959218)
static ssize_t elgamal_write(struct file *file, const char __user * in,
                 size_t size, loff_t * off)
{	

    struct locks *locks = file->private_data;
    ssize_t result = 0;
    int c = 0;

    //Eingaben die Länger als buffer_size sind kann das Modul zur Laufzeit nicht verarbeiten
    if (size > buffer_size) {
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
    return result;
}

//Aktualisiert anhand des neuen secret Wertes auch den openkey
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
long elgamal_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{	
	//Variable um Eingaben entgegen nehmen zu können
    brpa3_args variable;
    //Switch-Case um das eingegebene Kommando zu entschlüsseln
    switch(cmd)
    {	
    	//Setzt den secret Wert und aktualisiert den openkey
        case BRPA3_SET_SECRET:
            if (copy_from_user(&variable, (brpa3_args *)arg, sizeof(brpa3_args)))
            {
                return -EACCES;
            }
            update_keys(variable.value);
            break;
        //Setzt den openkey vom Sender
        case BRPA3_SET_OPENKEY:
            if (copy_from_user(&variable, (brpa3_args *)arg, sizeof(brpa3_args)))
            {
                return -EACCES;
            }
            openkey_sender = variable.value;
            break;
        //Gibt den aktuellen openkey Wert zurück
        case BRPA3_GET_OPENKEY:
            variable.value = openkey;
            if (copy_to_user((brpa3_args *)arg, &variable, sizeof(brpa3_args)))
            {
                return -EACCES;
            }
            break;
        //Fehlermeldung falls falschen Kommando eingegeben wurde
        default:
            return -EINVAL;
    }

    return 0;
}

//Open Methode wird aufgerufen wenn das Device geöffnet wird
static int elgamal_open(struct inode *inode, struct file *file)
{	
	//Erzeugt ein locks struct
	struct locks *locks;
	int err = 0;
	//Initialisiert die Locks
	locks = queue_lock_init();
	if(unlikely(!locks)){
		err = -ENOMEM;
		goto out;
	}
	//Speicher diese im Device 	
	file->private_data = locks;
	out:
		return err;
}

//Release wird aufgerufen wenn das Device geschlossen wird
static int elgamal_release(struct inode *inode, struct file *file)
{	
	//Holt sich die Locks
    struct locks *locks = file->private_data;
    //Gibt Speicher wieder frei
    queue_lock_exit(locks);

    return 0;
}

static struct file_operations elgamal_fops = {
    .owner = THIS_MODULE,
    .open = elgamal_open,
    .write = elgamal_write,
    .read = elgamal_read,
    .release = elgamal_release,
    .unlocked_ioctl = elgamal_ioctl,
    .llseek = noop_llseek
};

static struct miscdevice elgamal_misc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "brpa3_959042_959218",
    .fops = &elgamal_fops
};

static int __init elgamal_init(void)
{
    misc_register(&elgamal_misc_device);
    printk(KERN_INFO
        "brpa3_959_042 device has been registered.\n");
    msg = kzalloc(sizeof(char)*buffer_size, GFP_KERNEL);
    if (unlikely(!msg)) {
    	return -1;
    }
    msg_size = 0;
    return 0;   
}

static void __exit elgamal_cleanup(void)
{	
	kfree(msg);
    misc_deregister(&elgamal_misc_device);
    printk(KERN_INFO "Cleaning up module.\n");
}

module_init(elgamal_init);
module_exit(elgamal_cleanup);
