#ifndef _ZTE_PANEL_COMMON_H_
#define _ZTE_PANEL_COMMON_H_

int add_panel_config_prop(char *panel_name,
	char *ic_name, int max_x, int max_y);

void create_backlight_debug_file(void);

int get_debug_flag(void);
#endif
