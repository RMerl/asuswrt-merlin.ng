// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2011 Calxeda, Inc.
 * Copyright (c) 2014, NVIDIA CORPORATION.  All rights reserved.
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <mapmem.h>
#include <lcd.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <errno.h>
#include <linux/list.h>
#include <fs.h>
#include <splash.h>
#include <asm/io.h>

#include "menu.h"
#include "cli.h"

#define MAX_TFTP_PATH_LEN 127

const char *pxe_default_paths[] = {
#ifdef CONFIG_SYS_SOC
#ifdef CONFIG_SYS_BOARD
	"default-" CONFIG_SYS_ARCH "-" CONFIG_SYS_SOC "-" CONFIG_SYS_BOARD,
#endif
	"default-" CONFIG_SYS_ARCH "-" CONFIG_SYS_SOC,
#endif
	"default-" CONFIG_SYS_ARCH,
	"default",
	NULL
};

static bool is_pxe;

/*
 * Like env_get, but prints an error if envvar isn't defined in the
 * environment.  It always returns what env_get does, so it can be used in
 * place of env_get without changing error handling otherwise.
 */
static char *from_env(const char *envvar)
{
	char *ret;

	ret = env_get(envvar);

	if (!ret)
		printf("missing environment variable: %s\n", envvar);

	return ret;
}

#ifdef CONFIG_CMD_NET
/*
 * Convert an ethaddr from the environment to the format used by pxelinux
 * filenames based on mac addresses. Convert's ':' to '-', and adds "01-" to
 * the beginning of the ethernet address to indicate a hardware type of
 * Ethernet. Also converts uppercase hex characters into lowercase, to match
 * pxelinux's behavior.
 *
 * Returns 1 for success, -ENOENT if 'ethaddr' is undefined in the
 * environment, or some other value < 0 on error.
 */
static int format_mac_pxe(char *outbuf, size_t outbuf_len)
{
	uchar ethaddr[6];

	if (outbuf_len < 21) {
		printf("outbuf is too small (%zd < 21)\n", outbuf_len);

		return -EINVAL;
	}

	if (!eth_env_get_enetaddr_by_index("eth", eth_get_dev_index(), ethaddr))
		return -ENOENT;

	sprintf(outbuf, "01-%02x-%02x-%02x-%02x-%02x-%02x",
		ethaddr[0], ethaddr[1], ethaddr[2],
		ethaddr[3], ethaddr[4], ethaddr[5]);

	return 1;
}
#endif

/*
 * Returns the directory the file specified in the bootfile env variable is
 * in. If bootfile isn't defined in the environment, return NULL, which should
 * be interpreted as "don't prepend anything to paths".
 */
static int get_bootfile_path(const char *file_path, char *bootfile_path,
			     size_t bootfile_path_size)
{
	char *bootfile, *last_slash;
	size_t path_len = 0;

	/* Only syslinux allows absolute paths */
	if (file_path[0] == '/' && !is_pxe)
		goto ret;

	bootfile = from_env("bootfile");

	if (!bootfile)
		goto ret;

	last_slash = strrchr(bootfile, '/');

	if (last_slash == NULL)
		goto ret;

	path_len = (last_slash - bootfile) + 1;

	if (bootfile_path_size < path_len) {
		printf("bootfile_path too small. (%zd < %zd)\n",
				bootfile_path_size, path_len);

		return -1;
	}

	strncpy(bootfile_path, bootfile, path_len);

 ret:
	bootfile_path[path_len] = '\0';

	return 1;
}

static int (*do_getfile)(cmd_tbl_t *cmdtp, const char *file_path, char *file_addr);

#ifdef CONFIG_CMD_NET
static int do_get_tftp(cmd_tbl_t *cmdtp, const char *file_path, char *file_addr)
{
	char *tftp_argv[] = {"tftp", NULL, NULL, NULL};

	tftp_argv[1] = file_addr;
	tftp_argv[2] = (void *)file_path;

	if (do_tftpb(cmdtp, 0, 3, tftp_argv))
		return -ENOENT;

	return 1;
}
#endif

static char *fs_argv[5];

static int do_get_ext2(cmd_tbl_t *cmdtp, const char *file_path, char *file_addr)
{
#ifdef CONFIG_CMD_EXT2
	fs_argv[0] = "ext2load";
	fs_argv[3] = file_addr;
	fs_argv[4] = (void *)file_path;

	if (!do_ext2load(cmdtp, 0, 5, fs_argv))
		return 1;
#endif
	return -ENOENT;
}

static int do_get_fat(cmd_tbl_t *cmdtp, const char *file_path, char *file_addr)
{
#ifdef CONFIG_CMD_FAT
	fs_argv[0] = "fatload";
	fs_argv[3] = file_addr;
	fs_argv[4] = (void *)file_path;

	if (!do_fat_fsload(cmdtp, 0, 5, fs_argv))
		return 1;
#endif
	return -ENOENT;
}

static int do_get_any(cmd_tbl_t *cmdtp, const char *file_path, char *file_addr)
{
#ifdef CONFIG_CMD_FS_GENERIC
	fs_argv[0] = "load";
	fs_argv[3] = file_addr;
	fs_argv[4] = (void *)file_path;

	if (!do_load(cmdtp, 0, 5, fs_argv, FS_TYPE_ANY))
		return 1;
#endif
	return -ENOENT;
}

/*
 * As in pxelinux, paths to files referenced from files we retrieve are
 * relative to the location of bootfile. get_relfile takes such a path and
 * joins it with the bootfile path to get the full path to the target file. If
 * the bootfile path is NULL, we use file_path as is.
 *
 * Returns 1 for success, or < 0 on error.
 */
static int get_relfile(cmd_tbl_t *cmdtp, const char *file_path,
	unsigned long file_addr)
{
	size_t path_len;
	char relfile[MAX_TFTP_PATH_LEN+1];
	char addr_buf[18];
	int err;

	err = get_bootfile_path(file_path, relfile, sizeof(relfile));

	if (err < 0)
		return err;

	path_len = strlen(file_path);
	path_len += strlen(relfile);

	if (path_len > MAX_TFTP_PATH_LEN) {
		printf("Base path too long (%s%s)\n",
					relfile,
					file_path);

		return -ENAMETOOLONG;
	}

	strcat(relfile, file_path);

	printf("Retrieving file: %s\n", relfile);

	sprintf(addr_buf, "%lx", file_addr);

	return do_getfile(cmdtp, relfile, addr_buf);
}

/*
 * Retrieve the file at 'file_path' to the locate given by 'file_addr'. If
 * 'bootfile' was specified in the environment, the path to bootfile will be
 * prepended to 'file_path' and the resulting path will be used.
 *
 * Returns 1 on success, or < 0 for error.
 */
static int get_pxe_file(cmd_tbl_t *cmdtp, const char *file_path,
	unsigned long file_addr)
{
	unsigned long config_file_size;
	char *tftp_filesize;
	int err;
	char *buf;

	err = get_relfile(cmdtp, file_path, file_addr);

	if (err < 0)
		return err;

	/*
	 * the file comes without a NUL byte at the end, so find out its size
	 * and add the NUL byte.
	 */
	tftp_filesize = from_env("filesize");

	if (!tftp_filesize)
		return -ENOENT;

	if (strict_strtoul(tftp_filesize, 16, &config_file_size) < 0)
		return -EINVAL;

	buf = map_sysmem(file_addr + config_file_size, 1);
	*buf = '\0';
	unmap_sysmem(buf);

	return 1;
}

#ifdef CONFIG_CMD_NET

#define PXELINUX_DIR "pxelinux.cfg/"

/*
 * Retrieves a file in the 'pxelinux.cfg' folder. Since this uses get_pxe_file
 * to do the hard work, the location of the 'pxelinux.cfg' folder is generated
 * from the bootfile path, as described above.
 *
 * Returns 1 on success or < 0 on error.
 */
static int get_pxelinux_path(cmd_tbl_t *cmdtp, const char *file,
	unsigned long pxefile_addr_r)
{
	size_t base_len = strlen(PXELINUX_DIR);
	char path[MAX_TFTP_PATH_LEN+1];

	if (base_len + strlen(file) > MAX_TFTP_PATH_LEN) {
		printf("path (%s%s) too long, skipping\n",
				PXELINUX_DIR, file);
		return -ENAMETOOLONG;
	}

	sprintf(path, PXELINUX_DIR "%s", file);

	return get_pxe_file(cmdtp, path, pxefile_addr_r);
}

/*
 * Looks for a pxe file with a name based on the pxeuuid environment variable.
 *
 * Returns 1 on success or < 0 on error.
 */
static int pxe_uuid_path(cmd_tbl_t *cmdtp, unsigned long pxefile_addr_r)
{
	char *uuid_str;

	uuid_str = from_env("pxeuuid");

	if (!uuid_str)
		return -ENOENT;

	return get_pxelinux_path(cmdtp, uuid_str, pxefile_addr_r);
}

/*
 * Looks for a pxe file with a name based on the 'ethaddr' environment
 * variable.
 *
 * Returns 1 on success or < 0 on error.
 */
static int pxe_mac_path(cmd_tbl_t *cmdtp, unsigned long pxefile_addr_r)
{
	char mac_str[21];
	int err;

	err = format_mac_pxe(mac_str, sizeof(mac_str));

	if (err < 0)
		return err;

	return get_pxelinux_path(cmdtp, mac_str, pxefile_addr_r);
}

/*
 * Looks for pxe files with names based on our IP address. See pxelinux
 * documentation for details on what these file names look like.  We match
 * that exactly.
 *
 * Returns 1 on success or < 0 on error.
 */
static int pxe_ipaddr_paths(cmd_tbl_t *cmdtp, unsigned long pxefile_addr_r)
{
	char ip_addr[9];
	int mask_pos, err;

	sprintf(ip_addr, "%08X", ntohl(net_ip.s_addr));

	for (mask_pos = 7; mask_pos >= 0;  mask_pos--) {
		err = get_pxelinux_path(cmdtp, ip_addr, pxefile_addr_r);

		if (err > 0)
			return err;

		ip_addr[mask_pos] = '\0';
	}

	return -ENOENT;
}

/*
 * Entry point for the 'pxe get' command.
 * This Follows pxelinux's rules to download a config file from a tftp server.
 * The file is stored at the location given by the pxefile_addr_r environment
 * variable, which must be set.
 *
 * UUID comes from pxeuuid env variable, if defined
 * MAC addr comes from ethaddr env variable, if defined
 * IP
 *
 * see http://syslinux.zytor.com/wiki/index.php/PXELINUX
 *
 * Returns 0 on success or 1 on error.
 */
static int
do_pxe_get(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *pxefile_addr_str;
	unsigned long pxefile_addr_r;
	int err, i = 0;

	do_getfile = do_get_tftp;

	if (argc != 1)
		return CMD_RET_USAGE;

	pxefile_addr_str = from_env("pxefile_addr_r");

	if (!pxefile_addr_str)
		return 1;

	err = strict_strtoul(pxefile_addr_str, 16,
				(unsigned long *)&pxefile_addr_r);
	if (err < 0)
		return 1;

	/*
	 * Keep trying paths until we successfully get a file we're looking
	 * for.
	 */
	if (pxe_uuid_path(cmdtp, pxefile_addr_r) > 0 ||
	    pxe_mac_path(cmdtp, pxefile_addr_r) > 0 ||
	    pxe_ipaddr_paths(cmdtp, pxefile_addr_r) > 0) {
		printf("Config file found\n");

		return 0;
	}

	while (pxe_default_paths[i]) {
		if (get_pxelinux_path(cmdtp, pxe_default_paths[i],
				      pxefile_addr_r) > 0) {
			printf("Config file found\n");
			return 0;
		}
		i++;
	}

	printf("Config file not found\n");

	return 1;
}
#endif

/*
 * Wrapper to make it easier to store the file at file_path in the location
 * specified by envaddr_name. file_path will be joined to the bootfile path,
 * if any is specified.
 *
 * Returns 1 on success or < 0 on error.
 */
static int get_relfile_envaddr(cmd_tbl_t *cmdtp, const char *file_path, const char *envaddr_name)
{
	unsigned long file_addr;
	char *envaddr;

	envaddr = from_env(envaddr_name);

	if (!envaddr)
		return -ENOENT;

	if (strict_strtoul(envaddr, 16, &file_addr) < 0)
		return -EINVAL;

	return get_relfile(cmdtp, file_path, file_addr);
}

/*
 * A note on the pxe file parser.
 *
 * We're parsing files that use syslinux grammar, which has a few quirks.
 * String literals must be recognized based on context - there is no
 * quoting or escaping support. There's also nothing to explicitly indicate
 * when a label section completes. We deal with that by ending a label
 * section whenever we see a line that doesn't include.
 *
 * As with the syslinux family, this same file format could be reused in the
 * future for non pxe purposes. The only action it takes during parsing that
 * would throw this off is handling of include files. It assumes we're using
 * pxe, and does a tftp download of a file listed as an include file in the
 * middle of the parsing operation. That could be handled by refactoring it to
 * take a 'include file getter' function.
 */

/*
 * Describes a single label given in a pxe file.
 *
 * Create these with the 'label_create' function given below.
 *
 * name - the name of the menu as given on the 'menu label' line.
 * kernel - the path to the kernel file to use for this label.
 * append - kernel command line to use when booting this label
 * initrd - path to the initrd to use for this label.
 * attempted - 0 if we haven't tried to boot this label, 1 if we have.
 * localboot - 1 if this label specified 'localboot', 0 otherwise.
 * list - lets these form a list, which a pxe_menu struct will hold.
 */
struct pxe_label {
	char num[4];
	char *name;
	char *menu;
	char *kernel;
	char *config;
	char *append;
	char *initrd;
	char *fdt;
	char *fdtdir;
	int ipappend;
	int attempted;
	int localboot;
	int localboot_val;
	struct list_head list;
};

/*
 * Describes a pxe menu as given via pxe files.
 *
 * title - the name of the menu as given by a 'menu title' line.
 * default_label - the name of the default label, if any.
 * bmp - the bmp file name which is displayed in background
 * timeout - time in tenths of a second to wait for a user key-press before
 *           booting the default label.
 * prompt - if 0, don't prompt for a choice unless the timeout period is
 *          interrupted.  If 1, always prompt for a choice regardless of
 *          timeout.
 * labels - a list of labels defined for the menu.
 */
struct pxe_menu {
	char *title;
	char *default_label;
	char *bmp;
	int timeout;
	int prompt;
	struct list_head labels;
};

/*
 * Allocates memory for and initializes a pxe_label. This uses malloc, so the
 * result must be free()'d to reclaim the memory.
 *
 * Returns NULL if malloc fails.
 */
static struct pxe_label *label_create(void)
{
	struct pxe_label *label;

	label = malloc(sizeof(struct pxe_label));

	if (!label)
		return NULL;

	memset(label, 0, sizeof(struct pxe_label));

	return label;
}

/*
 * Free the memory used by a pxe_label, including that used by its name,
 * kernel, append and initrd members, if they're non NULL.
 *
 * So - be sure to only use dynamically allocated memory for the members of
 * the pxe_label struct, unless you want to clean it up first. These are
 * currently only created by the pxe file parsing code.
 */
static void label_destroy(struct pxe_label *label)
{
	if (label->name)
		free(label->name);

	if (label->kernel)
		free(label->kernel);

	if (label->config)
		free(label->config);

	if (label->append)
		free(label->append);

	if (label->initrd)
		free(label->initrd);

	if (label->fdt)
		free(label->fdt);

	if (label->fdtdir)
		free(label->fdtdir);

	free(label);
}

/*
 * Print a label and its string members if they're defined.
 *
 * This is passed as a callback to the menu code for displaying each
 * menu entry.
 */
static void label_print(void *data)
{
	struct pxe_label *label = data;
	const char *c = label->menu ? label->menu : label->name;

	printf("%s:\t%s\n", label->num, c);
}

/*
 * Boot a label that specified 'localboot'. This requires that the 'localcmd'
 * environment variable is defined. Its contents will be executed as U-Boot
 * command.  If the label specified an 'append' line, its contents will be
 * used to overwrite the contents of the 'bootargs' environment variable prior
 * to running 'localcmd'.
 *
 * Returns 1 on success or < 0 on error.
 */
static int label_localboot(struct pxe_label *label)
{
	char *localcmd;

	localcmd = from_env("localcmd");

	if (!localcmd)
		return -ENOENT;

	if (label->append) {
		char bootargs[CONFIG_SYS_CBSIZE];

		cli_simple_process_macros(label->append, bootargs);
		env_set("bootargs", bootargs);
	}

	debug("running: %s\n", localcmd);

	return run_command_list(localcmd, strlen(localcmd), 0);
}

/*
 * Boot according to the contents of a pxe_label.
 *
 * If we can't boot for any reason, we return.  A successful boot never
 * returns.
 *
 * The kernel will be stored in the location given by the 'kernel_addr_r'
 * environment variable.
 *
 * If the label specifies an initrd file, it will be stored in the location
 * given by the 'ramdisk_addr_r' environment variable.
 *
 * If the label specifies an 'append' line, its contents will overwrite that
 * of the 'bootargs' environment variable.
 */
static int label_boot(cmd_tbl_t *cmdtp, struct pxe_label *label)
{
	char *bootm_argv[] = { "bootm", NULL, NULL, NULL, NULL };
	char initrd_str[28];
	char mac_str[29] = "";
	char ip_str[68] = "";
	char *fit_addr = NULL;
	int bootm_argc = 2;
	int len = 0;
	ulong kernel_addr;
	void *buf;

	label_print(label);

	label->attempted = 1;

	if (label->localboot) {
		if (label->localboot_val >= 0)
			label_localboot(label);
		return 0;
	}

	if (label->kernel == NULL) {
		printf("No kernel given, skipping %s\n",
				label->name);
		return 1;
	}

	if (label->initrd) {
		if (get_relfile_envaddr(cmdtp, label->initrd, "ramdisk_addr_r") < 0) {
			printf("Skipping %s for failure retrieving initrd\n",
					label->name);
			return 1;
		}

		bootm_argv[2] = initrd_str;
		strncpy(bootm_argv[2], env_get("ramdisk_addr_r"), 18);
		strcat(bootm_argv[2], ":");
		strncat(bootm_argv[2], env_get("filesize"), 9);
	}

	if (get_relfile_envaddr(cmdtp, label->kernel, "kernel_addr_r") < 0) {
		printf("Skipping %s for failure retrieving kernel\n",
				label->name);
		return 1;
	}

	if (label->ipappend & 0x1) {
		sprintf(ip_str, " ip=%s:%s:%s:%s",
			env_get("ipaddr"), env_get("serverip"),
			env_get("gatewayip"), env_get("netmask"));
	}

#ifdef CONFIG_CMD_NET
	if (label->ipappend & 0x2) {
		int err;
		strcpy(mac_str, " BOOTIF=");
		err = format_mac_pxe(mac_str + 8, sizeof(mac_str) - 8);
		if (err < 0)
			mac_str[0] = '\0';
	}
#endif

	if ((label->ipappend & 0x3) || label->append) {
		char bootargs[CONFIG_SYS_CBSIZE] = "";
		char finalbootargs[CONFIG_SYS_CBSIZE];

		if (strlen(label->append ?: "") +
		    strlen(ip_str) + strlen(mac_str) + 1 > sizeof(bootargs)) {
			printf("bootarg overflow %zd+%zd+%zd+1 > %zd\n",
			       strlen(label->append ?: ""),
			       strlen(ip_str), strlen(mac_str),
			       sizeof(bootargs));
			return 1;
		} else {
			if (label->append)
				strncpy(bootargs, label->append,
					sizeof(bootargs));
			strcat(bootargs, ip_str);
			strcat(bootargs, mac_str);

			cli_simple_process_macros(bootargs, finalbootargs);
			env_set("bootargs", finalbootargs);
			printf("append: %s\n", finalbootargs);
		}
	}

	bootm_argv[1] = env_get("kernel_addr_r");
	/* for FIT, append the configuration identifier */
	if (label->config) {
		int len = strlen(bootm_argv[1]) + strlen(label->config) + 1;

		fit_addr = malloc(len);
		if (!fit_addr) {
			printf("malloc fail (FIT address)\n");
			return 1;
		}
		snprintf(fit_addr, len, "%s%s", bootm_argv[1], label->config);
		bootm_argv[1] = fit_addr;
	}

	/*
	 * fdt usage is optional:
	 * It handles the following scenarios. All scenarios are exclusive
	 *
	 * Scenario 1: If fdt_addr_r specified and "fdt" label is defined in
	 * pxe file, retrieve fdt blob from server. Pass fdt_addr_r to bootm,
	 * and adjust argc appropriately.
	 *
	 * Scenario 2: If there is an fdt_addr specified, pass it along to
	 * bootm, and adjust argc appropriately.
	 *
	 * Scenario 3: fdt blob is not available.
	 */
	bootm_argv[3] = env_get("fdt_addr_r");

	/* if fdt label is defined then get fdt from server */
	if (bootm_argv[3]) {
		char *fdtfile = NULL;
		char *fdtfilefree = NULL;

		if (label->fdt) {
			fdtfile = label->fdt;
		} else if (label->fdtdir) {
			char *f1, *f2, *f3, *f4, *slash;

			f1 = env_get("fdtfile");
			if (f1) {
				f2 = "";
				f3 = "";
				f4 = "";
			} else {
				/*
				 * For complex cases where this code doesn't
				 * generate the correct filename, the board
				 * code should set $fdtfile during early boot,
				 * or the boot scripts should set $fdtfile
				 * before invoking "pxe" or "sysboot".
				 */
				f1 = env_get("soc");
				f2 = "-";
				f3 = env_get("board");
				f4 = ".dtb";
			}

			len = strlen(label->fdtdir);
			if (!len)
				slash = "./";
			else if (label->fdtdir[len - 1] != '/')
				slash = "/";
			else
				slash = "";

			len = strlen(label->fdtdir) + strlen(slash) +
				strlen(f1) + strlen(f2) + strlen(f3) +
				strlen(f4) + 1;
			fdtfilefree = malloc(len);
			if (!fdtfilefree) {
				printf("malloc fail (FDT filename)\n");
				goto cleanup;
			}

			snprintf(fdtfilefree, len, "%s%s%s%s%s%s",
				 label->fdtdir, slash, f1, f2, f3, f4);
			fdtfile = fdtfilefree;
		}

		if (fdtfile) {
			int err = get_relfile_envaddr(cmdtp, fdtfile, "fdt_addr_r");
			free(fdtfilefree);
			if (err < 0) {
				printf("Skipping %s for failure retrieving fdt\n",
						label->name);
				goto cleanup;
			}
		} else {
			bootm_argv[3] = NULL;
		}
	}

	if (!bootm_argv[3])
		bootm_argv[3] = env_get("fdt_addr");

	if (bootm_argv[3]) {
		if (!bootm_argv[2])
			bootm_argv[2] = "-";
		bootm_argc = 4;
	}

	kernel_addr = genimg_get_kernel_addr(bootm_argv[1]);
	buf = map_sysmem(kernel_addr, 0);
	/* Try bootm for legacy and FIT format image */
	if (genimg_get_format(buf) != IMAGE_FORMAT_INVALID)
		do_bootm(cmdtp, 0, bootm_argc, bootm_argv);
#ifdef CONFIG_CMD_BOOTI
	/* Try booting an AArch64 Linux kernel image */
	else
		do_booti(cmdtp, 0, bootm_argc, bootm_argv);
#elif defined(CONFIG_CMD_BOOTZ)
	/* Try booting a Image */
	else
		do_bootz(cmdtp, 0, bootm_argc, bootm_argv);
#endif
	unmap_sysmem(buf);

cleanup:
	if (fit_addr)
		free(fit_addr);
	return 1;
}

/*
 * Tokens for the pxe file parser.
 */
enum token_type {
	T_EOL,
	T_STRING,
	T_EOF,
	T_MENU,
	T_TITLE,
	T_TIMEOUT,
	T_LABEL,
	T_KERNEL,
	T_LINUX,
	T_APPEND,
	T_INITRD,
	T_LOCALBOOT,
	T_DEFAULT,
	T_PROMPT,
	T_INCLUDE,
	T_FDT,
	T_FDTDIR,
	T_ONTIMEOUT,
	T_IPAPPEND,
	T_BACKGROUND,
	T_INVALID
};

/*
 * A token - given by a value and a type.
 */
struct token {
	char *val;
	enum token_type type;
};

/*
 * Keywords recognized.
 */
static const struct token keywords[] = {
	{"menu", T_MENU},
	{"title", T_TITLE},
	{"timeout", T_TIMEOUT},
	{"default", T_DEFAULT},
	{"prompt", T_PROMPT},
	{"label", T_LABEL},
	{"kernel", T_KERNEL},
	{"linux", T_LINUX},
	{"localboot", T_LOCALBOOT},
	{"append", T_APPEND},
	{"initrd", T_INITRD},
	{"include", T_INCLUDE},
	{"devicetree", T_FDT},
	{"fdt", T_FDT},
	{"devicetreedir", T_FDTDIR},
	{"fdtdir", T_FDTDIR},
	{"ontimeout", T_ONTIMEOUT,},
	{"ipappend", T_IPAPPEND,},
	{"background", T_BACKGROUND,},
	{NULL, T_INVALID}
};

/*
 * Since pxe(linux) files don't have a token to identify the start of a
 * literal, we have to keep track of when we're in a state where a literal is
 * expected vs when we're in a state a keyword is expected.
 */
enum lex_state {
	L_NORMAL = 0,
	L_KEYWORD,
	L_SLITERAL
};

/*
 * get_string retrieves a string from *p and stores it as a token in
 * *t.
 *
 * get_string used for scanning both string literals and keywords.
 *
 * Characters from *p are copied into t-val until a character equal to
 * delim is found, or a NUL byte is reached. If delim has the special value of
 * ' ', any whitespace character will be used as a delimiter.
 *
 * If lower is unequal to 0, uppercase characters will be converted to
 * lowercase in the result. This is useful to make keywords case
 * insensitive.
 *
 * The location of *p is updated to point to the first character after the end
 * of the token - the ending delimiter.
 *
 * On success, the new value of t->val is returned. Memory for t->val is
 * allocated using malloc and must be free()'d to reclaim it.  If insufficient
 * memory is available, NULL is returned.
 */
static char *get_string(char **p, struct token *t, char delim, int lower)
{
	char *b, *e;
	size_t len, i;

	/*
	 * b and e both start at the beginning of the input stream.
	 *
	 * e is incremented until we find the ending delimiter, or a NUL byte
	 * is reached. Then, we take e - b to find the length of the token.
	 */
	b = e = *p;

	while (*e) {
		if ((delim == ' ' && isspace(*e)) || delim == *e)
			break;
		e++;
	}

	len = e - b;

	/*
	 * Allocate memory to hold the string, and copy it in, converting
	 * characters to lowercase if lower is != 0.
	 */
	t->val = malloc(len + 1);
	if (!t->val)
		return NULL;

	for (i = 0; i < len; i++, b++) {
		if (lower)
			t->val[i] = tolower(*b);
		else
			t->val[i] = *b;
	}

	t->val[len] = '\0';

	/*
	 * Update *p so the caller knows where to continue scanning.
	 */
	*p = e;

	t->type = T_STRING;

	return t->val;
}

/*
 * Populate a keyword token with a type and value.
 */
static void get_keyword(struct token *t)
{
	int i;

	for (i = 0; keywords[i].val; i++) {
		if (!strcmp(t->val, keywords[i].val)) {
			t->type = keywords[i].type;
			break;
		}
	}
}

/*
 * Get the next token.  We have to keep track of which state we're in to know
 * if we're looking to get a string literal or a keyword.
 *
 * *p is updated to point at the first character after the current token.
 */
static void get_token(char **p, struct token *t, enum lex_state state)
{
	char *c = *p;

	t->type = T_INVALID;

	/* eat non EOL whitespace */
	while (isblank(*c))
		c++;

	/*
	 * eat comments. note that string literals can't begin with #, but
	 * can contain a # after their first character.
	 */
	if (*c == '#') {
		while (*c && *c != '\n')
			c++;
	}

	if (*c == '\n') {
		t->type = T_EOL;
		c++;
	} else if (*c == '\0') {
		t->type = T_EOF;
		c++;
	} else if (state == L_SLITERAL) {
		get_string(&c, t, '\n', 0);
	} else if (state == L_KEYWORD) {
		/*
		 * when we expect a keyword, we first get the next string
		 * token delimited by whitespace, and then check if it
		 * matches a keyword in our keyword list. if it does, it's
		 * converted to a keyword token of the appropriate type, and
		 * if not, it remains a string token.
		 */
		get_string(&c, t, ' ', 1);
		get_keyword(t);
	}

	*p = c;
}

/*
 * Increment *c until we get to the end of the current line, or EOF.
 */
static void eol_or_eof(char **c)
{
	while (**c && **c != '\n')
		(*c)++;
}

/*
 * All of these parse_* functions share some common behavior.
 *
 * They finish with *c pointing after the token they parse, and return 1 on
 * success, or < 0 on error.
 */

/*
 * Parse a string literal and store a pointer it at *dst. String literals
 * terminate at the end of the line.
 */
static int parse_sliteral(char **c, char **dst)
{
	struct token t;
	char *s = *c;

	get_token(c, &t, L_SLITERAL);

	if (t.type != T_STRING) {
		printf("Expected string literal: %.*s\n", (int)(*c - s), s);
		return -EINVAL;
	}

	*dst = t.val;

	return 1;
}

/*
 * Parse a base 10 (unsigned) integer and store it at *dst.
 */
static int parse_integer(char **c, int *dst)
{
	struct token t;
	char *s = *c;

	get_token(c, &t, L_SLITERAL);

	if (t.type != T_STRING) {
		printf("Expected string: %.*s\n", (int)(*c - s), s);
		return -EINVAL;
	}

	*dst = simple_strtol(t.val, NULL, 10);

	free(t.val);

	return 1;
}

static int parse_pxefile_top(cmd_tbl_t *cmdtp, char *p, unsigned long base,
	struct pxe_menu *cfg, int nest_level);

/*
 * Parse an include statement, and retrieve and parse the file it mentions.
 *
 * base should point to a location where it's safe to store the file, and
 * nest_level should indicate how many nested includes have occurred. For this
 * include, nest_level has already been incremented and doesn't need to be
 * incremented here.
 */
static int handle_include(cmd_tbl_t *cmdtp, char **c, unsigned long base,
				struct pxe_menu *cfg, int nest_level)
{
	char *include_path;
	char *s = *c;
	int err;
	char *buf;
	int ret;

	err = parse_sliteral(c, &include_path);

	if (err < 0) {
		printf("Expected include path: %.*s\n",
				 (int)(*c - s), s);
		return err;
	}

	err = get_pxe_file(cmdtp, include_path, base);

	if (err < 0) {
		printf("Couldn't retrieve %s\n", include_path);
		return err;
	}

	buf = map_sysmem(base, 0);
	ret = parse_pxefile_top(cmdtp, buf, base, cfg, nest_level);
	unmap_sysmem(buf);

	return ret;
}

/*
 * Parse lines that begin with 'menu'.
 *
 * base and nest are provided to handle the 'menu include' case.
 *
 * base should point to a location where it's safe to store the included file.
 *
 * nest_level should be 1 when parsing the top level pxe file, 2 when parsing
 * a file it includes, 3 when parsing a file included by that file, and so on.
 */
static int parse_menu(cmd_tbl_t *cmdtp, char **c, struct pxe_menu *cfg,
				unsigned long base, int nest_level)
{
	struct token t;
	char *s = *c;
	int err = 0;

	get_token(c, &t, L_KEYWORD);

	switch (t.type) {
	case T_TITLE:
		err = parse_sliteral(c, &cfg->title);

		break;

	case T_INCLUDE:
		err = handle_include(cmdtp, c, base, cfg,
						nest_level + 1);
		break;

	case T_BACKGROUND:
		err = parse_sliteral(c, &cfg->bmp);
		break;

	default:
		printf("Ignoring malformed menu command: %.*s\n",
				(int)(*c - s), s);
	}

	if (err < 0)
		return err;

	eol_or_eof(c);

	return 1;
}

/*
 * Handles parsing a 'menu line' when we're parsing a label.
 */
static int parse_label_menu(char **c, struct pxe_menu *cfg,
				struct pxe_label *label)
{
	struct token t;
	char *s;

	s = *c;

	get_token(c, &t, L_KEYWORD);

	switch (t.type) {
	case T_DEFAULT:
		if (!cfg->default_label)
			cfg->default_label = strdup(label->name);

		if (!cfg->default_label)
			return -ENOMEM;

		break;
	case T_LABEL:
		parse_sliteral(c, &label->menu);
		break;
	default:
		printf("Ignoring malformed menu command: %.*s\n",
				(int)(*c - s), s);
	}

	eol_or_eof(c);

	return 0;
}

/*
 * Handles parsing a 'kernel' label.
 * expecting "filename" or "<fit_filename>#cfg"
 */
static int parse_label_kernel(char **c, struct pxe_label *label)
{
	char *s;
	int err;

	err = parse_sliteral(c, &label->kernel);
	if (err < 0)
		return err;

	s = strstr(label->kernel, "#");
	if (!s)
		return 1;

	label->config = malloc(strlen(s) + 1);
	if (!label->config)
		return -ENOMEM;

	strcpy(label->config, s);
	*s = 0;

	return 1;
}

/*
 * Parses a label and adds it to the list of labels for a menu.
 *
 * A label ends when we either get to the end of a file, or
 * get some input we otherwise don't have a handler defined
 * for.
 *
 */
static int parse_label(char **c, struct pxe_menu *cfg)
{
	struct token t;
	int len;
	char *s = *c;
	struct pxe_label *label;
	int err;

	label = label_create();
	if (!label)
		return -ENOMEM;

	err = parse_sliteral(c, &label->name);
	if (err < 0) {
		printf("Expected label name: %.*s\n", (int)(*c - s), s);
		label_destroy(label);
		return -EINVAL;
	}

	list_add_tail(&label->list, &cfg->labels);

	while (1) {
		s = *c;
		get_token(c, &t, L_KEYWORD);

		err = 0;
		switch (t.type) {
		case T_MENU:
			err = parse_label_menu(c, cfg, label);
			break;

		case T_KERNEL:
		case T_LINUX:
			err = parse_label_kernel(c, label);
			break;

		case T_APPEND:
			err = parse_sliteral(c, &label->append);
			if (label->initrd)
				break;
			s = strstr(label->append, "initrd=");
			if (!s)
				break;
			s += 7;
			len = (int)(strchr(s, ' ') - s);
			label->initrd = malloc(len + 1);
			strncpy(label->initrd, s, len);
			label->initrd[len] = '\0';

			break;

		case T_INITRD:
			if (!label->initrd)
				err = parse_sliteral(c, &label->initrd);
			break;

		case T_FDT:
			if (!label->fdt)
				err = parse_sliteral(c, &label->fdt);
			break;

		case T_FDTDIR:
			if (!label->fdtdir)
				err = parse_sliteral(c, &label->fdtdir);
			break;

		case T_LOCALBOOT:
			label->localboot = 1;
			err = parse_integer(c, &label->localboot_val);
			break;

		case T_IPAPPEND:
			err = parse_integer(c, &label->ipappend);
			break;

		case T_EOL:
			break;

		default:
			/*
			 * put the token back! we don't want it - it's the end
			 * of a label and whatever token this is, it's
			 * something for the menu level context to handle.
			 */
			*c = s;
			return 1;
		}

		if (err < 0)
			return err;
	}
}

/*
 * This 16 comes from the limit pxelinux imposes on nested includes.
 *
 * There is no reason at all we couldn't do more, but some limit helps prevent
 * infinite (until crash occurs) recursion if a file tries to include itself.
 */
#define MAX_NEST_LEVEL 16

/*
 * Entry point for parsing a menu file. nest_level indicates how many times
 * we've nested in includes.  It will be 1 for the top level menu file.
 *
 * Returns 1 on success, < 0 on error.
 */
static int parse_pxefile_top(cmd_tbl_t *cmdtp, char *p, unsigned long base,
				struct pxe_menu *cfg, int nest_level)
{
	struct token t;
	char *s, *b, *label_name;
	int err;

	b = p;

	if (nest_level > MAX_NEST_LEVEL) {
		printf("Maximum nesting (%d) exceeded\n", MAX_NEST_LEVEL);
		return -EMLINK;
	}

	while (1) {
		s = p;

		get_token(&p, &t, L_KEYWORD);

		err = 0;
		switch (t.type) {
		case T_MENU:
			cfg->prompt = 1;
			err = parse_menu(cmdtp, &p, cfg,
				base + ALIGN(strlen(b) + 1, 4),
				nest_level);
			break;

		case T_TIMEOUT:
			err = parse_integer(&p, &cfg->timeout);
			break;

		case T_LABEL:
			err = parse_label(&p, cfg);
			break;

		case T_DEFAULT:
		case T_ONTIMEOUT:
			err = parse_sliteral(&p, &label_name);

			if (label_name) {
				if (cfg->default_label)
					free(cfg->default_label);

				cfg->default_label = label_name;
			}

			break;

		case T_INCLUDE:
			err = handle_include(cmdtp, &p,
				base + ALIGN(strlen(b), 4), cfg,
				nest_level + 1);
			break;

		case T_PROMPT:
			eol_or_eof(&p);
			break;

		case T_EOL:
			break;

		case T_EOF:
			return 1;

		default:
			printf("Ignoring unknown command: %.*s\n",
							(int)(p - s), s);
			eol_or_eof(&p);
		}

		if (err < 0)
			return err;
	}
}

/*
 * Free the memory used by a pxe_menu and its labels.
 */
static void destroy_pxe_menu(struct pxe_menu *cfg)
{
	struct list_head *pos, *n;
	struct pxe_label *label;

	if (cfg->title)
		free(cfg->title);

	if (cfg->default_label)
		free(cfg->default_label);

	list_for_each_safe(pos, n, &cfg->labels) {
		label = list_entry(pos, struct pxe_label, list);

		label_destroy(label);
	}

	free(cfg);
}

/*
 * Entry point for parsing a pxe file. This is only used for the top level
 * file.
 *
 * Returns NULL if there is an error, otherwise, returns a pointer to a
 * pxe_menu struct populated with the results of parsing the pxe file (and any
 * files it includes). The resulting pxe_menu struct can be free()'d by using
 * the destroy_pxe_menu() function.
 */
static struct pxe_menu *parse_pxefile(cmd_tbl_t *cmdtp, unsigned long menucfg)
{
	struct pxe_menu *cfg;
	char *buf;
	int r;

	cfg = malloc(sizeof(struct pxe_menu));

	if (!cfg)
		return NULL;

	memset(cfg, 0, sizeof(struct pxe_menu));

	INIT_LIST_HEAD(&cfg->labels);

	buf = map_sysmem(menucfg, 0);
	r = parse_pxefile_top(cmdtp, buf, menucfg, cfg, 1);
	unmap_sysmem(buf);

	if (r < 0) {
		destroy_pxe_menu(cfg);
		return NULL;
	}

	return cfg;
}

/*
 * Converts a pxe_menu struct into a menu struct for use with U-Boot's generic
 * menu code.
 */
static struct menu *pxe_menu_to_menu(struct pxe_menu *cfg)
{
	struct pxe_label *label;
	struct list_head *pos;
	struct menu *m;
	int err;
	int i = 1;
	char *default_num = NULL;

	/*
	 * Create a menu and add items for all the labels.
	 */
	m = menu_create(cfg->title, DIV_ROUND_UP(cfg->timeout, 10),
			cfg->prompt, label_print, NULL, NULL);

	if (!m)
		return NULL;

	list_for_each(pos, &cfg->labels) {
		label = list_entry(pos, struct pxe_label, list);

		sprintf(label->num, "%d", i++);
		if (menu_item_add(m, label->num, label) != 1) {
			menu_destroy(m);
			return NULL;
		}
		if (cfg->default_label &&
		    (strcmp(label->name, cfg->default_label) == 0))
			default_num = label->num;

	}

	/*
	 * After we've created items for each label in the menu, set the
	 * menu's default label if one was specified.
	 */
	if (default_num) {
		err = menu_default_set(m, default_num);
		if (err != 1) {
			if (err != -ENOENT) {
				menu_destroy(m);
				return NULL;
			}

			printf("Missing default: %s\n", cfg->default_label);
		}
	}

	return m;
}

/*
 * Try to boot any labels we have yet to attempt to boot.
 */
static void boot_unattempted_labels(cmd_tbl_t *cmdtp, struct pxe_menu *cfg)
{
	struct list_head *pos;
	struct pxe_label *label;

	list_for_each(pos, &cfg->labels) {
		label = list_entry(pos, struct pxe_label, list);

		if (!label->attempted)
			label_boot(cmdtp, label);
	}
}

/*
 * Boot the system as prescribed by a pxe_menu.
 *
 * Use the menu system to either get the user's choice or the default, based
 * on config or user input.  If there is no default or user's choice,
 * attempted to boot labels in the order they were given in pxe files.
 * If the default or user's choice fails to boot, attempt to boot other
 * labels in the order they were given in pxe files.
 *
 * If this function returns, there weren't any labels that successfully
 * booted, or the user interrupted the menu selection via ctrl+c.
 */
static void handle_pxe_menu(cmd_tbl_t *cmdtp, struct pxe_menu *cfg)
{
	void *choice;
	struct menu *m;
	int err;

#ifdef CONFIG_CMD_BMP
	/* display BMP if available */
	if (cfg->bmp) {
		if (get_relfile(cmdtp, cfg->bmp, load_addr)) {
			run_command("cls", 0);
			bmp_display(load_addr,
				    BMP_ALIGN_CENTER, BMP_ALIGN_CENTER);
		} else {
			printf("Skipping background bmp %s for failure\n",
			       cfg->bmp);
		}
	}
#endif

	m = pxe_menu_to_menu(cfg);
	if (!m)
		return;

	err = menu_get_choice(m, &choice);

	menu_destroy(m);

	/*
	 * err == 1 means we got a choice back from menu_get_choice.
	 *
	 * err == -ENOENT if the menu was setup to select the default but no
	 * default was set. in that case, we should continue trying to boot
	 * labels that haven't been attempted yet.
	 *
	 * otherwise, the user interrupted or there was some other error and
	 * we give up.
	 */

	if (err == 1) {
		err = label_boot(cmdtp, choice);
		if (!err)
			return;
	} else if (err != -ENOENT) {
		return;
	}

	boot_unattempted_labels(cmdtp, cfg);
}

#ifdef CONFIG_CMD_NET
/*
 * Boots a system using a pxe file
 *
 * Returns 0 on success, 1 on error.
 */
static int
do_pxe_boot(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned long pxefile_addr_r;
	struct pxe_menu *cfg;
	char *pxefile_addr_str;

	do_getfile = do_get_tftp;

	if (argc == 1) {
		pxefile_addr_str = from_env("pxefile_addr_r");
		if (!pxefile_addr_str)
			return 1;

	} else if (argc == 2) {
		pxefile_addr_str = argv[1];
	} else {
		return CMD_RET_USAGE;
	}

	if (strict_strtoul(pxefile_addr_str, 16, &pxefile_addr_r) < 0) {
		printf("Invalid pxefile address: %s\n", pxefile_addr_str);
		return 1;
	}

	cfg = parse_pxefile(cmdtp, pxefile_addr_r);

	if (cfg == NULL) {
		printf("Error parsing config file\n");
		return 1;
	}

	handle_pxe_menu(cmdtp, cfg);

	destroy_pxe_menu(cfg);

	copy_filename(net_boot_file_name, "", sizeof(net_boot_file_name));

	return 0;
}

static cmd_tbl_t cmd_pxe_sub[] = {
	U_BOOT_CMD_MKENT(get, 1, 1, do_pxe_get, "", ""),
	U_BOOT_CMD_MKENT(boot, 2, 1, do_pxe_boot, "", "")
};

static int do_pxe(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	cmd_tbl_t *cp;

	if (argc < 2)
		return CMD_RET_USAGE;

	is_pxe = true;

	/* drop initial "pxe" arg */
	argc--;
	argv++;

	cp = find_cmd_tbl(argv[0], cmd_pxe_sub, ARRAY_SIZE(cmd_pxe_sub));

	if (cp)
		return cp->cmd(cmdtp, flag, argc, argv);

	return CMD_RET_USAGE;
}

U_BOOT_CMD(
	pxe, 3, 1, do_pxe,
	"commands to get and boot from pxe files",
	"get - try to retrieve a pxe file using tftp\npxe "
	"boot [pxefile_addr_r] - boot from the pxe file at pxefile_addr_r\n"
);
#endif

/*
 * Boots a system using a local disk syslinux/extlinux file
 *
 * Returns 0 on success, 1 on error.
 */
static int do_sysboot(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned long pxefile_addr_r;
	struct pxe_menu *cfg;
	char *pxefile_addr_str;
	char *filename;
	int prompt = 0;

	is_pxe = false;

	if (argc > 1 && strstr(argv[1], "-p")) {
		prompt = 1;
		argc--;
		argv++;
	}

	if (argc < 4)
		return cmd_usage(cmdtp);

	if (argc < 5) {
		pxefile_addr_str = from_env("pxefile_addr_r");
		if (!pxefile_addr_str)
			return 1;
	} else {
		pxefile_addr_str = argv[4];
	}

	if (argc < 6)
		filename = env_get("bootfile");
	else {
		filename = argv[5];
		env_set("bootfile", filename);
	}

	if (strstr(argv[3], "ext2"))
		do_getfile = do_get_ext2;
	else if (strstr(argv[3], "fat"))
		do_getfile = do_get_fat;
	else if (strstr(argv[3], "any"))
		do_getfile = do_get_any;
	else {
		printf("Invalid filesystem: %s\n", argv[3]);
		return 1;
	}
	fs_argv[1] = argv[1];
	fs_argv[2] = argv[2];

	if (strict_strtoul(pxefile_addr_str, 16, &pxefile_addr_r) < 0) {
		printf("Invalid pxefile address: %s\n", pxefile_addr_str);
		return 1;
	}

	if (get_pxe_file(cmdtp, filename, pxefile_addr_r) < 0) {
		printf("Error reading config file\n");
		return 1;
	}

	cfg = parse_pxefile(cmdtp, pxefile_addr_r);

	if (cfg == NULL) {
		printf("Error parsing config file\n");
		return 1;
	}

	if (prompt)
		cfg->prompt = 1;

	handle_pxe_menu(cmdtp, cfg);

	destroy_pxe_menu(cfg);

	return 0;
}

U_BOOT_CMD(
	sysboot, 7, 1, do_sysboot,
	"command to get and boot from syslinux files",
	"[-p] <interface> <dev[:part]> <ext2|fat|any> [addr] [filename]\n"
	"    - load and parse syslinux menu file 'filename' from ext2, fat\n"
	"      or any filesystem on 'dev' on 'interface' to address 'addr'"
);
