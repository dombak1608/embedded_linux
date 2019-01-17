#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "hellochar"
#define CLASS_NAME  "hello"

static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static int major_number;
static char message[256] = {0};
static size_t size_of_message;
static int number_opens = 0;

static struct class *hello_char_class = NULL;
static struct device *hello_char_device = NULL;

static struct file_operations fops =
{
	.open = dev_open,
	.read = dev_read,
	.write = dev_write,
	.release = dev_release,
};

static int __init hello_char_init(void)
{
	major_number = register_chrdev(0, DEVICE_NAME, &fops);
	if (major_number < 0) {
		printk(KERN_ALERT "HelloChar failed to register a major number.\n");
		return major_number;
	}

	printk(KERN_INFO "HelloChar: registered correctly with major number %d\n", major_number);
	
	hello_char_class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(hello_char_class)) {
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register device class\n");
		return PTR_ERR(hello_char_class);
	}

	printk(KERN_INFO "Hello Char: device class registered correctly\n");

	hello_char_device = device_create(hello_char_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
	if (IS_ERR(hello_char_device)) {
		class_destroy(hello_char_class);
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the device\n");
		return PTR_ERR(hello_char_device);
	}

	printk(KERN_INFO "HelloChar: device class created correctly\n");
	return 0;
}

static void __exit hello_char_exit(void)
{
	device_destroy(hello_char_class, MKDEV(major_number, 0));
	class_unregister(hello_char_class);
	class_destroy(hello_char_class);
	unregister_chrdev(major_number, DEVICE_NAME);
	printk(KERN_INFO "HelloChar: Goodbye from Linux Kernel Module.\n");
}

static int dev_open(struct inode *inodep, struct file *filep)
{
	number_opens++;
	printk(KERN_INFO "HelloChar: Device has been opened %d time(s)\n", number_opens);
	return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
	ssize_t ret = 0;

	if (copy_to_user(buffer, message, size_of_message)) {
		ret = -EFAULT;
	} else {
		ret = size_of_message;
	}
	
	size_of_message = 0;
	return ret;
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
	sprintf(message, "%s", buffer);
	size_of_message = strlen(message);
	printk(KERN_INFO "HelloChar: Received %zu characters from the user\n", len);
	return len;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
	printk(KERN_INFO "HelloChar: Device successfully closed\n");
	return 0;
}

module_init(hello_char_init);
module_exit(hello_char_exit);

MODULE_LICENSE("GPL");

