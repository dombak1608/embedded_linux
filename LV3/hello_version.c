#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/utsname.h>

static struct timeval t;
static struct tm broken;

static char *who = NULL;
module_param(who, charp,0);

static int __init hello_init(void)
{
	do_gettimeofday(&t);
	time_to_tm(t.tv_sec, 0, &broken);
	pr_alert("Hello %s. Vrijeme: %d:%d:%d\n",who,broken.tm_hour, broken.tm_min, broken.tm_sec);
	pr_alert("linux version: %s\n",utsname()->version);
	return 0;
}
static void __exit hello_exit(void)
{
	pr_alert("Odem, LP\n");
}

module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");
