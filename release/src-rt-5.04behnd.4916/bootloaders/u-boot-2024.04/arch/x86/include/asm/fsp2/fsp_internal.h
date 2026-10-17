/* SPDX-License-Identifier: Intel */
/*
 * Copyright (C) 2015-2016 Intel Corp.
 * (Written by Alexandru Gagniuc <alexandrux.gagniuc@intel.com> for Intel Corp.)
 * Mostly taken from coreboot
 */

#ifndef __ASM_FSP_INTERNAL_H
#define __ASM_FSP_INTERNAL_H

struct binman_entry;
struct fsp_header;
struct fspm_upd;
struct fsps_upd;

enum fsp_type_t {
	FSP_M,
	FSP_S,
};

int fsp_get_header(ulong offset, ulong size, bool use_spi_flash,
		   struct fsp_header **fspp);

/**
 * fsp_locate_fsp() - Locate an FSP component
 *
 * This finds an FSP component by various methods. It is not as general-purpose
 * as it looks, since it expects FSP-M to be requested in SPL (only), and FSP-S
 * to be requested in U-Boot proper.
 *
 * @type: Component to locate
 * @entry: Returns location of component
 * @use_spi_flash: true to read using the Fast SPI driver, false to use
 *	memory-mapped SPI flash
 * @devp: Returns northbridge device
 * @hdrp: Returns FSP header
 * @rom_offsetp: If non-NULL, returns the offset to add to any image position to
 *	find the memory-mapped location of that position. For example, for ROM
 *	position 0x1000, it will be mapped into 0x1000 + *rom_offsetp.
 */
int fsp_locate_fsp(enum fsp_type_t type, struct binman_entry *entry,
		   bool use_spi_flash, struct udevice **devp,
		   struct fsp_header **hdrp, ulong *rom_offsetp);

/**
 * arch_fsps_preinit() - Perform init needed before calling FSP-S
 *
 * This allows use of probed drivers and PCI so is a convenient place to do any
 * init that is needed before FSP-S is called. After this, U-Boot relocates and
 * calls arch_fsp_init_r() before PCI is probed, and that function is not
 * allowed to probe PCI before calling FSP-S.
 */
int arch_fsps_preinit(void);

/**
 * fspm_update_config() - Set up the config structure for FSP-M
 *
 * @dev: Hostbridge device containing config
 * @upd: Config data to fill in
 * Return: 0 if OK, -ENOENT if OK but no MRC-cache data was found, other -ve on
 *	error
 */
int fspm_update_config(struct udevice *dev, struct fspm_upd *upd);

/**
 * fspm_done() - Indicate that memory init is complete
 *
 * This allows the board to do whatever post-init it needs before things
 * continue.
 *
 * @dev: Hostbridge device
 * Return: 0 if OK, -ve on error
 */
int fspm_done(struct udevice *dev);

/**
 * fsps_update_config() - Set up the config structure for FSP-S
 *
 * @dev: Hostbridge device containing config
 * @rom_offset: Value to add to convert from ROM offset to memory-mapped address
 * @upd: Config data to fill in
 * Return: 0 if OK, -ve on error
 */
int fsps_update_config(struct udevice *dev, ulong rom_offset,
		       struct fsps_upd *upd);

/**
 * prepare_mrc_cache() - Read the MRC cache into the product-data struct
 *
 * This looks for cached Memory-reference code (MRC) data and stores it into
 * @upd for use by the FSP-M binary.
 *
 * Return: 0 if OK, -ENOENT if no data (whereupon the caller can continue and
 *	expect a slower boot), other -ve value on other error
 */
int prepare_mrc_cache(struct fspm_upd *upd);

#endif
