/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Verified Boot for Embedded (VBE) vbe-simple common file
 *
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __VBE_SIMPLE_H
#define __VBE_SIMPLE_H

enum {
	MAX_VERSION_LEN		= 256,

	NVD_HDR_VER_SHIFT	= 0,
	NVD_HDR_VER_MASK	= 0xf,
	NVD_HDR_SIZE_SHIFT	= 4,
	NVD_HDR_SIZE_MASK	= 0xf << NVD_HDR_SIZE_SHIFT,

	/* Firmware key-version is in the top 16 bits of fw_ver */
	FWVER_KEY_SHIFT		= 16,
	FWVER_FW_MASK		= 0xffff,

	NVD_HDR_VER_CUR		= 1,	/* current version */
};

/** struct simple_priv - information read from the device tree */
struct simple_priv {
	u32 area_start;
	u32 area_size;
	u32 skip_offset;
	u32 state_offset;
	u32 state_size;
	u32 version_offset;
	u32 version_size;
	const char *storage;
};

/** struct simple_state - state information read from media
 *
 * @fw_version: Firmware version string
 * @fw_vernum: Firmware version number
 */
struct simple_state {
	char fw_version[MAX_VERSION_LEN];
	u32 fw_vernum;
};

/**
 * vbe_simple_read_fw_bootflow() - Read a bootflow for firmware
 *
 * Locates and loads the firmware image (FIT) needed for the next phase. The FIT
 * should ideally use external data, to reduce the amount of it that needs to be
 * read.
 *
 * @bdev: bootdev device containing the firmwre
 * @blow: Place to put the created bootflow, on success
 * @return 0 if OK, -ve on error
 */
int vbe_simple_read_bootflow_fw(struct udevice *dev, struct bootflow *bflow);

/**
 * vbe_simple_read_state() - Read the VBE simple state information
 *
 * @dev: VBE bootmeth
 * @state: Place to put the state
 * @return 0 if OK, -ve on error
 */
int vbe_simple_read_state(struct udevice *dev, struct simple_state *state);

#endif /* __VBE_SIMPLE_H */
