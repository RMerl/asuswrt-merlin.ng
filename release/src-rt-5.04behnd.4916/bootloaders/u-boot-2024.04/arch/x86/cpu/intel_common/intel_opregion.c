// SPDX-License-Identifier: GPL-2.0+
/*
 * Writing IntelGraphicsMem table for ACPI
 *
 * Copyright 2019 Google LLC
 * Modified from coreboot src/soc/intel/gma/opregion.c
 */

#include <common.h>
#include <binman.h>
#include <bloblist.h>
#include <dm.h>
#include <spi_flash.h>
#include <asm/intel_opregion.h>

static char vbt_data[8 << 10];

static int locate_vbt(char **vbtp, int *sizep)
{
	struct binman_entry vbt;
	struct udevice *dev;
	u32 vbtsig = 0;
	int size;
	int ret;

	ret = binman_entry_find("intel-vbt", &vbt);
	if (ret)
		return log_msg_ret("find VBT", ret);
	ret = uclass_first_device_err(UCLASS_SPI_FLASH, &dev);
	if (ret)
		return log_msg_ret("find flash", ret);
	size = vbt.size;
	if (size > sizeof(vbt_data))
		return log_msg_ret("vbt", -E2BIG);
	ret = spi_flash_read_dm(dev, vbt.image_pos, size, vbt_data);
	if (ret)
		return log_msg_ret("read", ret);

	memcpy(&vbtsig, vbt_data, sizeof(vbtsig));
	if (vbtsig != VBT_SIGNATURE) {
		log_err("Missing/invalid signature in VBT data file!\n");
		return -EINVAL;
	}

	log_debug("Found a VBT of %u bytes\n", size);
	*sizep = size;
	*vbtp = vbt_data;

	return 0;
}

/* Write ASLS PCI register and prepare SWSCI register */
static int intel_gma_opregion_register(struct udevice *dev, ulong opregion)
{
	int sci_reg;

	if (!device_active(dev))
		return -ENOENT;

	/*
	 * Intel BIOS Specification
	 * Chapter 5.3.7 "Initialise Hardware State"
	 */
	dm_pci_write_config32(dev, ASLS, opregion);

	/*
	 * Atom-based platforms use a combined SMI/SCI register,
	 * whereas non-Atom platforms use a separate SCI register
	 */
	if (IS_ENABLED(CONFIG_INTEL_GMA_SWSMISCI))
		sci_reg = SWSMISCI;
	else
		sci_reg = SWSCI;

	/*
	 * Intel's Windows driver relies on this:
	 * Intel BIOS Specification
	 * Chapter 5.4 "ASL Software SCI Handler"
	 */
	dm_pci_clrset_config16(dev, sci_reg, GSSCIE, SMISCISEL);

	return 0;
}

int intel_gma_init_igd_opregion(struct udevice *dev,
				struct igd_opregion *opregion)
{
	struct optionrom_vbt *vbt = NULL;
	char *vbt_buf;
	int vbt_size;
	int ret;

	ret = locate_vbt(&vbt_buf, &vbt_size);
	if (ret) {
		log_err("GMA: VBT couldn't be found\n");
		return log_msg_ret("find vbt", ret);
	}
	vbt = (struct optionrom_vbt *)vbt_buf;

	memset(opregion, '\0', sizeof(struct igd_opregion));

	memcpy(&opregion->header.signature, IGD_OPREGION_SIGNATURE,
	       sizeof(opregion->header.signature));
	memcpy(opregion->header.vbios_version, vbt->coreblock_biosbuild,
	       ARRAY_SIZE(vbt->coreblock_biosbuild));
	/* Extended VBT support */
	if (vbt->hdr_vbt_size > sizeof(opregion->vbt.gvd1)) {
		struct optionrom_vbt *ext_vbt;

		ret = bloblist_ensure_size(BLOBLISTT_INTEL_VBT,
					   vbt->hdr_vbt_size, 0,
					   (void **)&ext_vbt);
		if (ret) {
			log_err("GMA: Unable to add Ext VBT to bloblist\n");
			return log_msg_ret("blob", ret);
		}

		memcpy(ext_vbt, vbt, vbt->hdr_vbt_size);
		opregion->mailbox3.rvda = (uintptr_t)ext_vbt;
		opregion->mailbox3.rvds = vbt->hdr_vbt_size;
	} else {
		/* Raw VBT size which can fit in gvd1 */
		printf("copy to %p\n", opregion->vbt.gvd1);
		memcpy(opregion->vbt.gvd1, vbt, vbt->hdr_vbt_size);
	}

	/* 8kb */
	opregion->header.size = sizeof(struct igd_opregion) / 1024;

	/*
	 * Left-shift version field to accommodate Intel Windows driver quirk
	 * when not using a VBIOS.
	 * Required for Legacy boot + NGI, UEFI + NGI, and UEFI + GOP driver.
	 *
	 * No adverse effects when using VBIOS or booting Linux.
	 */
	opregion->header.version = IGD_OPREGION_VERSION << 24;

	/* We just assume we're mobile for now */
	opregion->header.mailboxes = MAILBOXES_MOBILE;

	/* Initialise Mailbox 1 */
	opregion->mailbox1.clid = 1;

	/* Initialise Mailbox 3 */
	opregion->mailbox3.bclp = IGD_BACKLIGHT_BRIGHTNESS;
	opregion->mailbox3.pfit = IGD_FIELD_VALID | IGD_PFIT_STRETCH;
	opregion->mailbox3.pcft = 0; /* should be (IMON << 1) & 0x3e */
	opregion->mailbox3.cblv = IGD_FIELD_VALID | IGD_INITIAL_BRIGHTNESS;
	opregion->mailbox3.bclm[0] = IGD_WORD_FIELD_VALID + 0x0000;
	opregion->mailbox3.bclm[1] = IGD_WORD_FIELD_VALID + 0x0a19;
	opregion->mailbox3.bclm[2] = IGD_WORD_FIELD_VALID + 0x1433;
	opregion->mailbox3.bclm[3] = IGD_WORD_FIELD_VALID + 0x1e4c;
	opregion->mailbox3.bclm[4] = IGD_WORD_FIELD_VALID + 0x2866;
	opregion->mailbox3.bclm[5] = IGD_WORD_FIELD_VALID + 0x327f;
	opregion->mailbox3.bclm[6] = IGD_WORD_FIELD_VALID + 0x3c99;
	opregion->mailbox3.bclm[7] = IGD_WORD_FIELD_VALID + 0x46b2;
	opregion->mailbox3.bclm[8] = IGD_WORD_FIELD_VALID + 0x50cc;
	opregion->mailbox3.bclm[9] = IGD_WORD_FIELD_VALID + 0x5ae5;
	opregion->mailbox3.bclm[10] = IGD_WORD_FIELD_VALID + 0x64ff;

	/* Write ASLS PCI register and prepare SWSCI register */
	ret = intel_gma_opregion_register(dev, (ulong)opregion);
	if (ret)
		return log_msg_ret("write asls", ret);

	return 0;
}
