/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Based on the linux multiplexer framework
 *
 * At its core, a multiplexer (or mux), also known as a data selector, is a
 * device that selects between several analog or digital input signals and
 * forwards it to a single output line. This notion can be extended to work
 * with buses, like a I2C bus multiplexer for example.
 *
 * Copyright (C) 2017 Axentia Technologies AB
 * Author: Peter Rosin <peda@axentia.se>
 *
 * Copyright (C) 2017-2018 Texas Instruments Incorporated - https://www.ti.com/
 * Jean-Jacques Hiblot <jjhiblot@ti.com>
 */

#ifndef _MUX_H_
#define _MUX_H_

#include <linux/errno.h>
#include <linux/types.h>

struct udevice;
struct mux_control;

#if IS_ENABLED(CONFIG_MULTIPLEXER)
/**
 * mux_control_states() - Query the number of multiplexer states.
 * @mux: The mux-control to query.
 *
 * Return: The number of multiplexer states.
 */
unsigned int mux_control_states(struct mux_control *mux);

/**
 * mux_control_select() - Select the given multiplexer state.
 * @mux: The mux-control to request a change of state from.
 * @state: The new requested state.
 *
 * On successfully selecting the mux-control state, it will be locked until
 * there is a call to mux_control_deselect(). If the mux-control is already
 * selected when mux_control_select() is called, the function will indicate
 * -EBUSY
 *
 * Therefore, make sure to call mux_control_deselect() when the operation is
 * complete and the mux-control is free for others to use, but do not call
 * mux_control_deselect() if mux_control_select() fails.
 *
 * Return: 0 when the mux-control state has the requested state or a negative
 * errno on error.
 */
int __must_check mux_control_select(struct mux_control *mux,
				    unsigned int state);
#define mux_control_try_select(mux, state) mux_control_select(mux, state)

/**
 * mux_control_deselect() - Deselect the previously selected multiplexer state.
 * @mux: The mux-control to deselect.
 *
 * It is required that a single call is made to mux_control_deselect() for
 * each and every successful call made to either of mux_control_select() or
 * mux_control_try_select().
 *
 * Return: 0 on success and a negative errno on error. An error can only
 * occur if the mux has an idle state. Note that even if an error occurs, the
 * mux-control is unlocked and is thus free for the next access.
 */
int mux_control_deselect(struct mux_control *mux);

/**
 * mux_get_by_index() = Get a mux by integer index.
 * @dev: The client device.
 * @index: The index of the mux to get.
 * @mux: A pointer to the 'mux_control' struct to initialize.
 *
 * This looks up and initializes a mux. The index is relative to the client
 * device.
 *
 * Return: 0 if OK, or a negative error code.
 */
int mux_get_by_index(struct udevice *dev, int index, struct mux_control **mux);

/**
 * mux_control_get() - Get the mux-control for a device.
 * @dev: The device that needs a mux-control.
 * @mux_name: The name identifying the mux-control.
 * @mux: A pointer to the mux-control pointer.
 *
 * Return: 0 of OK, or a negative error code.
 */
int mux_control_get(struct udevice *dev, const char *name,
		    struct mux_control **mux);

/**
 * mux_control_put() - Put away the mux-control for good.
 * @mux: The mux-control to put away.
 *
 * mux_control_put() reverses the effects of mux_control_get().
 */
void mux_control_put(struct mux_control *mux);

/**
 * devm_mux_control_get() - Get the mux-control for a device, with resource
 *			    management.
 * @dev: The device that needs a mux-control.
 * @mux_name: The name identifying the mux-control.
 *
 * Return: Pointer to the mux-control, or an ERR_PTR with a negative errno.
 */
struct mux_control *devm_mux_control_get(struct udevice *dev,
					 const char *mux_name);
/**
 * dm_mux_init() - Initialize the multiplexer controls to their default state.
 *
 * Return: 0 if OK, -errno otherwise.
 */
int dm_mux_init(void);

#else
unsigned int mux_control_states(struct mux_control *mux)
{
	return -ENOSYS;
}

int __must_check mux_control_select(struct mux_control *mux,
				    unsigned int state)
{
	return -ENOSYS;
}

#define mux_control_try_select(mux, state) mux_control_select(mux, state)

int mux_control_deselect(struct mux_control *mux)
{
	return -ENOSYS;
}

struct mux_control *mux_control_get(struct udevice *dev, const char *mux_name)
{
	return NULL;
}

void mux_control_put(struct mux_control *mux)
{
}

struct mux_control *devm_mux_control_get(struct udevice *dev,
					 const char *mux_name)
{
	return NULL;
}

int dm_mux_init(void)
{
	return -ENOSYS;
}
#endif

#endif
