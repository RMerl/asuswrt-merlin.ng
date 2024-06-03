/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2017
 * Mario Six,  Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#ifndef _GDSYS_SOC_H_
#define _GDSYS_SOC_H_

/**
 * gdsys_soc_get_fpga() - Retrieve pointer to parent bus' FPGA device
 * @child:	The child device on the FPGA bus needing access to the FPGA.
 * @fpga:	Pointer to the retrieved FPGA device.
 *
 * To access their register maps, devices on gdsys soc buses usually have
 * facilitate the accessor function of the IHS FPGA their parent bus is
 * attached to. To access the FPGA device from within the bus' children, this
 * function returns a pointer to it.
 *
 * Return: 0 on success, -ve on failure
 */
int gdsys_soc_get_fpga(struct udevice *child, struct udevice **fpga);
#endif /* _GDSYS_SOC_H_ */
