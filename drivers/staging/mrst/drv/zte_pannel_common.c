/*
* Panel common
* Authors: lijie
*/
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#include "zte_pannel_common.h"

#define LCD_PROP_NAME "Lcdscreen"
#define LCD_BACKLIGHT_DEBUG_FILE "driver/backlight_debug" 

extern int prop_add(char *devname, char *item, char *value);
static int zte_debug = 0;

int add_panel_config_prop(char *panel_name,
	char *ic_name, int max_x, int max_y) 
{
	int ret = 0;

	if (ic_name) {
		ret = prop_add(LCD_PROP_NAME, "ic-type", ic_name);
		if (ret) {
			printk("added prop ic-type error/n");
			goto failed;
		}
	}

	if (panel_name) {
		ret = prop_add(LCD_PROP_NAME, "class-type", panel_name);
		if (ret) {
			printk("added prop class-type error/n");
			goto failed;
		}
	}

	if (max_x > 0 || max_y > 0) {
		char buf[20] = {0};
		snprintf(buf, sizeof(buf), "%dx%d", max_x, max_y);
		ret = prop_add(LCD_PROP_NAME, "resolution", buf);
		if (ret) {
			printk("added prop resolution error/n");
			goto failed;
		}
	}
	
failed:
	return ret;
}

static
ssize_t backlight_debug_proc_write(struct file *file, const char *buffer,
				      unsigned long count, void *data) {
	char buf[2];

	if (count != sizeof(buf)) {
		return -EINVAL;
	} else {
		if (copy_from_user(buf, buffer, count))
			return -EINVAL;
		if (buf[count-1] != '\n')
			return -EINVAL;
		zte_debug = buf[0] - '0';
		printk("[%s] zte_debug :%d\n", zte_debug);
	}

	return count;
}

static ssize_t backlght_debug_proc_read(char *page, char **start, off_t off,
		int count, int *eof, void *data)
{
	sprintf(page, "%d\n", zte_debug);
	return strlen(page);
}

void create_backlight_debug_file(void) {
	struct proc_dir_entry *backlight_debug_file = NULL;
	backlight_debug_file =
		create_proc_entry(LCD_BACKLIGHT_DEBUG_FILE, 0660, NULL);

	if (backlight_debug_file) {
		backlight_debug_file->read_proc = backlght_debug_proc_read;
		backlight_debug_file->write_proc = backlight_debug_proc_write;
	}
}

int get_debug_flag(void) {
	return zte_debug;
}
