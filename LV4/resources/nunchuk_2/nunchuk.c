#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/delay.h>

static int
nunchuk_read_registers(struct i2c_client *client,
		u8 *recv)
{
	u8 buf[1];
	int ret;

	mdelay(10);

	buf[0] = 0x00;
	ret = i2c_master_send(client, buf, 1);
	if (ret != 1) {
		dev_err(&client->dev, "i2c send failed %d\n", ret);
		return ret < 0 ? ret : -EIO;
	}

	mdelay(10);

	ret = i2c_master_recv(client, recv, 6);
	if (ret != 6) {
		dev_err(&client->dev, "i2c recv failed %d\n", ret);
		return ret < 0 ? ret : -EIO;
	}

	return 0;
}

static int
nunchuk_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int ret;
	u8 buf[2];
	u8 recv[6];
	char z_pressed, c_pressed;
	int acc_x, acc_y, acc_z, joy_x, joy_y, temp;

	printk(KERN_ALERT "nunchuk_probe is called\n");

	buf[0] = 0x40;
	buf[1] = 0x00;
	ret = i2c_master_send(client, buf, 2);
	if (ret != 2) {
		dev_err(&client->dev, "i2c send failed %d\n", ret);
		goto exit_failed;
	}

	udelay(1000);

	buf[0] = 0x00;
	ret = i2c_master_send(client, buf, 1);
	if (ret != 1) {
		dev_err(&client->dev, "i2c send failed %d\n", ret);
		goto exit_failed;
	}

	ret = nunchuk_read_registers(client, recv);
	if (ret < 0) {
		dev_err(&client->dev, "nunchuk_read_registers failed %d\n", ret);
		goto exit_failed;
	}
	joy_x = recv[0];
	joy_y = recv[1];
	
	acc_x = recv[2];
	acc_x <<= 2;
	temp = recv[5] & 0x0C;
	temp >>= 2;
	acc_x = acc_x | temp;

	acc_y = recv[3];
	acc_y <<= 2;
	temp = recv[5] & 0x30;
	temp >>= 2;
	acc_y = acc_y | temp;

	acc_z = recv[4];
	acc_z <<= 2;
	temp = recv[5] & 0xC0;
	temp >>= 2;
	acc_z = acc_z | temp;

	c_pressed = recv[5] & 0x02;
	c_pressed >>= 1;
	c_pressed ^= 0x01;

	z_pressed = recv[5] & 0x01;
	z_pressed ^= 0x01;

	if (c_pressed) {
		dev_info(&client->dev, "C button pressed");
	}

	if (z_pressed) {
		dev_info(&client->dev, "Z button pressed");
	}
	dev_info(&client->dev, "joystick X: %d; Y: %d\naccelerometer X: %d; Y: %d; Z: %d\n" , joy_x , joy_y , acc_x , acc_y , acc_z);

	return 0;

exit_failed:
		return ret;
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
