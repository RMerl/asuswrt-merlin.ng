/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __pch_h
#define __pch_h

#include <linux/bitops.h>
#define PCH_RCBA		0xf0

#define BIOS_CTRL_BIOSWE	BIT(0)

/* All the supported PCH ioctls */
enum pch_req_t {
	/* Returns HDA config info if Azalia V1CTL enabled, -ENOENT if not */
	PCH_REQ_HDA_CONFIG,

	/* Fills out a struct pch_pmbase_info if available */
	PCH_REQ_PMBASE_INFO,

	PCH_REQ_TEST1,		/* Test requests for sandbox driver */
	PCH_REQ_TEST2,
	PCH_REQ_TEST3,

	PCH_REQ_COUNT,		/* Number of ioctrls supported */
};

/**
 * struct pch_pmbase_info - Information filled in by PCH_REQ_PMBASE_INFO
 *
 * @pmbase: IO address of power-management controller
 * @gpio0_en_ofs: Offset of GPIO0 enable register
 * @pm1_sts_ofs: Offset of status register
 * @pm1_cnt_ofs: Offset of control register
 */
struct pch_pmbase_info {
	u16 base;
	u8 gpio0_en_ofs;
	u8 pm1_sts_ofs;
	u8 pm1_cnt_ofs;
};

/**
 * struct pch_ops - Operations for the Platform Controller Hub
 *
 * Consider using ioctl() to add rarely used or driver-specific operations.
 */
struct pch_ops {
	/**
	 * get_spi_base() - get the address of SPI base
	 *
	 * @dev:	PCH device to check
	 * @sbasep:	Returns address of SPI base if available, else 0
	 * @return 0 if OK, -ve on error (e.g. there is no SPI base)
	 */
	int (*get_spi_base)(struct udevice *dev, ulong *sbasep);

	/**
	 * set_spi_protect() - set whether SPI flash is protected or not
	 *
	 * @dev:	PCH device to adjust
	 * @protect:	true to protect, false to unprotect
	 *
	 * @return 0 on success, -ENOSYS if not implemented
	 */
	int (*set_spi_protect)(struct udevice *dev, bool protect);

	/**
	 * get_gpio_base() - get the address of GPIO base
	 *
	 * @dev:	PCH device to check
	 * @gbasep:	Returns address of GPIO base if available, else 0
	 * @return 0 if OK, -ve on error (e.g. there is no GPIO base)
	 */
	int (*get_gpio_base)(struct udevice *dev, u32 *gbasep);

	/**
	 * get_io_base() - get the address of IO base
	 *
	 * @dev:	PCH device to check
	 * @iobasep:	Returns address of IO base if available, else 0
	 * @return 0 if OK, -ve on error (e.g. there is no IO base)
	 */
	int (*get_io_base)(struct udevice *dev, u32 *iobasep);

	/**
	 * ioctl() - perform misc read/write operations
	 *
	 * This is a catch-all operation intended to avoid adding lots of
	 * methods to this uclass, of which few are commonly used. Uncommon
	 * operations that pertain only to a few devices in this uclass should
	 * use this method instead of adding new methods.
	 *
	 * @dev:	PCH device to check
	 * @req:	PCH request ID
	 * @data:	Input/output data
	 * @size:	Size of input data (and maximum size of output data)
	 * @return size of output data on sucesss, -ve on error
	 */
	int (*ioctl)(struct udevice *dev, enum pch_req_t req, void *data,
		     int size);
};

#define pch_get_ops(dev)        ((struct pch_ops *)(dev)->driver->ops)

/**
 * pch_get_spi_base() - get the address of SPI base
 *
 * @dev:	PCH device to check
 * @sbasep:	Returns address of SPI base if available, else 0
 * Return: 0 if OK, -ve on error (e.g. there is no SPI base)
 */
int pch_get_spi_base(struct udevice *dev, ulong *sbasep);

/**
 * set_spi_protect() - set whether SPI flash is protected or not
 *
 * @dev:	PCH device to adjust
 * @protect:	true to protect, false to unprotect
 *
 * Return: 0 on success, -ENOSYS if not implemented
 */
int pch_set_spi_protect(struct udevice *dev, bool protect);

/**
 * pch_get_gpio_base() - get the address of GPIO base
 *
 * @dev:	PCH device to check
 * @gbasep:	Returns address of GPIO base if available, else 0
 * Return: 0 if OK, -ve on error (e.g. there is no GPIO base)
 */
int pch_get_gpio_base(struct udevice *dev, u32 *gbasep);

/**
 * pch_get_io_base() - get the address of IO base
 *
 * @dev:	PCH device to check
 * @iobasep:	Returns address of IO base if available, else 0
 * Return: 0 if OK, -ve on error (e.g. there is no IO base)
 */
int pch_get_io_base(struct udevice *dev, u32 *iobasep);

/**
 * pch_ioctl() - perform misc read/write operations
 *
 * This is a catch-all operation intended to avoid adding lots of
 * methods to this uclass, of which few are commonly used. Uncommon
 * operations that pertain only to a few devices in this uclass should
 * use this method instead of adding new methods.
 *
 * @dev:	PCH device to check
 * @req:	PCH request ID
 * @data:	Input/output data
 * @size:	Size of input data (and maximum size of output data)
 * Return: size of output data on sucesss, -ve on error
 */
int pch_ioctl(struct udevice *dev, ulong req, void *data, int size);

#endif
