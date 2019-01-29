#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/input-polldev.h>
#include <linux/gpio.h>

#define ledPort 22

struct nunchuk_dev {
	struct i2c_client *i2c_client;
	struct input_polled_dev *polled_input;
};

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


static void
nunchuk_poll(struct input_polled_dev *polled_input)
{
	u8 recv[6];
	u8 joystick[2];
	u16 accelerometer[3];
	int ret = 0;
	char z_pressed = 0, c_pressed = 0;

	struct nunchuk_dev *dev = polled_input->private;
	struct i2c_client *client = dev->i2c_client;

	ret = nunchuk_read_registers(client, recv);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to read Nunchuk registers\n");
		return;
	}

	c_pressed = recv[5] & 0x02;
	c_pressed >>= 1;
	c_pressed ^= 0x01;

	z_pressed = recv[5] & 0x01;
	z_pressed ^= 0x01;

	accelerometer[0] = recv[2] << 2;
	accelerometer[0] |= (recv[5] & 0x0C) >> 2;

	accelerometer[1] = recv[3] << 2;
	accelerometer[1] |= (recv[5] & 0x30) >> 4;

	accelerometer[2] = recv[4] << 2;
	accelerometer[2] |= (recv[5] & 0x60) >> 6;

	joystick[0] = recv[0];
	joystick[1] = recv[1];

	input_event(polled_input->input, EV_KEY, BTN_Z, z_pressed);
	input_event(polled_input->input, EV_KEY, BTN_C, c_pressed);
	//input_report_abs(polled_input->input, ABS_RX, accelerometer[0]);
	//input_report_abs(polled_input->input, ABS_RY, accelerometer[1]);
	//input_report_abs(polled_input->input, ABS_RZ, accelerometer[2]);
	input_report_abs(polled_input->input, ABS_X, joystick[0]);
	input_report_abs(polled_input->input, ABS_Y, joystick[1]);
	input_sync(polled_input->input);
	
	if(accelerometer[0]>693 || accelerometer[0]<331)
		gpio_set_value(ledPort,1);
	else
		gpio_set_value(ledPort,0);
}

static int
nunchuk_probe(struct i2c_client *client,
	      const struct i2c_device_id *id)
{
	int ret = 0;
	u8 buf[2];
	struct input_polled_dev *polled_input;
	struct input_dev *input;
	struct nunchuk_dev *dev;

	gpio_direction_output(ledPort,0);

	dev = devm_kzalloc(&client->dev, sizeof(struct nunchuk_dev), GFP_KERNEL);
	if (!dev) {
		dev_err(&client->dev, "failed to allocate memory for Nunchuk dev\n");
		goto exit_failed;
	}

	polled_input = input_allocate_polled_device();
	if (!polled_input) {
		dev_err(&client->dev, "failed to allocate memory for polled_input\n");
		goto exit_nunchuk_dev;
	}

	dev->i2c_client = client;
	dev->polled_input = polled_input;
	polled_input->private = dev;
	polled_input->poll = nunchuk_poll;
	polled_input->poll_interval = 50;
	i2c_set_clientdata(client, dev);
	input = polled_input->input;
	input->dev.parent = &client->dev;

	input->name = "Wii Nunchuk";
	input->id.bustype = BUS_I2C;

	set_bit(EV_ABS, input->evbit);
	set_bit(ABS_X, input->absbit);
	set_bit(ABS_Y, input->absbit);
	set_bit(ABS_RX, input->absbit);
	set_bit(ABS_RY, input->absbit);
	set_bit(ABS_RZ, input->absbit);
	set_bit(EV_KEY, input->evbit);
	set_bit(BTN_C, input->keybit);
	set_bit(BTN_Z, input->keybit);
	set_bit(INPUT_PROP_ACCELEROMETER, input->propbit);

	input_set_abs_params(input, ABS_X, 30, 220, 4, 8);

	if (!input->absinfo) {
		dev_err(&client->dev, "input->absinfo allocation failed\n");
		goto exit_nunchuk_dev;
	}

	input_set_abs_params(input, ABS_Y, 40, 200, 4, 8);

	input_set_abs_params(input, ABS_RX, 0, 0x3ff, 4, 8);
	input_set_abs_params(input, ABS_RY, 0, 0x3ff, 4, 8);
	input_set_abs_params(input, ABS_RZ, 0, 0x3ff, 4, 8);


	pr_info("nunchuk_probe is called\n");

	buf[0] = 0x40;
	buf[1] = 0x00;
	ret = i2c_master_send(client, buf, 2);
	if (ret != 2) {
		dev_err(&client->dev, "i2c send failed %d\n", ret);
		goto exit_polled_device;
	}

	udelay(1000);

	buf[0] = 0x00;
	ret = i2c_master_send(client, buf, 1);
	if (ret != 1) {
		dev_err(&client->dev, "i2c send failed %d\n", ret);
		goto exit_polled_device;
	}

	ret = input_register_polled_device(polled_input);
	if (ret != 0) {
		dev_err(&client->dev, "Failed to register polled input 0x%x\n", ret);
		goto exit_polled_device;
	}

	return 0;

exit_polled_device:
	input_free_polled_device(polled_input);
exit_nunchuk_dev:
	devm_kfree(&client->dev, dev);
exit_failed:
	return ret;

}

static int
nunchuk_remove(struct i2c_client *client)
{
	struct nunchuk_dev *dev;

	gpio_set_value(ledPort,0);
	gpio_free(ledPort);

	pr_info("nunchuk_remove is called\n");

	dev = i2c_get_clientdata(client);
	input_unregister_polled_device(dev->polled_input);
	input_free_polled_device(dev->polled_input);
	devm_kfree(&client->dev, dev);

	return 0;
}

static const struct i2c_device_id nunchuk_id[] = {
	{ "nunchuk", 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, nunchuk_id);

static struct of_device_id nunchuk_dt_match[] = {
	{ .compatible = "nintendo,nunchuk", },
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
