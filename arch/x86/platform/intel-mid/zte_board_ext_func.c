/********************************************************************
added by pengtao for zte ext functions
***********************************************************************/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/lnw_gpio.h>
#include <linux/gpio.h>

static int board_id_Intel = -1;

struct version_gpios {
	const char *name;
	int gpio;
};

static struct version_gpios hardware_ver_gpios[] = {
	{"VERSION0", 46},
	{"VERSION1", 47},
	{"VERSION2", 48},
};

#define ZTE_HVERPIN_NUM   3  // cell phone use 3 pins

int zte_get_board_id(void)
{
	return board_id_Intel;
}

int zte_get_board_id_func(void)
{
	int PinValue[ZTE_HVERPIN_NUM];
	int i, ret ;
	int VerNum = -1;

	for (i = 0; i < ZTE_HVERPIN_NUM; i++)
	{
		ret = gpio_request(hardware_ver_gpios[i].gpio, hardware_ver_gpios[i].name);
		if (ret < 0) {
			pr_err("%s: Unable to request gpio %d\n",
				__func__, hardware_ver_gpios[i].gpio);
			goto err_gpio_req;
		}

		ret = gpio_direction_input(hardware_ver_gpios[i].gpio);
		if (ret < 0) {
			pr_err("%s: Unable to set direction for gpio %d\n", __func__,
				hardware_ver_gpios[i].gpio);
			goto err_gpio_dir;
		}

		PinValue[i] = gpio_get_value(hardware_ver_gpios[i].gpio);
	}

	PinValue[0] = 0x01&(PinValue[0] >> 14);
	PinValue[1] = 0x01&(PinValue[1] >> 15);
	PinValue[2] = 0x01&(PinValue[2] >> 16);

	VerNum = PinValue[0] + PinValue[1]*2 + PinValue[2]*4;
	board_id_Intel = VerNum;
	printk("+zte_get_board_id= %x PinValue[0]=%x, PinValue[1]=%x,PinValue[2]=%x,+\n",VerNum,PinValue[0],PinValue[1] ,PinValue[2]);

	return ret;

	err_gpio_req:
	err_gpio_dir:
		printk("free gpio %d \n",hardware_ver_gpios[i].gpio);
		gpio_free(hardware_ver_gpios[i].gpio);
		return ret;
}

static ssize_t zte_read_hver(char *page, char **start, off_t off,
		int count, int *eof, void *data)
{
	int board_id = -1;
	int len = 0;

	board_id = zte_get_board_id();
	printk("zte_read_hver board_id=%d \n", board_id);
	
	switch (board_id)
	{
		case 0:
#if defined(CONFIG_PROJECT_V975)
			snprintf(page, 5, "wjdA");
#elif defined(CONFIG_PROJECT_P940V10)
			snprintf(page, 5, "wjeA");
#elif defined(CONFIG_PROJECT_P940T01)
			snprintf(page, 5, "tz6A");
#endif
			len = 5;
		break;

		case 1:
#if defined(CONFIG_PROJECT_V975)
      snprintf(page, 5, "wjdB");
#elif defined(CONFIG_PROJECT_P940V10)
			snprintf(page, 5, "wjeB");
#elif defined(CONFIG_PROJECT_P940T01)
			snprintf(page, 5, "tz6B");
#endif
			len = 5;
		break;

		case 2:
#if defined(CONFIG_PROJECT_V975)
			snprintf(page, 5, "wjdC");
#elif defined(CONFIG_PROJECT_P940V10)
			snprintf(page, 5, "wjeC");
#elif defined(CONFIG_PROJECT_P940T01)
			snprintf(page, 5, "tz6C");
#endif
			len = 5;
		break;

		case 3:
#if defined(CONFIG_PROJECT_V975)
			snprintf(page, 5, "wjdD");
#elif defined(CONFIG_PROJECT_P940V10)
			snprintf(page, 5, "wjeD");
#elif defined(CONFIG_PROJECT_P940T01)
			snprintf(page, 5, "tz6D");
#endif
			len = 5;
		break;

		default:
		snprintf(page, 4, "err");
		len = 4;
		break;
	}
	
	return len;
}

static void zte_creat_hver_proc_file(void)
{
	struct proc_dir_entry *prop_proc_file =
	create_proc_entry("driver/hardwareVersion", 0444, NULL);

	if (prop_proc_file) {
		prop_proc_file->read_proc = zte_read_hver;
		prop_proc_file->write_proc = NULL;
	}
}

static int __init zte_hver_proc_init(void)
{
	zte_creat_hver_proc_file();
	return 0;
}

arch_initcall(zte_hver_proc_init);
