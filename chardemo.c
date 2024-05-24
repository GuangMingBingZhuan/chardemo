#include <linux/printk.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/export.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/gfp.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/kern_levels.h>
#include <linux/types.h>

#include "chardemo.h"

#define DEVICE_NAME "chardemo"

struct chardemo_dev {
    struct cdev cdev;
    unsigned char *buffer;
};

static int major = CHARDEMO_MAGIC;
static size_t buffer_size = 4 * 1024;
static struct chardemo_dev *chardemo;

module_param(buffer_size, ulong, S_IRUGO);

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static loff_t device_llseek (struct file *, loff_t, int);
static long device_ioctl (struct file *, unsigned int, unsigned long);

static struct file_operations file_ops = {
    .owner = THIS_MODULE,
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release,
    .llseek = device_llseek,
    .unlocked_ioctl = device_ioctl,
};

static ssize_t device_read(struct file *file, char __user *buffer, size_t len, loff_t *offset)
{
    long long pos = *offset;
    size_t count = len;
    ssize_t ret = 0;
    struct chardemo_dev *dev = file->private_data;

    if (pos < 0 || pos >= buffer_size)
        ret = -EINVAL;
    if (pos + count > buffer_size)
        count = buffer_size - pos;
    
    if (copy_to_user(buffer, dev->buffer + pos, count))
        ret = -EFAULT;
    else {
        *offset += count;
        ret = count;

        printk(KERN_INFO "module chardemo read %lu bytes from %lld\n", count, pos);
    }

    return ret;
}

static ssize_t device_write(struct file *file, const char __user *buffer, size_t len, loff_t *offset)
{
    long long pos = *offset;
    size_t count = len;
    ssize_t ret = 0;
    struct chardemo_dev *dev = file->private_data;

    if (pos < 0 || pos >= buffer_size)
        ret = -EINVAL;
    if (pos + count > buffer_size)
        count = buffer_size - pos;

    if (copy_from_user(dev->buffer + pos, buffer, count))
        ret = -EFAULT;
    else {
        *offset += count;
        ret = count;

        printk(KERN_INFO "module chardemo wrote %lu bytes from %lld\n", count, pos);
    }

    return ret;
}

static int device_open(struct inode *inode, struct file *file)
{
    file->private_data = chardemo;
    try_module_get(THIS_MODULE);
    return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
    module_put(THIS_MODULE);
    return 0;
}

static loff_t device_llseek(struct file *file, loff_t offset, int whence)
{
    loff_t ret = 0;
    switch (whence) {
    case SEEK_SET:
        if (offset < 0 || offset >= buffer_size) {
            ret = -EINVAL;
            break;
        }
        file->f_pos = offset;
        ret = file->f_pos;
        break;
    case SEEK_CUR:
        if ((file->f_pos + offset >= buffer_size) || (file->f_pos + offset < 0)) {
            ret = -EINVAL;
            break;
        }
        file->f_pos += offset;
        ret = file->f_pos;
        break;
    case SEEK_END:
        if (offset >= 0 || offset + buffer_size < 0) {
            ret = -EINVAL;
            break;
        }
        file->f_pos = buffer_size + offset;
        ret = file->f_pos;
        break;
    default:
        ret = -EINVAL;
    }
    return ret;
}

static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct chardemo_dev *dev = file->private_data;

    switch(cmd) {
    case CHARDEMO_MEM_CLEAR:
        memset(dev->buffer, 0, buffer_size);
        printk(KERN_INFO "module chardemo buffer cleared\n");
        break;
    default:
        return -EINVAL;
    }
    return 0;
}

static int __init chardemo_init(void)
{
    int ret;
    dev_t devno = MKDEV(major, 0);

    printk(KERN_INFO "module chardemo initializing with buffer_size = 0x%lx\n", buffer_size);

    if (major)
        ret = register_chrdev_region(devno, 1, DEVICE_NAME);
    else {
        ret = alloc_chrdev_region(&devno, 0, 1, DEVICE_NAME);
        major = MAJOR(devno);
    }

    if (ret < 0)
        return ret;

    chardemo = kzalloc(sizeof(struct chardemo_dev), GFP_KERNEL);
    if (!chardemo) {
        unregister_chrdev_region(devno, 1);
        return -ENOMEM;
    }

    cdev_init(&chardemo->cdev, &file_ops);
    chardemo->cdev.owner = THIS_MODULE;
    ret = cdev_add(&chardemo->cdev, devno, 1);
    if (ret)
        printk(KERN_NOTICE "Error %d adding chardemo MAJOR: %d, MINOR: %d\n", ret, MAJOR(devno), MINOR(devno));

    printk(KERN_INFO "device chardemo MAJOR: %d, MINOR: %d\n", MAJOR(chardemo->cdev.dev), MINOR(chardemo->cdev.dev));

    chardemo->buffer = kzalloc(buffer_size, GFP_KERNEL);

    return ret;
}

static void __exit chardemo_exit(void)
{
    printk(KERN_INFO "module chardemo exiting\n");
    unregister_chrdev_region(chardemo->cdev.dev, 1);
    cdev_del(&chardemo->cdev);
    kfree(chardemo->buffer);
    kfree(chardemo);
}

module_init(chardemo_init);
module_exit(chardemo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("zhupw");
MODULE_DESCRIPTION("chardev demo");
