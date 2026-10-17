// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2011 Calxeda, Inc.
 * Copyright (c) 2014, NVIDIA CORPORATION.  All rights reserved.
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <env.h>
#include <image.h>
#include <log.h>
#include <malloc.h>
#include <mapmem.h>
#include <net.h>
#include <fdt_support.h>
#include <video.h>
#include <linux/libfdt.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <errno.h>
#include <linux/list.h>

#ifdef CONFIG_DM_RNG
#include <rng.h>
#endif

#include <splash.h>
#include <asm/io.h>

#include "menu.h"
#include "cli.h"

#include "pxe_utils.h"

#define MAX_TFTP_PATH_LEN 512

int pxe_get_file_size(ulong *sizep)
{
	const char *val;

	val = from_env("filesize");
	if (!val)
		return -ENOENT;

	if (strict_strtoul(val, 16, sizep) < 0)
		return -EINVAL;

	return 0;
}

/**
 * format_mac_pxe() - obtain a MAC address in the PXE format
 *
 * This produces a MAC-address string in the format for the current ethernet
 * device:
 *
 *   01-aa-bb-cc-dd-ee-ff
 *
 * where aa-ff is the MAC address in hex
 *
 * @outbuf: Buffer to write string to
 * @outbuf_len: length of buffer
 * Return: 1 if OK, -ENOSPC if buffer is too small, -ENOENT is there is no
 *	current ethernet device
 */
int format_mac_pxe(char *outbuf, size_t outbuf_len)
{
	uchar ethaddr[6];

	if (outbuf_len < 21) {
		printf("outbuf is too small (%zd < 21)\n", outbuf_len);
		return -ENOSPC;
	}

	if (!eth_env_get_enetaddr_by_index("eth", eth_get_dev_index(), ethaddr))
		return -ENOENT;

	sprintf(outbuf, "01-%02x-%02x-%02x-%02x-%02x-%02x",
		ethaddr[0], ethaddr[1], ethaddr[2],
		ethaddr[3], ethaddr[4], ethaddr[5]);

	return 1;
}

/**
 * get_relfile() - read a file relative to the PXE file
 *
 * As in pxelinux, paths to files referenced from files we retrieve are
 * relative to the location of bootfile. get_relfile takes such a path and
 * joins it with the bootfile path to get the full path to the target file. If
 * the bootfile path is NULL, we use file_path as is.
 *
 * @ctx: PXE context
 * @file_path: File path to read (relative to the PXE file)
 * @file_addr: Address to load file to
 * @filesizep: If not NULL, returns the file size in bytes
 * Returns 1 for success, or < 0 on error
 */
static int get_relfile(struct pxe_context *ctx, const char *file_path,
		       unsigned long file_addr, ulong *filesizep)
{
	size_t path_len;
	char relfile[MAX_TFTP_PATH_LEN + 1];
	char addr_buf[18];
	ulong size;
	int ret;

	if (file_path[0] == '/' && ctx->allow_abs_path)
		*relfile = '\0';
	else
		strncpy(relfile, ctx->bootdir, MAX_TFTP_PATH_LEN);

	path_len = strlen(file_path) + strlen(relfile);

	if (path_len > MAX_TFTP_PATH_LEN) {
		printf("Base path too long (%s%s)\n", relfile, file_path);

		return -ENAMETOOLONG;
	}

	strcat(relfile, file_path);

	printf("Retrieving file: %s\n", relfile);

	sprintf(addr_buf, "%lx", file_addr);

	ret = ctx->getfile(ctx, relfile, addr_buf, &size);
	if (ret < 0)
		return log_msg_ret("get", ret);
	if (filesizep)
		*filesizep = size;

	return 1;
}

/**
 * get_pxe_file() - read a file
 *
 * The file is read and nul-terminated
 *
 * @ctx: PXE context
 * @file_path: File path to read (relative to the PXE file)
 * @file_addr: Address to load file to
 * Returns 1 for success, or < 0 on error
 */
int get_pxe_file(struct pxe_context *ctx, const char *file_path,
		 ulong file_addr)
{
	ulong size;
	int err;
	char *buf;

	err = get_relfile(ctx, file_path, file_addr, &size);
	if (err < 0)
		return err;

	buf = map_sysmem(file_addr + size, 1);
	*buf = '\0';
	unmap_sysmem(buf);

	return 1;
}

#define PXELINUX_DIR "pxelinux.cfg/"

/**
 * get_pxelinux_path() - Get a file in the pxelinux.cfg/ directory
 *
 * @ctx: PXE context
 * @file: Filename to process (relative to pxelinux.cfg/)
 * Returns 1 for success, -ENAMETOOLONG if the resulting path is too long.
 *	or other value < 0 on other error
 */
int get_pxelinux_path(struct pxe_context *ctx, const char *file,
		      unsigned long pxefile_addr_r)
{
	size_t base_len = strlen(PXELINUX_DIR);
	char path[MAX_TFTP_PATH_LEN + 1];

	if (base_len + strlen(file) > MAX_TFTP_PATH_LEN) {
		printf("path (%s%s) too long, skipping\n",
		       PXELINUX_DIR, file);
		return -ENAMETOOLONG;
	}

	sprintf(path, PXELINUX_DIR "%s", file);

	return get_pxe_file(ctx, path, pxefile_addr_r);
}

/**
 * get_relfile_envaddr() - read a file to an address in an env var
 *
 * Wrapper to make it easier to store the file at file_path in the location
 * specified by envaddr_name. file_path will be joined to the bootfile path,
 * if any is specified.
 *
 * @ctx: PXE context
 * @file_path: File path to read (relative to the PXE file)
 * @envaddr_name: Name of environment variable which contains the address to
 *	load to
 * @filesizep: Returns the file size in bytes
 * Returns 1 on success, -ENOENT if @envaddr_name does not exist as an
 *	environment variable, -EINVAL if its format is not valid hex, or other
 *	value < 0 on other error
 */
static int get_relfile_envaddr(struct pxe_context *ctx, const char *file_path,
			       const char *envaddr_name, ulong *filesizep)
{
	unsigned long file_addr;
	char *envaddr;

	envaddr = from_env(envaddr_name);
	if (!envaddr)
		return -ENOENT;

	if (strict_strtoul(envaddr, 16, &file_addr) < 0)
		return -EINVAL;

	return get_relfile(ctx, file_path, file_addr, filesizep);
}

/**
 * label_create() - crate a new PXE label
 *
 * Allocates memory for and initializes a pxe_label. This uses malloc, so the
 * result must be free()'d to reclaim the memory.
 *
 * Returns a pointer to the label, or NULL if out of memory
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

/**
 * label_destroy() - free the memory used by a pxe_label
 *
 * This frees @label itself as well as memory used by its name,
 * kernel, config, append, initrd, fdt, fdtdir and fdtoverlay members, if
 * they're non-NULL.
 *
 * So - be sure to only use dynamically allocated memory for the members of
 * the pxe_label struct, unless you want to clean it up first. These are
 * currently only created by the pxe file parsing code.
 *
 * @label: Label to free
 */
static void label_destroy(struct pxe_label *label)
{
	free(label->name);
	free(label->kernel_label);
	free(label->kernel);
	free(label->config);
	free(label->append);
	free(label->initrd);
	free(label->fdt);
	free(label->fdtdir);
	free(label->fdtoverlays);
	free(label);
}

/**
 * label_print() - Print a label and its string members if they're defined
 *
 * This is passed as a callback to the menu code for displaying each
 * menu entry.
 *
 * @data: Label to print (is cast to struct pxe_label *)
 */
static void label_print(void *data)
{
	struct pxe_label *label = data;
	const char *c = label->menu ? label->menu : label->name;

	printf("%s:\t%s\n", label->num, c);
}

/**
 * label_localboot() - Boot a label that specified 'localboot'
 *
 * This requires that the 'localcmd' environment variable is defined. Its
 * contents will be executed as U-Boot commands.  If the label specified an
 * 'append' line, its contents will be used to overwrite the contents of the
 * 'bootargs' environment variable prior to running 'localcmd'.
 *
 * @label: Label to process
 * Returns 1 on success or < 0 on error
 */
static int label_localboot(struct pxe_label *label)
{
	char *localcmd;

	localcmd = from_env("localcmd");
	if (!localcmd)
		return -ENOENT;

	if (label->append) {
		char bootargs[CONFIG_SYS_CBSIZE];

		cli_simple_process_macros(label->append, bootargs,
					  sizeof(bootargs));
		env_set("bootargs", bootargs);
	}

	debug("running: %s\n", localcmd);

	return run_command_list(localcmd, strlen(localcmd), 0);
}

/*
 * label_boot_kaslrseed generate kaslrseed from hw rng
 */

static void label_boot_kaslrseed(void)
{
#ifdef CONFIG_DM_RNG
	ulong fdt_addr;
	struct fdt_header *working_fdt;
	size_t n = 0x8;
	struct udevice *dev;
	u64 *buf;
	int nodeoffset;
	int err;

	/* Get the main fdt and map it */
	fdt_addr = hextoul(env_get("fdt_addr_r"), NULL);
	working_fdt = map_sysmem(fdt_addr, 0);
	err = fdt_check_header(working_fdt);
	if (err)
		return;

	/* add extra size for holding kaslr-seed */
	/* err is new fdt size, 0 or negtive */
	err = fdt_shrink_to_minimum(working_fdt, 512);
	if (err <= 0)
		return;

	if (uclass_get_device(UCLASS_RNG, 0, &dev) || !dev) {
		printf("No RNG device\n");
		return;
	}

	nodeoffset = fdt_find_or_add_subnode(working_fdt, 0, "chosen");
	if (nodeoffset < 0) {
		printf("Reading chosen node failed\n");
		return;
	}

	buf = malloc(n);
	if (!buf) {
		printf("Out of memory\n");
		return;
	}

	if (dm_rng_read(dev, buf, n)) {
		printf("Reading RNG failed\n");
		goto err;
	}

	err = fdt_setprop(working_fdt, nodeoffset, "kaslr-seed", buf, sizeof(buf));
	if (err < 0) {
		printf("Unable to set kaslr-seed on chosen node: %s\n", fdt_strerror(err));
		goto err;
	}
err:
	free(buf);
#endif
	return;
}

/**
 * label_boot_fdtoverlay() - Loads fdt overlays specified in 'fdtoverlays'
 * or 'devicetree-overlay'
 *
 * @ctx: PXE context
 * @label: Label to process
 */
#ifdef CONFIG_OF_LIBFDT_OVERLAY
static void label_boot_fdtoverlay(struct pxe_context *ctx,
				  struct pxe_label *label)
{
	char *fdtoverlay = label->fdtoverlays;
	struct fdt_header *working_fdt;
	char *fdtoverlay_addr_env;
	ulong fdtoverlay_addr;
	ulong fdt_addr;
	int err;

	/* Get the main fdt and map it */
	fdt_addr = hextoul(env_get("fdt_addr_r"), NULL);
	working_fdt = map_sysmem(fdt_addr, 0);
	err = fdt_check_header(working_fdt);
	if (err)
		return;

	/* Get the specific overlay loading address */
	fdtoverlay_addr_env = env_get("fdtoverlay_addr_r");
	if (!fdtoverlay_addr_env) {
		printf("Invalid fdtoverlay_addr_r for loading overlays\n");
		return;
	}

	fdtoverlay_addr = hextoul(fdtoverlay_addr_env, NULL);

	/* Cycle over the overlay files and apply them in order */
	do {
		struct fdt_header *blob;
		char *overlayfile;
		char *end;
		int len;

		/* Drop leading spaces */
		while (*fdtoverlay == ' ')
			++fdtoverlay;

		/* Copy a single filename if multiple provided */
		end = strstr(fdtoverlay, " ");
		if (end) {
			len = (int)(end - fdtoverlay);
			overlayfile = malloc(len + 1);
			strncpy(overlayfile, fdtoverlay, len);
			overlayfile[len] = '\0';
		} else
			overlayfile = fdtoverlay;

		if (!strlen(overlayfile))
			goto skip_overlay;

		/* Load overlay file */
		err = get_relfile_envaddr(ctx, overlayfile, "fdtoverlay_addr_r",
					  NULL);
		if (err < 0) {
			printf("Failed loading overlay %s\n", overlayfile);
			goto skip_overlay;
		}

		/* Resize main fdt */
		fdt_shrink_to_minimum(working_fdt, 8192);

		blob = map_sysmem(fdtoverlay_addr, 0);
		err = fdt_check_header(blob);
		if (err) {
			printf("Invalid overlay %s, skipping\n",
			       overlayfile);
			goto skip_overlay;
		}

		err = fdt_overlay_apply_verbose(working_fdt, blob);
		if (err) {
			printf("Failed to apply overlay %s, skipping\n",
			       overlayfile);
			goto skip_overlay;
		}

skip_overlay:
		if (end)
			free(overlayfile);
	} while ((fdtoverlay = strstr(fdtoverlay, " ")));
}
#endif

/**
 * label_boot() - Boot according to the contents of a pxe_label
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
 *
 * @ctx: PXE context
 * @label: Label to process
 * Returns does not return on success, otherwise returns 0 if a localboot
 *	label was processed, or 1 on error
 */
static int label_boot(struct pxe_context *ctx, struct pxe_label *label)
{
	char *bootm_argv[] = { "bootm", NULL, NULL, NULL, NULL };
	char *zboot_argv[] = { "zboot", NULL, "0", NULL, NULL };
	char *kernel_addr = NULL;
	char *initrd_addr_str = NULL;
	char initrd_filesize[10];
	char initrd_str[28];
	char mac_str[29] = "";
	char ip_str[68] = "";
	char *fit_addr = NULL;
	int bootm_argc = 2;
	int zboot_argc = 3;
	int len = 0;
	ulong kernel_addr_r;
	void *buf;

	label_print(label);

	label->attempted = 1;

	if (label->localboot) {
		if (label->localboot_val >= 0)
			label_localboot(label);
		return 0;
	}

	if (!label->kernel) {
		printf("No kernel given, skipping %s\n",
		       label->name);
		return 1;
	}

	if (get_relfile_envaddr(ctx, label->kernel, "kernel_addr_r",
				NULL) < 0) {
		printf("Skipping %s for failure retrieving kernel\n",
		       label->name);
		return 1;
	}

	kernel_addr = env_get("kernel_addr_r");
	/* for FIT, append the configuration identifier */
	if (label->config) {
		int len = strlen(kernel_addr) + strlen(label->config) + 1;

		fit_addr = malloc(len);
		if (!fit_addr) {
			printf("malloc fail (FIT address)\n");
			return 1;
		}
		snprintf(fit_addr, len, "%s%s", kernel_addr, label->config);
		kernel_addr = fit_addr;
	}

	/* For FIT, the label can be identical to kernel one */
	if (label->initrd && !strcmp(label->kernel_label, label->initrd)) {
		initrd_addr_str =  kernel_addr;
	} else if (label->initrd) {
		ulong size;
		if (get_relfile_envaddr(ctx, label->initrd, "ramdisk_addr_r",
					&size) < 0) {
			printf("Skipping %s for failure retrieving initrd\n",
			       label->name);
			goto cleanup;
		}
		strcpy(initrd_filesize, simple_xtoa(size));
		initrd_addr_str = env_get("ramdisk_addr_r");
		size = snprintf(initrd_str, sizeof(initrd_str), "%s:%lx",
				initrd_addr_str, size);
		if (size >= sizeof(initrd_str))
			goto cleanup;
	}

	if (label->ipappend & 0x1) {
		sprintf(ip_str, " ip=%s:%s:%s:%s",
			env_get("ipaddr"), env_get("serverip"),
			env_get("gatewayip"), env_get("netmask"));
	}

	if (IS_ENABLED(CONFIG_CMD_NET))	{
		if (label->ipappend & 0x2) {
			int err;

			strcpy(mac_str, " BOOTIF=");
			err = format_mac_pxe(mac_str + 8, sizeof(mac_str) - 8);
			if (err < 0)
				mac_str[0] = '\0';
		}
	}

	if ((label->ipappend & 0x3) || label->append) {
		char bootargs[CONFIG_SYS_CBSIZE] = "";
		char finalbootargs[CONFIG_SYS_CBSIZE];

		if (strlen(label->append ?: "") +
		    strlen(ip_str) + strlen(mac_str) + 1 > sizeof(bootargs)) {
			printf("bootarg overflow %zd+%zd+%zd+1 > %zd\n",
			       strlen(label->append ?: ""),
			       strlen(ip_str), strlen(mac_str),
			       sizeof(bootargs));
			goto cleanup;
		}

		if (label->append)
			strncpy(bootargs, label->append, sizeof(bootargs));

		strcat(bootargs, ip_str);
		strcat(bootargs, mac_str);

		cli_simple_process_macros(bootargs, finalbootargs,
					  sizeof(finalbootargs));
		env_set("bootargs", finalbootargs);
		printf("append: %s\n", finalbootargs);
	}

	/*
	 * fdt usage is optional:
	 * It handles the following scenarios.
	 *
	 * Scenario 1: If fdt_addr_r specified and "fdt" or "fdtdir" label is
	 * defined in pxe file, retrieve fdt blob from server. Pass fdt_addr_r to
	 * bootm, and adjust argc appropriately.
	 *
	 * If retrieve fails and no exact fdt blob is specified in pxe file with
	 * "fdt" label, try Scenario 2.
	 *
	 * Scenario 2: If there is an fdt_addr specified, pass it along to
	 * bootm, and adjust argc appropriately.
	 *
	 * Scenario 3: If there is an fdtcontroladdr specified, pass it along to
	 * bootm, and adjust argc appropriately, unless the image type is fitImage.
	 *
	 * Scenario 4: fdt blob is not available.
	 */
	bootm_argv[3] = env_get("fdt_addr_r");

	/* For FIT, the label can be identical to kernel one */
	if (label->fdt && !strcmp(label->kernel_label, label->fdt)) {
		bootm_argv[3] = kernel_addr;
	/* if fdt label is defined then get fdt from server */
	} else if (bootm_argv[3]) {
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
				if (!f1) {
					f1 = "";
					f2 = "";
				}
				if (!f3) {
					f2 = "";
					f3 = "";
				}
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
			int err = get_relfile_envaddr(ctx, fdtfile,
						      "fdt_addr_r", NULL);

			free(fdtfilefree);
			if (err < 0) {
				bootm_argv[3] = NULL;

				if (label->fdt) {
					printf("Skipping %s for failure retrieving FDT\n",
					       label->name);
					goto cleanup;
				}

				if (label->fdtdir) {
					printf("Skipping fdtdir %s for failure retrieving dts\n",
						label->fdtdir);
				}
			}

			if (label->kaslrseed)
				label_boot_kaslrseed();

#ifdef CONFIG_OF_LIBFDT_OVERLAY
			if (label->fdtoverlays)
				label_boot_fdtoverlay(ctx, label);
#endif
		} else {
			bootm_argv[3] = NULL;
		}
	}

	bootm_argv[1] = kernel_addr;
	zboot_argv[1] = kernel_addr;

	if (initrd_addr_str) {
		bootm_argv[2] = initrd_str;
		bootm_argc = 3;

		zboot_argv[3] = initrd_addr_str;
		zboot_argv[4] = initrd_filesize;
		zboot_argc = 5;
	}

	if (!bootm_argv[3])
		bootm_argv[3] = env_get("fdt_addr");

	kernel_addr_r = genimg_get_kernel_addr(kernel_addr);
	buf = map_sysmem(kernel_addr_r, 0);

	if (!bootm_argv[3] && genimg_get_format(buf) != IMAGE_FORMAT_FIT)
		bootm_argv[3] = env_get("fdtcontroladdr");

	if (bootm_argv[3]) {
		if (!bootm_argv[2])
			bootm_argv[2] = "-";
		bootm_argc = 4;
	}

	/* Try bootm for legacy and FIT format image */
	if (genimg_get_format(buf) != IMAGE_FORMAT_INVALID &&
            IS_ENABLED(CONFIG_CMD_BOOTM))
		do_bootm(ctx->cmdtp, 0, bootm_argc, bootm_argv);
	/* Try booting an AArch64 Linux kernel image */
	else if (IS_ENABLED(CONFIG_CMD_BOOTI))
		do_booti(ctx->cmdtp, 0, bootm_argc, bootm_argv);
	/* Try booting a Image */
	else if (IS_ENABLED(CONFIG_CMD_BOOTZ))
		do_bootz(ctx->cmdtp, 0, bootm_argc, bootm_argv);
	/* Try booting an x86_64 Linux kernel image */
	else if (IS_ENABLED(CONFIG_CMD_ZBOOT))
		do_zboot_parent(ctx->cmdtp, 0, zboot_argc, zboot_argv, NULL);

	unmap_sysmem(buf);

cleanup:
	free(fit_addr);

	return 1;
}

/** enum token_type - Tokens for the pxe file parser */
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
	T_FDTOVERLAYS,
	T_ONTIMEOUT,
	T_IPAPPEND,
	T_BACKGROUND,
	T_KASLRSEED,
	T_INVALID
};

/** struct token - token - given by a value and a type */
struct token {
	char *val;
	enum token_type type;
};

/* Keywords recognized */
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
	{"fdtoverlays", T_FDTOVERLAYS},
	{"devicetree-overlay", T_FDTOVERLAYS},
	{"ontimeout", T_ONTIMEOUT,},
	{"ipappend", T_IPAPPEND,},
	{"background", T_BACKGROUND,},
	{"kaslrseed", T_KASLRSEED,},
	{NULL, T_INVALID}
};

/**
 * enum lex_state - lexer state
 *
 * Since pxe(linux) files don't have a token to identify the start of a
 * literal, we have to keep track of when we're in a state where a literal is
 * expected vs when we're in a state a keyword is expected.
 */
enum lex_state {
	L_NORMAL = 0,
	L_KEYWORD,
	L_SLITERAL
};

/**
 * get_string() - retrieves a string from *p and stores it as a token in *t.
 *
 * This is used for scanning both string literals and keywords.
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
 * Memory for t->val is allocated using malloc and must be free()'d to reclaim
 * it.
 *
 * @p: Points to a pointer to the current position in the input being processed.
 *	Updated to point at the first character after the current token
 * @t: Pointers to a token to fill in
 * @delim: Delimiter character to look for, either newline or space
 * @lower: true to convert the string to lower case when storing
 * Returns the new value of t->val, on success, NULL if out of memory
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
	b = *p;
	e = *p;
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

	/* Update *p so the caller knows where to continue scanning */
	*p = e;
	t->type = T_STRING;

	return t->val;
}

/**
 * get_keyword() - Populate a keyword token with a type and value
 *
 * Updates the ->type field based on the keyword string in @val
 * @t: Token to populate
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

/**
 * get_token() - Get the next token
 *
 * We have to keep track of which state we're in to know if we're looking to get
 * a string literal or a keyword.
 *
 * @p: Points to a pointer to the current position in the input being processed.
 *	Updated to point at the first character after the current token
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

/**
 * eol_or_eof() - Find end of line
 *
 * Increment *c until we get to the end of the current line, or EOF
 *
 * @c: Points to a pointer to the current position in the input being processed.
 *	Updated to point at the first character after the current token
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

static int parse_pxefile_top(struct pxe_context *ctx, char *p, ulong base,
			     struct pxe_menu *cfg, int nest_level);

/*
 * Parse an include statement, and retrieve and parse the file it mentions.
 *
 * base should point to a location where it's safe to store the file, and
 * nest_level should indicate how many nested includes have occurred. For this
 * include, nest_level has already been incremented and doesn't need to be
 * incremented here.
 */
static int handle_include(struct pxe_context *ctx, char **c, unsigned long base,
			  struct pxe_menu *cfg, int nest_level)
{
	char *include_path;
	char *s = *c;
	int err;
	char *buf;
	int ret;

	err = parse_sliteral(c, &include_path);
	if (err < 0) {
		printf("Expected include path: %.*s\n", (int)(*c - s), s);
		return err;
	}

	err = get_pxe_file(ctx, include_path, base);
	if (err < 0) {
		printf("Couldn't retrieve %s\n", include_path);
		return err;
	}

	buf = map_sysmem(base, 0);
	ret = parse_pxefile_top(ctx, buf, base, cfg, nest_level);
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
static int parse_menu(struct pxe_context *ctx, char **c, struct pxe_menu *cfg,
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
		err = handle_include(ctx, c, base, cfg, nest_level + 1);
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

	/* copy the kernel label to compare with FDT / INITRD when FIT is used */
	label->kernel_label = strdup(label->kernel);
	if (!label->kernel_label)
		return -ENOMEM;

	s = strstr(label->kernel, "#");
	if (!s)
		return 1;

	label->config = strdup(s);
	if (!label->config)
		return -ENOMEM;

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

		case T_FDTOVERLAYS:
			if (!label->fdtoverlays)
				err = parse_sliteral(c, &label->fdtoverlays);
			break;

		case T_LOCALBOOT:
			label->localboot = 1;
			err = parse_integer(c, &label->localboot_val);
			break;

		case T_IPAPPEND:
			err = parse_integer(c, &label->ipappend);
			break;

		case T_KASLRSEED:
			label->kaslrseed = 1;
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
static int parse_pxefile_top(struct pxe_context *ctx, char *p, unsigned long base,
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
			err = parse_menu(ctx, &p, cfg,
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
			err = handle_include(ctx, &p,
					     base + ALIGN(strlen(b), 4), cfg,
					     nest_level + 1);
			break;

		case T_PROMPT:
			err = parse_integer(&p, &cfg->prompt);
			// Do not fail if prompt configuration is undefined
			if (err <  0)
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
 */
void destroy_pxe_menu(struct pxe_menu *cfg)
{
	struct list_head *pos, *n;
	struct pxe_label *label;

	free(cfg->title);
	free(cfg->default_label);

	list_for_each_safe(pos, n, &cfg->labels) {
		label = list_entry(pos, struct pxe_label, list);

		label_destroy(label);
	}

	free(cfg);
}

struct pxe_menu *parse_pxefile(struct pxe_context *ctx, unsigned long menucfg)
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
	r = parse_pxefile_top(ctx, buf, menucfg, cfg, 1);
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
	char *label_override;
	int err;
	int i = 1;
	char *default_num = NULL;
	char *override_num = NULL;

	/*
	 * Create a menu and add items for all the labels.
	 */
	m = menu_create(cfg->title, DIV_ROUND_UP(cfg->timeout, 10),
			cfg->prompt, NULL, label_print, NULL, NULL);
	if (!m)
		return NULL;

	label_override = env_get("pxe_label_override");

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
		if (label_override && !strcmp(label->name, label_override))
			override_num = label->num;
	}


	if (label_override) {
		if (override_num)
			default_num = override_num;
		else
			printf("Missing override pxe label: %s\n",
			      label_override);
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
static void boot_unattempted_labels(struct pxe_context *ctx,
				    struct pxe_menu *cfg)
{
	struct list_head *pos;
	struct pxe_label *label;

	list_for_each(pos, &cfg->labels) {
		label = list_entry(pos, struct pxe_label, list);

		if (!label->attempted)
			label_boot(ctx, label);
	}
}

void handle_pxe_menu(struct pxe_context *ctx, struct pxe_menu *cfg)
{
	void *choice;
	struct menu *m;
	int err;

	if (IS_ENABLED(CONFIG_CMD_BMP)) {
		/* display BMP if available */
		if (cfg->bmp) {
			if (get_relfile(ctx, cfg->bmp, image_load_addr, NULL)) {
#if defined(CONFIG_VIDEO)
				struct udevice *dev;

				err = uclass_first_device_err(UCLASS_VIDEO, &dev);
				if (!err)
					video_clear(dev);
#endif
				bmp_display(image_load_addr,
					    BMP_ALIGN_CENTER, BMP_ALIGN_CENTER);
			} else {
				printf("Skipping background bmp %s for failure\n",
				       cfg->bmp);
			}
		}
	}

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
		err = label_boot(ctx, choice);
		if (!err)
			return;
	} else if (err != -ENOENT) {
		return;
	}

	boot_unattempted_labels(ctx, cfg);
}

int pxe_setup_ctx(struct pxe_context *ctx, struct cmd_tbl *cmdtp,
		  pxe_getfile_func getfile, void *userdata,
		  bool allow_abs_path, const char *bootfile, bool use_ipv6)
{
	const char *last_slash;
	size_t path_len = 0;

	memset(ctx, '\0', sizeof(*ctx));
	ctx->cmdtp = cmdtp;
	ctx->getfile = getfile;
	ctx->userdata = userdata;
	ctx->allow_abs_path = allow_abs_path;
	ctx->use_ipv6 = use_ipv6;

	/* figure out the boot directory, if there is one */
	if (bootfile && strlen(bootfile) >= MAX_TFTP_PATH_LEN)
		return -ENOSPC;
	ctx->bootdir = strdup(bootfile ? bootfile : "");
	if (!ctx->bootdir)
		return -ENOMEM;

	if (bootfile) {
		last_slash = strrchr(bootfile, '/');
		if (last_slash)
			path_len = (last_slash - bootfile) + 1;
	}
	ctx->bootdir[path_len] = '\0';

	return 0;
}

void pxe_destroy_ctx(struct pxe_context *ctx)
{
	free(ctx->bootdir);
}

int pxe_process(struct pxe_context *ctx, ulong pxefile_addr_r, bool prompt)
{
	struct pxe_menu *cfg;

	cfg = parse_pxefile(ctx, pxefile_addr_r);
	if (!cfg) {
		printf("Error parsing config file\n");
		return 1;
	}

	if (prompt)
		cfg->prompt = 1;

	handle_pxe_menu(ctx, cfg);

	destroy_pxe_menu(cfg);

	return 0;
}
