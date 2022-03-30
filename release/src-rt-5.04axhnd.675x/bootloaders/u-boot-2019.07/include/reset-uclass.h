/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#ifndef _RESET_UCLASS_H
#define _RESET_UCLASS_H

/* See reset.h for background documentation. */

#include <reset.h>

struct ofnode_phandle_args;
struct udevice;

/**
 * struct reset_ops - The functions that a reset controller driver must
 * implement.
 */
struct reset_ops {
	/**
	 * of_xlate - Translate a client's device-tree (OF) reset specifier.
	 *
	 * The reset core calls this function as the first step in implementing
	 * a client's reset_get_by_*() call.
	 *
	 * If this function pointer is set to NULL, the reset core will use a
	 * default implementation, which assumes #reset-cells = <1>, and that
	 * the DT cell contains a simple integer reset signal ID.
	 *
	 * At present, the reset API solely supports device-tree. If this
	 * changes, other xxx_xlate() functions may be added to support those
	 * other mechanisms.
	 *
	 * @reset_ctl:	The reset control struct to hold the translation result.
	 * @args:	The reset specifier values from device tree.
	 * @return 0 if OK, or a negative error code.
	 */
	int (*of_xlate)(struct reset_ctl *reset_ctl,
			struct ofnode_phandle_args *args);
	/**
	 * request - Request a translated reset control.
	 *
	 * The reset core calls this function as the second step in
	 * implementing a client's reset_get_by_*() call, following a
	 * successful xxx_xlate() call.
	 *
	 * @reset_ctl:	The reset control struct to request; this has been
	 *		filled in by a previoux xxx_xlate() function call.
	 * @return 0 if OK, or a negative error code.
	 */
	int (*request)(struct reset_ctl *reset_ctl);
	/**
	 * free - Free a previously requested reset control.
	 *
	 * This is the implementation of the client reset_free() API.
	 *
	 * @reset_ctl:	The reset control to free.
	 * @return 0 if OK, or a negative error code.
	 */
	int (*free)(struct reset_ctl *reset_ctl);
	/**
	 * rst_assert - Assert a reset signal.
	 *
	 * Note: This function is named rst_assert not assert to avoid
	 * conflicting with global macro assert().
	 *
	 * @reset_ctl:	The reset signal to assert.
	 * @return 0 if OK, or a negative error code.
	 */
	int (*rst_assert)(struct reset_ctl *reset_ctl);
	/**
	 * rst_deassert - Deassert a reset signal.
	 *
	 * @reset_ctl:	The reset signal to deassert.
	 * @return 0 if OK, or a negative error code.
	 */
	int (*rst_deassert)(struct reset_ctl *reset_ctl);
	/**
	 * rst_status - Check reset signal status.
	 *
	 * @reset_ctl:	The reset signal to check.
	 * @return 0 if deasserted, positive if asserted, or a negative
	 *           error code.
	 */
	int (*rst_status)(struct reset_ctl *reset_ctl);
};

#endif
