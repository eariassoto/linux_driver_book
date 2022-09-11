#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>

#define MOD_NAME "hello_dev"

static dev_t dev_number;
static struct cdev *driver_object;
static struct class *driver_class;
static struct device *hello_dev;

static char HELLO_WORLD[] = "Hello World!\n";

static int driver_open(struct inode *dev_data, struct file *instance)
{
    dev_info(hello_dev, "Opening driver...");
    return 0;
}

static int driver_close(struct inode *dev_data, struct file *instance)
{
    dev_info(hello_dev, "Closing driver...");
    return 0;
}

static ssize_t driver_read(struct file *instance, char __user *user, size_t count, loff_t *offset)
{
    unsigned long not_copied, to_copy, copied;

    to_copy = min(count, strlen(HELLO_WORLD) + 1);
    not_copied = copy_to_user(user, HELLO_WORLD, to_copy);

    copied = to_copy - not_copied;
    *offset = copied;

    return copied;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = driver_read,
    .open = driver_open,
    .release = driver_close,
};

static int __init mod_init(void)
{
    if (alloc_chrdev_region(&dev_number, 0, 1, MOD_NAME) < 0)
        return -EIO;

    driver_object = cdev_alloc();
    if (driver_object == NULL)
        goto free_dev_number;

    driver_object->owner = THIS_MODULE;
    driver_object->ops = &fops;
    if (cdev_add(driver_object, dev_number, 1))
        goto free_cdev;

    driver_class = class_create(THIS_MODULE, MOD_NAME);
    if (IS_ERR(driver_class))
    {
        pr_err("hello: no udev supported\n");
        goto free_cdev;
    }

    hello_dev = device_create(driver_class, NULL, dev_number, NULL, "%s", MOD_NAME);
    if(IS_ERR(hello_dev)){
        pr_err("hello: failed to create device\n");
        goto free_class;
    }
    return 0;
free_class:
    class_destroy(driver_class);
free_cdev:
    kobject_put(&driver_object->kobj);
free_dev_number:
    unregister_chrdev_region(dev_number, 1);

    return -EIO;
}

static void __exit mod_terminate(void)
{
    device_destroy(driver_class, dev_number);
    class_destroy(driver_class);
    cdev_del(driver_object);
    unregister_chrdev_region(dev_number, 1);
}

module_init(mod_init);
module_exit(mod_terminate);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Emmanuel Arias");
MODULE_DESCRIPTION("A virtual device that prints Hello World");
