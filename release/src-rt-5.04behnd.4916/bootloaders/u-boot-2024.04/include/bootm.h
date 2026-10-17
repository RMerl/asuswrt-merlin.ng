/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef _BOOTM_H
#define _BOOTM_H

#include <image.h>

struct boot_params;
struct cmd_tbl;

#define BOOTM_ERR_RESET		(-1)
#define BOOTM_ERR_OVERLAP		(-2)
#define BOOTM_ERR_UNIMPLEMENTED	(-3)

/**
 * struct bootm_info() - information used when processing images to boot
 *
 * These mirror the first three arguments of the bootm command. They are
 * designed to handle any type of image, but typically it is a FIT.
 *
 * @addr_img: Address of image to bootm, as passed to
 *	genimg_get_kernel_addr_fit() for processing:
 *
 *    NULL: Usees default load address, i.e. image_load_addr
 *    <addr>: Uses hex address
 *
 * For FIT:
 *    "[<addr>]#<conf>": Uses address (or image_load_addr) and also specifies
 *	the FIT configuration to use
 *    "[<addr>]:<subimage>": Uses address (or image_load_addr) and also
 *	specifies the subimage name containing the OS
 *
 * @conf_ramdisk: Address (or with FIT, the name) of the ramdisk image, as
 *	passed to boot_get_ramdisk() for processing, or NULL for none
 * @conf_fdt: Address (or with FIT, the name) of the FDT image, as passed to
 *	boot_get_fdt() for processing, or NULL for none
 * @boot_progress: true to show boot progress
 * @images: images information
 * @cmd_name: command which invoked this operation, e.g. "bootm"
 * @argc: Number of arguments to the command (excluding the actual command).
 *	This is 0 if there are no arguments
 * @argv: NULL-terminated list of arguments, or NULL if there are no arguments
 */
struct bootm_info {
	const char *addr_img;
	const char *conf_ramdisk;
	const char *conf_fdt;
	bool boot_progress;
	struct bootm_headers *images;
	const char *cmd_name;
	int argc;
	char *const *argv;
};

/**
 * bootm_init() - Set up a bootm_info struct with useful defaults
 *
 * Set up the struct with default values for all members:
 * @boot_progress is set to true and @images is set to the global images
 * variable. Everything else is set to NULL except @argc which is 0
 */
void bootm_init(struct bootm_info *bmi);

/*
 *  Continue booting an OS image; caller already has:
 *  - copied image header to global variable `header'
 *  - checked header magic number, checksums (both header & image),
 *  - verified image architecture (PPC) and type (KERNEL or MULTI),
 *  - loaded (first part of) image to header load address,
 *  - disabled interrupts.
 *
 * @flag: Flags indicating what to do (BOOTM_STATE_...)
 * bmi: Bootm information
 * Return: 1 on error. On success the OS boots so this function does
 * not return.
 */
typedef int boot_os_fn(int flag, struct bootm_info *bmi);

extern boot_os_fn do_bootm_linux;
extern boot_os_fn do_bootm_vxworks;

int do_bootelf(struct cmd_tbl *cmdtp, int fglag, int argc, char *const argv[]);

boot_os_fn *bootm_os_get_boot_func(int os);

#if defined(CONFIG_FIT_SIGNATURE)
int bootm_host_load_images(const void *fit, int cfg_noffset);
#endif

int boot_selected_os(int state, struct bootm_info *bmi, boot_os_fn *boot_fn);

ulong bootm_disable_interrupts(void);

/**
 * bootm_find_images() - find and locate various images
 *
 * @img_addr: Address of image being loaded
 * @conf_ramdisk: Indicates the ramdisk to use (typically second arg of bootm)
 * @conf_fdt: Indicates the FDT to use (typically third arg of bootm)
 * @start: OS image start address
 * @size: OS image size
 *
 * boot_find_images() will attempt to load an available ramdisk,
 * flattened device tree, as well as specifically marked
 * "loadable" images (loadables are FIT only)
 *
 * Note: bootm_find_images will skip an image if it is not found
 *
 * This is a special function used by booti/bootz
 *
 * Return:
 *     0, if all existing images were loaded correctly
 *     1, if an image is found but corrupted, or invalid
 */
int bootm_find_images(ulong img_addr, const char *conf_ramdisk,
		      const char *conf_fdt, ulong start, ulong size);

/*
 * Measure the boot images. Measurement is the process of hashing some binary
 * data and storing it into secure memory, i.e. TPM PCRs. In addition, each
 * measurement is logged into the platform event log such that the operating
 * system can access it and perform attestation of the boot.
 *
 * @images:	The structure containing the various images to boot (linux,
 *		initrd, dts, etc.)
 */
int bootm_measure(struct bootm_headers *images);

/**
 * bootm_run_states() - Execute selected states of the bootm command.
 *
 * Note that if states contains more than one flag it MUST contain
 * BOOTM_STATE_START, since this handles the addr_fit, conf_ramdisk and conf_fit
 * members of @bmi
 *
 * Also note that aside from boot_os_fn functions and bootm_load_os, no other
 * functions store the return value of in 'ret' may use a negative return
 * value, without special handling.
 *
 * @bmi: bootm information
 * @states	Mask containing states to run (BOOTM_STATE_...)
 * Return: 0 if ok, something else on error. Some errors will cause this
 *	function to perform a reboot! If states contains BOOTM_STATE_OS_GO
 *	then the intent is to boot an OS, so this function will not return
 *	unless the image type is standalone.
 */
int bootm_run_states(struct bootm_info *bmi, int states);

/**
 * boot_run() - Run the entire bootm/booti/bootz process
 *
 * This runs through the boot process from start to finish, with a base set of
 * states, along with the extra ones supplied.
 *
 * This uses bootm_run_states().
 *
 * Note that it is normally easier to use bootm_run(), etc. since they handle
 * the extra states correctly.
 *
 * @bmi: bootm information
 * @cmd: command being run, NULL if none
 * @extra_states: Mask of extra states to use for the boot
 * Return: 0 if ok, something else on error
 */
int boot_run(struct bootm_info *bmi, const char *cmd, int extra_states);

/**
 * bootm_run() - Run the entire bootm process
 *
 * This runs through the bootm process from start to finish, using the default
 * set of states.
 *
 * This uses bootm_run_states().
 *
 * @bmi: bootm information
 * Return: 0 if ok, something else on error
 */
int bootm_run(struct bootm_info *bmi);

/**
 * bootz_run() - Run the entire bootz process
 *
 * This runs through the bootz process from start to finish, using the default
 * set of states.
 *
 * This uses bootm_run_states().
 *
 * @bmi: bootm information
 * Return: 0 if ok, something else on error
 */
int bootz_run(struct bootm_info *bmi);

/**
 * booti_run() - Run the entire booti process
 *
 * This runs through the booti process from start to finish, using the default
 * set of states.
 *
 * This uses bootm_run_states().
 *
 * @bmi: bootm information
 * Return: 0 if ok, something else on error
 */
int booti_run(struct bootm_info *bmi);

void arch_preboot_os(void);

/*
 * boards should define this to disable devices when EFI exits from boot
 * services.
 *
 * TODO(sjg@chromium.org>): Update this to use driver model's device_remove().
 */
void board_quiesce_devices(void);

/**
 * switch_to_non_secure_mode() - switch to non-secure mode
 */
void switch_to_non_secure_mode(void);

/* Flags to control bootm_process_cmdline() */
enum bootm_cmdline_t {
	BOOTM_CL_SILENT	= 1 << 0,	/* Do silent console processing */
	BOOTM_CL_SUBST	= 1 << 1,	/* Do substitution */

	BOOTM_CL_ALL	= 3,		/* All substitutions */
};

/**
 * arch_preboot_os() - arch specific configuration before booting
 */
void arch_preboot_os(void);

/**
 * board_preboot_os() - board specific configuration before booting
 */
void board_preboot_os(void);

/*
 * bootm_process_cmdline() - Process fix-ups for the command line
 *
 * This handles:
 *
 *  - making Linux boot silently if requested ('silent_linux' envvar)
 *  - performing substitutions in the command line ('bootargs_subst' envvar)
 *
 * @maxlen must provide enough space for the string being processed plus the
 * resulting string
 *
 * @buf: buffer holding commandline string to adjust
 * @maxlen: Maximum length of buffer at @buf (including \0)
 * @flags: Flags to control what happens (see bootm_cmdline_t)
 * Return: 0 if OK, -ENOMEM if out of memory, -ENOSPC if the commandline is too
 *	long
 */
int bootm_process_cmdline(char *buf, int maxlen, int flags);

/**
 * bootm_process_cmdline_env() - Process fix-ups for the command line
 *
 * Updates the 'bootargs' envvar as required. This handles:
 *
 *  - making Linux boot silently if requested ('silent_linux' envvar)
 *  - performing substitutions in the command line ('bootargs_subst' envvar)
 *
 * @flags: Flags to control what happens (see bootm_cmdline_t)
 * Return: 0 if OK, -ENOMEM if out of memory
 */
int bootm_process_cmdline_env(int flags);

/**
 * zboot_start() - Boot a zimage
 *
 * Boot a zimage, given the component parts
 *
 * @addr: Address where the bzImage is moved before booting, either
 *	BZIMAGE_LOAD_ADDR or ZIMAGE_LOAD_ADDR
 * @base: Pointer to the boot parameters, typically at address
 *	DEFAULT_SETUP_BASE
 * @initrd: Address of the initial ramdisk, or 0 if none
 * @initrd_size: Size of the initial ramdisk, or 0 if none
 * @cmdline: Command line to use for booting
 * Return: -EFAULT on error (normally it does not return)
 */
int zboot_start(ulong addr, ulong size, ulong initrd, ulong initrd_size,
		ulong base, char *cmdline);

/*
 * zimage_get_kernel_version() - Get the version string from a kernel
 *
 * @params: boot_params pointer
 * @kernel_base: base address of kernel
 * Return: Kernel version as a NUL-terminated string
 */
const char *zimage_get_kernel_version(struct boot_params *params,
				      void *kernel_base);

/**
 * zimage_dump() - Dump the metadata of a zimage
 *
 * This shows all available information in a zimage that has been loaded.
 *
 * @base_ptr: Pointer to the boot parameters, typically at address
 *	DEFAULT_SETUP_BASE
 * @show_cmdline: true to show the full command line
 */
void zimage_dump(struct boot_params *base_ptr, bool show_cmdline);

/*
 * bootm_boot_start() - Boot an image at the given address
 *
 * @addr: Image address
 * @cmdline: Command line to set
 */
int bootm_boot_start(ulong addr, const char *cmdline);

#endif
