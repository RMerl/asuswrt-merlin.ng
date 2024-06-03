/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef _BOOTM_H
#define _BOOTM_H

#include <command.h>
#include <image.h>

#define BOOTM_ERR_RESET		(-1)
#define BOOTM_ERR_OVERLAP		(-2)
#define BOOTM_ERR_UNIMPLEMENTED	(-3)

/*
 *  Continue booting an OS image; caller already has:
 *  - copied image header to global variable `header'
 *  - checked header magic number, checksums (both header & image),
 *  - verified image architecture (PPC) and type (KERNEL or MULTI),
 *  - loaded (first part of) image to header load address,
 *  - disabled interrupts.
 *
 * @flag: Flags indicating what to do (BOOTM_STATE_...)
 * @argc: Number of arguments. Note that the arguments are shifted down
 *	 so that 0 is the first argument not processed by U-Boot, and
 *	 argc is adjusted accordingly. This avoids confusion as to how
 *	 many arguments are available for the OS.
 * @images: Pointers to os/initrd/fdt
 * @return 1 on error. On success the OS boots so this function does
 * not return.
 */
typedef int boot_os_fn(int flag, int argc, char * const argv[],
			bootm_headers_t *images);

extern boot_os_fn do_bootm_linux;
extern boot_os_fn do_bootm_vxworks;

int do_bootelf(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
void lynxkdi_boot(image_header_t *hdr);

boot_os_fn *bootm_os_get_boot_func(int os);

#if defined(CONFIG_FIT_SIGNATURE)
int bootm_host_load_images(const void *fit, int cfg_noffset);
#endif

int boot_selected_os(int argc, char * const argv[], int state,
		     bootm_headers_t *images, boot_os_fn *boot_fn);

ulong bootm_disable_interrupts(void);

/* This is a special function used by booti/bootz */
int bootm_find_images(int flag, int argc, char * const argv[]);

int do_bootm_states(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[],
		    int states, bootm_headers_t *images, int boot_progress);

void arch_preboot_os(void);

/**
 * bootm_decomp_image() - decompress the operating system
 *
 * @comp:	Compression algorithm that is used (IH_COMP_...)
 * @load:	Destination load address in U-Boot memory
 * @image_start Image start address (where we are decompressing from)
 * @type:	OS type (IH_OS_...)
 * @load_bug:	Place to decompress to
 * @image_buf:	Address to decompress from
 * @image_len:	Number of bytes in @image_buf to decompress
 * @unc_len:	Available space for decompression
 * @return 0 if OK, -ve on error (BOOTM_ERR_...)
 */
int bootm_decomp_image(int comp, ulong load, ulong image_start, int type,
		       void *load_buf, void *image_buf, ulong image_len,
		       uint unc_len, ulong *load_end);

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

#endif
