/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Based on the linux multiplexer framework
 *
 * Copyright (C) 2017 Axentia Technologies AB
 * Author: Peter Rosin <peda@axentia.se>
 *
 * Copyright (C) 2017-2018 Texas Instruments Incorporated - https://www.ti.com/
 * Jean-Jacques Hiblot <jjhiblot@ti.com>
 */

#ifndef _MUX_INTERNAL_H
#define _MUX_INTERNAL_H

/* See mux.h for background documentation. */

struct ofnode_phandle_args;

/**
 * struct mux_chip -	Represents a chip holding mux controllers.
 * @controllers:	Number of mux controllers handled by the chip.
 * @mux:		Array of mux controllers that are handled.
 *
 * This a per-device uclass-private data.
 */
struct mux_chip {
	unsigned int controllers;
	struct mux_control *mux;
};

/**
 * struct mux_control_ops -	Mux controller operations for a mux chip.
 * @set:			Set the state of the given mux controller.
 */
struct mux_control_ops {
	/**
	 * set - Apply a state to a multiplexer control
	 *
	 * @mux:	A multiplexer control
	 * @return 0 if OK, or a negative error code.
	 */
	int (*set)(struct mux_control *mux, int state);

	/**
	 * of_xlate - Translate a client's device-tree (OF) multiplexer
	 * specifier.
	 *
	 * If this function pointer is set to NULL, the multiplexer core will
	 * use a default implementation, which assumes #mux-control-cells = <1>
	 * and that the DT cell contains a simple integer channel ID.
	 *
	 * @dev_mux:	The multiplexer device. A single device may handle
	 *              several multiplexer controls.
	 * @args:	The multiplexer specifier values from device tree.
	 * @muxp:	(out) A multiplexer control
	 * @return 0 if OK, or a negative error code.
	 */
	int (*of_xlate)(struct mux_chip *dev_mux,
			struct ofnode_phandle_args *args,
			struct mux_control **muxp);
};

/**
 * struct mux_control -	Represents a mux controller.
 * @in_use:		Whether the mux controller is in use or not.
 * @dev:		The client device.
 * @cached_state:	The current mux controller state, or -1 if none.
 * @states:		The number of mux controller states.
 * @idle_state:		The mux controller state to use when inactive, or one
 *			of MUX_IDLE_AS_IS and MUX_IDLE_DISCONNECT.
 * @id:			The index of the mux controller within the mux chip
 *			it is a part of.
 *
 * Mux drivers may only change @states and @idle_state, and may only do so
 * between allocation and registration of the mux controller. Specifically,
 * @cached_state is internal to the mux core and should never be written by
 * mux drivers.
 */
struct mux_control {
	bool	in_use;
	struct udevice *dev;
	int cached_state;
	unsigned int states;
	int idle_state;
	int id;
};

/**
 * mux_control_get_index() - Get the index of the given mux controller
 * @mux:		The mux-control to get the index for.
 *
 * Return: The index of the mux controller within the mux chip the mux
 * controller is a part of.
 */
static inline unsigned int mux_control_get_index(struct mux_control *mux)
{
	return mux->id;
}

/**
 * mux_alloc_controllers() - Allocate the given number of mux controllers.
 * @dev:		The client device.
 * controllers:		Number of controllers to allocate.
 *
 * Return: 0 of OK, -errno otherwise.
 */
int mux_alloc_controllers(struct udevice *dev, unsigned int controllers);

#endif
