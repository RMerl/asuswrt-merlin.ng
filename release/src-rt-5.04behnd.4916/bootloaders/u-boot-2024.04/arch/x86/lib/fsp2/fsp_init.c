// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019 Google LLC
 */

#include <common.h>
#include <binman.h>
#include <binman_sym.h>
#include <bootstage.h>
#include <cbfs.h>
#include <dm.h>
#include <event.h>
#include <init.h>
#include <log.h>
#include <spi.h>
#include <spl.h>
#include <spi_flash.h>
#include <asm/intel_pinctrl.h>
#include <dm/uclass-internal.h>
#include <asm/fsp2/fsp_internal.h>

int fsp_setup_pinctrl(void)
{
	struct udevice *dev;
	ofnode node;
	int ret;

	/* Make sure pads are set up early in U-Boot */
	if (!ll_boot_init() || spl_phase() != PHASE_BOARD_F)
		return 0;

	/* Probe all pinctrl devices to set up the pads */
	ret = uclass_first_device_err(UCLASS_PINCTRL, &dev);
	if (ret)
		return log_msg_ret("no fsp pinctrl", ret);
	node = ofnode_path("fsp");
	if (!ofnode_valid(node))
		return log_msg_ret("no fsp params", -EINVAL);
	ret = pinctrl_config_pads_for_node(dev, node);
	if (ret)
		return log_msg_ret("pad config", ret);

	return ret;
}
EVENT_SPY_SIMPLE(EVT_DM_POST_INIT_F, fsp_setup_pinctrl);

#if !defined(CONFIG_TPL_BUILD)
binman_sym_declare(ulong, intel_fsp_m, image_pos);
binman_sym_declare(ulong, intel_fsp_m, size);

/**
 * get_cbfs_fsp() - Obtain the FSP by looking up in CBFS
 *
 * This looks up an FSP in a CBFS. It is used mostly for testing, when booting
 * U-Boot from a hybrid image containing coreboot as the first-stage bootloader.
 *
 * The typical use for this feature is when building a Chrome OS image which
 * includes coreboot in it. By adding U-Boot into the 'COREBOOT' CBFS as well,
 * it is possible to make coreboot chain-load U-Boot. Thus the initial stages of
 * the SoC init can be done by coreboot and the later stages by U-Boot. This is
 * a convenient way to start the porting work. The jump to U-Boot can then be
 * moved progressively earlier and earlier, until U-Boot takes over all the init
 * and you have a native port.
 *
 * This function looks up a CBFS at a known location and reads the FSP-M from it
 * so that U-Boot can init the memory.
 *
 * This function is not used in the normal boot but is kept here for future
 * development.
 *
 * @type; Type to look up (only FSP_M supported at present)
 * @map_base: Base memory address for mapped SPI
 * @entry: Returns an entry containing the position of the FSP image
 */
static int get_cbfs_fsp(enum fsp_type_t type, ulong map_base,
			struct binman_entry *entry)
{
	/*
	 * Use a hard-coded position of CBFS in the ROM for now. It would be
	 * possible to read the position using the FMAP in the ROM, but since
	 * this code is only used for development, it doesn't seem worth it.
	 * Use the 'cbfstool <image> layout' command to get these values, e.g.:
	 * 'COREBOOT' (CBFS, size 1814528, offset 2117632).
	 */
	ulong cbfs_base = 0x205000;
	struct cbfs_priv *cbfs;
	int ret;

	ret = cbfs_init_mem(map_base + cbfs_base, CBFS_SIZE_UNKNOWN, true,
			    &cbfs);
	if (ret)
		return ret;
	if (!ret) {
		const struct cbfs_cachenode *node;

		node = cbfs_find_file(cbfs, "fspm.bin");
		if (!node)
			return log_msg_ret("fspm node", -ENOENT);

		entry->image_pos = (ulong)node->data;
		entry->size = node->data_length;
	}

	return 0;
}

int fsp_locate_fsp(enum fsp_type_t type, struct binman_entry *entry,
		   bool use_spi_flash, struct udevice **devp,
		   struct fsp_header **hdrp, ulong *rom_offsetp)
{
	ulong mask = CONFIG_ROM_SIZE - 1;
	struct udevice *dev;
	ulong rom_offset = 0;
	uint map_size;
	ulong map_base;
	uint offset;
	int ret;

	/*
	 * Find the devices but don't probe them, since we don't want to
	 * auto-config PCI before silicon init runs
	 */
	ret = uclass_find_first_device(UCLASS_NORTHBRIDGE, &dev);
	if (ret)
		return log_msg_ret("Cannot get northbridge", ret);
	if (!use_spi_flash) {
		struct udevice *sf;

		/* Just use the SPI driver to get the memory map */
		ret = uclass_find_first_device(UCLASS_SPI_FLASH, &sf);
		if (ret)
			return log_msg_ret("Cannot get SPI flash", ret);
		ret = dm_spi_get_mmap(sf, &map_base, &map_size, &offset);
		if (ret)
			return log_msg_ret("Could not get flash mmap", ret);
	}

	if (spl_phase() >= PHASE_BOARD_F) {
		if (type != FSP_S)
			return -EPROTONOSUPPORT;
		ret = binman_entry_find("intel-fsp-s", entry);
		if (ret)
			return log_msg_ret("binman entry", ret);
		if (!use_spi_flash)
			rom_offset = (map_base & mask) - CONFIG_ROM_SIZE;
	} else {
		ret = -ENOENT;
		if (false)
			/*
			 * Support using a hybrid image build by coreboot. See
			 * the function comments for details
			 */
			ret = get_cbfs_fsp(type, map_base, entry);
		if (ret) {
			ulong mask = CONFIG_ROM_SIZE - 1;

			if (type != FSP_M)
				return -EPROTONOSUPPORT;
			entry->image_pos = binman_sym(ulong, intel_fsp_m,
						      image_pos);
			entry->size = binman_sym(ulong, intel_fsp_m, size);
			if (entry->image_pos != BINMAN_SYM_MISSING) {
				ret = 0;
				if (use_spi_flash)
					entry->image_pos &= mask;
				else
					entry->image_pos += (map_base & mask);
			} else {
				ret = -ENOENT;
			}
		}
	}
	if (ret)
		return log_msg_ret("Cannot find FSP", ret);
	entry->image_pos += rom_offset;

	/*
	 * Account for the time taken to read memory-mapped SPI flash since in
	 * this case we don't use the SPI driver and BOOTSTAGE_ID_ACCUM_SPI.
	 */
	if (!use_spi_flash)
		bootstage_start(BOOTSTAGE_ID_ACCUM_MMAP_SPI, "mmap_spi");
	ret = fsp_get_header(entry->image_pos, entry->size, use_spi_flash,
			     hdrp);
	if (!use_spi_flash)
		bootstage_accum(BOOTSTAGE_ID_ACCUM_MMAP_SPI);
	if (ret)
		return log_msg_ret("fsp_get_header", ret);
	*devp = dev;
	if (rom_offsetp)
		*rom_offsetp = rom_offset;

	return 0;
}
#endif
