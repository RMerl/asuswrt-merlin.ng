/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#ifndef _MAILBOX_H
#define _MAILBOX_H

/**
 * A mailbox is a hardware mechanism for transferring small fixed-size messages
 * and/or notifications between the CPU on which U-Boot runs and some other
 * device such as an auxiliary CPU running firmware or a hardware module.
 *
 * Data transfer is optional; a mailbox may consist solely of a notification
 * mechanism. When data transfer is implemented, it is via HW registers or
 * FIFOs, rather than via RAM-based buffers. The mailbox API generally
 * implements any communication protocol enforced solely by hardware, and
 * leaves any higher-level protocols to other layers.
 *
 * A mailbox channel is a bi-directional mechanism that can send a message or
 * notification to a single specific remote entity, and receive messages or
 * notifications from that entity. The size, content, and format of such
 * messages is defined by the mailbox implementation, or the remote entity with
 * which it communicates; there is no general standard at this API level.
 *
 * A driver that implements UCLASS_MAILBOX is a mailbox provider. A provider
 * will often implement multiple separate mailbox channels, since the hardware
 * it manages often has this capability. mailbox-uclass.h describes the
 * interface which mailbox providers must implement.
 *
 * Mailbox consumers/clients generate and send, or receive and process,
 * messages. This header file describes the API used by clients.
 */

struct udevice;

/**
 * struct mbox_chan - A handle to a single mailbox channel.
 *
 * Clients provide storage for channels. The content of the channel structure
 * is managed solely by the mailbox API and mailbox drivers. A mailbox channel
 * is initialized by "get"ing the mailbox. The channel struct is passed to all
 * other mailbox APIs to identify which mailbox to operate upon.
 *
 * @dev: The device which implements the mailbox.
 * @id: The mailbox channel ID within the provider.
 * @con_priv: Hook for controller driver to attach private data
 *
 * Currently, the mailbox API assumes that a single integer ID is enough to
 * identify and configure any mailbox channel for any mailbox provider. If this
 * assumption becomes invalid in the future, the struct could be expanded to
 * either (a) add more fields to allow mailbox providers to store additional
 * information, or (b) replace the id field with an opaque pointer, which the
 * provider would dynamically allocated during its .of_xlate op, and process
 * during is .request op. This may require the addition of an extra op to clean
 * up the allocation.
 */
struct mbox_chan {
	struct udevice *dev;
	/* Written by of_xlate.*/
	unsigned long id;
	void *con_priv;
};

/**
 * mbox_get_by_index - Get/request a mailbox by integer index
 *
 * This looks up and requests a mailbox channel. The index is relative to the
 * client device; each device is assumed to have n mailbox channels associated
 * with it somehow, and this function finds and requests one of them. The
 * mapping of client device channel indices to provider channels may be via
 * device-tree properties, board-provided mapping tables, or some other
 * mechanism.
 *
 * @dev:	The client device.
 * @index:	The index of the mailbox channel to request, within the
 *		client's list of channels.
 * @chan	A pointer to a channel object to initialize.
 * Return: 0 if OK, or a negative error code.
 */
int mbox_get_by_index(struct udevice *dev, int index, struct mbox_chan *chan);

/**
 * mbox_get_by_name - Get/request a mailbox by name
 *
 * This looks up and requests a mailbox channel. The name is relative to the
 * client device; each device is assumed to have n mailbox channels associated
 * with it somehow, and this function finds and requests one of them. The
 * mapping of client device channel names to provider channels may be via
 * device-tree properties, board-provided mapping tables, or some other
 * mechanism.
 *
 * @dev:	The client device.
 * @name:	The name of the mailbox channel to request, within the client's
 *		list of channels.
 * @chan	A pointer to a channel object to initialize.
 * Return: 0 if OK, or a negative error code.
 */
int mbox_get_by_name(struct udevice *dev, const char *name,
		     struct mbox_chan *chan);

/**
 * mbox_free - Free a previously requested mailbox channel.
 *
 * @chan:	A channel object that was previously successfully requested by
 *		calling mbox_get_by_*().
 * Return: 0 if OK, or a negative error code.
 */
int mbox_free(struct mbox_chan *chan);

/**
 * mbox_send - Send a message over a mailbox channel
 *
 * This function will send a message to the remote entity. It may return before
 * the remote entity has received and/or processed the message.
 *
 * @chan:	A channel object that was previously successfully requested by
 *		calling mbox_get_by_*().
 * @data:	A pointer to the message to transfer. The format and size of
 *		the memory region pointed at by @data is determined by the
 *		mailbox provider. Providers that solely transfer notifications
 *		will ignore this parameter.
 * Return: 0 if OK, or a negative error code.
 */
int mbox_send(struct mbox_chan *chan, const void *data);

/**
 * mbox_recv - Receive any available message from a mailbox channel
 *
 * This function will wait (up to the specified @timeout_us) for a message to
 * be sent by the remote entity, and write the content of any such message
 * into a caller-provided buffer.
 *
 * @chan:	A channel object that was previously successfully requested by
 *		calling mbox_get_by_*().
 * @data:	A pointer to the buffer to receive the message. The format and
 *		size of the memory region pointed at by @data is determined by
 *		the mailbox provider. Providers that solely transfer
 *		notifications will ignore this parameter.
 * @timeout_us:	The maximum time to wait for a message to be available, in
 *		micro-seconds. A value of 0 does not wait at all.
 * Return: 0 if OK, -ENODATA if no message was available, or a negative error
 * code.
 */
int mbox_recv(struct mbox_chan *chan, void *data, ulong timeout_us);

#endif
