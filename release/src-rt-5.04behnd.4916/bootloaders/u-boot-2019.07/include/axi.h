/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2017, 2018
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#ifndef _AXI_H_
#define _AXI_H_

/**
 * enum axi_size_t - Determine size of AXI transfer
 * @AXI_SIZE_8:  AXI sransfer is 8-bit wide
 * @AXI_SIZE_16: AXI sransfer is 16-bit wide
 * @AXI_SIZE_32: AXI sransfer is 32-bit wide
 */
enum axi_size_t {
	AXI_SIZE_8,
	AXI_SIZE_16,
	AXI_SIZE_32,
};

struct axi_ops {
	/**
	 * read() - Read a single value from a specified address on a AXI bus
	 * @dev:	AXI bus to read from.
	 * @address:	The address to read from.
	 * @data:	Pointer to a variable that takes the data value read
	 *		from the address on the AXI bus.
	 * @size:	The size of the data to be read.
	 *
	 * Return: 0 if OK, -ve on error.
	 */
	int (*read)(struct udevice *dev, ulong address, void *data,
		    enum axi_size_t size);

	/**
	 * write() - Write a single value to a specified address on a AXI bus
	 * @dev:	AXI bus to write to.
	 * @address:	The address to write to.
	 * @data:	Pointer to the data value to be written to the address
	 *		on the AXI bus.
	 * @size:	The size of the data to write.
	 *
	 * Return 0 if OK, -ve on error.
	 */
	int (*write)(struct udevice *dev, ulong address, void *data,
		     enum axi_size_t size);
};

#define axi_get_ops(dev)	((struct axi_ops *)(dev)->driver->ops)

/**
 * axi_read() - Read a single value from a specified address on a AXI bus
 * @dev:	AXI bus to read from.
 * @address:	The address to read from.
 * @data:	Pointer to a variable that takes the data value read from the
 *              address on the AXI bus.
 * @size:	The size of the data to write.
 *
 * Return: 0 if OK, -ve on error.
 */
int axi_read(struct udevice *dev, ulong address, void *data,
	     enum axi_size_t size);

/**
 * axi_write() - Write a single value to a specified address on a AXI bus
 * @dev:	AXI bus to write to.
 * @address:	The address to write to.
 * @data:	Pointer to the data value to be written to the address on the
 *		AXI bus.
 * @size:	The size of the data to write.
 *
 * Return: 0 if OK, -ve on error.
 */
int axi_write(struct udevice *dev, ulong address, void *data,
	      enum axi_size_t size);

struct axi_emul_ops {
	/**
	 * read() - Read a single value from a specified address on a AXI bus
	 * @dev:	AXI bus to read from.
	 * @address:	The address to read from.
	 * @data:	Pointer to a variable that takes the data value read
	 *		from the address on the AXI bus.
	 * @size:	The size of the data to be read.
	 *
	 * Return: 0 if OK, -ve on error.
	 */
	int (*read)(struct udevice *dev, ulong address, void *data,
		    enum axi_size_t size);

	/**
	 * write() - Write a single value to a specified address on a AXI bus
	 * @dev:	AXI bus to write to.
	 * @address:	The address to write to.
	 * @data:	Pointer to the data value to be written to the address
	 *		on the AXI bus.
	 * @size:	The size of the data to write.
	 *
	 * Return: 0 if OK, -ve on error.
	 */
	int (*write)(struct udevice *dev, ulong address, void *data,
		     enum axi_size_t size);

	/**
	 * get_store() - Get address of internal storage of a emulated AXI
	 *		 device
	 * @dev:	Emulated AXI device to get the pointer of the internal
	 *		storage for.
	 * @storep:	Pointer to the internal storage of the emulated AXI
	 *		device.
	 *
	 * Return: 0 if OK, -ve on error.
	 */
	int (*get_store)(struct udevice *dev, u8 **storep);
};

#endif
