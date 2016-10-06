#include <linux/synaptics_i2c_rmi_ex.h>
#include <linux/gpio.h>
#include <linux/delay.h>

#define SYNAPTICS_GPIO_PIN		62
#define SYNAPTICS_GPIO_RESET_PIN 	58
static int synaptics_init(void)
{
	int ret;

      printk(KERN_ERR "synaptics_init\n");//tong test

	ret = gpio_request(SYNAPTICS_GPIO_PIN, "synaptics_irq");
	if (ret < 0) {
		pr_err("%s: gpio request failed\n", __func__);
		return ret;
	}

	ret = gpio_direction_input(SYNAPTICS_GPIO_PIN);
	if (ret < 0) {
		pr_err("%s: gpio direction config failed\n", __func__);
		gpio_free(SYNAPTICS_GPIO_PIN);
		return ret;
	}

	ret = gpio_request(SYNAPTICS_GPIO_RESET_PIN, "synaptics_reset");
	if (ret < 0) {
		pr_err("%s: gpio reset request failed\n", __func__);
		return ret;
	}

	ret = gpio_direction_output(SYNAPTICS_GPIO_RESET_PIN, 0);
	if (ret < 0) {
		pr_err("%s: gpio reset direction config failed\n", __func__);
		gpio_free(SYNAPTICS_GPIO_RESET_PIN);
		return ret;
	}
	mdelay(20);
	gpio_set_value(SYNAPTICS_GPIO_RESET_PIN, 1);
	mdelay(250);

	printk(KERN_ERR"zhangna request success\n");

	return 0;
}

void *synaptics_platform_data(void *info)
{
	static struct synaptics_ts_platform_data synaptics_pdata = {
		.init_platform_hw = synaptics_init,
		.irq_gpio = SYNAPTICS_GPIO_PIN,
	};

	return &synaptics_pdata;
}