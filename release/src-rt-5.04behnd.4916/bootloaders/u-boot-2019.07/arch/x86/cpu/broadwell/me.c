// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016 Google, Inc
 *
 * Based on code from coreboot src/soc/intel/broadwell/me_status.c
 */

#include <common.h>
#include <errno.h>
#include <asm/arch/me.h>

static inline void me_read_dword_ptr(struct udevice *dev, void *ptr, int offset)
{
	u32 dword;

	dm_pci_read_config32(dev, offset, &dword);
	memcpy(ptr, &dword, sizeof(dword));
}

int intel_me_hsio_version(struct udevice *dev, uint16_t *versionp,
			  uint16_t *checksump)
{
	int count;
	u32 hsiover;
	struct me_hfs hfs;

	/* Query for HSIO version, overloads H_GS and HFS */
	dm_pci_write_config32(dev, PCI_ME_H_GS,
			      ME_HSIO_MESSAGE | ME_HSIO_CMD_GETHSIOVER);

	/* Must wait for ME acknowledgement */
	for (count = ME_RETRY; count > 0; --count) {
		me_read_dword_ptr(dev, &hfs, PCI_ME_HFS);
		if (hfs.bios_msg_ack)
			break;
		udelay(ME_DELAY);
	}
	if (!count) {
		debug("ERROR: ME failed to respond\n");
		return -ETIMEDOUT;
	}

	/* HSIO version should be in HFS_5 */
	dm_pci_read_config32(dev, PCI_ME_HFS5, &hsiover);
	*versionp = hsiover >> 16;
	*checksump = hsiover & 0xffff;

	debug("ME: HSIO Version            : %d (CRC 0x%04x)\n",
	      *versionp, *checksump);

	/* Reset registers to normal behavior */
	dm_pci_write_config32(dev, PCI_ME_H_GS,
			      ME_HSIO_MESSAGE | ME_HSIO_CMD_GETHSIOVER);

	return 0;
}
