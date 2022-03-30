#ifndef __SAMSUNG_MISC_COMMON_H__
#define __SAMSUNG_MISC_COMMON_H__

#ifdef CONFIG_REVISION_TAG
u32 get_board_rev(void);
#endif

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
void set_board_info(void);
#endif

#ifdef CONFIG_LCD_MENU
enum {
	BOOT_MODE_INFO,
	BOOT_MODE_THOR,
	BOOT_MODE_UMS,
	BOOT_MODE_DFU,
	BOOT_MODE_GPT,
	BOOT_MODE_ENV,
	BOOT_MODE_EXIT,
};

void keys_init(void);
void check_boot_mode(void);
#endif /* CONFIG_LCD_MENU */

#ifdef CONFIG_CMD_BMP
void draw_logo(void);
#endif

#ifdef CONFIG_SET_DFU_ALT_INFO
char *get_dfu_alt_system(char *interface, char *devstr);
char *get_dfu_alt_boot(char *interface, char *devstr);
#endif
void set_board_type(void);
void set_board_revision(void);
const char *get_board_type(void);

#endif /* __SAMSUNG_MISC_COMMON_H__ */
