// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2014 Google, Inc
 *
 * From coreboot, originally based on the Linux kernel (drivers/pci/pci.c).
 *
 * Modifications are:
 * Copyright (C) 2003-2004 Linux Networx
 * (Written by Eric Biederman <ebiederman@lnxi.com> for Linux Networx)
 * Copyright (C) 2003-2006 Ronald G. Minnich <rminnich@gmail.com>
 * Copyright (C) 2004-2005 Li-Ta Lo <ollie@lanl.gov>
 * Copyright (C) 2005-2006 Tyan
 * (Written by Yinghai Lu <yhlu@tyan.com> for Tyan)
 * Copyright (C) 2005-2009 coresystems GmbH
 * (Written by Stefan Reinauer <stepan@coresystems.de> for coresystems GmbH)
 *
 * PCI Bus Services, see include/linux/pci.h for further explanation.
 *
 * Copyright 1993 -- 1997 Drew Eckhardt, Frederic Potter,
 * David Mosberger-Tang
 *
 * Copyright 1997 -- 1999 Martin Mares <mj@atrey.karlin.mff.cuni.cz>
 */

#include <common.h>
#include <bios_emul.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <pci.h>
#include <pci_rom.h>
#include <vbe.h>
#include <video.h>
#include <video_fb.h>
#include <linux/screen_info.h>

#ifdef CONFIG_X86
#include <asm/acpi_s3.h>
DECLARE_GLOBAL_DATA_PTR;
#endif

__weak bool board_should_run_oprom(struct udevice *dev)
{
#if defined(CONFIG_X86) && defined(CONFIG_HAVE_ACPI_RESUME)
	if (gd->arch.prev_sleep_state == ACPI_S3) {
		if (IS_ENABLED(CONFIG_S3_VGA_ROM_RUN))
			return true;
		else
			return false;
	}
#endif

	return true;
}

__weak bool board_should_load_oprom(struct udevice *dev)
{
	return true;
}

__weak uint32_t board_map_oprom_vendev(uint32_t vendev)
{
	return vendev;
}

static int pci_rom_probe(struct udevice *dev, struct pci_rom_header **hdrp)
{
	struct pci_child_platdata *pplat = dev_get_parent_platdata(dev);
	struct pci_rom_header *rom_header;
	struct pci_rom_data *rom_data;
	u16 rom_vendor, rom_device;
	u32 rom_class;
	u32 vendev;
	u32 mapped_vendev;
	u32 rom_address;

	vendev = pplat->vendor << 16 | pplat->device;
	mapped_vendev = board_map_oprom_vendev(vendev);
	if (vendev != mapped_vendev)
		debug("Device ID mapped to %#08x\n", mapped_vendev);

#ifdef CONFIG_VGA_BIOS_ADDR
	rom_address = CONFIG_VGA_BIOS_ADDR;
#else

	dm_pci_read_config32(dev, PCI_ROM_ADDRESS, &rom_address);
	if (rom_address == 0x00000000 || rom_address == 0xffffffff) {
		debug("%s: rom_address=%x\n", __func__, rom_address);
		return -ENOENT;
	}

	/* Enable expansion ROM address decoding. */
	dm_pci_write_config32(dev, PCI_ROM_ADDRESS,
			      rom_address | PCI_ROM_ADDRESS_ENABLE);
#endif
	debug("Option ROM address %x\n", rom_address);
	rom_header = (struct pci_rom_header *)(unsigned long)rom_address;

	debug("PCI expansion ROM, signature %#04x, INIT size %#04x, data ptr %#04x\n",
	      le16_to_cpu(rom_header->signature),
	      rom_header->size * 512, le16_to_cpu(rom_header->data));

	if (le16_to_cpu(rom_header->signature) != PCI_ROM_HDR) {
		printf("Incorrect expansion ROM header signature %04x\n",
		       le16_to_cpu(rom_header->signature));
#ifndef CONFIG_VGA_BIOS_ADDR
		/* Disable expansion ROM address decoding */
		dm_pci_write_config32(dev, PCI_ROM_ADDRESS, rom_address);
#endif
		return -EINVAL;
	}

	rom_data = (((void *)rom_header) + le16_to_cpu(rom_header->data));
	rom_vendor = le16_to_cpu(rom_data->vendor);
	rom_device = le16_to_cpu(rom_data->device);

	debug("PCI ROM image, vendor ID %04x, device ID %04x,\n",
	      rom_vendor, rom_device);

	/* If the device id is mapped, a mismatch is expected */
	if ((pplat->vendor != rom_vendor || pplat->device != rom_device) &&
	    (vendev == mapped_vendev)) {
		printf("ID mismatch: vendor ID %04x, device ID %04x\n",
		       rom_vendor, rom_device);
		/* Continue anyway */
	}

	rom_class = (le16_to_cpu(rom_data->class_hi) << 8) | rom_data->class_lo;
	debug("PCI ROM image, Class Code %06x, Code Type %02x\n",
	      rom_class, rom_data->type);

	if (pplat->class != rom_class) {
		debug("Class Code mismatch ROM %06x, dev %06x\n",
		      rom_class, pplat->class);
	}
	*hdrp = rom_header;

	return 0;
}

/**
 * pci_rom_load() - Load a ROM image and return a pointer to it
 *
 * @rom_header:		Pointer to ROM image
 * @ram_headerp:	Returns a pointer to the image in RAM
 * @allocedp:		Returns true if @ram_headerp was allocated and needs
 *			to be freed
 * @return 0 if OK, -ve on error. Note that @allocedp is set up regardless of
 * the error state. Even if this function returns an error, it may have
 * allocated memory.
 */
static int pci_rom_load(struct pci_rom_header *rom_header,
			struct pci_rom_header **ram_headerp, bool *allocedp)
{
	struct pci_rom_data *rom_data;
	unsigned int rom_size;
	unsigned int image_size = 0;
	void *target;

	*allocedp = false;
	do {
		/* Get next image, until we see an x86 version */
		rom_header = (struct pci_rom_header *)((void *)rom_header +
							    image_size);

		rom_data = (struct pci_rom_data *)((void *)rom_header +
				le16_to_cpu(rom_header->data));

		image_size = le16_to_cpu(rom_data->ilen) * 512;
	} while ((rom_data->type != 0) && (rom_data->indicator == 0));

	if (rom_data->type != 0)
		return -EACCES;

	rom_size = rom_header->size * 512;

#ifdef PCI_VGA_RAM_IMAGE_START
	target = (void *)PCI_VGA_RAM_IMAGE_START;
#else
	target = (void *)malloc(rom_size);
	if (!target)
		return -ENOMEM;
	*allocedp = true;
#endif
	if (target != rom_header) {
		ulong start = get_timer(0);

		debug("Copying VGA ROM Image from %p to %p, 0x%x bytes\n",
		      rom_header, target, rom_size);
		memcpy(target, rom_header, rom_size);
		if (memcmp(target, rom_header, rom_size)) {
			printf("VGA ROM copy failed\n");
			return -EFAULT;
		}
		debug("Copy took %lums\n", get_timer(start));
	}
	*ram_headerp = target;

	return 0;
}

struct vbe_mode_info mode_info;

void setup_video(struct screen_info *screen_info)
{
	struct vesa_mode_info *vesa = &mode_info.vesa;

	/* Sanity test on VESA parameters */
	if (!vesa->x_resolution || !vesa->y_resolution)
		return;

	screen_info->orig_video_isVGA = VIDEO_TYPE_VLFB;

	screen_info->lfb_width = vesa->x_resolution;
	screen_info->lfb_height = vesa->y_resolution;
	screen_info->lfb_depth = vesa->bits_per_pixel;
	screen_info->lfb_linelength = vesa->bytes_per_scanline;
	screen_info->lfb_base = vesa->phys_base_ptr;
	screen_info->lfb_size =
		ALIGN(screen_info->lfb_linelength * screen_info->lfb_height,
		      65536);
	screen_info->lfb_size >>= 16;
	screen_info->red_size = vesa->red_mask_size;
	screen_info->red_pos = vesa->red_mask_pos;
	screen_info->green_size = vesa->green_mask_size;
	screen_info->green_pos = vesa->green_mask_pos;
	screen_info->blue_size = vesa->blue_mask_size;
	screen_info->blue_pos = vesa->blue_mask_pos;
	screen_info->rsvd_size = vesa->reserved_mask_size;
	screen_info->rsvd_pos = vesa->reserved_mask_pos;
}

int dm_pci_run_vga_bios(struct udevice *dev, int (*int15_handler)(void),
			int exec_method)
{
	struct pci_child_platdata *pplat = dev_get_parent_platdata(dev);
	struct pci_rom_header *rom = NULL, *ram = NULL;
	int vesa_mode = -1;
	bool emulate, alloced;
	int ret;

	/* Only execute VGA ROMs */
	if (((pplat->class >> 8) ^ PCI_CLASS_DISPLAY_VGA) & 0xff00) {
		debug("%s: Class %#x, should be %#x\n", __func__, pplat->class,
		      PCI_CLASS_DISPLAY_VGA);
		return -ENODEV;
	}

	if (!board_should_load_oprom(dev))
		return log_msg_ret("Should not load OPROM", -ENXIO);

	ret = pci_rom_probe(dev, &rom);
	if (ret)
		return ret;

	ret = pci_rom_load(rom, &ram, &alloced);
	if (ret)
		goto err;

	if (!board_should_run_oprom(dev)) {
		ret = -ENXIO;
		goto err;
	}

#if defined(CONFIG_FRAMEBUFFER_SET_VESA_MODE) && \
		defined(CONFIG_FRAMEBUFFER_VESA_MODE)
	vesa_mode = CONFIG_FRAMEBUFFER_VESA_MODE;
#endif
	debug("Selected vesa mode %#x\n", vesa_mode);

	if (exec_method & PCI_ROM_USE_NATIVE) {
#ifdef CONFIG_X86
		emulate = false;
#else
		if (!(exec_method & PCI_ROM_ALLOW_FALLBACK)) {
			printf("BIOS native execution is only available on x86\n");
			ret = -ENOSYS;
			goto err;
		}
		emulate = true;
#endif
	} else {
#ifdef CONFIG_BIOSEMU
		emulate = true;
#else
		if (!(exec_method & PCI_ROM_ALLOW_FALLBACK)) {
			printf("BIOS emulation not available - see CONFIG_BIOSEMU\n");
			ret = -ENOSYS;
			goto err;
		}
		emulate = false;
#endif
	}

	if (emulate) {
#ifdef CONFIG_BIOSEMU
		BE_VGAInfo *info;

		ret = biosemu_setup(dev, &info);
		if (ret)
			goto err;
		biosemu_set_interrupt_handler(0x15, int15_handler);
		ret = biosemu_run(dev, (uchar *)ram, 1 << 16, info,
				  true, vesa_mode, &mode_info);
		if (ret)
			goto err;
#endif
	} else {
#if defined(CONFIG_X86) && (CONFIG_IS_ENABLED(X86_32BIT_INIT) || CONFIG_TPL)
		bios_set_interrupt_handler(0x15, int15_handler);

		bios_run_on_x86(dev, (unsigned long)ram, vesa_mode,
				&mode_info);
#endif
	}
	debug("Final vesa mode %#x\n", mode_info.video_mode);
	ret = 0;

err:
	if (alloced)
		free(ram);
	return ret;
}

#ifdef CONFIG_DM_VIDEO
int vbe_setup_video_priv(struct vesa_mode_info *vesa,
			 struct video_priv *uc_priv,
			 struct video_uc_platdata *plat)
{
	if (!vesa->x_resolution)
		return log_msg_ret("No x resolution", -ENXIO);
	uc_priv->xsize = vesa->x_resolution;
	uc_priv->ysize = vesa->y_resolution;
	uc_priv->line_length = vesa->bytes_per_scanline;
	switch (vesa->bits_per_pixel) {
	case 32:
	case 24:
		uc_priv->bpix = VIDEO_BPP32;
		break;
	case 16:
		uc_priv->bpix = VIDEO_BPP16;
		break;
	default:
		return -EPROTONOSUPPORT;
	}
	plat->base = vesa->phys_base_ptr;
	plat->size = vesa->bytes_per_scanline * vesa->y_resolution;

	return 0;
}

int vbe_setup_video(struct udevice *dev, int (*int15_handler)(void))
{
	struct video_uc_platdata *plat = dev_get_uclass_platdata(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	int ret;

	/* If we are running from EFI or coreboot, this can't work */
	if (!ll_boot_init()) {
		printf("Not available (previous bootloader prevents it)\n");
		return -EPERM;
	}
	bootstage_start(BOOTSTAGE_ID_ACCUM_LCD, "vesa display");
	ret = dm_pci_run_vga_bios(dev, int15_handler, PCI_ROM_USE_NATIVE |
					PCI_ROM_ALLOW_FALLBACK);
	bootstage_accum(BOOTSTAGE_ID_ACCUM_LCD);
	if (ret) {
		debug("failed to run video BIOS: %d\n", ret);
		return ret;
	}

	ret = vbe_setup_video_priv(&mode_info.vesa, uc_priv, plat);
	if (ret) {
		debug("No video mode configured\n");
		return ret;
	}

	printf("Video: %dx%dx%d\n", uc_priv->xsize, uc_priv->ysize,
	       mode_info.vesa.bits_per_pixel);

	return 0;
}
#endif
