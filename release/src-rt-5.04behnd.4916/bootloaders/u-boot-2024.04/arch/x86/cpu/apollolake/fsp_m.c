// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <asm/arch/iomap.h>
#include <asm/arch/fsp_bindings.h>
#include <asm/fsp2/fsp_internal.h>
#include <dm/uclass-internal.h>

int fspm_update_config(struct udevice *dev, struct fspm_upd *upd)
{
	struct fsp_m_config *cfg = &upd->config;
	struct fspm_arch_upd *arch = &upd->arch;
	int cache_ret = 0;
	ofnode node;
	int ret;

	arch->nvs_buffer_ptr = NULL;
	cache_ret = prepare_mrc_cache(upd);
	if (cache_ret && cache_ret != -ENOENT)
		return log_msg_ret("mrc", cache_ret);
	arch->stack_base = (void *)(CONFIG_SYS_CAR_ADDR + CONFIG_SYS_CAR_SIZE -
		 arch->stack_size);
	arch->boot_loader_tolum_size = 0;
	arch->boot_mode = cache_ret ? FSP_BOOT_WITH_FULL_CONFIGURATION :
		FSP_BOOT_ASSUMING_NO_CONFIGURATION_CHANGES;

	node = dev_ofnode(dev);
	if (!ofnode_valid(node))
		return log_msg_ret("node", -ENOENT);
	node = ofnode_find_subnode(node, "fsp-m");
	if (!ofnode_valid(node))
		return log_msg_ret("fspm", -ENOENT);

	ret = fsp_m_update_config_from_dtb(node, cfg);
	if (ret)
		return log_msg_ret("dtb", cache_ret);

	return cache_ret;
}

/*
 * The FSP-M binary appears to break the SPI controller. It can be fixed by
 * writing the BAR again, so do that here
 */
int fspm_done(struct udevice *dev)
{
	struct udevice *spi;
	int ret;

	/* Don't probe the device, since that reads the BAR */
	ret = uclass_find_first_device(UCLASS_SPI, &spi);
	if (ret)
		return log_msg_ret("SPI", ret);
	if (!spi)
		return log_msg_ret("no SPI", -ENODEV);

	dm_pci_write_config32(spi, PCI_BASE_ADDRESS_0,
			      IOMAP_SPI_BASE | PCI_BASE_ADDRESS_SPACE_MEMORY);

	return 0;
}
