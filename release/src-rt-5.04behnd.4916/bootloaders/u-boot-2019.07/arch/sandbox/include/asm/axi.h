/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2018
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#ifndef __asm_axi_h
#define __asm_axi_h

#define axi_emul_get_ops(dev)	((struct axi_emul_ops *)(dev)->driver->ops)

/**
 * axi_sandbox_get_emul() - Retrieve a pointer to a AXI emulation device
 * @bus:     The AXI bus from which to retrieve a emulation device
 * @address: The address of a transfer that should be handled by a emulation
 *	     device
 * @length:  The data width of a transfer that should be handled by a emulation
 *	     device
 * @emulp:   Pointer to a buffer receiving the emulation device that handles
 *	     the transfer specified by the address and length parameters
 *
 * To test the AXI uclass, we implement a simple AXI emulation device, which is
 * a virtual device on a AXI bus that exposes a simple storage interface: When
 * reading and writing from the device, the addresses are translated to offsets
 * within the device's storage. For write accesses the data is written to the
 * specified storage offset, and for read accesses the data is read from the
 * specified storage offset.
 *
 * A DTS entry might look like this:
 *
 * axi: axi@0 {
 *	compatible = "sandbox,axi";
 *	#address-cells = <0x1>;
 *	#size-cells = <0x1>;
 *	store@0 {
 *		compatible = "sandbox,sandbox_store";
 *		reg = <0x0 0x400>;
 *	};
 * };
 *
 * This function may then be used to retrieve the pointer to the sandbox_store
 * emulation device given the AXI bus device, and the data (address, data
 * width) of a AXI transfer which should be handled by a emulation device.
 *
 * Return: 0 of OK, -ENODEV if no device capable of handling the specified
 *	   transfer exists or the device could not be retrieved
 */
int axi_sandbox_get_emul(struct udevice *bus, ulong address, uint length,
			 struct udevice **emulp);
/**
 * axi_get_store() - Get address of internal storage of a emulated AXI device
 * @dev:	Emulated AXI device to get the pointer of the internal storage
 *		for.
 * @storep:	Pointer to the internal storage of the emulated AXI device.
 *
 * To preset or read back the contents internal storage of the emulated AXI
 * device, this function returns the pointer to the storage. Changes to the
 * contents of the storage are reflected when using the AXI read/write API
 * methods, and vice versa, so by using this method expected read data can be
 * set up in advance, and written data can be checked in unit tests.
 *
 * Return: 0 if OK, -ve on error.
 */
int axi_get_store(struct udevice *dev, u8 **storep);

#endif /* __asm_axi_h */
