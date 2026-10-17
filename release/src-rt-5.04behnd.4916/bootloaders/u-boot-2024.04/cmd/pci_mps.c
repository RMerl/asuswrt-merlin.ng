// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2022 Microsoft Corporation <www.microsoft.com>
 * Stephen Carlson <stcarlso@linux.microsoft.com>
 *
 * PCI Express Maximum Packet Size (MPS) configuration
 */

#include <common.h>
#include <bootretry.h>
#include <cli.h>
#include <command.h>
#include <console.h>
#include <dm.h>
#include <init.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <pci.h>

#define PCI_MPS_SAFE 0
#define PCI_MPS_PEER2PEER 1

static int pci_mps_find_safe(struct udevice *bus, unsigned int *min_mps,
			     unsigned int *n)
{
	struct udevice *dev;
	int res = 0, addr;
	unsigned int mpss;
	u32 regval;

	if (!min_mps || !n)
		return -EINVAL;

	for (device_find_first_child(bus, &dev);
	     dev;
	     device_find_next_child(&dev)) {
		addr = dm_pci_find_capability(dev, PCI_CAP_ID_EXP);
		if (addr <= 0)
			continue;

		res = dm_pci_read_config32(dev, addr + PCI_EXP_DEVCAP,
					   &regval);
		if (res != 0)
			return res;
		mpss = (unsigned int)(regval & PCI_EXP_DEVCAP_PAYLOAD);
		*n += 1;
		if (mpss < *min_mps)
			*min_mps = mpss;
	}

	return res;
}

static int pci_mps_set_bus(struct udevice *bus, unsigned int target)
{
	struct udevice *dev;
	u32 mpss, target_mps = (u32)(target << 5);
	u16 mps;
	int res = 0, addr;

	for (device_find_first_child(bus, &dev);
	     dev && res == 0;
	     device_find_next_child(&dev)) {
		addr = dm_pci_find_capability(dev, PCI_CAP_ID_EXP);
		if (addr <= 0)
			continue;

		res = dm_pci_read_config32(dev, addr + PCI_EXP_DEVCAP,
					   &mpss);
		if (res != 0)
			return res;

		/* Do not set device above its maximum MPSS */
		mpss = (mpss & PCI_EXP_DEVCAP_PAYLOAD) << 5;
		if (target_mps < mpss)
			mps = (u16)target_mps;
		else
			mps = (u16)mpss;
		res = dm_pci_clrset_config16(dev, addr + PCI_EXP_DEVCTL,
					     PCI_EXP_DEVCTL_PAYLOAD, mps);
	}

	return res;
}

/*
 * Sets the MPS of each PCI Express device to the specified policy.
 */
static int pci_mps_set(int policy)
{
	struct udevice *bus;
	int i, res = 0;
	/* 0 = 128B, min value for hotplug */
	unsigned int mps = 0;

	if (policy == PCI_MPS_SAFE) {
		unsigned int min_mps = PCI_EXP_DEVCAP_PAYLOAD_4096B, n = 0;

		/* Find maximum MPS supported by all devices */
		for (i = 0;
		     uclass_get_device_by_seq(UCLASS_PCI, i, &bus) == 0 &&
		     res == 0;
		     i++)
			res = pci_mps_find_safe(bus, &min_mps, &n);

		/* If no devices were found, do not reconfigure */
		if (n == 0)
			return res;
		mps = min_mps;
	}

	/* This message is checked by the sandbox test */
	printf("Setting MPS of all devices to %uB\n", 128U << mps);
	for (i = 0;
	     uclass_get_device_by_seq(UCLASS_PCI, i, &bus) == 0 && res == 0;
	     i++)
		res = pci_mps_set_bus(bus, mps);

	return res;
}

/*
 * PCI MPS tuning commands
 *
 * Syntax:
 *	pci_mps safe
 *	pci_mps peer2peer
 */
static int do_pci_mps(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	char cmd = 'u';
	int ret = 0;

	if (argc > 1)
		cmd = argv[1][0];

	switch (cmd) {
	case 's':		/* safe */
		ret = pci_mps_set(PCI_MPS_SAFE);
		break;
	case 'p':		/* peer2peer/hotplug */
		ret = pci_mps_set(PCI_MPS_PEER2PEER);
		break;
	default:		/* usage, help */
		goto usage;
	}

	return ret;
usage:
	return CMD_RET_USAGE;
}

/***************************************************/

U_BOOT_LONGHELP(pci_mps,
	"safe\n"
	"    - Set PCI Express MPS of all devices to safe values\n"
	"pci_mps peer2peer\n"
	"    - Set PCI Express MPS of all devices to support hotplug and peer-to-peer DMA\n");

U_BOOT_CMD(pci_mps, 2, 0, do_pci_mps,
	   "configure PCI Express MPS", pci_mps_help_text);
