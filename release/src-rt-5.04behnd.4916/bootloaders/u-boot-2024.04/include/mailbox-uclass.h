/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#ifndef _MAILBOX_UCLASS_H
#define _MAILBOX_UCLASS_H

/* See mailbox.h for background documentation. */

#include <mailbox.h>

struct udevice;

/**
 * struct mbox_ops - The functions that a mailbox driver must implement.
 */
struct mbox_ops {
	/**
	 * of_xlate - Translate a client's device-tree (OF) mailbox specifier.
	 *
	 * The mailbox core calls this function as the first step in
	 * implementing a client's mbox_get_by_*() call.
	 *
	 * If this function pointer is set to NULL, the mailbox core will use
	 * a default implementation, which assumes #mbox-cells = <1>, and that
	 * the DT cell contains a simple integer channel ID.
	 *
	 * At present, the mailbox API solely supports device-tree. If this
	 * changes, other xxx_xlate() functions may be added to support those
	 * other mechanisms.
	 *
	 * @chan:	The channel to hold the translation result.
	 * @args:	The mailbox specifier values from device tree.
	 * @return 0 if OK, or a negative error code.
	 */
	int (*of_xlate)(struct mbox_chan *chan,
			struct ofnode_phandle_args *args);
	/**
	 * request - Request a translated channel.
	 *
	 * The mailbox core calls this function as the second step in
	 * implementing a client's mbox_get_by_*() call, following a successful
	 * xxx_xlate() call.
	 *
	 * @chan:	The channel to request; this has been filled in by a
	 *		previoux xxx_xlate() function call.
	 * @return 0 if OK, or a negative error code.
	 */
	int (*request)(struct mbox_chan *chan);
	/**
	 * rfree - Free a previously requested channel.
	 *
	 * This is the implementation of the client mbox_free() API.
	 *
	 * @chan:	The channel to free.
	 * @return 0 if OK, or a negative error code.
	 */
	int (*rfree)(struct mbox_chan *chan);
	/**
	* send - Send a message over a mailbox channel
	*
	* @chan:	The channel to send to the message to.
	* @data:	A pointer to the message to send.
	* Return: 0 if OK, or a negative error code.
	*/
	int (*send)(struct mbox_chan *chan, const void *data);
	/**
	* recv - Receive any available message from the channel.
	*
	* This function does not block. If not message is immediately
	* available, the function should return an error.
	*
	* @chan:	The channel to receive to the message from.
	* @data:	A pointer to the buffer to hold the received message.
	* Return: 0 if OK, -ENODATA if no message was available, or a negative
	* error code.
	*/
	int (*recv)(struct mbox_chan *chan, void *data);
};

#endif
