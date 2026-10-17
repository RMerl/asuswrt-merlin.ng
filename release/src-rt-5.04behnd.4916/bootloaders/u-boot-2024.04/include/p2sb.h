/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __p2sb_h
#define __p2sb_h

/* Port Id lives in bits 23:16 and register offset lives in 15:0 of address */
#define PCR_PORTID_SHIFT	16

#if !defined(__ACPI__)

/* These registers contain IOAPIC and HPET devfn */
#define PCH_P2SB_IBDF		0x6c
#define PCH_P2SB_HBDF		0x70

/**
 * struct p2sb_child_plat - Information about each child of a p2sb device
 *
 * @pid: Port ID for this child
 */
struct p2sb_child_plat {
	uint pid;
};

/**
 * struct p2sb_uc_priv - information for the uclass about each device
 *
 * This must be set up by the driver when it is probed
 *
 * @mmio_base: Base address of P2SB region
 */
struct p2sb_uc_priv {
	uint mmio_base;
};

/**
 * struct p2sb_ops - Operations for the P2SB
 */
struct p2sb_ops {
	/**
	 * set_hide() - Set/clear the 'hide' bit on the p2sb
	 *
	 * This device can be hidden from the PCI bus if needed. This method
	 * can be called before the p2sb is probed.
	 *
	 * @dev: P2SB device
	 * @hide: true to hide the device, false to show it
	 * @return 0 if OK, -ve on error
	 */
	int (*set_hide)(struct udevice *dev, bool hide);
};

#define p2sb_get_ops(dev)        ((struct p2sb_ops *)(dev)->driver->ops)

/**
 * p2sb_set_hide() - Set/clear the 'hide' bit on the p2sb
 *
 * This device can be hidden from the PCI bus if needed. This method
 * can be called before the p2sb is probed.
 *
 * @dev: P2SB device
 * @hide: true to hide the device, false to show it
 * Return: 0 if OK, -ve on error
 */
int p2sb_set_hide(struct udevice *dev, bool hide);

/**
 * pcr_read32/16/8() - Read from a PCR device
 *
 * Reads data from a PCR device within the P2SB
 *
 * @dev: Device to read from
 * @offset: Offset within device to read
 * Return: value read
 */
uint pcr_read32(struct udevice *dev, uint offset);
uint pcr_read16(struct udevice *dev, uint offset);
uint pcr_read8(struct udevice *dev, uint offset);

/**
 * pcr_read32/16/8() - Write to a PCR device
 *
 * Writes data to a PCR device within the P2SB
 *
 * @dev: Device to write to
 * @offset: Offset within device to write
 * @data: Data to write
 */
void pcr_write32(struct udevice *dev, uint offset, uint data);
void pcr_write16(struct udevice *dev, uint offset, uint data);
void pcr_write8(struct udevice *dev, uint offset, uint data);

/**
 * pcr_clrsetbits32/16/8() - Update a PCR device
 *
 * Updates dat in a PCR device within the P2SB
 *
 * This reads from the device, clears and set bits, then writes back.
 *
 * new_data = (old_data & ~clr) | set
 *
 * @dev: Device to update
 * @offset: Offset within device to update
 * @clr: Bits to clear after reading
 * @set: Bits to set before writing
 */
void pcr_clrsetbits32(struct udevice *dev, uint offset, uint clr, uint set);
void pcr_clrsetbits16(struct udevice *dev, uint offset, uint clr, uint set);
void pcr_clrsetbits8(struct udevice *dev, uint offset, uint clr, uint set);

static inline void pcr_setbits32(struct udevice *dev, uint offset, uint set)
{
	return pcr_clrsetbits32(dev, offset, 0, set);
}

static inline void pcr_setbits16(struct udevice *dev, uint offset, uint set)
{
	return pcr_clrsetbits16(dev, offset, 0, set);
}

static inline void pcr_setbits8(struct udevice *dev, uint offset, uint set)
{
	return pcr_clrsetbits8(dev, offset, 0, set);
}

static inline void pcr_clrbits32(struct udevice *dev, uint offset, uint clr)
{
	return pcr_clrsetbits32(dev, offset, clr, 0);
}

static inline void pcr_clrbits16(struct udevice *dev, uint offset, uint clr)
{
	return pcr_clrsetbits16(dev, offset, clr, 0);
}

static inline void pcr_clrbits8(struct udevice *dev, uint offset, uint clr)
{
	return pcr_clrsetbits8(dev, offset, clr, 0);
}

/**
 * p2sb_set_port_id() - Set the port ID for a p2sb child device
 *
 * This must be called in a device's bind() method when OF_PLATDATA is used
 * since the uclass cannot access the device's of-platdata.
 *
 * @dev: Child device (whose parent is UCLASS_P2SB)
 * @portid: Port ID of child device
 * Return: 0 if OK, -ENODEV is the p2sb device could not be found
 */
int p2sb_set_port_id(struct udevice *dev, int portid);

/**
 * p2sb_get_port_id() - Get the port ID for a p2sb child device
 *
 * @dev: Child device (whose parent is UCLASS_P2SB)
 * Return: Port ID of that child
 */
int p2sb_get_port_id(struct udevice *dev);

/**
 * pcr_reg_address() Convert an offset in p2sb space to an absolute address
 *
 * @dev: Child device (whose parent is UCLASS_P2SB)
 * @offset: Offset within that child's address space
 * Return: pointer to that offset within the child's address space
 */
void *pcr_reg_address(struct udevice *dev, uint offset);

#endif /* !__ACPI__ */

#endif
