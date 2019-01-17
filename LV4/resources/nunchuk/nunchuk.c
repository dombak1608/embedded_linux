#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>

static int
nunchuk_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	printk(KERN_ALERT "nunchuk_probe is called\n");
	return 0;
}

static int
nunchuk_remove(struct i2c_client *client)
{
	printk(KERN_ALERT "nunchuk_remove ic called\n");
	return 0;
}

static const struct i2c_device_id nunchuk_id[] = {
	{"nunchuk", 0},
	{},
};
MODULE_DEVICE_TABLE(i2c, nunchuk_id);

static struct of_device_id nunchuk_dt_match[] = {
	{.compatible = "nintendo,nunchuk",},
	{ },
};

static struct i2c_driver nunchuk_driver = {
	.driver = {
		.name = "nunchuk",
		.owner = THIS_MODULE,
		.of_match_table = nunchuk_dt_match,
	},
	.probe = nunchuk_probe,
	.remove = nunchuk_remove,
	.id_table = nunchuk_id,
};

module_i2c_driver(nunchuk_driver);
MODULE_LICENSE("GPL");
