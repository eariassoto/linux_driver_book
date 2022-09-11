#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>

#define MOD_NAME "template_module"

static dev_t dev_number;
static struct cdev *driver_object;
struct class *driver_class;

static struct file_operations fops = {};

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
        goto free_cdev;

    device_create(driver_class, NULL, dev_number, NULL, "%s", MOD_NAME);
    return 0;

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
MODULE_DESCRIPTION("Simple module template without any features");
