/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h> // file_operations
#include "aesdchar.h"
int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("KrishnaPraveen Pushpagiri"); /** TODO: fill in your name **/
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;

int aesd_open(struct inode *inode, struct file *filp)
{
    struct aesd_dev * devptr;
	
	PDEBUG("open");
    /**
     * TODO: handle open
    */
	devptr = container_of(inode->i_cdev, struct aesd_dev, cdev);
	filp->private_data = devptr;
	 
    return 0;
}

int aesd_release(struct inode *inode, struct file *filp)
{
    PDEBUG("release");
    /**
     * TODO: handle release
     */
    return 0;
}

ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = 0;
	ssize_t entry_offset, available_bytes;
    struct aesd_dev * devptr;
	struct aesd_buffer_entry *buff_data;
	
	PDEBUG("read %zu bytes with offset %lld",count,*f_pos);
    /**
     * TODO: handle read
     */
	devptr = filp->private_data;
	
	if (mutex_lock_interruptible(&devptr->cdevmtx))
		return -ERESTARTSYS;
	
	buff_data = aesd_circular_buffer_find_entry_offset_for_fpos(&devptr->buffer, *f_pos, &entry_offset);
	
	if (buff_data==NULL) {
		goto out;
	}
	
	available_bytes = buff_data->size - entry_offset;
	
	if (count < available_bytes)	
		retval = count;
	else
		retval = available_bytes;
	
	if (copy_to_user(buf, (buff_data->buffptr + entry_offset), retval))	{
		retval = -EFAULT;
		goto out;
	}
	
	*f_pos = *f_pos + retval;

out: 	
	mutex_unlock(&devptr->cdevmtx);
	
    return retval;
}

ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = -ENOMEM;
	ssize_t notcopied;
    struct aesd_dev * devptr;
	//struct aesd_buffer_entry entry;
    PDEBUG("write %zu bytes with offset %lld",count,*f_pos);
	/**
     * TODO: handle write
     */
	
	devptr = filp->private_data;
	
	if (mutex_lock_interruptible(&devptr->cdevmtx))
		return -ERESTARTSYS;
	
	if(devptr->entry.size == 0)	{
		devptr->entry.buffptr = kzalloc(count, GFP_KERNEL);
	}
	else	{
		devptr->entry.buffptr = krealloc(devptr->entry.buffptr, (devptr->entry.size + count), GFP_KERNEL);
	}
	
	if (devptr->entry.buffptr == NULL)	{
		goto out;
	}
	
	retval = count;
	notcopied = copy_from_user((void *)(&devptr->entry.buffptr[devptr->entry.size]), buf, count);
	
	if (notcopied)	{
		retval -= notcopied;
	}
	devptr->entry.size += retval;//(count - notcopied);
		
	if (strchr((char *)(devptr->entry.buffptr), '\n'))	{
		kfree(aesd_circular_buffer_add_entry(&devptr->buffer,&devptr->entry));
		devptr->entry.buffptr = NULL;
		devptr->entry.size = 0;
	}
	
	
out: 	
	mutex_unlock(&devptr->cdevmtx);
	
    return retval;
}
struct file_operations aesd_fops = {
    .owner =    THIS_MODULE,
    .read =     aesd_read,
    .write =    aesd_write,
    .open =     aesd_open,
    .release =  aesd_release,
};

static int aesd_setup_cdev(struct aesd_dev *dev)
{
    int err, devno = MKDEV(aesd_major, aesd_minor);

    cdev_init(&dev->cdev, &aesd_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &aesd_fops;
    err = cdev_add (&dev->cdev, devno, 1);
    if (err) {
        printk(KERN_ERR "Error %d adding aesd cdev", err);
    }
    return err;
}



int aesd_init_module(void)
{
    dev_t dev = 0;
    int result;
    result = alloc_chrdev_region(&dev, aesd_minor, 1,
            "aesdchar");
    aesd_major = MAJOR(dev);
    if (result < 0) {
        printk(KERN_WARNING "Can't get major %d\n", aesd_major);
        return result;
    }
    memset(&aesd_device,0,sizeof(struct aesd_dev));

    /**
     * TODO: initialize the AESD specific portion of the device
     */
	//REGISTER MUTEX
	mutex_init(&aesd_device.cdevmtx);
	
    result = aesd_setup_cdev(&aesd_device);

    if( result ) {
        unregister_chrdev_region(dev, 1);
    }
    return result;

}

void aesd_cleanup_module(void)
{
    dev_t devno = MKDEV(aesd_major, aesd_minor);

    cdev_del(&aesd_device.cdev);

    /**
     * TODO: cleanup AESD specific poritions here as necessary
     */
	 
	/*UNREGISTER*/

    unregister_chrdev_region(devno, 1);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
