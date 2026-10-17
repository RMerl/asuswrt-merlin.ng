/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  Menu-driven UEFI Variable maintenance
 *
 *  Copyright (c) 2022 Masahisa Kojima, Linaro Limited
 */

#ifndef _EFI_CONFIG_H
#define _EFI_CONFIG_H

#include <efi_loader.h>
#include <menu.h>

#define EFICONFIG_ENTRY_NUM_MAX (INT_MAX - 1)
#define EFICONFIG_VOLUME_PATH_MAX 512
#define EFICONFIG_FILE_PATH_MAX 512
#define EFICONFIG_FILE_PATH_BUF_SIZE (EFICONFIG_FILE_PATH_MAX * sizeof(u16))

extern const char *eficonfig_menu_desc;
typedef efi_status_t (*eficonfig_entry_func)(void *data);

/**
 * struct eficonfig_entry - menu entry structure
 *
 * @num:	menu entry index
 * @title:	title of entry
 * @key:	unique key
 * @efi_menu:	pointer to the menu structure
 * @func:	callback function to be called when this entry is selected
 * @data:	data to be passed to the callback function, caller must free() this pointer
 * @list:	list structure
 */
struct eficonfig_entry {
	u32 num;
	char *title;
	char key[3];
	struct efimenu *efi_menu;
	eficonfig_entry_func func;
	void *data;
	struct list_head list;
};

/**
 * struct efimenu - efi menu structure
 *
 * @delay:		delay for autoboot
 * @active:		active menu entry index
 * @count:		total count of menu entry
 * @menu_header:	menu header string
 * @menu_desc:		menu description string
 * @list:		menu entry list structure
 * @start:		top menu index to draw
 * @end:		bottom menu index to draw
 */
struct efimenu {
	int delay;
	int active;
	int count;
	char *menu_header;
	const char *menu_desc;
	struct list_head list;
	int start;
	int end;
};

/**
 * struct eficonfig_item - structure to construct eficonfig_entry
 *
 * @title:	title of entry
 * @func:	callback function to be called when this entry is selected
 * @data:	data to be passed to the callback function
 */
struct eficonfig_item {
	char *title;
	eficonfig_entry_func func;
	void *data;
};

/**
 * struct eficonfig_select_file_info - structure to be used for file selection
 *
 * @current_volume:	pointer to the efi_simple_file_system_protocol
 * @dp_volume:		pointer to device path of the selected device
 * @current_path:	pointer to the selected file path string
 * @filepath_list:	list_head structure for file path list
 * @file_selectred:	flag indicates file selecting status
 */
struct eficonfig_select_file_info {
	struct efi_simple_file_system_protocol *current_volume;
	struct efi_device_path *dp_volume;
	u16 *current_path;
	struct list_head filepath_list;
	bool file_selected;
};

void eficonfig_print_msg(char *msg);
void eficonfig_print_entry(void *data);
void eficonfig_display_statusline(struct menu *m);
char *eficonfig_choice_entry(void *data);
void eficonfig_destroy(struct efimenu *efi_menu);
efi_status_t eficonfig_process_quit(void *data);
efi_status_t eficonfig_process_common(struct efimenu *efi_menu,
				      char *menu_header, const char *menu_desc,
				      void (*display_statusline)(struct menu *),
				      void (*item_data_print)(void *),
				      char *(*item_choice)(void *));
efi_status_t eficonfig_process_select_file(void *data);
efi_status_t eficonfig_append_menu_entry(struct efimenu *efi_menu,
					 char *title, eficonfig_entry_func func,
					 void *data);
efi_status_t eficonfig_append_quit_entry(struct efimenu *efi_menu);
struct efi_device_path *eficonfig_create_device_path(struct efi_device_path *dp_volume,
						     u16 *current_path);
void *eficonfig_create_fixed_menu(const struct eficonfig_item *items, int count);
#ifdef CONFIG_EFI_SECURE_BOOT
efi_status_t eficonfig_process_secure_boot_config(void *data);
#endif

#endif
