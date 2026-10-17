// SPDX-License-Identifier: GPL-2.0+
/*
 * 'bootflow' command
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <bootdev.h>
#include <bootflow.h>
#include <bootm.h>
#include <bootstd.h>
#include <command.h>
#include <console.h>
#include <dm.h>
#include <mapmem.h>

/**
 * report_bootflow_err() - Report where a bootflow failed
 *
 * When a bootflow does not make it to the 'loaded' state, something went wrong.
 * Print a helpful message if there is an error
 *
 * @bflow: Bootflow to process
 * @err: Error code (0 if none)
 */
static void report_bootflow_err(struct bootflow *bflow, int err)
{
	if (!err)
		return;

	/* Indent out to 'Method' */
	printf("     ** ");

	switch (bflow->state) {
	case BOOTFLOWST_BASE:
		printf("No media/partition found");
		break;
	case BOOTFLOWST_MEDIA:
		printf("No partition found");
		break;
	case BOOTFLOWST_PART:
		printf("No filesystem found");
		break;
	case BOOTFLOWST_FS:
		printf("File not found");
		break;
	case BOOTFLOWST_FILE:
		printf("File cannot be loaded");
		break;
	case BOOTFLOWST_READY:
		printf("Ready");
		break;
	case BOOTFLOWST_COUNT:
		break;
	}

	printf(", err=%dE\n", err);
}

/**
 * show_bootflow() - Show the status of a bootflow
 *
 * @seq: Bootflow index
 * @bflow: Bootflow to show
 * @errors: True to show the error received, if any
 */
static void show_bootflow(int index, struct bootflow *bflow, bool errors)
{
	printf("%3x  %-11s  %-6s  %-9.9s %4x  %-25.25s %s\n", index,
	       bflow->method->name, bootflow_state_get_name(bflow->state),
	       bflow->dev ? dev_get_uclass_name(dev_get_parent(bflow->dev)) :
	       "(none)", bflow->part, bflow->name, bflow->fname ?: "");
	if (errors)
		report_bootflow_err(bflow, bflow->err);
}

static void show_header(void)
{
	printf("Seq  Method       State   Uclass    Part  Name                      Filename\n");
	printf("---  -----------  ------  --------  ----  ------------------------  ----------------\n");
}

static void show_footer(int count, int num_valid)
{
	printf("---  -----------  ------  --------  ----  ------------------------  ----------------\n");
	printf("(%d bootflow%s, %d valid)\n", count, count != 1 ? "s" : "",
	       num_valid);
}

/**
 * bootflow_handle_menu() - Handle running the menu and updating cur bootflow
 *
 * This shows the menu, allows the user to select something and then prints
 * what happened
 *
 * @std: bootstd information
 * @text_mode: true to run the menu in text mode
 * @bflowp: Returns selected bootflow, on success
 * Return: 0 on success (a bootflow was selected), -EAGAIN if nothing was
 *	chosen, other -ve value on other error
 */
__maybe_unused static int bootflow_handle_menu(struct bootstd_priv *std,
					       bool text_mode,
					       struct bootflow **bflowp)
{
	struct bootflow *bflow;
	int ret;

	ret = bootflow_menu_run(std, text_mode, &bflow);
	if (ret) {
		if (ret == -EAGAIN) {
			printf("Nothing chosen\n");
			std->cur_bootflow = NULL;
		} else {
			printf("Menu failed (err=%d)\n", ret);
		}

		return ret;
	}

	printf("Selected: %s\n", bflow->os_name ? bflow->os_name : bflow->name);
	std->cur_bootflow = bflow;
	*bflowp = bflow;

	return 0;
}

static int do_bootflow_scan(struct cmd_tbl *cmdtp, int flag, int argc,
			    char *const argv[])
{
	struct bootstd_priv *std;
	struct bootflow_iter iter;
	struct udevice *dev = NULL;
	struct bootflow bflow;
	bool all = false, boot = false, errors = false, no_global = false;
	bool list = false, no_hunter = false, menu = false, text_mode = false;
	int num_valid = 0;
	const char *label = NULL;
	bool has_args;
	int ret, i;
	int flags;

	ret = bootstd_get_priv(&std);
	if (ret)
		return CMD_RET_FAILURE;

	has_args = argc > 1 && *argv[1] == '-';
	if (IS_ENABLED(CONFIG_CMD_BOOTFLOW_FULL)) {
		if (has_args) {
			all = strchr(argv[1], 'a');
			boot = strchr(argv[1], 'b');
			errors = strchr(argv[1], 'e');
			no_global = strchr(argv[1], 'G');
			list = strchr(argv[1], 'l');
			no_hunter = strchr(argv[1], 'H');
			menu = strchr(argv[1], 'm');
			text_mode = strchr(argv[1], 't');
			argc--;
			argv++;
		}
		if (argc > 1)
			label = argv[1];
		if (!label)
			dev = std->cur_bootdev;
	} else {
		if (has_args) {
			printf("Flags not supported: enable CONFIG_BOOTSTD_FULL\n");
			return CMD_RET_USAGE;
		}
		boot = true;
	}

	std->cur_bootflow = NULL;

	flags = 0;
	if (list)
		flags |= BOOTFLOWIF_SHOW;
	if (all)
		flags |= BOOTFLOWIF_ALL;
	if (no_global)
		flags |= BOOTFLOWIF_SKIP_GLOBAL;
	if (!no_hunter)
		flags |= BOOTFLOWIF_HUNT;

	/*
	 * If we have a device, just scan for bootflows attached to that device
	 */
	if (list) {
		printf("Scanning for bootflows ");
		if (dev)
			printf("in bootdev '%s'\n", dev->name);
		else if (label)
			printf("with label '%s'\n", label);
		else
			printf("in all bootdevs\n");
		show_header();
	}
	if (dev)
		bootdev_clear_bootflows(dev);
	else
		bootstd_clear_glob();
	for (i = 0,
	     ret = bootflow_scan_first(dev, label, &iter, flags, &bflow);
	     i < 1000 && ret != -ENODEV;
	     i++, ret = bootflow_scan_next(&iter, &bflow)) {
		bflow.err = ret;
		if (!ret)
			num_valid++;
		ret = bootdev_add_bootflow(&bflow);
		if (ret) {
			printf("Out of memory\n");
			return CMD_RET_FAILURE;
		}
		if (list)
			show_bootflow(i, &bflow, errors);
		if (!menu && boot && !bflow.err)
			bootflow_run_boot(&iter, &bflow);
	}
	bootflow_iter_uninit(&iter);
	if (list)
		show_footer(i, num_valid);

	if (IS_ENABLED(CONFIG_CMD_BOOTFLOW_FULL) && IS_ENABLED(CONFIG_EXPO)) {
		if (!num_valid && !list) {
			printf("No bootflows found; try again with -l\n");
		} else if (menu) {
			struct bootflow *sel_bflow;

			ret = bootflow_handle_menu(std, text_mode, &sel_bflow);
			if (!ret && boot) {
				ret = console_clear();
				if (ret) {
					log_err("Failed to clear console: %dE\n",
						ret);
					return ret;
				}

				bootflow_run_boot(NULL, sel_bflow);
			}
		}
	}

	return 0;
}

#ifdef CONFIG_CMD_BOOTFLOW_FULL
static int do_bootflow_list(struct cmd_tbl *cmdtp, int flag, int argc,
			    char *const argv[])
{
	struct bootstd_priv *std;
	struct udevice *dev;
	struct bootflow *bflow;
	int num_valid = 0;
	bool errors = false;
	int ret, i;

	if (argc > 1 && *argv[1] == '-')
		errors = strchr(argv[1], 'e');

	ret = bootstd_get_priv(&std);
	if (ret)
		return CMD_RET_FAILURE;
	dev = std->cur_bootdev;

	/* If we have a device, just list bootflows attached to that device */
	if (dev) {
		printf("Showing bootflows for bootdev '%s'\n", dev->name);
		show_header();
		for (ret = bootdev_first_bootflow(dev, &bflow), i = 0;
		     !ret;
		     ret = bootdev_next_bootflow(&bflow), i++) {
			num_valid += bflow->state == BOOTFLOWST_READY;
			show_bootflow(i, bflow, errors);
		}
	} else {
		printf("Showing all bootflows\n");
		show_header();
		for (ret = bootflow_first_glob(&bflow), i = 0;
		     !ret;
		     ret = bootflow_next_glob(&bflow), i++) {
			num_valid += bflow->state == BOOTFLOWST_READY;
			show_bootflow(i, bflow, errors);
		}
	}
	show_footer(i, num_valid);

	return 0;
}

static int do_bootflow_select(struct cmd_tbl *cmdtp, int flag, int argc,
			      char *const argv[])
{
	struct bootstd_priv *std;
	struct bootflow *bflow, *found;
	struct udevice *dev;
	const char *name;
	char *endp;
	int seq, i;
	int ret;

	ret = bootstd_get_priv(&std);
	if (ret)
		return CMD_RET_FAILURE;
;
	if (argc < 2) {
		std->cur_bootflow = NULL;
		return 0;
	}
	dev = std->cur_bootdev;

	name = argv[1];
	seq = simple_strtol(name, &endp, 16);
	found = NULL;

	/*
	 * If we have a bootdev device, only allow selection of bootflows
	 * attached to that device
	 */
	if (dev) {
		for (ret = bootdev_first_bootflow(dev, &bflow), i = 0;
		     !ret;
		     ret = bootdev_next_bootflow(&bflow), i++) {
			if (*endp ? !strcmp(bflow->name, name) : i == seq) {
				found = bflow;
				break;
			}
		}
	} else {
		for (ret = bootflow_first_glob(&bflow), i = 0;
		     !ret;
		     ret = bootflow_next_glob(&bflow), i++) {
			if (*endp ? !strcmp(bflow->name, name) : i == seq) {
				found = bflow;
				break;
			}
		}
	}

	if (!found) {
		printf("Cannot find bootflow '%s' ", name);
		if (dev)
			printf("in bootdev '%s' ", dev->name);
		printf("(err=%d)\n", ret);
		return CMD_RET_FAILURE;
	}
	std->cur_bootflow = found;
	if (IS_ENABLED(CONFIG_BOOTSTD_FULL)) {
		if (env_set("bootargs", found->cmdline)) {
			printf("Cannot set bootargs\n");
			return CMD_RET_FAILURE;
		}
	}

	return 0;
}

static int do_bootflow_info(struct cmd_tbl *cmdtp, int flag, int argc,
			    char *const argv[])
{
	struct bootstd_priv *std;
	struct bootflow *bflow;
	bool x86_setup = false;
	bool dump = false;
	int ret;

	if (argc > 1 && *argv[1] == '-') {
		dump = strchr(argv[1], 'd');
		x86_setup = strchr(argv[1], 's');
	}

	ret = bootstd_get_priv(&std);
	if (ret)
		return CMD_RET_FAILURE;

	if (!std->cur_bootflow) {
		printf("No bootflow selected\n");
		return CMD_RET_FAILURE;
	}
	bflow = std->cur_bootflow;

	if (IS_ENABLED(CONFIG_X86) && x86_setup) {
		zimage_dump(bflow->x86_setup, false);

		return 0;
	}

	printf("Name:      %s\n", bflow->name);
	printf("Device:    %s\n", bflow->dev->name);
	printf("Block dev: %s\n", bflow->blk ? bflow->blk->name : "(none)");
	printf("Method:    %s\n", bflow->method->name);
	printf("State:     %s\n", bootflow_state_get_name(bflow->state));
	printf("Partition: %d\n", bflow->part);
	printf("Subdir:    %s\n", bflow->subdir ? bflow->subdir : "(none)");
	printf("Filename:  %s\n", bflow->fname);
	printf("Buffer:    %lx\n", (ulong)map_to_sysmem(bflow->buf));
	printf("Size:      %x (%d bytes)\n", bflow->size, bflow->size);
	printf("OS:        %s\n", bflow->os_name ? bflow->os_name : "(none)");
	printf("Cmdline:   ");
	if (bflow->cmdline)
		puts(bflow->cmdline);
	else
		puts("(none)");
	putc('\n');
	if (bflow->x86_setup)
		printf("X86 setup: %p\n", bflow->x86_setup);
	printf("Logo:      %s\n", bflow->logo ?
	       simple_xtoa((ulong)map_to_sysmem(bflow->logo)) : "(none)");
	if (bflow->logo) {
		printf("Logo size: %x (%d bytes)\n", bflow->logo_size,
		       bflow->logo_size);
	}
	printf("FDT:       %s\n", bflow->fdt_fname);
	if (bflow->fdt_fname) {
		printf("FDT size:  %x (%d bytes)\n", bflow->fdt_size,
		       bflow->fdt_size);
		printf("FDT addr:  %lx\n", bflow->fdt_addr);
	}
	printf("Error:     %d\n", bflow->err);
	if (dump && bflow->buf) {
		/* Set some sort of maximum on the size */
		int size = min(bflow->size, 10 << 10);
		int i;

		printf("Contents:\n\n");
		for (i = 0; i < size; i++) {
			putc(bflow->buf[i]);
			if (!(i % 128) && ctrlc()) {
				printf("...interrupted\n");
				break;
			}
		}
	}

	return 0;
}

static int do_bootflow_read(struct cmd_tbl *cmdtp, int flag, int argc,
			    char *const argv[])
{
	struct bootstd_priv *std;
	struct bootflow *bflow;
	int ret;

	ret = bootstd_get_priv(&std);
	if (ret)
		return CMD_RET_FAILURE;

	/*
	 * Require a current bootflow. Users can use 'bootflow scan -b' to
	 * automatically scan and boot, if needed.
	 */
	if (!std->cur_bootflow) {
		printf("No bootflow selected\n");
		return CMD_RET_FAILURE;
	}
	bflow = std->cur_bootflow;
	ret = bootflow_read_all(bflow);
	if (ret) {
		printf("Failed: err=%dE\n", ret);
		return CMD_RET_FAILURE;
	}

	return 0;
}

static int do_bootflow_boot(struct cmd_tbl *cmdtp, int flag, int argc,
			    char *const argv[])
{
	struct bootstd_priv *std;
	struct bootflow *bflow;
	int ret;

	ret = bootstd_get_priv(&std);
	if (ret)
		return CMD_RET_FAILURE;

	/*
	 * Require a current bootflow. Users can use 'bootflow scan -b' to
	 * automatically scan and boot, if needed.
	 */
	if (!std->cur_bootflow) {
		printf("No bootflow selected\n");
		return CMD_RET_FAILURE;
	}
	bflow = std->cur_bootflow;
	ret = bootflow_run_boot(NULL, bflow);
	if (ret)
		return CMD_RET_FAILURE;

	return 0;
}

static int do_bootflow_menu(struct cmd_tbl *cmdtp, int flag, int argc,
			    char *const argv[])
{
	struct bootstd_priv *std;
	struct bootflow *bflow;
	bool text_mode = false;
	int ret;

	if (!IS_ENABLED(CONFIG_EXPO)) {
		printf("Menu not supported\n");
		return CMD_RET_FAILURE;
	}

	if (argc > 1 && *argv[1] == '-')
		text_mode = strchr(argv[1], 't');

	ret = bootstd_get_priv(&std);
	if (ret)
		return CMD_RET_FAILURE;

	ret = bootflow_handle_menu(std, text_mode, &bflow);
	if (ret)
		return CMD_RET_FAILURE;

	return 0;
}

static int do_bootflow_cmdline(struct cmd_tbl *cmdtp, int flag, int argc,
			       char *const argv[])
{
	struct bootstd_priv *std;
	struct bootflow *bflow;
	const char *op, *arg, *val = NULL;
	int ret;

	if (argc < 3)
		return CMD_RET_USAGE;

	ret = bootstd_get_priv(&std);
	if (ret)
		return CMD_RET_FAILURE;

	bflow = std->cur_bootflow;
	if (!bflow) {
		printf("No bootflow selected\n");
		return CMD_RET_FAILURE;
	}

	op = argv[1];
	arg = argv[2];
	if (*op == 's') {
		val = argv[3] ?: (const char *)BOOTFLOWCL_EMPTY;
	}

	switch (*op) {
	case 'c':	/* clear */
		val = "";
		fallthrough;
	case 's':	/* set */
	case 'd':	/* delete */
		ret = bootflow_cmdline_set_arg(bflow, arg, val, true);
		break;
	case 'g':	/* get */
		ret = bootflow_cmdline_get_arg(bflow, arg, &val);
		if (ret >= 0)
			printf("%.*s\n", ret, val);
		break;
	case 'a':	/* auto */
		ret = bootflow_cmdline_auto(bflow, arg);
		break;
	}
	switch (ret) {
	case -E2BIG:
		printf("Argument too long\n");
		break;
	case -ENOENT:
		printf("Argument not found\n");
		break;
	case -EINVAL:
		printf("Mismatched quotes\n");
		break;
	case -EBADF:
		printf("Value must be quoted\n");
		break;
	default:
		if (ret < 0)
			printf("Unknown error: %dE\n", ret);
	}
	if (ret < 0)
		return CMD_RET_FAILURE;

	return 0;
}
#endif /* CONFIG_CMD_BOOTFLOW_FULL */

U_BOOT_LONGHELP(bootflow,
#ifdef CONFIG_CMD_BOOTFLOW_FULL
	"scan [-abeGl] [bdev]  - scan for valid bootflows (-l list, -a all, -e errors, -b boot, -G no global)\n"
	"bootflow list [-e]             - list scanned bootflows (-e errors)\n"
	"bootflow select [<num>|<name>] - select a bootflow\n"
	"bootflow info [-ds]            - show info on current bootflow (-d dump bootflow)\n"
	"bootflow read                  - read all current-bootflow files\n"
	"bootflow boot                  - boot current bootflow\n"
	"bootflow menu [-t]             - show a menu of available bootflows\n"
	"bootflow cmdline [set|get|clear|delete|auto] <param> [<value>] - update cmdline"
#else
	"scan - boot first available bootflow\n"
#endif
	);

U_BOOT_CMD_WITH_SUBCMDS(bootflow, "Boot flows", bootflow_help_text,
	U_BOOT_SUBCMD_MKENT(scan, 3, 1, do_bootflow_scan),
#ifdef CONFIG_CMD_BOOTFLOW_FULL
	U_BOOT_SUBCMD_MKENT(list, 2, 1, do_bootflow_list),
	U_BOOT_SUBCMD_MKENT(select, 2, 1, do_bootflow_select),
	U_BOOT_SUBCMD_MKENT(info, 2, 1, do_bootflow_info),
	U_BOOT_SUBCMD_MKENT(read, 1, 1, do_bootflow_read),
	U_BOOT_SUBCMD_MKENT(boot, 1, 1, do_bootflow_boot),
	U_BOOT_SUBCMD_MKENT(menu, 2, 1, do_bootflow_menu),
	U_BOOT_SUBCMD_MKENT(cmdline, 4, 1, do_bootflow_cmdline),
#endif
);
