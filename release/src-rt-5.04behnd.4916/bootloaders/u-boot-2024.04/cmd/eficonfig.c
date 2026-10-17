// SPDX-License-Identifier: GPL-2.0+
/*
 *  Menu-driven UEFI Variable maintenance
 *
 *  Copyright (c) 2022 Masahisa Kojima, Linaro Limited
 */

#include <ansi.h>
#include <cli.h>
#include <common.h>
#include <charset.h>
#include <efi_loader.h>
#include <efi_load_initrd.h>
#include <efi_config.h>
#include <efi_variable.h>
#include <log.h>
#include <malloc.h>
#include <menu.h>
#include <sort.h>
#include <watchdog.h>
#include <asm/unaligned.h>
#include <linux/delay.h>

static struct efi_simple_text_input_protocol *cin;
const char *eficonfig_menu_desc =
	"  Press UP/DOWN to move, ENTER to select, ESC to quit";

static const char *eficonfig_change_boot_order_desc =
	"  Press UP/DOWN to move, +/- to change orde\n"
	"  Press SPACE to activate or deactivate the entry\n"
	"  CTRL+S to save, ESC to quit";

static struct efi_simple_text_output_protocol *cout;
static int avail_row;

#define EFICONFIG_DESCRIPTION_MAX 32
#define EFICONFIG_OPTIONAL_DATA_MAX 64
#define EFICONFIG_MENU_HEADER_ROW_NUM 3
#define EFICONFIG_MENU_DESC_ROW_NUM 5

/**
 * struct eficonfig_filepath_info - structure to be used to store file path
 *
 * @name:	file or directory name
 * @list:	list structure
 */
struct eficonfig_filepath_info {
	char *name;
	struct list_head list;
};

/**
 * struct eficonfig_boot_option - structure to be used for updating UEFI boot option
 *
 * @file_info:		user selected file info
 * @initrd_info:	user selected initrd file info
 * @boot_index:		index of the boot option
 * @description:	pointer to the description string
 * @optional_data:	pointer to the optional_data
 * @edit_completed:	flag indicates edit complete
 */
struct eficonfig_boot_option {
	struct eficonfig_select_file_info file_info;
	struct eficonfig_select_file_info initrd_info;
	unsigned int boot_index;
	u16 *description;
	u16 *optional_data;
	bool edit_completed;
};

/**
 * struct eficonfig_volume_entry_data - structure to be used to store volume info
 *
 * @file_info:	pointer to file info structure
 * @v:		pointer to the protocol interface
 * @dp:		pointer to the device path
 */
struct eficonfig_volume_entry_data {
	struct eficonfig_select_file_info *file_info;
	struct efi_simple_file_system_protocol *v;
	struct efi_device_path *dp;
};

/**
 * struct eficonfig_file_entry_data - structure to be used to store file info
 *
 * @file_info:		pointer to file info structure
 * @is_directory:	flag to identify the directory or file
 * @file_name:		name of directory or file
 */
struct eficonfig_file_entry_data {
	struct eficonfig_select_file_info *file_info;
	bool is_directory;
	char *file_name;
};

/**
 * struct eficonfig_boot_selection_data - structure to be used to select the boot option entry
 *
 * @boot_index:	index of the boot option
 * @selected:		pointer to store the selected index in the BootOrder variable
 */
struct eficonfig_boot_selection_data {
	u16 boot_index;
	int *selected;
};

/**
 * struct eficonfig_boot_order_data - structure to be used to update BootOrder variable
 *
 * @boot_index:		boot option index
 * @active:		flag to include the boot option into BootOrder variable
 */
struct eficonfig_boot_order_data {
	u32 boot_index;
	bool active;
};

/**
 * struct eficonfig_save_boot_order_data - structure to be used to change boot order
 *
 * @efi_menu:		pointer to efimenu structure
 * @selected:		flag to indicate user selects "Save" entry
 */
struct eficonfig_save_boot_order_data {
	struct efimenu *efi_menu;
	bool selected;
};

/**
 * struct eficonfig_menu_adjust - update start and end entry index
 *
 * @efi_menu:	pointer to efimenu structure
 * @add:	flag to add or substract the index
 */
static void eficonfig_menu_adjust(struct efimenu *efi_menu, bool add)
{
	if (add)
		++efi_menu->active;
	else
		--efi_menu->active;

	if (add && efi_menu->end < efi_menu->active) {
		efi_menu->start++;
		efi_menu->end++;
	} else if (!add && efi_menu->start > efi_menu->active) {
		efi_menu->start--;
		efi_menu->end--;
	}
}
#define eficonfig_menu_up(_a) eficonfig_menu_adjust(_a, false)
#define eficonfig_menu_down(_a) eficonfig_menu_adjust(_a, true)

/**
 * eficonfig_print_msg() - print message
 *
 * display the message to the user, user proceeds the screen
 * with any key press.
 *
 * @items:		pointer to the structure of each menu entry
 * @count:		the number of menu entry
 * @menu_header:	pointer to the menu header string
 * Return:	status code
 */
void eficonfig_print_msg(char *msg)
{
	/* Flush input */
	while (tstc())
		getchar();

	printf(ANSI_CURSOR_HIDE
	       ANSI_CLEAR_CONSOLE
	       ANSI_CURSOR_POSITION
	       "%s\n\n  Press any key to continue", 3, 4, msg);

	getchar();
}

/**
 * eficonfig_print_entry() - print each menu entry
 *
 * @data:	pointer to the data associated with each menu entry
 */
void eficonfig_print_entry(void *data)
{
	struct eficonfig_entry *entry = data;
	bool reverse = (entry->efi_menu->active == entry->num);

	if (entry->efi_menu->start > entry->num || entry->efi_menu->end < entry->num)
		return;

	printf(ANSI_CURSOR_POSITION, (entry->num - entry->efi_menu->start) +
	       EFICONFIG_MENU_HEADER_ROW_NUM + 1, 7);

	if (reverse)
		puts(ANSI_COLOR_REVERSE);

	printf(ANSI_CLEAR_LINE "%s", entry->title);

	if (reverse)
		puts(ANSI_COLOR_RESET);
}

/**
 * eficonfig_display_statusline() - print status line
 *
 * @m:	pointer to the menu structure
 */
void eficonfig_display_statusline(struct menu *m)
{
	struct eficonfig_entry *entry;

	if (menu_default_choice(m, (void *)&entry) < 0)
		return;

	printf(ANSI_CURSOR_POSITION
	      "\n%s\n"
	       ANSI_CURSOR_POSITION ANSI_CLEAR_LINE ANSI_CURSOR_POSITION
	       "%s"
	       ANSI_CLEAR_LINE_TO_END,
	       1, 1, entry->efi_menu->menu_header, avail_row + 4, 1,
	       avail_row + 5, 1, entry->efi_menu->menu_desc);
}

/**
 * eficonfig_choice_entry() - user key input handler
 *
 * @data:	pointer to the efimenu structure
 * Return:	key string to identify the selected entry
 */
char *eficonfig_choice_entry(void *data)
{
	struct cli_ch_state s_cch, *cch = &s_cch;
	struct list_head *pos, *n;
	struct eficonfig_entry *entry;
	enum bootmenu_key key = BKEY_NONE;
	struct efimenu *efi_menu = data;

	cli_ch_init(cch);

	while (1) {
		key = bootmenu_loop((struct bootmenu_data *)efi_menu, cch);

		switch (key) {
		case BKEY_UP:
			if (efi_menu->active > 0)
				eficonfig_menu_up(efi_menu);

			/* no menu key selected, regenerate menu */
			return NULL;
		case BKEY_DOWN:
			if (efi_menu->active < efi_menu->count - 1)
				eficonfig_menu_down(efi_menu);

			/* no menu key selected, regenerate menu */
			return NULL;
		case BKEY_SELECT:
			list_for_each_safe(pos, n, &efi_menu->list) {
				entry = list_entry(pos, struct eficonfig_entry, list);
				if (entry->num == efi_menu->active)
					return entry->key;
			}
			break;
		case BKEY_QUIT:
			/* Quit by choosing the last entry */
			entry = list_last_entry(&efi_menu->list, struct eficonfig_entry, list);
			return entry->key;
		default:
			/* Pressed key is not valid, no need to regenerate the menu */
			break;
		}
	}
}

/**
 * eficonfig_destroy() - destroy efimenu
 *
 * @efi_menu:	pointer to the efimenu structure
 */
void eficonfig_destroy(struct efimenu *efi_menu)
{
	struct list_head *pos, *n;
	struct eficonfig_entry *entry;

	if (!efi_menu)
		return;

	list_for_each_safe(pos, n, &efi_menu->list) {
		entry = list_entry(pos, struct eficonfig_entry, list);
		free(entry->title);
		list_del(&entry->list);
		free(entry);
	}
	free(efi_menu->menu_header);
	free(efi_menu);
}

/**
 * eficonfig_process_quit() - callback function for "Quit" entry
 *
 * @data:	pointer to the data
 * Return:	status code
 */
efi_status_t eficonfig_process_quit(void *data)
{
	return EFI_ABORTED;
}

/**
 * eficonfig_append_menu_entry() - append menu item
 *
 * @efi_menu:	pointer to the efimenu structure
 * @title:	pointer to the entry title
 * @func:	callback of each entry
 * @data:	pointer to the data to be passed to each entry callback
 * Return:	status code
 */
efi_status_t eficonfig_append_menu_entry(struct efimenu *efi_menu,
					 char *title, eficonfig_entry_func func,
					 void *data)
{
	struct eficonfig_entry *entry;

	if (efi_menu->count >= EFICONFIG_ENTRY_NUM_MAX)
		return EFI_OUT_OF_RESOURCES;

	entry = calloc(1, sizeof(struct eficonfig_entry));
	if (!entry)
		return EFI_OUT_OF_RESOURCES;

	entry->title = title;
	sprintf(entry->key, "%d", efi_menu->count);
	entry->efi_menu = efi_menu;
	entry->func = func;
	entry->data = data;
	entry->num = efi_menu->count++;
	list_add_tail(&entry->list, &efi_menu->list);

	return EFI_SUCCESS;
}

/**
 * eficonfig_append_quit_entry() - append quit entry
 *
 * @efi_menu:	pointer to the efimenu structure
 * Return:	status code
 */
efi_status_t eficonfig_append_quit_entry(struct efimenu *efi_menu)
{
	char *title;
	efi_status_t ret;

	title = strdup("Quit");
	if (!title)
		return EFI_OUT_OF_RESOURCES;

	ret = eficonfig_append_menu_entry(efi_menu, title, eficonfig_process_quit, NULL);
	if (ret != EFI_SUCCESS)
		free(title);

	return ret;
}

/**
 * eficonfig_create_fixed_menu() - create fixed entry menu structure
 *
 * @items:	pointer to the menu entry item
 * @count:	the number of menu entry
 * Return:	pointer to the efimenu structure
 */
void *eficonfig_create_fixed_menu(const struct eficonfig_item *items, int count)
{
	u32 i;
	char *title;
	efi_status_t ret;
	struct efimenu *efi_menu;
	const struct eficonfig_item *iter = items;

	efi_menu = calloc(1, sizeof(struct efimenu));
	if (!efi_menu)
		return NULL;

	INIT_LIST_HEAD(&efi_menu->list);
	for (i = 0; i < count; i++, iter++) {
		title = strdup(iter->title);
		if (!title)
			goto out;

		ret = eficonfig_append_menu_entry(efi_menu, title, iter->func, iter->data);
		if (ret != EFI_SUCCESS) {
			free(title);
			goto out;
		}
	}

	return efi_menu;
out:
	eficonfig_destroy(efi_menu);

	return NULL;
}

/**
 * eficonfig_process_common() - main handler for UEFI menu
 *
 * Construct the structures required to show the menu, then handle
 * the user input interacting with u-boot menu functions.
 *
 * @efi_menu:		pointer to the efimenu structure
 * @menu_header:	pointer to the menu header string
 * @menu_desc:		pointer to the menu description
 * @display_statusline:	function pointer to draw statusline
 * @item_data_print:	function pointer to draw the menu item
 * @item_choice:	function pointer to handle the key press
 * Return:		status code
 */
efi_status_t eficonfig_process_common(struct efimenu *efi_menu,
				      char *menu_header, const char *menu_desc,
				      void (*display_statusline)(struct menu *),
				      void (*item_data_print)(void *),
				      char *(*item_choice)(void *))
{
	struct menu *menu;
	void *choice = NULL;
	struct list_head *pos, *n;
	struct eficonfig_entry *entry;
	efi_status_t ret = EFI_SUCCESS;

	if (efi_menu->count > EFICONFIG_ENTRY_NUM_MAX)
		return EFI_OUT_OF_RESOURCES;

	efi_menu->delay = -1;
	efi_menu->active = 0;
	efi_menu->start = 0;
	efi_menu->end = avail_row - 1;

	if (menu_header) {
		efi_menu->menu_header = strdup(menu_header);
		if (!efi_menu->menu_header)
			return EFI_OUT_OF_RESOURCES;
	}
	if (menu_desc)
		efi_menu->menu_desc = menu_desc;

	menu = menu_create(NULL, 0, 1, display_statusline, item_data_print,
			   item_choice, efi_menu);
	if (!menu)
		return EFI_INVALID_PARAMETER;

	list_for_each_safe(pos, n, &efi_menu->list) {
		entry = list_entry(pos, struct eficonfig_entry, list);
		if (!menu_item_add(menu, entry->key, entry)) {
			ret = EFI_INVALID_PARAMETER;
			goto out;
		}
	}

	entry = list_first_entry_or_null(&efi_menu->list, struct eficonfig_entry, list);
	if (entry)
		menu_default_set(menu, entry->key);

	printf(ANSI_CURSOR_HIDE
	       ANSI_CLEAR_CONSOLE
	       ANSI_CURSOR_POSITION, 1, 1);

	if (menu_get_choice(menu, &choice)) {
		entry = choice;
		if (entry->func)
			ret = entry->func(entry->data);
	}
out:
	menu_destroy(menu);

	printf(ANSI_CLEAR_CONSOLE
	       ANSI_CURSOR_POSITION
	       ANSI_CURSOR_SHOW, 1, 1);

	return ret;
}

/**
 * eficonfig_volume_selected() - handler of volume selection
 *
 * @data:	pointer to the data of selected entry
 * Return:	status code
 */
static efi_status_t eficonfig_volume_selected(void *data)
{
	struct eficonfig_volume_entry_data *info = data;

	if (info) {
		info->file_info->current_volume = info->v;
		info->file_info->dp_volume = info->dp;
	}

	return EFI_SUCCESS;
}

/**
 * eficonfig_create_device_path() - create device path
 *
 * @dp_volume:	pointer to the volume
 * @current_path: pointer to the file path u16 string
 * Return:
 * device path or NULL. Caller must free the returned value
 */
struct efi_device_path *eficonfig_create_device_path(struct efi_device_path *dp_volume,
						     u16 *current_path)
{
	char *p;
	void *buf;
	efi_uintn_t fp_size;
	struct efi_device_path *dp;
	struct efi_device_path_file_path *fp;

	fp_size = sizeof(struct efi_device_path) + u16_strsize(current_path);
	buf = calloc(1, fp_size + sizeof(END));
	if (!buf)
		return NULL;

	fp = buf;
	fp->dp.type = DEVICE_PATH_TYPE_MEDIA_DEVICE,
	fp->dp.sub_type = DEVICE_PATH_SUB_TYPE_FILE_PATH,
	fp->dp.length = (u16)fp_size;
	u16_strcpy(fp->str, current_path);

	p = buf;
	p += fp_size;
	*((struct efi_device_path *)p) = END;

	dp = efi_dp_shorten(dp_volume);
	if (!dp)
		dp = dp_volume;
	dp = efi_dp_concat(dp, &fp->dp, false);
	free(buf);

	return dp;
}

/**
 * eficonfig_file_selected() - handler of file selection
 *
 * @data:	pointer to the data of selected entry
 * Return:	status code
 */
static efi_status_t eficonfig_file_selected(void *data)
{
	u16 *tmp;
	struct eficonfig_file_entry_data *info = data;

	if (!info)
		return EFI_INVALID_PARAMETER;

	if (!strcmp(info->file_name, "..\\")) {
		struct eficonfig_filepath_info *iter;
		struct list_head *pos, *n;
		int is_last;
		char *filepath;
		tmp = info->file_info->current_path;

		memset(info->file_info->current_path, 0, EFICONFIG_FILE_PATH_BUF_SIZE);
		filepath = calloc(1, EFICONFIG_FILE_PATH_MAX);
		if (!filepath)
			return EFI_OUT_OF_RESOURCES;

		list_for_each_safe(pos, n, &info->file_info->filepath_list) {
			iter = list_entry(pos, struct eficonfig_filepath_info, list);

			is_last = list_is_last(&iter->list, &info->file_info->filepath_list);
			if (is_last) {
				list_del(&iter->list);
				free(iter->name);
				free(iter);
				break;
			}
			strlcat(filepath, iter->name, EFICONFIG_FILE_PATH_MAX);
		}
		utf8_utf16_strcpy(&tmp, filepath);
	} else {
		size_t new_len;
		struct eficonfig_filepath_info *filepath_info;

		new_len = u16_strlen(info->file_info->current_path) +
				     strlen(info->file_name);
		if (new_len >= EFICONFIG_FILE_PATH_MAX) {
			eficonfig_print_msg("File path is too long!");
			return EFI_INVALID_PARAMETER;
		}
		tmp = &info->file_info->current_path[u16_strlen(info->file_info->current_path)];
		utf8_utf16_strcpy(&tmp, info->file_name);

		filepath_info = calloc(1, sizeof(struct eficonfig_filepath_info));
		if (!filepath_info)
			return EFI_OUT_OF_RESOURCES;

		filepath_info->name = strdup(info->file_name);
		if (!filepath_info->name) {
			free(filepath_info);
			return EFI_OUT_OF_RESOURCES;
		}
		list_add_tail(&filepath_info->list, &info->file_info->filepath_list);

		if (!info->is_directory)
			info->file_info->file_selected = true;
	}

	return EFI_SUCCESS;
}

/**
 * eficonfig_select_volume() - construct the volume selection menu
 *
 * @file_info:	pointer to the file selection structure
 * Return:	status code
 */
static efi_status_t eficonfig_select_volume(struct eficonfig_select_file_info *file_info)
{
	u32 i;
	efi_status_t ret;
	efi_uintn_t count;
	struct efimenu *efi_menu;
	struct list_head *pos, *n;
	struct efi_handler *handler;
	struct eficonfig_entry *entry;
	struct efi_device_path *device_path;
	efi_handle_t *volume_handles = NULL;
	struct efi_simple_file_system_protocol *v;

	ret = efi_locate_handle_buffer_int(BY_PROTOCOL, &efi_simple_file_system_protocol_guid,
					   NULL, &count, (efi_handle_t **)&volume_handles);
	if (ret != EFI_SUCCESS) {
		eficonfig_print_msg("No block device found!");
		return ret;
	}

	efi_menu = calloc(1, sizeof(struct efimenu));
	if (!efi_menu)
		return EFI_OUT_OF_RESOURCES;

	INIT_LIST_HEAD(&efi_menu->list);
	for (i = 0; i < count; i++) {
		char *devname;
		struct efi_block_io *block_io;
		struct eficonfig_volume_entry_data *info;

		if (efi_menu->count >= EFICONFIG_ENTRY_NUM_MAX - 1)
			break;

		ret = efi_search_protocol(volume_handles[i],
					  &efi_simple_file_system_protocol_guid, &handler);
		if (ret != EFI_SUCCESS)
			continue;
		ret = efi_protocol_open(handler, (void **)&v, efi_root, NULL,
					EFI_OPEN_PROTOCOL_GET_PROTOCOL);
		if (ret != EFI_SUCCESS)
			continue;

		ret = efi_search_protocol(volume_handles[i], &efi_guid_device_path, &handler);
		if (ret != EFI_SUCCESS)
			continue;
		ret = efi_protocol_open(handler, (void **)&device_path,
					efi_root, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
		if (ret != EFI_SUCCESS)
			continue;

		ret = efi_search_protocol(volume_handles[i], &efi_block_io_guid, &handler);
		if (ret != EFI_SUCCESS)
			continue;
		ret = efi_protocol_open(handler, (void **)&block_io,
					efi_root, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
		if (ret != EFI_SUCCESS)
			continue;

		info = calloc(1, sizeof(struct eficonfig_volume_entry_data));
		if (!info) {
			ret = EFI_OUT_OF_RESOURCES;
			goto out;
		}

		devname = calloc(1, BOOTMENU_DEVICE_NAME_MAX);
		if (!devname) {
			free(info);
			ret = EFI_OUT_OF_RESOURCES;
			goto out;
		}
		ret = efi_disk_get_device_name(volume_handles[i], devname,
					       BOOTMENU_DEVICE_NAME_MAX);
		if (ret != EFI_SUCCESS) {
			free(info);
			goto out;
		}

		info->v = v;
		info->dp = device_path;
		info->file_info = file_info;
		ret = eficonfig_append_menu_entry(efi_menu, devname, eficonfig_volume_selected,
						  info);
		if (ret != EFI_SUCCESS) {
			free(info);
			goto out;
		}
	}

	ret = eficonfig_append_quit_entry(efi_menu);
	if (ret != EFI_SUCCESS)
		goto out;

	ret = eficonfig_process_common(efi_menu, "  ** Select Volume **",
				       eficonfig_menu_desc,
				       eficonfig_display_statusline,
				       eficonfig_print_entry,
				       eficonfig_choice_entry);

out:
	efi_free_pool(volume_handles);
	list_for_each_safe(pos, n, &efi_menu->list) {
		entry = list_entry(pos, struct eficonfig_entry, list);
		free(entry->data);
	}
	eficonfig_destroy(efi_menu);

	return ret;
}

/**
 * sort_file() - sort the file name in ascii order
 *
 * @data1:	pointer to the file entry data
 * @data2:	pointer to the file entry data
 * Return:	-1 if the data1 file name is less than data2 file name,
 *		0 if both file name match,
 *		1 if the data1 file name is greater thant data2 file name.
 */
static int sort_file(const void *arg1, const void *arg2)
{
	const struct eficonfig_file_entry_data *data1, *data2;

	data1 = *((const struct eficonfig_file_entry_data **)arg1);
	data2 = *((const struct eficonfig_file_entry_data **)arg2);

	return strcasecmp(data1->file_name, data2->file_name);
}

/**
 * eficonfig_create_file_entry() - construct the file menu entry
 *
 * @efi_menu:	pointer to the efimenu structure
 * @count:	number of the directory and file
 * @tmp_infos:	pointer to the entry data array
 * @f:		pointer to the file handle
 * @buf:	pointer to the buffer to store the directory information
 * @file_info:	pointer to the file selection structure
 * Return:	status code
 */
static efi_status_t
eficonfig_create_file_entry(struct efimenu *efi_menu, u32 count,
			    struct eficonfig_file_entry_data **tmp_infos,
			    struct efi_file_handle *f, struct efi_file_info *buf,
			    struct eficonfig_select_file_info *file_info)
{
	char *name, *p;
	efi_uintn_t len;
	efi_status_t ret;
	u32 i, entry_num = 0;
	struct eficonfig_file_entry_data *info;

	EFI_CALL(f->setpos(f, 0));
	/* Read directory and construct menu structure */
	for (i = 0; i < count; i++) {
		if (entry_num >= EFICONFIG_ENTRY_NUM_MAX - 1)
			break;

		len = sizeof(struct efi_file_info) + EFICONFIG_FILE_PATH_BUF_SIZE;
		ret = EFI_CALL(f->read(f, &len, buf));
		if (ret != EFI_SUCCESS || len == 0)
			break;

		info = calloc(1, sizeof(struct eficonfig_file_entry_data));
		if (!info) {
			ret = EFI_OUT_OF_RESOURCES;
			goto out;
		}

		/* append '\\' at the end of directory name */
		name = calloc(1, utf16_utf8_strlen(buf->file_name) + 2);
		if (!name) {
			ret = EFI_OUT_OF_RESOURCES;
			free(info);
			goto out;
		}
		p = name;
		utf16_utf8_strcpy(&p, buf->file_name);
		if (buf->attribute & EFI_FILE_DIRECTORY) {
			/* filter out u'.' */
			if (!u16_strcmp(buf->file_name, u".")) {
				free(info);
				free(name);
				continue;
			}
			name[u16_strlen(buf->file_name)] = '\\';
			info->is_directory = true;
		}

		info->file_name = name;
		info->file_info = file_info;
		tmp_infos[entry_num++] = info;
	}

	qsort(tmp_infos, entry_num, sizeof(*tmp_infos),
	      (int (*)(const void *, const void *))sort_file);

	for (i = 0; i < entry_num; i++) {
		ret = eficonfig_append_menu_entry(efi_menu, tmp_infos[i]->file_name,
						  eficonfig_file_selected, tmp_infos[i]);
		if (ret != EFI_SUCCESS)
			goto out;
	}

out:
	return ret;
}

/**
 * eficonfig_show_file_selection() - construct the file selection menu
 *
 * @file_info:	pointer to the file selection structure
 * @root:	pointer to the file handle
 * Return:	status code
 */
static efi_status_t eficonfig_show_file_selection(struct eficonfig_select_file_info *file_info,
						  struct efi_file_handle *root)
{
	u32 count = 0, i;
	efi_uintn_t len;
	efi_status_t ret;
	struct efimenu *efi_menu;
	struct efi_file_handle *f;
	struct efi_file_info *buf;
	struct eficonfig_file_entry_data **tmp_infos;

	buf = calloc(1, sizeof(struct efi_file_info) + EFICONFIG_FILE_PATH_BUF_SIZE);
	if (!buf)
		return EFI_OUT_OF_RESOURCES;

	while (!file_info->file_selected) {
		efi_menu = calloc(1, sizeof(struct efimenu));
		if (!efi_menu) {
			ret = EFI_OUT_OF_RESOURCES;
			goto out;
		}
		INIT_LIST_HEAD(&efi_menu->list);

		ret = EFI_CALL(root->open(root, &f, file_info->current_path,
					  EFI_FILE_MODE_READ, 0));
		if (ret != EFI_SUCCESS) {
			eficonfig_print_msg("Reading volume failed!");
			free(efi_menu);
			ret = EFI_ABORTED;
			goto out;
		}

		/* Count the number of directory entries */
		for (;;) {
			len = sizeof(struct efi_file_info) + EFICONFIG_FILE_PATH_BUF_SIZE;
			ret = EFI_CALL(f->read(f, &len, buf));
			if (ret != EFI_SUCCESS || len == 0)
				break;

			count++;
		}

		/* allocate array to sort the entry */
		tmp_infos = calloc(count, sizeof(*tmp_infos));
		if (!tmp_infos) {
			ret = EFI_OUT_OF_RESOURCES;
			goto err;
		}

		ret = eficonfig_create_file_entry(efi_menu, count, tmp_infos,
						  f, buf, file_info);
		if (ret != EFI_SUCCESS)
			goto err;

		ret = eficonfig_append_quit_entry(efi_menu);
		if (ret != EFI_SUCCESS)
			goto err;

		ret = eficonfig_process_common(efi_menu, "  ** Select File **",
					       eficonfig_menu_desc,
					       eficonfig_display_statusline,
					       eficonfig_print_entry,
					       eficonfig_choice_entry);
err:
		EFI_CALL(f->close(f));
		eficonfig_destroy(efi_menu);

		if (tmp_infos) {
			for (i = 0; i < count; i++)
				free(tmp_infos[i]);
		}

		free(tmp_infos);

		if (ret != EFI_SUCCESS)
			break;
	}

out:
	free(buf);

	return ret;
}

/**
 * handle_user_input() - handle user input
 *
 * @buf:	pointer to the buffer
 * @buf_size:	size of the buffer
 * @cursor_col:	cursor column for user input
 * @msg:	pointer to the string to display
 * Return:	status code
 */
static efi_status_t handle_user_input(u16 *buf, int buf_size,
				      int cursor_col, char *msg)
{
	u16 *tmp;
	efi_status_t ret;

	printf(ANSI_CLEAR_CONSOLE
	       ANSI_CURSOR_POSITION
	       "%s"
	       ANSI_CURSOR_POSITION
	       "  Press ENTER to complete, ESC to quit",
	       0, 1, msg, 8, 1);

	/* tmp is used to accept user cancel */
	tmp = calloc(1, buf_size * sizeof(u16));
	if (!tmp)
		return EFI_OUT_OF_RESOURCES;

	ret = efi_console_get_u16_string(cin, tmp, buf_size, NULL, 4, cursor_col);
	if (ret == EFI_SUCCESS)
		u16_strcpy(buf, tmp);

	free(tmp);

	/* to stay the parent menu */
	ret = (ret == EFI_ABORTED) ? EFI_NOT_READY : ret;

	return ret;
}

/**
 * eficonfig_boot_add_enter_description() - handle user input for description
 *
 * @data:	pointer to the internal boot option structure
 * Return:	status code
 */
static efi_status_t eficonfig_boot_add_enter_description(void *data)
{
	struct eficonfig_boot_option *bo = data;

	return handle_user_input(bo->description, EFICONFIG_DESCRIPTION_MAX, 22,
				 "\n  ** Edit Description **\n"
				 "\n"
				 "  enter description: ");
}

/**
 * eficonfig_boot_add_optional_data() - handle user input for optional data
 *
 * @data:	pointer to the internal boot option structure
 * Return:	status code
 */
static efi_status_t eficonfig_boot_add_optional_data(void *data)
{
	struct eficonfig_boot_option *bo = data;

	return handle_user_input(bo->optional_data, EFICONFIG_OPTIONAL_DATA_MAX, 24,
				 "\n  ** Edit Optional Data **\n"
				 "\n"
				 "  enter optional data:");
}

/**
 * eficonfig_boot_edit_save() - handler to save the boot option
 *
 * @data:	pointer to the internal boot option structure
 * Return:	status code
 */
static efi_status_t eficonfig_boot_edit_save(void *data)
{
	struct eficonfig_boot_option *bo = data;

	if (u16_strlen(bo->description) == 0) {
		eficonfig_print_msg("Boot Description is empty!");
		bo->edit_completed = false;
		return EFI_NOT_READY;
	}
	if (u16_strlen(bo->file_info.current_path) == 0) {
		eficonfig_print_msg("File is not selected!");
		bo->edit_completed = false;
		return EFI_NOT_READY;
	}

	bo->edit_completed = true;

	return EFI_SUCCESS;
}

/**
 * eficonfig_process_clear_file_selection() - callback function for "Clear" entry
 *
 * @data:	pointer to the data
 * Return:	status code
 */
efi_status_t eficonfig_process_clear_file_selection(void *data)
{
	struct eficonfig_select_file_info *file_info = data;

	/* clear the existing file information */
	file_info->current_volume = NULL;
	file_info->current_path[0] = u'\0';
	file_info->dp_volume = NULL;

	return EFI_ABORTED;
}

static struct eficonfig_item select_file_menu_items[] = {
	{"Select File", eficonfig_process_select_file},
	{"Clear", eficonfig_process_clear_file_selection},
	{"Quit", eficonfig_process_quit},
};

/**
 * eficonfig_process_show_file_option() - display select file option
 *
 * @file_info:	pointer to the file information structure
 * Return:	status code
 */
efi_status_t eficonfig_process_show_file_option(void *data)
{
	efi_status_t ret;
	struct efimenu *efi_menu;

	select_file_menu_items[0].data = data;
	select_file_menu_items[1].data = data;
	efi_menu = eficonfig_create_fixed_menu(select_file_menu_items,
					       ARRAY_SIZE(select_file_menu_items));
	if (!efi_menu)
		return EFI_OUT_OF_RESOURCES;

	ret = eficonfig_process_common(efi_menu, "  ** Update File **",
				       eficonfig_menu_desc,
				       eficonfig_display_statusline,
				       eficonfig_print_entry,
				       eficonfig_choice_entry);
	if (ret != EFI_SUCCESS) /* User selects "Clear" or "Quit" */
		ret = EFI_NOT_READY;

	eficonfig_destroy(efi_menu);

	return ret;
}

/**
 * eficonfig_process_select_file() - handle user file selection
 *
 * @data:	pointer to the data
 * Return:	status code
 */
efi_status_t eficonfig_process_select_file(void *data)
{
	size_t len;
	efi_status_t ret;
	struct list_head *pos, *n;
	struct efi_file_handle *root;
	struct eficonfig_filepath_info *item;
	struct eficonfig_select_file_info *tmp = NULL;
	struct eficonfig_select_file_info *file_info = data;

	tmp = calloc(1, sizeof(struct eficonfig_select_file_info));
	if (!tmp)
		return EFI_OUT_OF_RESOURCES;

	tmp->current_path = calloc(1, EFICONFIG_FILE_PATH_BUF_SIZE);
	if (!tmp->current_path) {
		free(tmp);
		return EFI_OUT_OF_RESOURCES;
	}
	INIT_LIST_HEAD(&tmp->filepath_list);

	while (!tmp->file_selected) {
		tmp->current_volume = NULL;
		memset(tmp->current_path, 0, EFICONFIG_FILE_PATH_BUF_SIZE);

		ret = eficonfig_select_volume(tmp);
		if (ret != EFI_SUCCESS)
			goto out;

		if (!tmp->current_volume)
			return EFI_INVALID_PARAMETER;

		ret = EFI_CALL(tmp->current_volume->open_volume(tmp->current_volume, &root));
		if (ret != EFI_SUCCESS)
			goto out;

		ret = eficonfig_show_file_selection(tmp, root);
		if (ret == EFI_ABORTED)
			continue;
		if (ret != EFI_SUCCESS)
			goto out;
	}

out:
	if (ret == EFI_SUCCESS) {
		len = u16_strlen(tmp->current_path);
		len = (len >= EFICONFIG_FILE_PATH_MAX) ? (EFICONFIG_FILE_PATH_MAX - 1) : len;
		memcpy(file_info->current_path, tmp->current_path, len * sizeof(u16));
		file_info->current_path[len] = u'\0';
		file_info->current_volume = tmp->current_volume;
		file_info->dp_volume = tmp->dp_volume;
	}

	list_for_each_safe(pos, n, &tmp->filepath_list) {
		item = list_entry(pos, struct eficonfig_filepath_info, list);
		list_del(&item->list);
		free(item->name);
		free(item);
	}
	free(tmp->current_path);
	free(tmp);

	/* to stay the parent menu */
	ret = (ret == EFI_ABORTED) ? EFI_NOT_READY : ret;

	return ret;
}

/**
 * eficonfig_set_boot_option() - set boot option
 *
 * @varname:		pointer to variable name
 * @dp:			pointer to device path
 * @label:		pointer to label string
 * @optional_data:	pointer to optional data
 * Return:		status code
 */
static efi_status_t eficonfig_set_boot_option(u16 *varname, struct efi_device_path *dp,
					      efi_uintn_t dp_size, u16 *label, char *optional_data)
{
	void *p = NULL;
	efi_status_t ret;
	efi_uintn_t size;
	struct efi_load_option lo;

	lo.file_path = dp;
	lo.file_path_length = dp_size;
	lo.attributes = LOAD_OPTION_ACTIVE;
	lo.optional_data = optional_data;
	lo.label = label;

	size = efi_serialize_load_option(&lo, (u8 **)&p);
	if (!size)
		return EFI_INVALID_PARAMETER;

	ret = efi_set_variable_int(varname, &efi_global_variable_guid,
				   EFI_VARIABLE_NON_VOLATILE |
				   EFI_VARIABLE_BOOTSERVICE_ACCESS |
				   EFI_VARIABLE_RUNTIME_ACCESS,
				   size, p, false);
	free(p);

	return ret;
}

/**
 * create_boot_option_entry() - create boot option entry
 *
 * @efi_menu:	pointer to the efimenu structure
 * @title:	pointer to the entry title
 * @val:	pointer to boot option label
 * @func:	callback of each entry
 * @data:	pointer to the data to be passed to each entry callback
 * Return:	status code
 */
static efi_status_t create_boot_option_entry(struct efimenu *efi_menu, char *title, u16 *val,
					     eficonfig_entry_func func, void *data)
{
	u32 len;
	char *p, *buf;

	len = strlen(title) + 1;
	if (val)
		len += utf16_utf8_strlen(val);
	buf = calloc(1, len);
	if (!buf)
		return EFI_OUT_OF_RESOURCES;

	strcpy(buf, title);
	if (val) {
		p = buf + strlen(title);
		utf16_utf8_strcpy(&p, val);
	}

	return eficonfig_append_menu_entry(efi_menu, buf, func, data);
}

/**
 * prepare_file_selection_entry() - prepare file selection entry
 *
 * @efi_menu:	pointer to the efimenu structure
 * @title:	pointer to the title string
 * @file_info:	pointer to the file info
 * Return:	status code
 */
static efi_status_t prepare_file_selection_entry(struct efimenu *efi_menu, char *title,
						 struct eficonfig_select_file_info *file_info)
{
	u32 len;
	efi_status_t ret;
	u16 *file_name = NULL, *p;
	efi_handle_t handle;
	char *devname;

	devname = calloc(1, EFICONFIG_VOLUME_PATH_MAX + 1);
	if (!devname)
		return EFI_OUT_OF_RESOURCES;

	/* get the device name only when the user already selected the file path */
	handle = efi_dp_find_obj(file_info->dp_volume, NULL, NULL);
	if (handle) {
		ret = efi_disk_get_device_name(handle, devname, EFICONFIG_VOLUME_PATH_MAX);
		if (ret != EFI_SUCCESS)
			goto out;
	}

	/*
	 * If the preconfigured volume does not exist in the system, display the text
	 * converted volume device path instead of U-Boot friendly name(e.g. "usb 0:1").
	 */
	if (!handle && file_info->dp_volume) {
		u16 *dp_str;
		char *q = devname;

		dp_str = efi_dp_str(file_info->dp_volume);
		if (dp_str)
			utf16_utf8_strncpy(&q, dp_str, EFICONFIG_VOLUME_PATH_MAX);

		efi_free_pool(dp_str);
	}

	/* append u'/' to devname, it is just for display purpose. */
	if (file_info->current_path[0] != u'\0' && file_info->current_path[0] != u'/')
		strlcat(devname, "/", EFICONFIG_VOLUME_PATH_MAX + 1);

	len = strlen(devname);
	len += utf16_utf8_strlen(file_info->current_path) + 1;
	file_name = calloc(1, len * sizeof(u16));
	if (!file_name) {
		ret = EFI_OUT_OF_RESOURCES;
		goto out;
	}

	p = file_name;
	utf8_utf16_strcpy(&p, devname);
	u16_strlcat(file_name, file_info->current_path, len);
	ret = create_boot_option_entry(efi_menu, title, file_name,
				       eficonfig_process_show_file_option, file_info);
out:
	free(devname);
	free(file_name);

	return ret;
}

/**
 * eficonfig_show_boot_option() - prepare menu entry for editing boot option
 *
 * Construct the structures to create edit boot option menu
 *
 * @bo:		pointer to the boot option
 * @header_str:	pointer to the header string
 * Return:	status code
 */
static efi_status_t eficonfig_show_boot_option(struct eficonfig_boot_option *bo,
					       char *header_str)
{
	efi_status_t ret;
	struct efimenu *efi_menu;

	efi_menu = calloc(1, sizeof(struct efimenu));
	if (!efi_menu)
		return EFI_OUT_OF_RESOURCES;

	INIT_LIST_HEAD(&efi_menu->list);

	ret = create_boot_option_entry(efi_menu, "Description: ", bo->description,
				       eficonfig_boot_add_enter_description, bo);
	if (ret != EFI_SUCCESS)
		goto out;

	ret = prepare_file_selection_entry(efi_menu, "File: ", &bo->file_info);
	if (ret != EFI_SUCCESS)
		goto out;

	ret = prepare_file_selection_entry(efi_menu, "Initrd File: ", &bo->initrd_info);
	if (ret != EFI_SUCCESS)
		goto out;

	ret = create_boot_option_entry(efi_menu, "Optional Data: ", bo->optional_data,
				       eficonfig_boot_add_optional_data, bo);
	if (ret != EFI_SUCCESS)
		goto out;

	ret = create_boot_option_entry(efi_menu, "Save", NULL,
				       eficonfig_boot_edit_save, bo);
	if (ret != EFI_SUCCESS)
		goto out;

	ret = create_boot_option_entry(efi_menu, "Quit", NULL,
				       eficonfig_process_quit, NULL);
	if (ret != EFI_SUCCESS)
		goto out;

	ret = eficonfig_process_common(efi_menu, header_str,
				       eficonfig_menu_desc,
				       eficonfig_display_statusline,
				       eficonfig_print_entry,
				       eficonfig_choice_entry);

out:
	eficonfig_destroy(efi_menu);

	return ret;
}

/**
 * fill_file_info() - fill the file info from efi_device_path structure
 *
 * @dp:		pointer to the device path
 * @file_info:	pointer to the file info structure
 * @device_dp:	pointer to the volume device path
 */
static void fill_file_info(struct efi_device_path *dp,
			   struct eficonfig_select_file_info *file_info,
			   struct efi_device_path *device_dp)
{
	u16 *file_str, *p;
	struct efi_device_path *file_dp = NULL;

	efi_dp_split_file_path(dp, &device_dp, &file_dp);
	file_info->dp_volume = device_dp;

	if (file_dp) {
		file_str = efi_dp_str(file_dp);
		/*
		 * efi_convert_device_path_to_text() automatically adds u'/' at the
		 * beginning of file name, remove u'/' before copying to current_path
		 */
		p = file_str;
		if (p[0] == u'/')
			p++;

		u16_strcpy(file_info->current_path, p);
		efi_free_pool(file_dp);
		efi_free_pool(file_str);
	}
}

/**
 * eficonfig_edit_boot_option() - prepare boot option structure for editing
 *
 * Construct the boot option structure and copy the existing value
 *
 * @varname:		pointer to the UEFI variable name
 * @bo:			pointer to the boot option
 * @load_option:	pointer to the load option
 * @load_option_size:	size of the load option
 * @header_str:		pointer to the header string
 * Return	:	status code
 */
static efi_status_t eficonfig_edit_boot_option(u16 *varname, struct eficonfig_boot_option *bo,
					       void *load_option, efi_uintn_t load_option_size,
					       char *header_str)
{
	size_t len;
	efi_status_t ret;
	char *tmp = NULL, *p;
	struct efi_load_option lo = {0};
	efi_uintn_t final_dp_size;
	struct efi_device_path *dp = NULL;
	efi_uintn_t size = load_option_size;
	struct efi_device_path *final_dp = NULL;
	struct efi_device_path *device_dp = NULL;
	struct efi_device_path *initrd_dp = NULL;
	struct efi_device_path *initrd_device_dp = NULL;

	const struct efi_initrd_dp id_dp = {
		.vendor = {
			{
			DEVICE_PATH_TYPE_MEDIA_DEVICE,
			DEVICE_PATH_SUB_TYPE_VENDOR_PATH,
			sizeof(id_dp.vendor),
			},
			EFI_INITRD_MEDIA_GUID,
		},
		.end = {
			DEVICE_PATH_TYPE_END,
			DEVICE_PATH_SUB_TYPE_END,
			sizeof(id_dp.end),
		}
	};

	bo->file_info.current_path = calloc(1, EFICONFIG_FILE_PATH_BUF_SIZE);
	if (!bo->file_info.current_path) {
		ret =  EFI_OUT_OF_RESOURCES;
		goto out;
	}

	bo->initrd_info.current_path = calloc(1, EFICONFIG_FILE_PATH_BUF_SIZE);
	if (!bo->file_info.current_path) {
		ret =  EFI_OUT_OF_RESOURCES;
		goto out;
	}

	bo->description = calloc(1, EFICONFIG_DESCRIPTION_MAX * sizeof(u16));
	if (!bo->description) {
		ret =  EFI_OUT_OF_RESOURCES;
		goto out;
	}

	bo->optional_data = calloc(1, EFICONFIG_OPTIONAL_DATA_MAX * sizeof(u16));
	if (!bo->optional_data) {
		ret =  EFI_OUT_OF_RESOURCES;
		goto out;
	}

	/* copy the preset value */
	if (load_option) {
		ret = efi_deserialize_load_option(&lo, load_option, &size);
		if (ret != EFI_SUCCESS)
			goto out;

		if (!lo.label) {
			ret = EFI_INVALID_PARAMETER;
			goto out;
		}
		/* truncate the long label string */
		if (u16_strlen(lo.label) >= EFICONFIG_DESCRIPTION_MAX)
			lo.label[EFICONFIG_DESCRIPTION_MAX - 1] = u'\0';

		u16_strcpy(bo->description, lo.label);

		/* EFI image file path is a first instance */
		if (lo.file_path)
			fill_file_info(lo.file_path, &bo->file_info, device_dp);

		/* Initrd file path(optional) is placed at second instance. */
		initrd_dp = efi_dp_from_lo(&lo, &efi_lf2_initrd_guid);
		if (initrd_dp) {
			fill_file_info(initrd_dp, &bo->initrd_info, initrd_device_dp);
			efi_free_pool(initrd_dp);
		}

		if (size > 0)
			memcpy(bo->optional_data, lo.optional_data, size);
	}

	while (1) {
		ret = eficonfig_show_boot_option(bo, header_str);
		if (ret == EFI_SUCCESS && bo->edit_completed)
			break;
		if (ret == EFI_NOT_READY)
			continue;
		if (ret != EFI_SUCCESS)
			goto out;
	}

	if (bo->initrd_info.dp_volume) {
		dp = eficonfig_create_device_path(bo->initrd_info.dp_volume,
						 bo->initrd_info.current_path);
		if (!dp) {
			ret = EFI_OUT_OF_RESOURCES;
			goto out;
		}
		initrd_dp = efi_dp_concat((const struct efi_device_path *)&id_dp,
					  dp, false);
		efi_free_pool(dp);
	}

	dp = eficonfig_create_device_path(bo->file_info.dp_volume, bo->file_info.current_path);
	if (!dp) {
		ret = EFI_OUT_OF_RESOURCES;
		goto out;
	}
	final_dp_size = efi_dp_size(dp) + sizeof(END);
	if (initrd_dp) {
		final_dp = efi_dp_concat(dp, initrd_dp, true);
		final_dp_size += efi_dp_size(initrd_dp) + sizeof(END);
	} else {
		final_dp = efi_dp_dup(dp);
	}
	efi_free_pool(dp);

	if (!final_dp)
		goto out;

	if (utf16_utf8_strlen(bo->optional_data)) {
		len = utf16_utf8_strlen(bo->optional_data) + 1;
		tmp = calloc(1, len);
		if (!tmp)
			goto out;
		p = tmp;
		utf16_utf8_strncpy(&p, bo->optional_data, u16_strlen(bo->optional_data));
	}

	ret = eficonfig_set_boot_option(varname, final_dp, final_dp_size, bo->description, tmp);
out:
	free(tmp);
	free(bo->optional_data);
	free(bo->description);
	free(bo->file_info.current_path);
	free(bo->initrd_info.current_path);
	efi_free_pool(device_dp);
	efi_free_pool(initrd_device_dp);
	efi_free_pool(initrd_dp);
	efi_free_pool(final_dp);

	return ret;
}

/**
 * eficonfig_process_add_boot_option() - handler to add boot option
 *
 * @data:	pointer to the data for each entry
 * Return:	status code
 */
static efi_status_t eficonfig_process_add_boot_option(void *data)
{
	u16 varname[9];
	efi_status_t ret;
	struct eficonfig_boot_option *bo = NULL;

	bo = calloc(1, sizeof(struct eficonfig_boot_option));
	if (!bo)
		return EFI_OUT_OF_RESOURCES;

	ret = efi_bootmgr_get_unused_bootoption(varname, sizeof(varname), &bo->boot_index);
	if (ret != EFI_SUCCESS)
		return ret;

	ret = eficonfig_edit_boot_option(varname, bo, NULL, 0,  "  ** Add Boot Option ** ");
	if (ret != EFI_SUCCESS)
		goto out;

	ret = efi_bootmgr_append_bootorder((u16)bo->boot_index);
	if (ret != EFI_SUCCESS)
		goto out;

out:
	free(bo);

	/* to stay the parent menu */
	ret = (ret == EFI_ABORTED) ? EFI_SUCCESS : ret;

	return ret;
}

/**
 * eficonfig_process_boot_selected() - handler to select boot option entry
 *
 * @data:	pointer to the data for each entry
 * Return:	status code
 */
static efi_status_t eficonfig_process_boot_selected(void *data)
{
	struct eficonfig_boot_selection_data *info = data;

	if (info)
		*info->selected = info->boot_index;

	return EFI_SUCCESS;
}

/**
 * eficonfig_add_boot_selection_entry() - add boot option menu entry
 *
 * @efi_menu:	pointer to store the efimenu structure
 * @boot_index:	boot option index to be added
 * @selected:	pointer to store the selected boot option index
 * Return:	status code
 */
static efi_status_t eficonfig_add_boot_selection_entry(struct efimenu *efi_menu,
						       unsigned int boot_index,
						       unsigned int *selected)
{
	char *buf, *p;
	efi_status_t ret;
	efi_uintn_t size;
	void *load_option;
	struct efi_load_option lo;
	u16 varname[] = u"Boot####";
	struct eficonfig_boot_selection_data *info;

	efi_create_indexed_name(varname, sizeof(varname), "Boot", boot_index);
	load_option = efi_get_var(varname, &efi_global_variable_guid, &size);
	if (!load_option)
		return EFI_SUCCESS;

	ret = efi_deserialize_load_option(&lo, load_option, &size);
	if (ret != EFI_SUCCESS) {
		log_warning("Invalid load option for %ls\n", varname);
		free(load_option);
		return ret;
	}

	if (size >= sizeof(efi_guid_t) &&
	    !guidcmp(lo.optional_data, &efi_guid_bootmenu_auto_generated)) {
		/*
		 * auto generated entry has GUID in optional_data,
		 * skip auto generated entry because it will be generated
		 * again even if it is edited or deleted.
		 */
		free(load_option);
		return EFI_SUCCESS;
	}

	info = calloc(1, sizeof(struct eficonfig_boot_selection_data));
	if (!info) {
		free(load_option);
		return EFI_OUT_OF_RESOURCES;
	}

	buf = calloc(1, utf16_utf8_strlen(lo.label) + 1);
	if (!buf) {
		free(load_option);
		free(info);
		return EFI_OUT_OF_RESOURCES;
	}
	p = buf;
	utf16_utf8_strcpy(&p, lo.label);
	info->boot_index = boot_index;
	info->selected = selected;
	ret = eficonfig_append_menu_entry(efi_menu, buf, eficonfig_process_boot_selected, info);
	if (ret != EFI_SUCCESS) {
		free(load_option);
		free(info);
		return ret;
	}
	free(load_option);

	return EFI_SUCCESS;
}

/**
 * eficonfig_show_boot_selection() - construct boot option menu entry
 *
 * @selected:	pointer to store the selected boot option index
 * Return:	status code
 */
static efi_status_t eficonfig_show_boot_selection(unsigned int *selected)
{
	u32 i;
	u16 *bootorder;
	efi_status_t ret;
	u16 *var_name16 = NULL;
	efi_uintn_t num, size, buf_size;
	struct efimenu *efi_menu;
	struct list_head *pos, *n;
	struct eficonfig_entry *entry;

	efi_menu = calloc(1, sizeof(struct efimenu));
	if (!efi_menu)
		return EFI_OUT_OF_RESOURCES;

	bootorder = efi_get_var(u"BootOrder", &efi_global_variable_guid, &size);

	INIT_LIST_HEAD(&efi_menu->list);
	num = size / sizeof(u16);
	/* list the load option in the order of BootOrder variable */
	for (i = 0; i < num; i++) {
		ret = eficonfig_add_boot_selection_entry(efi_menu, bootorder[i], selected);
		if (ret != EFI_SUCCESS)
			goto out;

		if (efi_menu->count >= EFICONFIG_ENTRY_NUM_MAX - 1)
			break;
	}

	/* list the remaining load option not included in the BootOrder */
	buf_size = 128;
	var_name16 = malloc(buf_size);
	if (!var_name16)
		return EFI_OUT_OF_RESOURCES;

	var_name16[0] = 0;
	for (;;) {
		int index;
		efi_guid_t guid;

		ret = efi_next_variable_name(&buf_size, &var_name16, &guid);
		if (ret == EFI_NOT_FOUND)
			break;
		if (ret != EFI_SUCCESS)
			goto out;

		if (efi_varname_is_load_option(var_name16, &index)) {
			/* If the index is included in the BootOrder, skip it */
			if (efi_search_bootorder(bootorder, num, index, NULL))
				continue;

			ret = eficonfig_add_boot_selection_entry(efi_menu, index, selected);
			if (ret != EFI_SUCCESS)
				goto out;
		}

		if (efi_menu->count >= EFICONFIG_ENTRY_NUM_MAX - 1)
			break;
	}

	ret = eficonfig_append_quit_entry(efi_menu);
	if (ret != EFI_SUCCESS)
		goto out;

	ret = eficonfig_process_common(efi_menu, "  ** Select Boot Option **",
				       eficonfig_menu_desc,
				       eficonfig_display_statusline,
				       eficonfig_print_entry,
				       eficonfig_choice_entry);
out:
	list_for_each_safe(pos, n, &efi_menu->list) {
		entry = list_entry(pos, struct eficonfig_entry, list);
		free(entry->data);
	}
	eficonfig_destroy(efi_menu);

	free(var_name16);

	return ret;
}

/**
 * eficonfig_process_edit_boot_option() - handler to edit boot option
 *
 * @data:	pointer to the data for each entry
 * Return:	status code
 */
static efi_status_t eficonfig_process_edit_boot_option(void *data)
{
	efi_status_t ret;
	efi_uintn_t size;
	struct eficonfig_boot_option *bo = NULL;

	while (1) {
		unsigned int selected;
		void *load_option;
		u16 varname[] = u"Boot####";

		ret = eficonfig_show_boot_selection(&selected);
		if (ret != EFI_SUCCESS)
			break;

		bo = calloc(1, sizeof(struct eficonfig_boot_option));
		if (!bo) {
			ret = EFI_OUT_OF_RESOURCES;
			goto out;
		}

		bo->boot_index = selected;
		efi_create_indexed_name(varname, sizeof(varname), "Boot", selected);
		load_option = efi_get_var(varname, &efi_global_variable_guid, &size);
		if (!load_option) {
			free(bo);
			ret = EFI_NOT_FOUND;
			goto out;
		}

		ret = eficonfig_edit_boot_option(varname, bo, load_option, size,
						 "  ** Edit Boot Option ** ");

		free(load_option);
		free(bo);
		if (ret != EFI_SUCCESS && ret != EFI_ABORTED)
			break;
	}
out:
	/* to stay the parent menu */
	ret = (ret == EFI_ABORTED) ? EFI_NOT_READY : ret;

	return ret;
}

/**
 * eficonfig_print_change_boot_order_entry() - print the boot option entry
 *
 * @data:	pointer to the data associated with each menu entry
 */
static void eficonfig_print_change_boot_order_entry(void *data)
{
	struct eficonfig_entry *entry = data;
	bool reverse = (entry->efi_menu->active == entry->num);

	if (entry->efi_menu->start > entry->num || entry->efi_menu->end < entry->num)
		return;

	printf(ANSI_CURSOR_POSITION ANSI_CLEAR_LINE,
	       (entry->num - entry->efi_menu->start) + EFICONFIG_MENU_HEADER_ROW_NUM + 1, 7);

	if (reverse)
		puts(ANSI_COLOR_REVERSE);

	if (entry->num < entry->efi_menu->count - 2) {
		if (((struct eficonfig_boot_order_data *)entry->data)->active)
			printf("[*]  ");
		else
			printf("[ ]  ");
	}

	printf("%s", entry->title);

	if (reverse)
		puts(ANSI_COLOR_RESET);
}

/**
 * eficonfig_choice_change_boot_order() - user key input handler
 *
 * @data:	pointer to the menu entry
 * Return:	key string to identify the selected entry
 */
char *eficonfig_choice_change_boot_order(void *data)
{
	struct cli_ch_state s_cch, *cch = &s_cch;
	struct list_head *pos, *n;
	struct efimenu *efi_menu = data;
	enum bootmenu_key key = BKEY_NONE;
	struct eficonfig_entry *entry, *tmp;

	cli_ch_init(cch);
	while (1) {
		key = bootmenu_loop(NULL, cch);

		switch (key) {
		case BKEY_PLUS:
			if (efi_menu->active > 0 &&
			    efi_menu->active < efi_menu->count - 2) {
				list_for_each_safe(pos, n, &efi_menu->list) {
					entry = list_entry(pos, struct eficonfig_entry, list);
					if (entry->num == efi_menu->active)
						break;
				}
				tmp = list_entry(pos->prev, struct eficonfig_entry, list);
				entry->num--;
				tmp->num++;
				list_del(&tmp->list);
				list_add(&tmp->list, &entry->list);

				eficonfig_menu_up(efi_menu);
			}
			return NULL;
		case BKEY_UP:
			if (efi_menu->active > 0)
				eficonfig_menu_up(efi_menu);

			return NULL;
		case BKEY_MINUS:
			if (efi_menu->active < efi_menu->count - 3) {
				list_for_each_safe(pos, n, &efi_menu->list) {
					entry = list_entry(pos, struct eficonfig_entry, list);
					if (entry->num == efi_menu->active)
						break;
				}
				tmp = list_entry(pos->next, struct eficonfig_entry, list);
				entry->num++;
				tmp->num--;
				list_del(&entry->list);
				list_add(&entry->list, &tmp->list);

				eficonfig_menu_down(efi_menu);
			}
			return NULL;
		case BKEY_DOWN:
			if (efi_menu->active < efi_menu->count - 1)
				eficonfig_menu_down(efi_menu);

			return NULL;
		case BKEY_SAVE:
			/* force to select "Save" entry */
			efi_menu->active = efi_menu->count - 2;
			fallthrough;
		case BKEY_SELECT:
			/* "Save" */
			if (efi_menu->active == efi_menu->count - 2) {
				list_for_each_prev_safe(pos, n, &efi_menu->list) {
					entry = list_entry(pos, struct eficonfig_entry, list);
					if (entry->num == efi_menu->active)
						break;
				}
				return entry->key;
			}
			/* "Quit" */
			if (efi_menu->active == efi_menu->count - 1) {
				entry = list_last_entry(&efi_menu->list,
							struct eficonfig_entry,
							list);
				return entry->key;
			}
			/* Pressed key is not valid, wait next key press */
			break;
		case BKEY_SPACE:
			if (efi_menu->active < efi_menu->count - 2) {
				list_for_each_safe(pos, n, &efi_menu->list) {
					entry = list_entry(pos, struct eficonfig_entry, list);
					if (entry->num == efi_menu->active) {
						struct eficonfig_boot_order_data *data = entry->data;

						data->active = !data->active;
						return NULL;
					}
				}
			}
			/* Pressed key is not valid, wait next key press */
			break;
		case BKEY_QUIT:
			entry = list_last_entry(&efi_menu->list,
						struct eficonfig_entry, list);
			return entry->key;
		default:
			/* Pressed key is not valid, wait next key press */
			break;
		}
	}
}

/**
 * eficonfig_process_save_boot_order() - callback function for "Save" entry
 *
 * @data:	pointer to the data
 * Return:	status code
 */
static efi_status_t eficonfig_process_save_boot_order(void *data)
{
	u32 count = 0;
	efi_status_t ret;
	efi_uintn_t size;
	struct list_head *pos, *n;
	u16 *new_bootorder;
	struct efimenu *efi_menu;
	struct eficonfig_entry *entry;
	struct eficonfig_save_boot_order_data *save_data = data;

	efi_menu = save_data->efi_menu;

	/*
	 * The change boot order menu always has "Save" and "Quit" entries.
	 * !(efi_menu->count - 2) means there is no user defined boot option.
	 */
	if (!(efi_menu->count - 2))
		return EFI_SUCCESS;

	new_bootorder = calloc(1, (efi_menu->count - 2) * sizeof(u16));
	if (!new_bootorder) {
		ret = EFI_OUT_OF_RESOURCES;
		goto out;
	}

	/* create new BootOrder */
	count = 0;
	list_for_each_safe(pos, n, &efi_menu->list) {
		struct eficonfig_boot_order_data *data;

		entry = list_entry(pos, struct eficonfig_entry, list);
		/* exit the loop when iteration reaches "Save" */
		if (!strncmp(entry->title, "Save", strlen("Save")))
			break;

		data = entry->data;
		if (data->active)
			new_bootorder[count++] = data->boot_index;
	}

	size = count * sizeof(u16);
	ret = efi_set_variable_int(u"BootOrder", &efi_global_variable_guid,
				   EFI_VARIABLE_NON_VOLATILE |
				   EFI_VARIABLE_BOOTSERVICE_ACCESS |
				   EFI_VARIABLE_RUNTIME_ACCESS,
				   size, new_bootorder, false);

	save_data->selected = true;
out:
	free(new_bootorder);

	return ret;
}

/**
 * eficonfig_add_change_boot_order_entry() - add boot order entry
 *
 * @efi_menu:	pointer to the efimenu structure
 * @boot_index:	boot option index to be added
 * @active:	flag to include the boot option into BootOrder
 * Return:	status code
 */
static efi_status_t eficonfig_add_change_boot_order_entry(struct efimenu *efi_menu,
							  u32 boot_index, bool active)
{
	char *title, *p;
	efi_status_t ret;
	efi_uintn_t size;
	void *load_option;
	struct efi_load_option lo;
	u16 varname[] = u"Boot####";
	struct eficonfig_boot_order_data *data;

	efi_create_indexed_name(varname, sizeof(varname), "Boot", boot_index);
	load_option = efi_get_var(varname, &efi_global_variable_guid, &size);
	if (!load_option)
		return EFI_SUCCESS;

	ret = efi_deserialize_load_option(&lo, load_option, &size);
	if (ret != EFI_SUCCESS)
		goto out;

	data = calloc(1, sizeof(*data));
	if (!data) {
		ret = EFI_OUT_OF_RESOURCES;
		goto out;
	}

	title = calloc(1, utf16_utf8_strlen(lo.label) + 1);
	if (!title) {
		free(data);
		ret = EFI_OUT_OF_RESOURCES;
		goto out;
	}
	p = title;
	utf16_utf8_strcpy(&p, lo.label);

	data->boot_index = boot_index;
	data->active = active;

	ret = eficonfig_append_menu_entry(efi_menu, title, NULL, data);
	if (ret != EFI_SUCCESS) {
		free(data);
		free(title);
		goto out;
	}

out:
	free(load_option);

	return ret;
}

/**
 * eficonfig_create_change_boot_order_entry() - create boot order entry
 *
 * @efi_menu:	pointer to the efimenu structure
 * @bootorder:	pointer to the BootOrder variable
 * @num:	number of BootOrder entry
 * Return:	status code
 */
static efi_status_t eficonfig_create_change_boot_order_entry(struct efimenu *efi_menu,
							     u16 *bootorder, efi_uintn_t num)
{
	u32 i;
	char *title;
	efi_status_t ret;
	u16 *var_name16 = NULL;
	efi_uintn_t size, buf_size;
	struct eficonfig_save_boot_order_data *save_data;

	/* list the load option in the order of BootOrder variable */
	for (i = 0; i < num; i++) {
		if (efi_menu->count >= EFICONFIG_ENTRY_NUM_MAX - 2)
			break;

		ret = eficonfig_add_change_boot_order_entry(efi_menu, bootorder[i], true);
		if (ret != EFI_SUCCESS)
			goto out;
	}

	/* list the remaining load option not included in the BootOrder */
	buf_size = 128;
	var_name16 = malloc(buf_size);
	if (!var_name16)
		return EFI_OUT_OF_RESOURCES;

	var_name16[0] = 0;
	for (;;) {
		int index;
		efi_guid_t guid;

		if (efi_menu->count >= EFICONFIG_ENTRY_NUM_MAX - 2)
			break;

		size = buf_size;
		ret = efi_next_variable_name(&buf_size, &var_name16, &guid);
		if (ret == EFI_NOT_FOUND)
			break;
		if (ret != EFI_SUCCESS)
			goto out;

		if (efi_varname_is_load_option(var_name16, &index)) {
			/* If the index is included in the BootOrder, skip it */
			if (efi_search_bootorder(bootorder, num, index, NULL))
				continue;

			ret = eficonfig_add_change_boot_order_entry(efi_menu, index, false);
			if (ret != EFI_SUCCESS)
				goto out;
		}
	}

	/* add "Save" and "Quit" entries */
	title = strdup("Save");
	if (!title) {
		ret = EFI_OUT_OF_RESOURCES;
		goto out;
	}

	save_data = malloc(sizeof(struct eficonfig_save_boot_order_data));
	if (!save_data) {
		ret = EFI_OUT_OF_RESOURCES;
		goto out;
	}
	save_data->efi_menu = efi_menu;
	save_data->selected = false;

	ret = eficonfig_append_menu_entry(efi_menu, title,
					  eficonfig_process_save_boot_order,
					  save_data);
	if (ret != EFI_SUCCESS)
		goto out;

	ret = eficonfig_append_quit_entry(efi_menu);
	if (ret != EFI_SUCCESS)
		goto out;

	efi_menu->active = 0;
out:
	free(var_name16);

	return ret;
}

/**
 * eficonfig_process_change_boot_order() - handler to change boot order
 *
 * @data:	pointer to the data for each entry
 * Return:	status code
 */
static efi_status_t eficonfig_process_change_boot_order(void *data)
{
	u16 *bootorder;
	efi_status_t ret;
	efi_uintn_t num, size;
	struct list_head *pos, *n;
	struct eficonfig_entry *entry;
	struct efimenu *efi_menu;

	efi_menu = calloc(1, sizeof(struct efimenu));
	if (!efi_menu)
		return EFI_OUT_OF_RESOURCES;

	bootorder = efi_get_var(u"BootOrder", &efi_global_variable_guid, &size);

	INIT_LIST_HEAD(&efi_menu->list);
	num = size / sizeof(u16);
	ret = eficonfig_create_change_boot_order_entry(efi_menu, bootorder, num);
	if (ret != EFI_SUCCESS)
		goto out;

	while (1) {
		ret = eficonfig_process_common(efi_menu,
					       "  ** Change Boot Order **",
					       eficonfig_change_boot_order_desc,
					       eficonfig_display_statusline,
					       eficonfig_print_change_boot_order_entry,
					       eficonfig_choice_change_boot_order);
		/* exit from the menu if user selects the "Save" entry. */
		if (ret == EFI_SUCCESS && efi_menu->active == (efi_menu->count - 2)) {
			list_for_each_prev_safe(pos, n, &efi_menu->list) {
				entry = list_entry(pos, struct eficonfig_entry, list);
				if (entry->num == efi_menu->active)
					break;
			}
			if (((struct eficonfig_save_boot_order_data *)entry->data)->selected)
				break;
		}
		if (ret != EFI_SUCCESS)
			break;
	}
out:
	free(bootorder);
	list_for_each_safe(pos, n, &efi_menu->list) {
		entry = list_entry(pos, struct eficonfig_entry, list);
		free(entry->data);
	}
	eficonfig_destroy(efi_menu);

	/* to stay the parent menu */
	ret = (ret == EFI_ABORTED) ? EFI_NOT_READY : ret;

	return ret;
}

/**
 * eficonfig_process_delete_boot_option() - handler to delete boot option
 *
 * @data:	pointer to the data for each entry
 * Return:	status code
 */
static efi_status_t eficonfig_process_delete_boot_option(void *data)
{
	efi_status_t ret;
	unsigned int selected;

	while (1) {
		ret = eficonfig_show_boot_selection(&selected);
		if (ret == EFI_SUCCESS)
			ret = efi_bootmgr_delete_boot_option(selected);

		if (ret != EFI_SUCCESS)
			break;
	}

	/* to stay the parent menu */
	ret = (ret == EFI_ABORTED) ? EFI_NOT_READY : ret;

	return ret;
}

/**
 * eficonfig_init() - do required initialization for eficonfig command
 *
 * Return:	status code
 */
static efi_status_t eficonfig_init(void)
{
	efi_status_t ret = EFI_SUCCESS;
	static bool init;
	struct efi_handler *handler;
	unsigned long columns, rows;

	if (!init) {
		ret = efi_search_protocol(efi_root, &efi_guid_text_input_protocol, &handler);
		if (ret != EFI_SUCCESS)
			return ret;

		ret = efi_protocol_open(handler, (void **)&cin, efi_root, NULL,
					EFI_OPEN_PROTOCOL_GET_PROTOCOL);
		if (ret != EFI_SUCCESS)
			return ret;
		ret = efi_search_protocol(efi_root, &efi_guid_text_output_protocol, &handler);
		if (ret != EFI_SUCCESS)
			return ret;

		ret = efi_protocol_open(handler, (void **)&cout, efi_root, NULL,
					EFI_OPEN_PROTOCOL_GET_PROTOCOL);
		if (ret != EFI_SUCCESS)
			return ret;

		cout->query_mode(cout, cout->mode->mode, &columns, &rows);
		avail_row = rows - (EFICONFIG_MENU_HEADER_ROW_NUM +
				    EFICONFIG_MENU_DESC_ROW_NUM);
		if (avail_row <= 0) {
			eficonfig_print_msg("Console size is too small!");
			return EFI_INVALID_PARAMETER;
		}
		/* TODO: Should we check the minimum column size? */
	}

	init = true;

	return ret;
}

static const struct eficonfig_item maintenance_menu_items[] = {
	{"Add Boot Option", eficonfig_process_add_boot_option},
	{"Edit Boot Option", eficonfig_process_edit_boot_option},
	{"Change Boot Order", eficonfig_process_change_boot_order},
	{"Delete Boot Option", eficonfig_process_delete_boot_option},
#if (IS_ENABLED(CONFIG_EFI_SECURE_BOOT) && IS_ENABLED(CONFIG_EFI_MM_COMM_TEE))
	{"Secure Boot Configuration", eficonfig_process_secure_boot_config},
#endif
	{"Quit", eficonfig_process_quit},
};

/**
 * do_eficonfig() - execute `eficonfig` command
 *
 * @cmdtp:	table entry describing command
 * @flag:	bitmap indicating how the command was invoked
 * @argc:	number of arguments
 * @argv:	command line arguments
 * Return:	status code
 */
static int do_eficonfig(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	efi_status_t ret;
	struct efimenu *efi_menu;

	if (argc > 1)
		return CMD_RET_USAGE;

	ret = efi_init_obj_list();
	if (ret != EFI_SUCCESS) {
		log_err("Error: Cannot initialize UEFI sub-system, r = %lu\n",
			ret & ~EFI_ERROR_MASK);

		return CMD_RET_FAILURE;
	}

	ret = eficonfig_init();
	if (ret != EFI_SUCCESS)
		return CMD_RET_FAILURE;

	ret = efi_bootmgr_update_media_device_boot_option();
	if (ret != EFI_SUCCESS)
		return ret;

	while (1) {
		efi_menu = eficonfig_create_fixed_menu(maintenance_menu_items,
						       ARRAY_SIZE(maintenance_menu_items));
		if (!efi_menu)
			return CMD_RET_FAILURE;

		ret = eficonfig_process_common(efi_menu,
					       "  ** UEFI Maintenance Menu **",
					       eficonfig_menu_desc,
					       eficonfig_display_statusline,
					       eficonfig_print_entry,
					       eficonfig_choice_entry);
		eficonfig_destroy(efi_menu);

		if (ret == EFI_ABORTED)
			break;
	}

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	eficonfig, 1, 0, do_eficonfig,
	"provide menu-driven UEFI variable maintenance interface",
	""
);
