// SPDX-License-Identifier: GPL-2.0
/*
 * From Coreboot src/southbridge/intel/bd82x6x/early_me.c
 *
 * Copyright (C) 2011 The Chromium OS Authors. All rights reserved.
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <sysreset.h>
#include <asm/pci.h>
#include <asm/cpu.h>
#include <asm/processor.h>
#include <asm/arch/me.h>
#include <asm/arch/pch.h>
#include <asm/io.h>
#include <linux/delay.h>

static const char *const me_ack_values[] = {
	[ME_HFS_ACK_NO_DID]	= "No DID Ack received",
	[ME_HFS_ACK_RESET]	= "Non-power cycle reset",
	[ME_HFS_ACK_PWR_CYCLE]	= "Power cycle reset",
	[ME_HFS_ACK_S3]		= "Go to S3",
	[ME_HFS_ACK_S4]		= "Go to S4",
	[ME_HFS_ACK_S5]		= "Go to S5",
	[ME_HFS_ACK_GBL_RESET]	= "Global Reset",
	[ME_HFS_ACK_CONTINUE]	= "Continue to boot"
};

int intel_early_me_init(struct udevice *me_dev)
{
	int count;
	struct me_uma uma;
	struct me_hfs hfs;

	debug("Intel ME early init\n");

	/* Wait for ME UMA SIZE VALID bit to be set */
	for (count = ME_RETRY; count > 0; --count) {
		pci_read_dword_ptr(me_dev, &uma, PCI_ME_UMA);
		if (uma.valid)
			break;
		udelay(ME_DELAY);
	}
	if (!count) {
		printf("ERROR: ME is not ready!\n");
		return -EBUSY;
	}

	/* Check for valid firmware */
	pci_read_dword_ptr(me_dev, &hfs, PCI_ME_HFS);
	if (hfs.fpt_bad) {
		printf("WARNING: ME has bad firmware\n");
		return -EBADF;
	}

	debug("Intel ME firmware is ready\n");

	return 0;
}

int intel_early_me_uma_size(struct udevice *me_dev)
{
	struct me_uma uma;

	pci_read_dword_ptr(me_dev, &uma, PCI_ME_UMA);
	if (uma.valid) {
		debug("ME: Requested %uMB UMA\n", uma.size);
		return uma.size;
	}

	debug("ME: Invalid UMA size\n");
	return -EINVAL;
}

static inline void set_global_reset(struct udevice *dev, int enable)
{
	u32 etr3;

	dm_pci_read_config32(dev, ETR3, &etr3);

	/* Clear CF9 Without Resume Well Reset Enable */
	etr3 &= ~ETR3_CWORWRE;

	/* CF9GR indicates a Global Reset */
	if (enable)
		etr3 |= ETR3_CF9GR;
	else
		etr3 &= ~ETR3_CF9GR;

	dm_pci_write_config32(dev, ETR3, etr3);
}

int intel_early_me_init_done(struct udevice *dev, struct udevice *me_dev,
			     uint status)
{
	int count;
	u32 mebase_l, mebase_h;
	struct me_hfs hfs;
	struct me_did did = {
		.init_done = ME_INIT_DONE,
		.status = status
	};

	/* MEBASE from MESEG_BASE[35:20] */
	dm_pci_read_config32(PCH_DEV, PCI_CPU_MEBASE_L, &mebase_l);
	dm_pci_read_config32(PCH_DEV, PCI_CPU_MEBASE_H, &mebase_h);
	mebase_h &= 0xf;
	did.uma_base = (mebase_l >> 20) | (mebase_h << 12);

	/* Send message to ME */
	debug("ME: Sending Init Done with status: %d, UMA base: 0x%04x\n",
	      status, did.uma_base);

	pci_write_dword_ptr(me_dev, &did, PCI_ME_H_GS);

	/* Must wait for ME acknowledgement */
	for (count = ME_RETRY; count > 0; --count) {
		pci_read_dword_ptr(me_dev, &hfs, PCI_ME_HFS);
		if (hfs.bios_msg_ack)
			break;
		udelay(ME_DELAY);
	}
	if (!count) {
		printf("ERROR: ME failed to respond\n");
		return -ETIMEDOUT;
	}

	/* Return the requested BIOS action */
	debug("ME: Requested BIOS Action: %s\n", me_ack_values[hfs.ack_data]);

	/* Check status after acknowledgement */
	intel_me_status(me_dev);

	switch (hfs.ack_data) {
	case ME_HFS_ACK_CONTINUE:
		/* Continue to boot */
		return 0;
	case ME_HFS_ACK_RESET:
		/* Non-power cycle reset */
		set_global_reset(dev, 0);
		sysreset_walk_halt(SYSRESET_COLD);
		break;
	case ME_HFS_ACK_PWR_CYCLE:
		/* Power cycle reset */
		set_global_reset(dev, 0);
		sysreset_walk_halt(SYSRESET_COLD);
		break;
	case ME_HFS_ACK_GBL_RESET:
		/* Global reset */
		set_global_reset(dev, 1);
		sysreset_walk_halt(SYSRESET_COLD);
		break;
	case ME_HFS_ACK_S3:
	case ME_HFS_ACK_S4:
	case ME_HFS_ACK_S5:
		break;
	}

	return -EINVAL;
}

static const struct udevice_id ivybridge_syscon_ids[] = {
	{ .compatible = "intel,me", .data = X86_SYSCON_ME },
	{ }
};

U_BOOT_DRIVER(syscon_intel_me) = {
	.name = "intel_me_syscon",
	.id = UCLASS_SYSCON,
	.of_match = ivybridge_syscon_ids,
};
