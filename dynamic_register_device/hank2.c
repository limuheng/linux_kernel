#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

#define DEVICE_NAME "hankdev"
#define NUM_DEVICES 2
#define BUF_LEN 128 /* Max length of the message from the device */

static int general_open(struct inode *, struct file *);
static int hank_open(struct inode *, struct file *);
static int hank_release(struct inode *, struct file *);
static ssize_t hank_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t hank_write(struct file *, const char __user *, size_t, loff_t *);
static ssize_t hank_write2(struct file *, const char __user *, size_t, loff_t *);

static int major_num = 0; /* major number assigned to our device driver */
static struct cdev hank_dev;
static struct class *cls;

static struct file_operations init_fops = {
    .owner = THIS_MODULE,
    .open = general_open
};

static struct file_operations hankdev_fops = {
    .owner = THIS_MODULE,
    .read = hank_read,
    .write = hank_write, 
    .open = hank_open,
    .release = hank_release,
};

static struct file_operations hankdev_fops2 = {
    .owner = THIS_MODULE,
    .read = hank_read,
    .write = hank_write2, 
    .open = hank_open,
    .release = hank_release,
};

static char msg[BUF_LEN]; /* The msg the device will give when asked */
static char msg2[BUF_LEN]; /* The msg the device will give when asked */

static int __init hank_init(void) {
    dev_t dev = MKDEV(major_num, 0);
    int i = 0;

    printk(KERN_INFO "Init Hank Module !\n");

    // alloc_chrdev_region returns 0 if success
    if (alloc_chrdev_region(&dev, 0, NUM_DEVICES, DEVICE_NAME) != 0) {
        pr_alert("[Module][%d] Failed to dynamic allocate major number for device: %s.\n",
                    current->pid, DEVICE_NAME);
        return -1;
    }

    major_num = MAJOR(dev);

    cdev_init(&hank_dev, &init_fops);
    hank_dev.owner = THIS_MODULE;
    hank_dev.ops = &init_fops;

    // cdev_add returns 0 if success
    if (cdev_add(&hank_dev, MKDEV(major_num, 0), NUM_DEVICES) != 0) {
        pr_alert("[Module][%d] Failed to add char device for device: %s.\n",
                    current->pid, DEVICE_NAME);
        return -1;
    }

    // A warpper macro for printk (linux/printk.h)
    pr_info("[Module][%d] Assign major number %d to Hank device driver.\n", current->pid, major_num);
 
    // register class to /sys/class
    cls = class_create(THIS_MODULE, DEVICE_NAME);

    // create device to /sys/class/<DEVICE_NAME>/device_name
    // create multiple devices
    for (i = 0; i < NUM_DEVICES; i++) {
        device_create(cls, NULL, MKDEV(major_num, i), NULL, "hank%d", i);
        pr_info("Devices created on /dev/hank%d\n", i);
    }

    return 0;
}
  
static void __exit hank_exit(void) {
    dev_t dev = MKDEV(major_num, 0);
    int i = 0;
    // destroy multiple devices
    for (i = 0; i < NUM_DEVICES; i++) {
        device_destroy(cls, MKDEV(major_num, i));
        pr_info("Devices /dev/hank%d destroyed\n", i);
    }

    // unregister class
    class_destroy(cls);

    cdev_del(&hank_dev);
    // release major umber
    unregister_chrdev_region(dev, NUM_DEVICES);

    printk(KERN_INFO "Exit Hank Module !\n");
}

static int general_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "[Module] general_open\n");

    switch (iminor(inode)) {
        case 0:
            file->f_op = &hankdev_fops;
            file->private_data = "1";
            break;
        case 1:
            file->f_op = &hankdev_fops2;
            file->private_data = "2";
            break;
        default:
            pr_err("[Module][%d] Try to open device with unsupported minor number: %d\n",
                     current->pid, iminor(inode));
            return -ENXIO;
    }

    if (file->f_op && file->f_op->open) {
        return file->f_op->open(inode, file);
    }

    return 0;
}

static int hank_open(struct inode *inode, struct file *file) {
    pr_info("[Module][%d] Hank device with minor %d was released\n", current->pid, iminor(inode));
    return 0;
}

static int hank_release(struct inode *inode, struct file *file) {
    pr_info("[Module][%d] Hank device with minor %d was released\n", current->pid, iminor(inode));
    return 0;
}

static ssize_t hank_read(struct file *file, char __user *buf /* buffer from user space */,
        size_t length, loff_t *offset) {
    size_t size = 0;
    int unread = 0;
    char *message = msg;

    if (strcmp((char *)file->private_data, "1") == 0) {
        message = msg;
    } else {
        message = msg2;
    }

    if (length < BUF_LEN) {
        size = length;
    } else {
        size = BUF_LEN;
    }

    unread = copy_to_user(buf, message, size);
    message = NULL;
    return size - unread;
    return 0;
}

static ssize_t hank_write(struct file *file, const char __user *buf /* buffer from user space */,
        size_t length, loff_t *offset) {
    int unwrite = 0;
    // clear device buffer
    memset(msg, 0x0, sizeof(msg));

    if (length > BUF_LEN) {
        length = BUF_LEN;
    }

    unwrite = copy_from_user(msg, buf, length);

    pr_info("[Module][%d] User write %s, unwrite: %d\n", current->pid, msg, unwrite);
    return length - unwrite;
}

static ssize_t hank_write2(struct file *file, const char __user *buf /* buffer from user space */,
        size_t length, loff_t *offset) {
    int unwrite = 0;
    // clear device buffer
    memset(msg2, 0x0, sizeof(msg2));

    if (length > BUF_LEN) {
        length = BUF_LEN;
    }

    unwrite = copy_from_user(msg2, buf, length);
    // override the first character
    msg2[0] = '2';

    pr_info("[Module][%d] User write %s, unwrite: %d\n", current->pid, msg2, unwrite);
    return length - unwrite;
}

// Will be invoked when insmod  
module_init(hank_init);
// Will be invoked when rmmod
module_exit(hank_exit);

MODULE_DESCRIPTION("A character device driver exercise");
MODULE_VERSION("1.0");
MODULE_AUTHOR("Hank Li");
MODULE_LICENSE("GPL");