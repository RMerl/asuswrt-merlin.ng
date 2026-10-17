/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#ifndef _CLK_UCLASS_H
#define _CLK_UCLASS_H

/* See clk.h for background documentation. */

#include <clk.h>

struct ofnode_phandle_args;

/**
 * struct clk_ops - The functions that a clock driver must implement.
 * @of_xlate: Translate a client's device-tree (OF) clock specifier.
 * @request: Request a translated clock.
 * @round_rate: Adjust a rate to the exact rate a clock can provide.
 * @get_rate: Get current clock rate.
 * @set_rate: Set current clock rate.
 * @set_parent: Set current clock parent
 * @enable: Enable a clock.
 * @disable: Disable a clock.
 * @dump: Print clock information.
 *
 * The individual methods are described more fully below.
 */
struct clk_ops {
	int (*of_xlate)(struct clk *clock,
			struct ofnode_phandle_args *args);
	int (*request)(struct clk *clock);
	ulong (*round_rate)(struct clk *clk, ulong rate);
	ulong (*get_rate)(struct clk *clk);
	ulong (*set_rate)(struct clk *clk, ulong rate);
	int (*set_parent)(struct clk *clk, struct clk *parent);
	int (*enable)(struct clk *clk);
	int (*disable)(struct clk *clk);
#if IS_ENABLED(CONFIG_CMD_CLK)
	void (*dump)(struct udevice *dev);
#endif
};

#if 0 /* For documentation only */
/**
 * of_xlate() - Translate a client's device-tree (OF) clock specifier.
 * @clock:	The clock struct to hold the translation result.
 * @args:	The clock specifier values from device tree.
 *
 * The clock core calls this function as the first step in implementing
 * a client's clk_get_by_*() call.
 *
 * If this function pointer is set to NULL, the clock core will use a
 * default implementation, which assumes #clock-cells = <1>, and that
 * the DT cell contains a simple integer clock ID.
 *
 * This function should be a simple translation of @args into @clock->id and
 * (optionally) @clock->data. All other processing, allocation, or error
 * checking should take place in request().
 *
 * At present, the clock API solely supports device-tree. If this
 * changes, other xxx_xlate() functions may be added to support those
 * other mechanisms.
 *
 * Return:
 * * 0 on success
 * * -%EINVAL if @args does not have the correct format. For example, it could
 *   have too many/few arguments.
 * * -%ENOENT if @args has the correct format but cannot be translated. This can
 *   happen if translation involves a table lookup and @args is not present.
 */
int of_xlate(struct clk *clock, struct ofnode_phandle_args *args);

/**
 * request() - Request a translated clock.
 * @clock:	The clock struct to request; this has been filled in by
 *		a previoux xxx_xlate() function call, or by the caller
 *		of clk_request().
 *
 * The clock core calls this function as the second step in
 * implementing a client's clk_get_by_*() call, following a successful
 * xxx_xlate() call, or as the only step in implementing a client's
 * clk_request() call.
 *
 * This is the right place to do bounds checking (rejecting invalid or
 * unimplemented clocks), allocate resources, or perform other setup not done
 * during driver probe(). Most clock drivers should allocate resources in their
 * probe() function, but it is possible to lazily initialize something here.
 *
 * Return:
 * * 0 on success
 * * -%ENOENT, if there is no clock corresponding to @clock->id and
 *   @clock->data.
 */
int request(struct clk *clock);

/**
 * round_rate() - Adjust a rate to the exact rate a clock can provide.
 * @clk:	The clock to query.
 * @rate:	Desired clock rate in Hz.
 *
 * This function returns a new rate which can be provided to set_rate(). This
 * new rate should be the closest rate to @rate which can be set without
 * rounding. The following pseudo-code should hold::
 *
 *   for all rate in range(ULONG_MAX):
 *     rounded = round_rate(clk, rate)
 *     new_rate = set_rate(clk, rate)
 *     assert(IS_ERR_VALUE(new_rate) || new_rate == rounded)
 *
 * Return:
 * * The rounded rate in Hz on success
 * * A negative error value from another API (such as clk_get_rate()). This
 *   function must not return an error for any other reason.
 */
ulong round_rate(struct clk *clk, ulong rate);

/**
 * get_rate() - Get current clock rate.
 * @clk:	The clock to query.
 *
 * This returns the current rate of a clock. If the clock is disabled, it
 * returns the rate at which the clock would run if it was enabled. The
 * following pseudo-code should hold::
 *
 *   disable(clk)
 *   rate = get_rate(clk)
 *   enable(clk)
 *   assert(get_rate(clk) == rate)
 *
 * Return:
 * * The rate of @clk
 * * -%ENOSYS if this function is not implemented for @clk
 * * -%ENOENT if @clk->id is invalid. Prefer using an assert instead, and doing
 *   this check in request().
 * * Another negative error value (such as %EIO or %ECOMM) if the rate could
 *   not be determined due to a bus error.
 */
ulong get_rate(struct clk *clk);

/**
 * set_rate() - Set current clock rate.
 * @clk:	The clock to manipulate.
 * @rate:	New clock rate in Hz.
 *
 * Set the rate of @clk to @rate. The actual rate may be rounded. However,
 * excessive rounding should be avoided. It is left to the driver author's
 * discretion when this function should attempt to round and when it should
 * return an error. For example, a dividing clock might use the following
 * pseudo-logic when implemening this function::
 *
 *   divisor = parent_rate / rate
 *   if divisor < min || divisor > max:
 *     return -EINVAL
 *
 * If there is any concern about rounding, prefer to let consumers make the
 * decision by calling round_rate().
 *
 * Return:
 * * The new rate on success
 * * -%ENOSYS if this function is not implemented for @clk
 * * -%ENOENT if @clk->id is invalid. Prefer using an assert instead, and doing
 *   this check in request().
 * * -%EINVAL if @rate is not valid for @clk.
 * * Another negative error value (such as %EIO or %ECOMM) if the rate could
 *   not be set due to a bus error.
 */
ulong set_rate(struct clk *clk, ulong rate);

/**
 * set_parent() - Set current clock parent
 * @clk:        The clock to manipulate.
 * @parent:     New clock parent.
 *
 * Set the current parent of @clk to @parent. The rate of the clock may be
 * modified by this call. If @clk was enabled before this function, it should
 * remain enabled after this function, although it may be temporarily disabled
 * if necessary.
 *
 * Return:
 * * 0 on success
 * * -%ENOSYS if this function is not implemented for @clk
 * * -%ENOENT if @clk->id or @parent->id is invalid. Prefer using an assert
 *   instead, and doing this check in request().
 * * -%EINVAL if @parent is not a valid parent for @clk.
 * * Another negative error value (such as %EIO or %ECOMM) if the parent could
 *   not be set due to a bus error.
 */
int set_parent(struct clk *clk, struct clk *parent);

/**
 * enable() - Enable a clock.
 * @clk:	The clock to manipulate.
 *
 * Enable (un-gate) the clock. This function should not modify the rate of the
 * clock (see get_rate() for details).
 *
 * Return:
 * * 0 on success
 * * -%ENOSYS if this function is not implemented for @clk
 * * -%ENOENT if @clk->id is invalid. Prefer using an assert instead, and doing
 *   this check in request().
 * * Another negative error value (such as %EIO or %ECOMM) if the clock could
 *   not be enabled due to a bus error.
 */
int enable(struct clk *clk);

/**
 * disable() - Disable a clock.
 * @clk:	The clock to manipulate.
 *
 * Disable (gate) the clock. This function should not modify the rate of the
 * clock (see get_rate() for details).
 *
 * * 0 on success
 * * -%ENOSYS if this function is not implemented for @clk
 * * -%ENOENT if @clk->id is invalid. Prefer using an assert instead, and doing
 *   this check in request().
 * * Another negative error value (such as %EIO or %ECOMM) if the clock could
 *   not be disabled due to a bus error.
 */
int disable(struct clk *clk);

/**
 * dump() - Print clock information.
 * @dev:	The clock device to dump.
 *
 * If present, this function is called by "clk dump" command for each
 * bound device.
 */
void dump(struct udevice *dev);
#endif

#endif
