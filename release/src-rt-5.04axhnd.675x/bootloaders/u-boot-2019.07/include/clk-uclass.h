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
 */
struct clk_ops {
	/**
	 * of_xlate - Translate a client's device-tree (OF) clock specifier.
	 *
	 * The clock core calls this function as the first step in implementing
	 * a client's clk_get_by_*() call.
	 *
	 * If this function pointer is set to NULL, the clock core will use a
	 * default implementation, which assumes #clock-cells = <1>, and that
	 * the DT cell contains a simple integer clock ID.
	 *
	 * At present, the clock API solely supports device-tree. If this
	 * changes, other xxx_xlate() functions may be added to support those
	 * other mechanisms.
	 *
	 * @clock:	The clock struct to hold the translation result.
	 * @args:	The clock specifier values from device tree.
	 * @return 0 if OK, or a negative error code.
	 */
	int (*of_xlate)(struct clk *clock,
			struct ofnode_phandle_args *args);
	/**
	 * request - Request a translated clock.
	 *
	 * The clock core calls this function as the second step in
	 * implementing a client's clk_get_by_*() call, following a successful
	 * xxx_xlate() call, or as the only step in implementing a client's
	 * clk_request() call.
	 *
	 * @clock:	The clock struct to request; this has been fille in by
	 *		a previoux xxx_xlate() function call, or by the caller
	 *		of clk_request().
	 * @return 0 if OK, or a negative error code.
	 */
	int (*request)(struct clk *clock);
	/**
	 * free - Free a previously requested clock.
	 *
	 * This is the implementation of the client clk_free() API.
	 *
	 * @clock:	The clock to free.
	 * @return 0 if OK, or a negative error code.
	 */
	int (*free)(struct clk *clock);
	/**
	 * get_rate() - Get current clock rate.
	 *
	 * @clk:	The clock to query.
	 * @return clock rate in Hz, or -ve error code
	 */
	ulong (*get_rate)(struct clk *clk);
	/**
	 * set_rate() - Set current clock rate.
	 *
	 * @clk:	The clock to manipulate.
	 * @rate:	New clock rate in Hz.
	 * @return new rate, or -ve error code.
	 */
	ulong (*set_rate)(struct clk *clk, ulong rate);
	/**
	 * set_parent() - Set current clock parent
	 *
	 * @clk:        The clock to manipulate.
	 * @parent:     New clock parent.
	 * @return zero on success, or -ve error code.
	 */
	int (*set_parent)(struct clk *clk, struct clk *parent);
	/**
	 * enable() - Enable a clock.
	 *
	 * @clk:	The clock to manipulate.
	 * @return zero on success, or -ve error code.
	 */
	int (*enable)(struct clk *clk);
	/**
	 * disable() - Disable a clock.
	 *
	 * @clk:	The clock to manipulate.
	 * @return zero on success, or -ve error code.
	 */
	int (*disable)(struct clk *clk);
};

#endif
