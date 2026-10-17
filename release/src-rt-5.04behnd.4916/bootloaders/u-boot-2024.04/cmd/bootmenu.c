// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011-2013 Pali Rohár <pali@kernel.org>
 */

#include <charset.h>
#include <cli.h>
#include <common.h>
#include <command.h>
#include <ansi.h>
#include <efi_config.h>
#include <efi_variable.h>
#include <env.h>
#include <log.h>
#include <menu.h>
#include <watchdog.h>
#include <malloc.h>
#include <linux/delay.h>
#include <linux/string.h>

/* maximum bootmenu entries */
#define MAX_COUNT	99

/* maximal size of bootmenu env
 *  9 = strlen("bootmenu_")
 *  2 = strlen(MAX_COUNT)
 *  1 = NULL term
 */
#define MAX_ENV_SIZE	(9 + 2 + 1)

enum bootmenu_ret {
	BOOTMENU_RET_SUCCESS = 0,
	BOOTMENU_RET_FAIL,
	BOOTMENU_RET_QUIT,
	BOOTMENU_RET_UPDATED,
};

enum boot_type {
	BOOTMENU_TYPE_NONE = 0,
	BOOTMENU_TYPE_BOOTMENU,
	BOOTMENU_TYPE_UEFI_BOOT_OPTION,
};

struct bootmenu_entry {
	unsigned short int num;		/* unique number 0 .. MAX_COUNT */
	char key[3];			/* key identifier of number */
	char *title;			/* title of entry */
	char *command;			/* hush command of entry */
	enum boot_type type;		/* boot type of entry */
	u16 bootorder;			/* order for each boot type */
	struct bootmenu_data *menu;	/* this bootmenu */
	struct bootmenu_entry *next;	/* next menu entry (num+1) */
};

static char *bootmenu_getoption(unsigned short int n)
{
	char name[MAX_ENV_SIZE];

	if (n > MAX_COUNT)
		return NULL;

	sprintf(name, "bootmenu_%d", n);
	return env_get(name);
}

static void bootmenu_print_entry(void *data)
{
	struct bootmenu_entry *entry = data;
	int reverse = (entry->menu->active == entry->num);

	/*
	 * Move cursor to line where the entry will be drown (entry->num)
	 * First 3 lines contain bootmenu header + 1 empty line
	 */
	printf(ANSI_CURSOR_POSITION, entry->num + 4, 7);

	if (reverse)
		puts(ANSI_COLOR_REVERSE);

	printf("%s", entry->title);

	if (reverse)
		puts(ANSI_COLOR_RESET);
}

static char *bootmenu_choice_entry(void *data)
{
	struct cli_ch_state s_cch, *cch = &s_cch;
	struct bootmenu_data *menu = data;
	struct bootmenu_entry *iter;
	enum bootmenu_key key = BKEY_NONE;
	int i;

	cli_ch_init(cch);

	while (1) {
		if (menu->delay >= 0) {
			/* Autoboot was not stopped */
			key = bootmenu_autoboot_loop(menu, cch);
		} else {
			/* Some key was pressed, so autoboot was stopped */
			key = bootmenu_loop(menu, cch);
		}

		switch (key) {
		case BKEY_UP:
			if (menu->active > 0)
				--menu->active;
			/* no menu key selected, regenerate menu */
			return NULL;
		case BKEY_DOWN:
			if (menu->active < menu->count - 1)
				++menu->active;
			/* no menu key selected, regenerate menu */
			return NULL;
		case BKEY_SELECT:
			iter = menu->first;
			for (i = 0; i < menu->active; ++i)
				iter = iter->next;
			return iter->key;
		case BKEY_QUIT:
			/* Quit by choosing the last entry */
			iter = menu->first;
			while (iter->next)
				iter = iter->next;
			return iter->key;
		default:
			break;
		}
	}

	/* never happens */
	debug("bootmenu: this should not happen");
	return NULL;
}

static void bootmenu_destroy(struct bootmenu_data *menu)
{
	struct bootmenu_entry *iter = menu->first;
	struct bootmenu_entry *next;

	while (iter) {
		next = iter->next;
		free(iter->title);
		free(iter->command);
		free(iter);
		iter = next;
	}
	free(menu);
}

/**
 * prepare_bootmenu_entry() - generate the bootmenu_xx entries
 *
 * This function read the "bootmenu_x" U-Boot environment variable
 * and generate the bootmenu entries.
 *
 * @menu:	pointer to the bootmenu structure
 * @current:	pointer to the last bootmenu entry list
 * @index:	pointer to the index of the last bootmenu entry,
 *		the number of bootmenu entry is added by this function
 * Return:	1 on success, negative value on error
 */
static int prepare_bootmenu_entry(struct bootmenu_data *menu,
				  struct bootmenu_entry **current,
				  unsigned short int *index)
{
	char *sep;
	const char *option;
	unsigned short int i = *index;
	struct bootmenu_entry *entry = NULL;
	struct bootmenu_entry *iter = *current;

	while ((option = bootmenu_getoption(i))) {

		/* bootmenu_[num] format is "[title]=[commands]" */
		sep = strchr(option, '=');
		if (!sep) {
			printf("Invalid bootmenu entry: %s\n", option);
			break;
		}

		entry = malloc(sizeof(struct bootmenu_entry));
		if (!entry)
			return -ENOMEM;

		entry->title = strndup(option, sep - option);
		if (!entry->title) {
			free(entry);
			return -ENOMEM;
		}

		entry->command = strdup(sep + 1);
		if (!entry->command) {
			free(entry->title);
			free(entry);
			return -ENOMEM;
		}

		sprintf(entry->key, "%d", i);

		entry->num = i;
		entry->menu = menu;
		entry->type = BOOTMENU_TYPE_BOOTMENU;
		entry->bootorder = i;
		entry->next = NULL;

		if (!iter)
			menu->first = entry;
		else
			iter->next = entry;

		iter = entry;
		++i;

		if (i == MAX_COUNT - 1)
			break;
	}

	*index = i;
	*current = iter;

	return 1;
}

#if (IS_ENABLED(CONFIG_CMD_BOOTEFI_BOOTMGR)) && (IS_ENABLED(CONFIG_CMD_EFICONFIG))
/**
 * prepare_uefi_bootorder_entry() - generate the uefi bootmenu entries
 *
 * This function read the "BootOrder" UEFI variable
 * and generate the bootmenu entries in the order of "BootOrder".
 *
 * @menu:	pointer to the bootmenu structure
 * @current:	pointer to the last bootmenu entry list
 * @index:	pointer to the index of the last bootmenu entry,
 *		the number of uefi entry is added by this function
 * Return:	1 on success, negative value on error
 */
static int prepare_uefi_bootorder_entry(struct bootmenu_data *menu,
					struct bootmenu_entry **current,
					unsigned short int *index)
{
	u16 *bootorder;
	efi_status_t ret;
	unsigned short j;
	efi_uintn_t num, size;
	void *load_option;
	struct efi_load_option lo;
	u16 varname[] = u"Boot####";
	unsigned short int i = *index;
	struct bootmenu_entry *entry = NULL;
	struct bootmenu_entry *iter = *current;

	bootorder = efi_get_var(u"BootOrder", &efi_global_variable_guid, &size);
	if (!bootorder)
		return -ENOENT;

	num = size / sizeof(u16);
	for (j = 0; j < num; j++) {
		entry = malloc(sizeof(struct bootmenu_entry));
		if (!entry)
			return -ENOMEM;

		efi_create_indexed_name(varname, sizeof(varname),
					"Boot", bootorder[j]);
		load_option = efi_get_var(varname, &efi_global_variable_guid, &size);
		if (!load_option)
			continue;

		ret = efi_deserialize_load_option(&lo, load_option, &size);
		if (ret != EFI_SUCCESS) {
			log_warning("Invalid load option for %ls\n", varname);
			free(load_option);
			free(entry);
			continue;
		}

		if (lo.attributes & LOAD_OPTION_ACTIVE) {
			char *buf;

			buf = calloc(1, utf16_utf8_strlen(lo.label) + 1);
			if (!buf) {
				free(load_option);
				free(entry);
				free(bootorder);
				return -ENOMEM;
			}
			entry->title = buf;
			utf16_utf8_strncpy(&buf, lo.label, u16_strlen(lo.label));
			entry->command = strdup("bootefi bootmgr");
			sprintf(entry->key, "%d", i);
			entry->num = i;
			entry->menu = menu;
			entry->type = BOOTMENU_TYPE_UEFI_BOOT_OPTION;
			entry->bootorder = bootorder[j];
			entry->next = NULL;

			if (!iter)
				menu->first = entry;
			else
				iter->next = entry;

			iter = entry;
			i++;
		}

		free(load_option);

		if (i == MAX_COUNT - 1)
			break;
	}

	free(bootorder);
	*index = i;
	*current = iter;

	return 1;
}
#endif

static struct bootmenu_data *bootmenu_create(int delay)
{
	int ret;
	unsigned short int i = 0;
	struct bootmenu_data *menu;
	struct bootmenu_entry *iter = NULL;
	struct bootmenu_entry *entry;
	char *default_str;

	menu = malloc(sizeof(struct bootmenu_data));
	if (!menu)
		return NULL;

	menu->delay = delay;
	menu->active = 0;
	menu->first = NULL;

	default_str = env_get("bootmenu_default");
	if (default_str)
		menu->active = (int)simple_strtol(default_str, NULL, 10);

	ret = prepare_bootmenu_entry(menu, &iter, &i);
	if (ret < 0)
		goto cleanup;

#if (IS_ENABLED(CONFIG_CMD_BOOTEFI_BOOTMGR)) && (IS_ENABLED(CONFIG_CMD_EFICONFIG))
	if (i < MAX_COUNT - 1) {
		efi_status_t efi_ret;

		/*
		 * UEFI specification requires booting from removal media using
		 * a architecture-specific default image name such as BOOTAA64.EFI.
		 */
		efi_ret = efi_bootmgr_update_media_device_boot_option();
		if (efi_ret != EFI_SUCCESS)
			goto cleanup;

		ret = prepare_uefi_bootorder_entry(menu, &iter, &i);
		if (ret < 0 && ret != -ENOENT)
			goto cleanup;
	}
#endif

	/* Add Exit entry at the end */
	if (i <= MAX_COUNT - 1) {
		entry = malloc(sizeof(struct bootmenu_entry));
		if (!entry)
			goto cleanup;

		/* Add Quit entry if exiting bootmenu is disabled */
		if (!IS_ENABLED(CONFIG_BOOTMENU_DISABLE_UBOOT_CONSOLE))
			entry->title = strdup("Exit");
		else
			entry->title = strdup("Quit");

		if (!entry->title) {
			free(entry);
			goto cleanup;
		}

		entry->command = strdup("");
		if (!entry->command) {
			free(entry->title);
			free(entry);
			goto cleanup;
		}

		sprintf(entry->key, "%d", i);

		entry->num = i;
		entry->menu = menu;
		entry->type = BOOTMENU_TYPE_NONE;
		entry->next = NULL;

		if (!iter)
			menu->first = entry;
		else
			iter->next = entry;

		iter = entry;
		++i;
	}

	menu->count = i;

	if ((menu->active >= menu->count)||(menu->active < 0)) { //ensure active menuitem is inside menu
		printf("active menuitem (%d) is outside menu (0..%d)\n",menu->active,menu->count-1);
		menu->active=0;
	}

	return menu;

cleanup:
	bootmenu_destroy(menu);
	return NULL;
}

static void menu_display_statusline(struct menu *m)
{
	struct bootmenu_entry *entry;
	struct bootmenu_data *menu;

	if (menu_default_choice(m, (void *)&entry) < 0)
		return;

	menu = entry->menu;

	printf(ANSI_CURSOR_POSITION, 1, 1);
	puts(ANSI_CLEAR_LINE);
	printf(ANSI_CURSOR_POSITION, 2, 3);
	puts("*** U-Boot Boot Menu ***");
	puts(ANSI_CLEAR_LINE_TO_END);
	printf(ANSI_CURSOR_POSITION, 3, 1);
	puts(ANSI_CLEAR_LINE);

	/* First 3 lines are bootmenu header + 2 empty lines between entries */
	printf(ANSI_CURSOR_POSITION, menu->count + 5, 1);
	puts(ANSI_CLEAR_LINE);
	printf(ANSI_CURSOR_POSITION, menu->count + 6, 3);
	puts("Press UP/DOWN to move, ENTER to select, ESC to quit");
	puts(ANSI_CLEAR_LINE_TO_END);
	printf(ANSI_CURSOR_POSITION, menu->count + 7, 1);
	puts(ANSI_CLEAR_LINE);
}

static void handle_uefi_bootnext(void)
{
	u16 bootnext;
	efi_status_t ret;
	efi_uintn_t size;

	/* Initialize EFI drivers */
	ret = efi_init_obj_list();
	if (ret != EFI_SUCCESS) {
		log_err("Error: Cannot initialize UEFI sub-system, r = %lu\n",
			ret & ~EFI_ERROR_MASK);

		return;
	}

	/* If UEFI BootNext variable is set, boot the BootNext load option */
	size = sizeof(u16);
	ret = efi_get_variable_int(u"BootNext",
				   &efi_global_variable_guid,
				   NULL, &size, &bootnext, NULL);
	if (ret == EFI_SUCCESS)
		/* BootNext does exist here, try to boot */
		run_command("bootefi bootmgr", 0);
}

static enum bootmenu_ret bootmenu_show(int delay)
{
	int cmd_ret;
	int init = 0;
	void *choice = NULL;
	char *title = NULL;
	char *command = NULL;
	struct menu *menu;
	struct bootmenu_entry *iter;
	int ret = BOOTMENU_RET_SUCCESS;
	struct bootmenu_data *bootmenu;
	efi_status_t efi_ret = EFI_SUCCESS;
	char *option, *sep;

	if (IS_ENABLED(CONFIG_CMD_BOOTEFI_BOOTMGR))
		handle_uefi_bootnext();

	/* If delay is 0 do not create menu, just run first entry */
	if (delay == 0) {
		option = bootmenu_getoption(0);
		if (!option) {
			puts("bootmenu option 0 was not found\n");
			return BOOTMENU_RET_FAIL;
		}
		sep = strchr(option, '=');
		if (!sep) {
			puts("bootmenu option 0 is invalid\n");
			return BOOTMENU_RET_FAIL;
		}
		cmd_ret = run_command(sep + 1, 0);
		return (cmd_ret == CMD_RET_SUCCESS ? BOOTMENU_RET_SUCCESS : BOOTMENU_RET_FAIL);
	}

	bootmenu = bootmenu_create(delay);
	if (!bootmenu)
		return BOOTMENU_RET_FAIL;

	menu = menu_create(NULL, bootmenu->delay, 1, menu_display_statusline,
			   bootmenu_print_entry, bootmenu_choice_entry,
			   bootmenu);
	if (!menu) {
		bootmenu_destroy(bootmenu);
		return BOOTMENU_RET_FAIL;
	}

	for (iter = bootmenu->first; iter; iter = iter->next) {
		if (menu_item_add(menu, iter->key, iter) != 1)
			goto cleanup;
	}

	/* Default menu entry is always first */
	menu_default_set(menu, "0");

	puts(ANSI_CURSOR_HIDE);
	puts(ANSI_CLEAR_CONSOLE);
	printf(ANSI_CURSOR_POSITION, 1, 1);

	init = 1;

	if (menu_get_choice(menu, &choice) == 1) {
		iter = choice;
		title = strdup(iter->title);
		command = strdup(iter->command);

		/* last entry exits bootmenu */
		if (iter->num == iter->menu->count - 1) {
			ret = BOOTMENU_RET_QUIT;
			goto cleanup;
		}
	} else {
		goto cleanup;
	}

	/*
	 * If the selected entry is UEFI BOOT####, set the BootNext variable.
	 * Then uefi bootmgr is invoked by the preset command in iter->command.
	 */
	if (IS_ENABLED(CONFIG_CMD_BOOTEFI_BOOTMGR)) {
		if (iter->type == BOOTMENU_TYPE_UEFI_BOOT_OPTION) {
			/*
			 * UEFI specification requires BootNext variable needs non-volatile
			 * attribute, but this BootNext is only used inside of U-Boot and
			 * removed by efi bootmgr once BootNext is processed.
			 * So this BootNext can be volatile.
			 */
			efi_ret = efi_set_variable_int(u"BootNext", &efi_global_variable_guid,
						       EFI_VARIABLE_BOOTSERVICE_ACCESS |
						       EFI_VARIABLE_RUNTIME_ACCESS,
						       sizeof(u16), &iter->bootorder, false);
			if (efi_ret != EFI_SUCCESS)
				goto cleanup;
		}
	}

cleanup:
	menu_destroy(menu);
	bootmenu_destroy(bootmenu);

	if (init) {
		puts(ANSI_CURSOR_SHOW);
		puts(ANSI_CLEAR_CONSOLE);
		printf(ANSI_CURSOR_POSITION, 1, 1);
	}

	if (title && command) {
		debug("Starting entry '%s'\n", title);
		free(title);
		if (efi_ret == EFI_SUCCESS)
			cmd_ret = run_command(command, 0);
		free(command);
	}

#ifdef CFG_POSTBOOTMENU
	run_command(CFG_POSTBOOTMENU, 0);
#endif

	if (efi_ret != EFI_SUCCESS || cmd_ret != CMD_RET_SUCCESS)
		ret = BOOTMENU_RET_FAIL;

	return ret;
}

#ifdef CONFIG_AUTOBOOT_MENU_SHOW
int menu_show(int bootdelay)
{
	int ret;

	while (1) {
		ret = bootmenu_show(bootdelay);
		bootdelay = -1;
		if (ret == BOOTMENU_RET_UPDATED)
			continue;

		if (IS_ENABLED(CONFIG_BOOTMENU_DISABLE_UBOOT_CONSOLE)) {
			if (ret == BOOTMENU_RET_QUIT) {
				/* default boot process */
				if (IS_ENABLED(CONFIG_CMD_BOOTEFI_BOOTMGR))
					run_command("bootefi bootmgr", 0);

				run_command("run bootcmd", 0);
			}
		} else {
			break;
		}
	}

	return -1; /* -1 - abort boot and run monitor code */
}
#endif

int do_bootmenu(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	char *delay_str = NULL;
	int delay = 10;

#if defined(CONFIG_BOOTDELAY) && (CONFIG_BOOTDELAY >= 0)
	delay = CONFIG_BOOTDELAY;
#endif

	if (argc >= 2)
		delay_str = argv[1];

	if (!delay_str)
		delay_str = env_get("bootmenu_delay");

	if (delay_str)
		delay = (int)simple_strtol(delay_str, NULL, 10);

	bootmenu_show(delay);
	return 0;
}

U_BOOT_CMD(
	bootmenu, 2, 1, do_bootmenu,
	"ANSI terminal bootmenu",
	"[delay]\n"
	"    - show ANSI terminal bootmenu with autoboot delay"
);
