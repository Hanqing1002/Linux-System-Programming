#include<linux/mutex.h>
#include<linux/module.h>
#include<linux/moduleparam.h>
#include<linux/kernel.h>
#include<linux/device.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/ioctl.h>
#include <linux/slab.h>		/* kmalloc() */
#include <asm/uaccess.h>	/* copy_*_user */
#include<linux/uaccess.h>
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/fcntl.h>	/* O_ACCMODE */

#ifndef MYCDEV_MAJOR
#define MYCDEV_MAJOR 0
#endif

#ifndef MYCDEV_NR_DEVS
#define MYCDEV_NR_DEVS 3
#endif

#define MYCDEV_MAGIC 'k'
#define MYCDEV_MAXNR 1
#define ASP_CLEAR_BUF _IO(MYCDEV_MAJOR,1)
#define MYCDEV_NAME "mycdev"

/*Custom data structure*/
struct asp_mycdev{
    struct cdev dev;
    char ramdisk[16*PAGE_SIZE];
    int ramdisk_size;
    struct mutex lock;
    int devNo;
};
static struct class *my_class; // Global variable for the device class

int mycdev_major = MYCDEV_MAJOR;
int mycdev_minor = 0;
int mycdev_nr_devs = MYCDEV_NR_DEVS;

module_param(mycdev_major,int,S_IRUGO);
module_param(mycdev_minor,int,S_IRUGO);
module_param(mycdev_nr_devs,int,S_IRUGO);

struct asp_mycdev *mycdevices;


int mycdev_open(struct inode *inode, struct file *filp){
    struct asp_mycdev *dev;
    dev = container_of(inode->i_cdev, struct asp_mycdev,dev);
    filp->private_data = dev;
	/*
    if((filp->f_flags & O_ACCMODE)==O_WRONLY){
        if(mutex_lock_interruptible(&dev->lock))
            return -ERESTARTSYS;
        mycdev_trim(dev);
        mutex_unlock(&dev->lock);
    }*/
    printk(KERN_INFO "mycdev: opened device");
    return 0;
}

ssize_t mycdev_read(struct file *filp, char *buffer, size_t bufSize, loff_t *curOffset){
    struct asp_mycdev *dev = filp->private_data;
    ssize_t retval = 0;
    if(mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;
    if(*curOffset >= dev->ramdisk_size)
        goto out;
    if(*curOffset+bufSize > dev->ramdisk_size)
        bufSize = dev->ramdisk_size - *curOffset;

    printk(KERN_INFO "mycdev: reading from device");

    retval = bufSize - copy_to_user(buffer, &(dev->ramdisk)+*curOffset, bufSize);
    if(retval!=bufSize){
        retval = -EFAULT;
        goto out;
    }
    *curOffset +=bufSize;
  out:
    mutex_unlock(&dev->lock);
    return retval;
}

ssize_t mycdev_write(struct file *filp, const char *buffer, size_t bufSize, loff_t *curOffset){
    struct asp_mycdev *dev = filp->private_data;
    ssize_t retval = -ENOMEM;
    if(mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;
    // fill in with zeros
    if(*curOffset > dev->ramdisk_size){
		if(memset(&(dev->ramdisk)+dev->ramdisk_size,'0',(*curOffset-dev->ramdisk_size))==NULL){
			printk(KERN_INFO "wrong");
    		mutex_unlock(&dev->lock);
			return -EFAULT;
		}
		//printk(KERN_INFO "mycdev: 0 is inserted");
    }     
    printk(KERN_INFO "mycdev: writing to device");
    if(copy_from_user(&(dev->ramdisk) + *curOffset, buffer, bufSize)){
        retval = -EFAULT;
        goto out;
    }
    *curOffset +=bufSize;
    retval = bufSize;
    if(dev->ramdisk_size < *curOffset)
        dev->ramdisk_size = *curOffset;
  out:
    mutex_unlock(&dev->lock);
    return retval;
}

loff_t mycdev_lseek(struct file *filp, loff_t off, int whence){
	struct asp_mycdev *dev = filp->private_data;
	loff_t testpos; 
	if(mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;
	printk(KERN_INFO "mycdev: lseek is invoked");
    switch(whence){
	  case 0: /* SEEK_SET */
		testpos = off;
		break;

	  case 1: /* SEEK_CUR */
		testpos = filp->f_pos + off;
		break;

	  case 2: /* SEEK_END */
		testpos = dev->ramdisk_size + off;
		break;

	  default: /* can't happen */
		return -EINVAL; 
    }
    if(testpos < 0) return -EINVAL;
	
	if(testpos>dev->ramdisk_size){		
		if(memset(&(dev->ramdisk)+dev->ramdisk_size,'0',(testpos-dev->ramdisk_size))==NULL){ //???? why '0' is okay, 0 is not?
			printk(KERN_INFO "wrong");
    		mutex_unlock(&dev->lock);
			return -EFAULT;
		}
		printk(KERN_INFO "mycdev: 0 is inserted");
		dev->ramdisk_size = testpos;
	}
	
    filp->f_pos = testpos;

	mutex_unlock(&dev->lock);
    return testpos;
}

long mycdev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){
	
    int err=0, retval=0;
    struct asp_mycdev *dev = filp->private_data;

    //if(_IOC_TYPE(cmd)!=MYCDEV_MAGIC) return -ENOTTY;
    if(_IOC_NR(cmd)>MYCDEV_MAXNR) 
		return -ENOTTY;
    if(_IOC_DIR(cmd) & _IOC_READ)
        err = !access_ok(VERIFY_WRITE,(void __user *)arg,_IOC_SIZE(cmd));
    if(_IOC_DIR(cmd) & _IOC_WRITE)
        err = !access_ok(VERIFY_READ,(void __user *)arg,_IOC_SIZE(cmd));
    if(err) return -EFAULT;


	printk(KERN_INFO "ioctl is invoked");
    switch(cmd){
        case ASP_CLEAR_BUF:
            if(mutex_lock_interruptible(&dev->lock))
                return -ERESTARTSYS;
            memset(dev->ramdisk,0,dev->ramdisk_size);
    		dev->ramdisk_size = 0;
            filp->f_pos = 0;
			printk(KERN_INFO "mycdev: cleared!");
            mutex_unlock(&dev->lock);
            break;
        default:
            return -ENOTTY;
    }
    return retval;
}

int mycdev_close(struct inode *inode, struct file *flip){
    printk(KERN_INFO "mycdev: closed device");
    return 0;
}


struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = mycdev_open,
    .release = mycdev_close,
    .read = mycdev_read,
    .write = mycdev_write,
    .llseek = mycdev_lseek,
    .unlocked_ioctl = mycdev_ioctl
};

static void __exit mycdev_exit(void){
    int i;
    dev_t devno = MKDEV(mycdev_major,mycdev_minor);
    if(mycdevices)
        for(i=0;i<mycdev_nr_devs;i++){
            cdev_del(&mycdevices[i].dev);
            device_destroy(my_class, MKDEV(mycdev_major,mycdev_minor+i));
        }
    class_destroy(my_class);
    unregister_chrdev_region(devno, mycdev_nr_devs);
    kfree(mycdevices); // free momory after unregister
    printk(KERN_INFO "mycdev: unloaded module");
}


static int __init mycdev_init(void){
    int retval,i;
    dev_t dev = 0;
    
    struct device *dev_ret;

    if(mycdev_major){ //static allocation of major number
        dev = MKDEV(mycdev_major,mycdev_minor);
        retval = register_chrdev_region(dev,mycdev_nr_devs,"mycdev");
    } else{
        retval = alloc_chrdev_region(&dev, mycdev_minor,
                    mycdev_nr_devs,"mycdev");
        mycdev_major = MAJOR(dev); //dynamic allocation
    }
    if(retval<0){
        printk(KERN_ALERT "mycdev: failed to allocate major number");
        return retval;
    }

    my_class = class_create(THIS_MODULE,"mycdev");
    if(!my_class){
        unregister_chrdev_region(dev, mycdev_nr_devs);
        return PTR_ERR(my_class);
    }

    printk(KERN_INFO "mycdev: major number- %d",mycdev_major);

    /*allocate the devices dynamically*/
    mycdevices = kmalloc(mycdev_nr_devs * sizeof(struct asp_mycdev),
                    GFP_KERNEL);
    if(!mycdevices){
        mycdev_exit();
        return -ENOMEM;
    }
    memset(mycdevices, 0, mycdev_nr_devs * sizeof(struct asp_mycdev));
    /*Initialize each device*/
    
    for(i=0;i<mycdev_nr_devs;i++){
	    //printk(KERN_INFO "lala");
        mutex_init(&mycdevices[i].lock);
		
        dev = MKDEV(mycdev_major,mycdev_minor+i);
        dev_ret = device_create(my_class,NULL,dev,NULL,"%s%d",MYCDEV_NAME,i); //!!!PAY ATTENTION!!
        if (!dev_ret)
            mycdev_exit();
        cdev_init(&(mycdevices+i)->dev,&fops);
        (mycdevices+i)->dev.owner = THIS_MODULE;
      	(mycdevices+i)->dev.ops = &fops;
        retval = cdev_add(&(mycdevices+i)->dev,dev,1);
        if(retval)
            printk(KERN_ALERT "mycdev: failed to add mycdev%d to kernel", i);
    }
    return 0; // succeed;
}


module_init(mycdev_init);
module_exit(mycdev_exit);
MODULE_LICENSE("GPL");
