// Merkulov E. V. Ⓒ  2022
// Модуль инерциального управления БЛА самолетного и ракетного типа для: ARM Cortex-A53, ATmega16U2, ARM Cortex™ A8, Atom T5700, Altera Nios II Gen2, MKE15Z256VLH7
// Допускается к публикации

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "imu_device"
#define CLASS_NAME "imu"

static int majorNumber;
static char *imu_data = NULL;
static struct class* imuClass = NULL;
static struct device* imuDevice = NULL;

static int     imu_open(struct inode *, struct file *);
static int     imu_release(struct inode *, struct file *);
static ssize_t imu_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t imu_write(struct file *, const char __user *, size_t, loff_t *);

static struct file_operations fops = {
    .open = imu_open,
    .read = imu_read,
    .write = imu_write,
    .release = imu_release,
};

static int __init imu_init(void) {
    printk(KERN_INFO "IMU Driver: Initializing the IMU Driver
");

    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber<0) {
        printk(KERN_ALERT "IMU Driver: Failed to register a major number
");
        return majorNumber;
    }
    printk(KERN_INFO "IMU Driver: Registered correctly with major number %d
", majorNumber);

    imuClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(imuClass)) {
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "IMU Driver: Failed to register device class
");
        return PTR_ERR(imuClass);
    }
    printk(KERN_INFO "IMU Driver: Device class registered correctly
");

    imuDevice = device_create(imuClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(imuDevice)) {
        class_destroy(imuClass);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "IMU Driver: Failed to create the device
");
        return PTR_ERR(imuDevice);
    }
    printk(KERN_INFO "IMU Driver: Device created correctly
");

    imu_data = kmalloc(256, GFP_KERNEL);
    if (!imu_data) {
        printk(KERN_ALERT "IMU Driver: Failed to allocate memory
");
        return -ENOMEM;
    }

    return 0;
}

static void __exit imu_exit(void) {
    kfree(imu_data);
    device_destroy(imuClass, MKDEV(majorNumber, 0));
    class_unregister(imuClass);
    class_destroy(imuClass);
    unregister_chrdev(majorNumber, DEVICE_NAME);
    printk(KERN_INFO "IMU Driver: Goodbye from the IMU Driver!\n");
}

static int imu_open(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "IMU Driver: Device has been opened
");
    return 0;
}

static ssize_t imu_read(struct file *filep, char __user *buffer, size_t len, loff_t *offset) {
    int error_count = 0;
    error_count = copy_to_user(buffer, imu_data, len);

    if (error_count == 0) {
        printk(KERN_INFO "IMU Driver: Sent %zu characters to the user
", len);
        return len;
    }
    else {
        printk(KERN_ALERT "IMU Driver: Failed to send %d characters to the user
", error_count);
        return -EFAULT;
    }
}

static ssize_t imu_write(struct file *filep, const char __user *buffer, size_t len, loff_t *offset) {
    copy_from_user(imu_data, buffer, len);
    printk(KERN_INFO "IMU Driver: Received %zu characters from the user
", len);
    return len;
}

static int imu_release(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "IMU Driver: Device successfully closed
");
    return 0;
}

module_init(imu_init);
module_exit(imu_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple Linux char driver for IMU");
MODULE_VERSION("0.1");
