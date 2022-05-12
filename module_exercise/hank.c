#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#define DEVICE_NAME "hankdev"
#define NUM_DEVICES 3
#define BUF_LEN 128 /* Max length of the message from the device */

static int hank_open(struct inode *, struct file *);
static int hank_release(struct inode *, struct file *);
static ssize_t hank_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t hank_write(struct file *, const char __user *, size_t, loff_t *);

static int major_num; /* major number assigned to our device driver */
static struct class *cls;
 
static struct file_operations hankdev_fops = {
    .owner = THIS_MODULE,
    .read = hank_read,
    .write = hank_write, 
    .open = hank_open,
    .release = hank_release,
};

static char msg[BUF_LEN]; /* The msg the device will give when asked */

static int __init hank_init(void) {
    //int i = 0;
    printk(KERN_INFO "Init Hank Module !\n");

    // register device driver with dynamic major number assigned by kernel
    major_num = register_chrdev(0, DEVICE_NAME, &hankdev_fops);

    if (major_num < 0) {
        pr_alert("[Module][%d] Registering char device failed with %d\n", current->pid, major_num);
        return major_num;
    }
 
    // A warpper macro for printk (linux/printk.h)
    pr_info("[Module][%d] Assign major number %d to Hank device driver.\n", current->pid, major_num);
 
    cls = class_create(THIS_MODULE, DEVICE_NAME);

    // create single device
    device_create(cls, NULL, MKDEV(major_num, 0), NULL, "hank%d", 0);
    pr_info("[Module][%d] Devices created on /dev/hank%d\n", current->pid, 0);
 
    // create multiple devices
    // for (i = 0; i < NUM_DEVICES; i++) {
    //     device_create(cls, NULL, MKDEV(major_num, i), NULL, "hank%d", i);
    //     pr_info("Devices created on /dev/hank%d\n", i);
    // }

    return 0;
}
  
static void __exit hank_exit(void) {
    //int i = 0;
    printk(KERN_INFO "Exit Hank Module !\n");

    // destroy single device
    device_destroy(cls, MKDEV(major_num, 0));
    pr_info("[Module][%d] Devices /dev/hank%d destroyed\n", current->pid, 0);
 
    // destroy multiple devices
    // for (i = 0; i < NUM_DEVICES; i++) {
    //     device_destroy(cls, MKDEV(major_num, i));
    //     pr_info("Devices /dev/hank%d destroyed\n", i);
    // }

    class_destroy(cls);

    // unregister character device driver
    unregister_chrdev(major_num, DEVICE_NAME);
}

static int hank_open(struct inode *inode, struct file *file) {
    pr_info("[Module][%d] Hank device with minor %d was opened\n", current->pid, iminor(inode));
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

    if (length < BUF_LEN) {
        size = length;
    } else {
        size = BUF_LEN;
    }

    unread = copy_to_user(buf, msg, size);
    return size - unread;
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

// Will be invoked when insmod  
module_init(hank_init);
// Will be invoked when rmmod
module_exit(hank_exit);

MODULE_DESCRIPTION("A character device driver exercise");
MODULE_VERSION("1.0");
MODULE_AUTHOR("Hank Li");
MODULE_LICENSE("GPL");