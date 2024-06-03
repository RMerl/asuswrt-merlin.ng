// SPDX-License-Identifier: GPL-2.0
/*
 * Intel HDA audio (Azalia) for ivybridge
 *
 * Originally from coreboot file bd82x6x/azalia.c
 *
 * Copyright (C) 2008 Advanced Micro Devices, Inc.
 * Copyright (C) 2008-2009 coresystems GmbH
 * Copyright (C) 2011 The ChromiumOS Authors.
 * Copyright 2018 Google LLC
 */

#define LOG_CATEGORY UCLASS_SOUND

#include <common.h>
#include <dm.h>
#include <hda_codec.h>
#include <pch.h>
#include <sound.h>

static int bd82x6x_azalia_probe(struct udevice *dev)
{
	struct pci_child_platdata *plat;
	struct hda_codec_priv *priv;
	struct udevice *pch;
	u32 codec_mask;
	int conf;
	int ret;

	/* Only init after relocation */
	if (!(gd->flags & GD_FLG_RELOC))
		return 0;

	ret = hda_codec_init(dev);
	if (ret) {
		log_debug("Cannot set up HDA codec (err=%d)\n", ret);
		return ret;
	}
	priv = dev_get_priv(dev);

	ret = uclass_first_device_err(UCLASS_PCH, &pch);
	log_debug("PCH %p %s\n", pch, pch->name);
	if (ret)
		return ret;

	conf = pch_ioctl(pch, PCH_REQ_HDA_CONFIG, NULL, 0);
	log_debug("conf = %x\n", conf);
	if (conf >= 0) {
		dm_pci_clrset_config32(dev, 0x120, 7 << 24 | 0xfe,
				       1 << 24 | /* 2 << 24 for server */
				       conf);

		dm_pci_clrset_config16(dev, 0x78, 0, 1 << 1);
	} else {
		log_debug("V1CTL disabled\n");
	}
	dm_pci_clrset_config32(dev, 0x114, 0xfe, 0);

	/* Set VCi enable bit */
	dm_pci_clrset_config32(dev, 0x120, 0, 1U << 31);

	/* Enable HDMI codec */
	dm_pci_clrset_config32(dev, 0xc4, 0, 1 << 1);
	dm_pci_clrset_config8(dev, 0x43, 0, 1 << 6);

	/* Additional programming steps */
	dm_pci_clrset_config32(dev, 0xc4, 0, 1 << 13);
	dm_pci_clrset_config32(dev, 0xc4, 0, 1 << 10);
	dm_pci_clrset_config32(dev, 0xd0, 1U << 31, 0);

	/* Additional step on Panther Point */
	plat = dev_get_parent_platdata(dev);
	if (plat->device == PCI_DEVICE_ID_INTEL_PANTHERPOINT_HDA)
		dm_pci_clrset_config32(dev, 0xc4, 0, 1 << 17);

	dm_pci_write_config8(dev, 0x3c, 0xa); /* unused? */

	/* Audio Control: Select Azalia mode */
	dm_pci_clrset_config8(dev, 0x40, 0, 1);
	dm_pci_clrset_config8(dev, 0x4d, 1 << 7, 0); /* Docking not supported */
	codec_mask = hda_codec_detect(priv->regs);
	log_debug("codec_mask = %02x\n", codec_mask);

	if (codec_mask) {
		ret = hda_codecs_init(dev, priv->regs, codec_mask);
		if (ret) {
			log_err("Codec init failed (err=%d)\n", ret);
			return ret;
		}
	}

	/* Enable dynamic clock gating */
	dm_pci_clrset_config8(dev, 0x43, 7, BIT(2) | BIT(0));

	ret = hda_codec_finish_init(dev);
	if (ret) {
		log_debug("Cannot set up HDA codec (err=%d)\n", ret);
		return ret;
	}

	return 0;
}

static int bd82x6x_azalia_setup(struct udevice *dev)
{
	return 0;
}

int bd82x6x_azalia_start_beep(struct udevice *dev, int frequency_hz)
{
	return hda_codec_start_beep(dev, frequency_hz);
}

int bd82x6x_azalia_stop_beep(struct udevice *dev)
{
	return hda_codec_stop_beep(dev);
}

static const struct sound_ops bd82x6x_azalia_ops = {
	.setup		= bd82x6x_azalia_setup,
	.start_beep	= bd82x6x_azalia_start_beep,
	.stop_beep	= bd82x6x_azalia_stop_beep,
};

static const struct udevice_id bd82x6x_azalia_ids[] = {
	{ .compatible = "intel,hd-audio" },
	{ }
};

U_BOOT_DRIVER(bd82x6x_azalia_drv) = {
	.name		= "bd82x6x-hda",
	.id		= UCLASS_SOUND,
	.of_match	= bd82x6x_azalia_ids,
	.probe		= bd82x6x_azalia_probe,
	.ops		= &bd82x6x_azalia_ops,
	.priv_auto_alloc_size	= sizeof(struct hda_codec_priv),
};
