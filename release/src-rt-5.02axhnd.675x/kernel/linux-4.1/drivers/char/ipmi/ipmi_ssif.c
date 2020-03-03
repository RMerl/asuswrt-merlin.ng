/*
 * ipmi_ssif.c
 *
 * The interface to the IPMI driver for SMBus access to a SMBus
 * compliant device.  Called SSIF by the IPMI spec.
 *
 * Author: Intel Corporation
 *         Todd Davis <todd.c.davis@intel.com>
 *
 * Rewritten by Corey Minyard <minyard@acm.org> to support the
 * non-blocking I2C interface, add support for multi-part
 * transactions, add PEC support, and general clenaup.
 *
 * Copyright 2003 Intel Corporation
 * Copyright 2005 MontaVista Software
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2 of the License, or (at your
 *  option) any later version.
 */

/*
 * This file holds the "policy" for the interface to the SSIF state
 * machine.  It does the configuration, handles timers and interrupts,
 * and drives the real SSIF state machine.
 */

/*
 * TODO: Figure out how to use SMB alerts.  This will require a new
 * interface into the I2C driver, I believe.
 */

#if defined(MODVERSIONS)
#include <linux/modversions.h>
#endif

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/ipmi_smi.h>
#include <linux/init.h>
#include <linux/dmi.h>
#include <linux/kthread.h>
#include <linux/acpi.h>
#include <linux/ctype.h>

#define PFX "ipmi_ssif: "
#define DEVICE_NAME "ipmi_ssif"

#define IPMI_GET_SYSTEM_INTERFACE_CAPABILITIES_CMD	0x57

#define	SSIF_IPMI_REQUEST			2
#define	SSIF_IPMI_MULTI_PART_REQUEST_START	6
#define	SSIF_IPMI_MULTI_PART_REQUEST_MIDDLE	7
#define	SSIF_IPMI_RESPONSE			3
#define	SSIF_IPMI_MULTI_PART_RESPONSE_MIDDLE	9

/* ssif_debug is a bit-field
 *	SSIF_DEBUG_MSG -	commands and their responses
 *	SSIF_DEBUG_STATES -	message states
 *	SSIF_DEBUG_TIMING -	 Measure times between events in the driver
 */
#define SSIF_DEBUG_TIMING	4
#define SSIF_DEBUG_STATE	2
#define SSIF_DEBUG_MSG		1
#define SSIF_NODEBUG		0
#define SSIF_DEFAULT_DEBUG	(SSIF_NODEBUG)

/*
 * Timer values
 */
#define SSIF_MSG_USEC		20000	/* 20ms between message tries. */
#define SSIF_MSG_PART_USEC	5000	/* 5ms for a message part */

/* How many times to we retry sending/receiving the message. */
#define	SSIF_SEND_RETRIES	5
#define	SSIF_RECV_RETRIES	250

#define SSIF_MSG_MSEC		(SSIF_MSG_USEC / 1000)
#define SSIF_MSG_JIFFIES	((SSIF_MSG_USEC * 1000) / TICK_NSEC)
#define SSIF_MSG_PART_JIFFIES	((SSIF_MSG_PART_USEC * 1000) / TICK_NSEC)

enum ssif_intf_state {
	SSIF_NORMAL,
	SSIF_GETTING_FLAGS,
	SSIF_GETTING_EVENTS,
	SSIF_CLEARING_FLAGS,
	SSIF_GETTING_MESSAGES,
	/* FIXME - add watchdog stuff. */
};

#define SSIF_IDLE(ssif)	 ((ssif)->ssif_state == SSIF_NORMAL \
			  && (ssif)->curr_msg == NULL)

/*
 * Indexes into stats[] in ssif_info below.
 */
enum ssif_stat_indexes {
	/* Number of total messages sent. */
	SSIF_STAT_sent_messages = 0,

	/*
	 * Number of message parts sent.  Messages may be broken into
	 * parts if they are long.
	 */
	SSIF_STAT_sent_messages_parts,

	/*
	 * Number of time a message was retried.
	 */
	SSIF_STAT_send_retries,

	/*
	 * Number of times the send of a message failed.
	 */
	SSIF_STAT_send_errors,

	/*
	 * Number of message responses received.
	 */
	SSIF_STAT_received_messages,

	/*
	 * Number of message fragments received.
	 */
	SSIF_STAT_received_message_parts,

	/*
	 * Number of times the receive of a message was retried.
	 */
	SSIF_STAT_receive_retries,

	/*
	 * Number of errors receiving messages.
	 */
	SSIF_STAT_receive_errors,

	/*
	 * Number of times a flag fetch was requested.
	 */
	SSIF_STAT_flag_fetches,

	/*
	 * Number of times the hardware didn't follow the state machine.
	 */
	SSIF_STAT_hosed,

	/*
	 * Number of received events.
	 */
	SSIF_STAT_events,

	/* Number of asyncronous messages received. */
	SSIF_STAT_incoming_messages,

	/* Number of watchdog pretimeouts. */
	SSIF_STAT_watchdog_pretimeouts,

	/* Number of alers received. */
	SSIF_STAT_alerts,

	/* Always add statistics before this value, it must be last. */
	SSIF_NUM_STATS
};

struct ssif_addr_info {
	unsigned short addr;
	struct i2c_board_info binfo;
	char *adapter_name;
	int debug;
	int slave_addr;
	enum ipmi_addr_src addr_src;
	union ipmi_smi_info_union addr_info;

	struct mutex clients_mutex;
	struct list_head clients;

	struct list_head link;
};

struct ssif_info;

typedef void (*ssif_i2c_done)(struct ssif_info *ssif_info, int result,
			     unsigned char *data, unsigned int len);

struct ssif_info {
	ipmi_smi_t          intf;
	int                 intf_num;
	spinlock_t	    lock;
	struct ipmi_smi_msg *waiting_msg;
	struct ipmi_smi_msg *curr_msg;
	enum ssif_intf_state ssif_state;
	unsigned long       ssif_debug;

	struct ipmi_smi_handlers handlers;

	enum ipmi_addr_src addr_source; /* ACPI, PCI, SMBIOS, hardcode, etc. */
	union ipmi_smi_info_union addr_info;

	/*
	 * Flags from the last GET_MSG_FLAGS command, used when an ATTN
	 * is set to hold the flags until we are done handling everything
	 * from the flags.
	 */
#define RECEIVE_MSG_AVAIL	0x01
#define EVENT_MSG_BUFFER_FULL	0x02
#define WDT_PRE_TIMEOUT_INT	0x08
	unsigned char       msg_flags;

	u8		    global_enables;
	bool		    has_event_buffer;
	bool		    supports_alert;

	/*
	 * Used to tell what we should do with alerts.  If we are
	 * waiting on a response, read the data immediately.
	 */
	bool		    got_alert;
	bool		    waiting_alert;

	/*
	 * If set to true, this will request events the next time the
	 * state machine is idle.
	 */
	bool                req_events;

	/*
	 * If set to true, this will request flags the next time the
	 * state machine is idle.
	 */
	bool                req_flags;

	/*
	 * Used to perform timer operations when run-to-completion
	 * mode is on.  This is a countdown timer.
	 */
	int                 rtc_us_timer;

	/* Used for sending/receiving data.  +1 for the length. */
	unsigned char data[IPMI_MAX_MSG_LENGTH + 1];
	unsigned int  data_len;

	/* Temp receive buffer, gets copied into data. */
	unsigned char recv[I2C_SMBUS_BLOCK_MAX];

	struct i2c_client *client;
	ssif_i2c_done done_handler;

	/* Thread interface handling */
	struct task_struct *thread;
	struct completion wake_thread;
	bool stopping;
	int i2c_read_write;
	int i2c_command;
	unsigned char *i2c_data;
	unsigned int i2c_size;

	/* From the device id response. */
	struct ipmi_device_id device_id;

	struct timer_list retry_timer;
	int retries_left;

	/* Info from SSIF cmd */
	unsigned char max_xmit_msg_size;
	unsigned char max_recv_msg_size;
	unsigned int  multi_support;
	int           supports_pec;

#define SSIF_NO_MULTI		0
#define SSIF_MULTI_2_PART	1
#define SSIF_MULTI_n_PART	2
	unsigned char *multi_data;
	unsigned int  multi_len;
	unsigned int  multi_pos;

	atomic_t stats[SSIF_NUM_STATS];
};

#define ssif_inc_stat(ssif, stat) \
	atomic_inc(&(ssif)->stats[SSIF_STAT_ ## stat])
#define ssif_get_stat(ssif, stat) \
	((unsigned int) atomic_read(&(ssif)->stats[SSIF_STAT_ ## stat]))

static bool initialized;

static atomic_t next_intf = ATOMIC_INIT(0);

static void return_hosed_msg(struct ssif_info *ssif_info,
			     struct ipmi_smi_msg *msg);
static void start_next_msg(struct ssif_info *ssif_info, unsigned long *flags);
static int start_send(struct ssif_info *ssif_info,
		      unsigned char   *data,
		      unsigned int    len);

static unsigned long *ipmi_ssif_lock_cond(struct ssif_info *ssif_info,
					  unsigned long *flags)
{
	spin_lock_irqsave(&ssif_info->lock, *flags);
	return flags;
}

static void ipmi_ssif_unlock_cond(struct ssif_info *ssif_info,
				  unsigned long *flags)
{
	spin_unlock_irqrestore(&ssif_info->lock, *flags);
}

static void deliver_recv_msg(struct ssif_info *ssif_info,
			     struct ipmi_smi_msg *msg)
{
	ipmi_smi_t    intf = ssif_info->intf;

	if (!intf) {
		ipmi_free_smi_msg(msg);
	} else if (msg->rsp_size < 0) {
		return_hosed_msg(ssif_info, msg);
		pr_err(PFX
		       "Malformed message in deliver_recv_msg: rsp_size = %d\n",
		       msg->rsp_size);
	} else {
		ipmi_smi_msg_received(intf, msg);
	}
}

static void return_hosed_msg(struct ssif_info *ssif_info,
			     struct ipmi_smi_msg *msg)
{
	ssif_inc_stat(ssif_info, hosed);

	/* Make it a response */
	msg->rsp[0] = msg->data[0] | 4;
	msg->rsp[1] = msg->data[1];
	msg->rsp[2] = 0xFF; /* Unknown error. */
	msg->rsp_size = 3;

	deliver_recv_msg(ssif_info, msg);
}

/*
 * Must be called with the message lock held.  This will release the
 * message lock.  Note that the caller will check SSIF_IDLE and start a
 * new operation, so there is no need to check for new messages to
 * start in here.
 */
static void start_clear_flags(struct ssif_info *ssif_info, unsigned long *flags)
{
	unsigned char msg[3];

	ssif_info->msg_flags &= ~WDT_PRE_TIMEOUT_INT;
	ssif_info->ssif_state = SSIF_CLEARING_FLAGS;
	ipmi_ssif_unlock_cond(ssif_info, flags);

	/* Make sure the watchdog pre-timeout flag is not set at startup. */
	msg[0] = (IPMI_NETFN_APP_REQUEST << 2);
	msg[1] = IPMI_CLEAR_MSG_FLAGS_CMD;
	msg[2] = WDT_PRE_TIMEOUT_INT;

	if (start_send(ssif_info, msg, 3) != 0) {
		/* Error, just go to normal state. */
		ssif_info->ssif_state = SSIF_NORMAL;
	}
}

static void start_flag_fetch(struct ssif_info *ssif_info, unsigned long *flags)
{
	unsigned char mb[2];

	ssif_info->req_flags = false;
	ssif_info->ssif_state = SSIF_GETTING_FLAGS;
	ipmi_ssif_unlock_cond(ssif_info, flags);

	mb[0] = (IPMI_NETFN_APP_REQUEST << 2);
	mb[1] = IPMI_GET_MSG_FLAGS_CMD;
	if (start_send(ssif_info, mb, 2) != 0)
		ssif_info->ssif_state = SSIF_NORMAL;
}

static void check_start_send(struct ssif_info *ssif_info, unsigned long *flags,
			     struct ipmi_smi_msg *msg)
{
	if (start_send(ssif_info, msg->data, msg->data_size) != 0) {
		unsigned long oflags;

		flags = ipmi_ssif_lock_cond(ssif_info, &oflags);
		ssif_info->curr_msg = NULL;
		ssif_info->ssif_state = SSIF_NORMAL;
		ipmi_ssif_unlock_cond(ssif_info, flags);
		ipmi_free_smi_msg(msg);
	}
}

static void start_event_fetch(struct ssif_info *ssif_info, unsigned long *flags)
{
	struct ipmi_smi_msg *msg;

	ssif_info->req_events = false;

	msg = ipmi_alloc_smi_msg();
	if (!msg) {
		ssif_info->ssif_state = SSIF_NORMAL;
		ipmi_ssif_unlock_cond(ssif_info, flags);
		return;
	}

	ssif_info->curr_msg = msg;
	ssif_info->ssif_state = SSIF_GETTING_EVENTS;
	ipmi_ssif_unlock_cond(ssif_info, flags);

	msg->data[0] = (IPMI_NETFN_APP_REQUEST << 2);
	msg->data[1] = IPMI_READ_EVENT_MSG_BUFFER_CMD;
	msg->data_size = 2;

	check_start_send(ssif_info, flags, msg);
}

static void start_recv_msg_fetch(struct ssif_info *ssif_info,
				 unsigned long *flags)
{
	struct ipmi_smi_msg *msg;

	msg = ipmi_alloc_smi_msg();
	if (!msg) {
		ssif_info->ssif_state = SSIF_NORMAL;
		ipmi_ssif_unlock_cond(ssif_info, flags);
		return;
	}

	ssif_info->curr_msg = msg;
	ssif_info->ssif_state = SSIF_GETTING_MESSAGES;
	ipmi_ssif_unlock_cond(ssif_info, flags);

	msg->data[0] = (IPMI_NETFN_APP_REQUEST << 2);
	msg->data[1] = IPMI_GET_MSG_CMD;
	msg->data_size = 2;

	check_start_send(ssif_info, flags, msg);
}

/*
 * Must be called with the message lock held.  This will release the
 * message lock.  Note that the caller will check SSIF_IDLE and start a
 * new operation, so there is no need to check for new messages to
 * start in here.
 */
static void handle_flags(struct ssif_info *ssif_info, unsigned long *flags)
{
	if (ssif_info->msg_flags & WDT_PRE_TIMEOUT_INT) {
		ipmi_smi_t intf = ssif_info->intf;
		/* Watchdog pre-timeout */
		ssif_inc_stat(ssif_info, watchdog_pretimeouts);
		start_clear_flags(ssif_info, flags);
		if (intf)
			ipmi_smi_watchdog_pretimeout(intf);
	} else if (ssif_info->msg_flags & RECEIVE_MSG_AVAIL)
		/* Messages available. */
		start_recv_msg_fetch(ssif_info, flags);
	else if (ssif_info->msg_flags & EVENT_MSG_BUFFER_FULL)
		/* Events available. */
		start_event_fetch(ssif_info, flags);
	else {
		ssif_info->ssif_state = SSIF_NORMAL;
		ipmi_ssif_unlock_cond(ssif_info, flags);
	}
}

static int ipmi_ssif_thread(void *data)
{
	struct ssif_info *ssif_info = data;

	while (!kthread_should_stop()) {
		int result;

		/* Wait for something to do */
		result = wait_for_completion_interruptible(
						&ssif_info->wake_thread);
		if (ssif_info->stopping)
			break;
		if (result == -ERESTARTSYS)
			continue;
		init_completion(&ssif_info->wake_thread);

		if (ssif_info->i2c_read_write == I2C_SMBUS_WRITE) {
			result = i2c_smbus_write_block_data(
				ssif_info->client, ssif_info->i2c_command,
				ssif_info->i2c_data[0],
				ssif_info->i2c_data + 1);
			ssif_info->done_handler(ssif_info, result, NULL, 0);
		} else {
			result = i2c_smbus_read_block_data(
				ssif_info->client, ssif_info->i2c_command,
				ssif_info->i2c_data);
			if (result < 0)
				ssif_info->done_handler(ssif_info, result,
							NULL, 0);
			else
				ssif_info->done_handler(ssif_info, 0,
							ssif_info->i2c_data,
							result);
		}
	}

	return 0;
}

static int ssif_i2c_send(struct ssif_info *ssif_info,
			ssif_i2c_done handler,
			int read_write, int command,
			unsigned char *data, unsigned int size)
{
	ssif_info->done_handler = handler;

	ssif_info->i2c_read_write = read_write;
	ssif_info->i2c_command = command;
	ssif_info->i2c_data = data;
	ssif_info->i2c_size = size;
	complete(&ssif_info->wake_thread);
	return 0;
}


static void msg_done_handler(struct ssif_info *ssif_info, int result,
			     unsigned char *data, unsigned int len);

static void start_get(struct ssif_info *ssif_info)
{
	int rv;

	ssif_info->rtc_us_timer = 0;
	ssif_info->multi_pos = 0;

	rv = ssif_i2c_send(ssif_info, msg_done_handler, I2C_SMBUS_READ,
			  SSIF_IPMI_RESPONSE,
			  ssif_info->recv, I2C_SMBUS_BLOCK_DATA);
	if (rv < 0) {
		/* request failed, just return the error. */
		if (ssif_info->ssif_debug & SSIF_DEBUG_MSG)
			pr_info("Error from i2c_non_blocking_op(5)\n");

		msg_done_handler(ssif_info, -EIO, NULL, 0);
	}
}

static void retry_timeout(unsigned long data)
{
	struct ssif_info *ssif_info = (void *) data;
	unsigned long oflags, *flags;
	bool waiting;

	if (ssif_info->stopping)
		return;

	flags = ipmi_ssif_lock_cond(ssif_info, &oflags);
	waiting = ssif_info->waiting_alert;
	ssif_info->waiting_alert = false;
	ipmi_ssif_unlock_cond(ssif_info, flags);

	if (waiting)
		start_get(ssif_info);
}


static void ssif_alert(struct i2c_client *client, unsigned int data)
{
	struct ssif_info *ssif_info = i2c_get_clientdata(client);
	unsigned long oflags, *flags;
	bool do_get = false;

	ssif_inc_stat(ssif_info, alerts);

	flags = ipmi_ssif_lock_cond(ssif_info, &oflags);
	if (ssif_info->waiting_alert) {
		ssif_info->waiting_alert = false;
		del_timer(&ssif_info->retry_timer);
		do_get = true;
	} else if (ssif_info->curr_msg) {
		ssif_info->got_alert = true;
	}
	ipmi_ssif_unlock_cond(ssif_info, flags);
	if (do_get)
		start_get(ssif_info);
}

static int start_resend(struct ssif_info *ssif_info);

static void msg_done_handler(struct ssif_info *ssif_info, int result,
			     unsigned char *data, unsigned int len)
{
	struct ipmi_smi_msg *msg;
	unsigned long oflags, *flags;
	int rv;

	/*
	 * We are single-threaded here, so no need for a lock until we
	 * start messing with driver states or the queues.
	 */

	if (result < 0) {
		ssif_info->retries_left--;
		if (ssif_info->retries_left > 0) {
			ssif_inc_stat(ssif_info, receive_retries);

			flags = ipmi_ssif_lock_cond(ssif_info, &oflags);
			ssif_info->waiting_alert = true;
			ssif_info->rtc_us_timer = SSIF_MSG_USEC;
			mod_timer(&ssif_info->retry_timer,
				  jiffies + SSIF_MSG_JIFFIES);
			ipmi_ssif_unlock_cond(ssif_info, flags);
			return;
		}

		ssif_inc_stat(ssif_info, receive_errors);

		if  (ssif_info->ssif_debug & SSIF_DEBUG_MSG)
			pr_info("Error in msg_done_handler: %d\n", result);
		len = 0;
		goto continue_op;
	}

	if ((len > 1) && (ssif_info->multi_pos == 0)
				&& (data[0] == 0x00) && (data[1] == 0x01)) {
		/* Start of multi-part read.  Start the next transaction. */
		int i;

		ssif_inc_stat(ssif_info, received_message_parts);

		/* Remove the multi-part read marker. */
		len -= 2;
		for (i = 0; i < len; i++)
			ssif_info->data[i] = data[i+2];
		ssif_info->multi_len = len;
		ssif_info->multi_pos = 1;

		rv = ssif_i2c_send(ssif_info, msg_done_handler, I2C_SMBUS_READ,
				  SSIF_IPMI_MULTI_PART_RESPONSE_MIDDLE,
				  ssif_info->recv, I2C_SMBUS_BLOCK_DATA);
		if (rv < 0) {
			if (ssif_info->ssif_debug & SSIF_DEBUG_MSG)
				pr_info("Error from i2c_non_blocking_op(1)\n");

			result = -EIO;
		} else
			return;
	} else if (ssif_info->multi_pos) {
		/* Middle of multi-part read.  Start the next transaction. */
		int i;
		unsigned char blocknum;

		if (len == 0) {
			result = -EIO;
			if (ssif_info->ssif_debug & SSIF_DEBUG_MSG)
				pr_info(PFX "Middle message with no data\n");

			goto continue_op;
		}

		blocknum = data[0];

		if (ssif_info->multi_len + len - 1 > IPMI_MAX_MSG_LENGTH) {
			/* Received message too big, abort the operation. */
			result = -E2BIG;
			if (ssif_info->ssif_debug & SSIF_DEBUG_MSG)
				pr_info("Received message too big\n");

			goto continue_op;
		}

		/* Remove the blocknum from the data. */
		len--;
		for (i = 0; i < len; i++)
			ssif_info->data[i + ssif_info->multi_len] = data[i + 1];
		ssif_info->multi_len += len;
		if (blocknum == 0xff) {
			/* End of read */
			len = ssif_info->multi_len;
			data = ssif_info->data;
		} else if (blocknum + 1 != ssif_info->multi_pos) {
			/*
			 * Out of sequence block, just abort.  Block
			 * numbers start at zero for the second block,
			 * but multi_pos starts at one, so the +1.
			 */
			result = -EIO;
		} else {
			ssif_inc_stat(ssif_info, received_message_parts);

			ssif_info->multi_pos++;

			rv = ssif_i2c_send(ssif_info, msg_done_handler,
					   I2C_SMBUS_READ,
					   SSIF_IPMI_MULTI_PART_RESPONSE_MIDDLE,
					   ssif_info->recv,
					   I2C_SMBUS_BLOCK_DATA);
			if (rv < 0) {
				if (ssif_info->ssif_debug & SSIF_DEBUG_MSG)
					pr_info(PFX
						"Error from ssif_i2c_send\n");

				result = -EIO;
			} else
				return;
		}
	}

	if (result < 0) {
		ssif_inc_stat(ssif_info, receive_errors);
	} else {
		ssif_inc_stat(ssif_info, received_messages);
		ssif_inc_stat(ssif_info, received_message_parts);
	}


 continue_op:
	if (ssif_info->ssif_debug & SSIF_DEBUG_STATE)
		pr_info(PFX "DONE 1: state = %d, result=%d.\n",
			ssif_info->ssif_state, result);

	flags = ipmi_ssif_lock_cond(ssif_info, &oflags);
	msg = ssif_info->curr_msg;
	if (msg) {
		msg->rsp_size = len;
		if (msg->rsp_size > IPMI_MAX_MSG_LENGTH)
			msg->rsp_size = IPMI_MAX_MSG_LENGTH;
		memcpy(msg->rsp, data, msg->rsp_size);
		ssif_info->curr_msg = NULL;
	}

	switch (ssif_info->ssif_state) {
	case SSIF_NORMAL:
		ipmi_ssif_unlock_cond(ssif_info, flags);
		if (!msg)
			break;

		if (result < 0)
			return_hosed_msg(ssif_info, msg);
		else
			deliver_recv_msg(ssif_info, msg);
		break;

	case SSIF_GETTING_FLAGS:
		/* We got the flags from the SSIF, now handle them. */
		if ((result < 0) || (len < 4) || (data[2] != 0)) {
			/*
			 * Error fetching flags, or invalid length,
			 * just give up for now.
			 */
			ssif_info->ssif_state = SSIF_NORMAL;
			ipmi_ssif_unlock_cond(ssif_info, flags);
			pr_warn(PFX "Error getting flags: %d %d, %x\n",
			       result, len, data[2]);
		} else if (data[0] != (IPMI_NETFN_APP_REQUEST | 1) << 2
			   || data[1] != IPMI_GET_MSG_FLAGS_CMD) {
			/*
			 * Don't abort here, maybe it was a queued
			 * response to a previous command.
			 */
			ipmi_ssif_unlock_cond(ssif_info, flags);
			pr_warn(PFX "Invalid response getting flags: %x %x\n",
				data[0], data[1]);
		} else {
			ssif_inc_stat(ssif_info, flag_fetches);
			ssif_info->msg_flags = data[3];
			handle_flags(ssif_info, flags);
		}
		break;

	case SSIF_CLEARING_FLAGS:
		/* We cleared the flags. */
		if ((result < 0) || (len < 3) || (data[2] != 0)) {
			/* Error clearing flags */
			pr_warn(PFX "Error clearing flags: %d %d, %x\n",
			       result, len, data[2]);
		} else if (data[0] != (IPMI_NETFN_APP_REQUEST | 1) << 2
			   || data[1] != IPMI_CLEAR_MSG_FLAGS_CMD) {
			pr_warn(PFX "Invalid response clearing flags: %x %x\n",
				data[0], data[1]);
		}
		ssif_info->ssif_state = SSIF_NORMAL;
		ipmi_ssif_unlock_cond(ssif_info, flags);
		break;

	case SSIF_GETTING_EVENTS:
		if ((result < 0) || (len < 3) || (msg->rsp[2] != 0)) {
			/* Error getting event, probably done. */
			msg->done(msg);

			/* Take off the event flag. */
			ssif_info->msg_flags &= ~EVENT_MSG_BUFFER_FULL;
			handle_flags(ssif_info, flags);
		} else if (msg->rsp[0] != (IPMI_NETFN_APP_REQUEST | 1) << 2
			   || msg->rsp[1] != IPMI_READ_EVENT_MSG_BUFFER_CMD) {
			pr_warn(PFX "Invalid response getting events: %x %x\n",
				msg->rsp[0], msg->rsp[1]);
			msg->done(msg);
			/* Take off the event flag. */
			ssif_info->msg_flags &= ~EVENT_MSG_BUFFER_FULL;
			handle_flags(ssif_info, flags);
		} else {
			handle_flags(ssif_info, flags);
			ssif_inc_stat(ssif_info, events);
			deliver_recv_msg(ssif_info, msg);
		}
		break;

	case SSIF_GETTING_MESSAGES:
		if ((result < 0) || (len < 3) || (msg->rsp[2] != 0)) {
			/* Error getting event, probably done. */
			msg->done(msg);

			/* Take off the msg flag. */
			ssif_info->msg_flags &= ~RECEIVE_MSG_AVAIL;
			handle_flags(ssif_info, flags);
		} else if (msg->rsp[0] != (IPMI_NETFN_APP_REQUEST | 1) << 2
			   || msg->rsp[1] != IPMI_GET_MSG_CMD) {
			pr_warn(PFX "Invalid response clearing flags: %x %x\n",
				msg->rsp[0], msg->rsp[1]);
			msg->done(msg);

			/* Take off the msg flag. */
			ssif_info->msg_flags &= ~RECEIVE_MSG_AVAIL;
			handle_flags(ssif_info, flags);
		} else {
			ssif_inc_stat(ssif_info, incoming_messages);
			handle_flags(ssif_info, flags);
			deliver_recv_msg(ssif_info, msg);
		}
		break;
	}

	flags = ipmi_ssif_lock_cond(ssif_info, &oflags);
	if (SSIF_IDLE(ssif_info) && !ssif_info->stopping) {
		if (ssif_info->req_events)
			start_event_fetch(ssif_info, flags);
		else if (ssif_info->req_flags)
			start_flag_fetch(ssif_info, flags);
		else
			start_next_msg(ssif_info, flags);
	} else
		ipmi_ssif_unlock_cond(ssif_info, flags);

	if (ssif_info->ssif_debug & SSIF_DEBUG_STATE)
		pr_info(PFX "DONE 2: state = %d.\n", ssif_info->ssif_state);
}

static void msg_written_handler(struct ssif_info *ssif_info, int result,
				unsigned char *data, unsigned int len)
{
	int rv;

	/* We are single-threaded here, so no need for a lock. */
	if (result < 0) {
		ssif_info->retries_left--;
		if (ssif_info->retries_left > 0) {
			if (!start_resend(ssif_info)) {
				ssif_inc_stat(ssif_info, send_retries);
				return;
			}
			/* request failed, just return the error. */
			ssif_inc_stat(ssif_info, send_errors);

			if (ssif_info->ssif_debug & SSIF_DEBUG_MSG)
				pr_info(PFX
					"Out of retries in msg_written_handler\n");
			msg_done_handler(ssif_info, -EIO, NULL, 0);
			return;
		}

		ssif_inc_stat(ssif_info, send_errors);

		/*
		 * Got an error on transmit, let the done routine
		 * handle it.
		 */
		if (ssif_info->ssif_debug & SSIF_DEBUG_MSG)
			pr_info("Error in msg_written_handler: %d\n", result);

		msg_done_handler(ssif_info, result, NULL, 0);
		return;
	}

	if (ssif_info->multi_data) {
		/*
		 * In the middle of a multi-data write.  See the comment
		 * in the SSIF_MULTI_n_PART case in the probe function
		 * for details on the intricacies of this.
		 */
		int left;
		unsigned char *data_to_send;

		ssif_inc_stat(ssif_info, sent_messages_parts);

		left = ssif_info->multi_len - ssif_info->multi_pos;
		if (left > 32)
			left = 32;
		/* Length byte. */
		ssif_info->multi_data[ssif_info->multi_pos] = left;
		data_to_send = ssif_info->multi_data + ssif_info->multi_pos;
		ssif_info->multi_pos += left;
		if (left < 32)
			/*
			 * Write is finished.  Note that we must end
			 * with a write of less than 32 bytes to
			 * complete the transaction, even if it is
			 * zero bytes.
			 */
			ssif_info->multi_data = NULL;

		rv = ssif_i2c_send(ssif_info, msg_written_handler,
				  I2C_SMBUS_WRITE,
				  SSIF_IPMI_MULTI_PART_REQUEST_MIDDLE,
				  data_to_send,
				  I2C_SMBUS_BLOCK_DATA);
		if (rv < 0) {
			/* request failed, just return the error. */
			ssif_inc_stat(ssif_info, send_errors);

			if (ssif_info->ssif_debug & SSIF_DEBUG_MSG)
				pr_info("Error from i2c_non_blocking_op(3)\n");
			msg_done_handler(ssif_info, -EIO, NULL, 0);
		}
	} else {
		unsigned long oflags, *flags;
		bool got_alert;

		ssif_inc_stat(ssif_info, sent_messages);
		ssif_inc_stat(ssif_info, sent_messages_parts);

		flags = ipmi_ssif_lock_cond(ssif_info, &oflags);
		got_alert = ssif_info->got_alert;
		if (got_alert) {
			ssif_info->got_alert = false;
			ssif_info->waiting_alert = false;
		}

		if (got_alert) {
			ipmi_ssif_unlock_cond(ssif_info, flags);
			/* The alert already happened, try now. */
			retry_timeout((unsigned long) ssif_info);
		} else {
			/* Wait a jiffie then request the next message */
			ssif_info->waiting_alert = true;
			ssif_info->retries_left = SSIF_RECV_RETRIES;
			ssif_info->rtc_us_timer = SSIF_MSG_PART_USEC;
			mod_timer(&ssif_info->retry_timer,
				  jiffies + SSIF_MSG_PART_JIFFIES);
			ipmi_ssif_unlock_cond(ssif_info, flags);
		}
	}
}

static int start_resend(struct ssif_info *ssif_info)
{
	int rv;
	int command;

	ssif_info->got_alert = false;

	if (ssif_info->data_len > 32) {
		command = SSIF_IPMI_MULTI_PART_REQUEST_START;
		ssif_info->multi_data = ssif_info->data;
		ssif_info->multi_len = ssif_info->data_len;
		/*
		 * Subtle thing, this is 32, not 33, because we will
		 * overwrite the thing at position 32 (which was just
		 * transmitted) with the new length.
		 */
		ssif_info->multi_pos = 32;
		ssif_info->data[0] = 32;
	} else {
		ssif_info->multi_data = NULL;
		command = SSIF_IPMI_REQUEST;
		ssif_info->data[0] = ssif_info->data_len;
	}

	rv = ssif_i2c_send(ssif_info, msg_written_handler, I2C_SMBUS_WRITE,
			  command, ssif_info->data, I2C_SMBUS_BLOCK_DATA);
	if (rv && (ssif_info->ssif_debug & SSIF_DEBUG_MSG))
		pr_info("Error from i2c_non_blocking_op(4)\n");
	return rv;
}

static int start_send(struct ssif_info *ssif_info,
		      unsigned char   *data,
		      unsigned int    len)
{
	if (len > IPMI_MAX_MSG_LENGTH)
		return -E2BIG;
	if (len > ssif_info->max_xmit_msg_size)
		return -E2BIG;

	ssif_info->retries_left = SSIF_SEND_RETRIES;
	memcpy(ssif_info->data + 1, data, len);
	ssif_info->data_len = len;
	return start_resend(ssif_info);
}

/* Must be called with the message lock held. */
static void start_next_msg(struct ssif_info *ssif_info, unsigned long *flags)
{
	struct ipmi_smi_msg *msg;
	unsigned long oflags;

 restart:
	if (!SSIF_IDLE(ssif_info)) {
		ipmi_ssif_unlock_cond(ssif_info, flags);
		return;
	}

	if (!ssif_info->waiting_msg) {
		ssif_info->curr_msg = NULL;
		ipmi_ssif_unlock_cond(ssif_info, flags);
	} else {
		int rv;

		ssif_info->curr_msg = ssif_info->waiting_msg;
		ssif_info->waiting_msg = NULL;
		ipmi_ssif_unlock_cond(ssif_info, flags);
		rv = start_send(ssif_info,
				ssif_info->curr_msg->data,
				ssif_info->curr_msg->data_size);
		if (rv) {
			msg = ssif_info->curr_msg;
			ssif_info->curr_msg = NULL;
			return_hosed_msg(ssif_info, msg);
			flags = ipmi_ssif_lock_cond(ssif_info, &oflags);
			goto restart;
		}
	}
}

static void sender(void                *send_info,
		   struct ipmi_smi_msg *msg)
{
	struct ssif_info *ssif_info = (struct ssif_info *) send_info;
	unsigned long oflags, *flags;

	BUG_ON(ssif_info->waiting_msg);
	ssif_info->waiting_msg = msg;

	flags = ipmi_ssif_lock_cond(ssif_info, &oflags);
	start_next_msg(ssif_info, flags);

	if (ssif_info->ssif_debug & SSIF_DEBUG_TIMING) {
		struct timeval t;

		do_gettimeofday(&t);
		pr_info("**Enqueue %02x %02x: %ld.%6.6ld\n",
		       msg->data[0], msg->data[1],
		       (long) t.tv_sec, (long) t.tv_usec);
	}
}

static int get_smi_info(void *send_info, struct ipmi_smi_info *data)
{
	struct ssif_info *ssif_info = send_info;

	data->addr_src = ssif_info->addr_source;
	data->dev = &ssif_info->client->dev;
	data->addr_info = ssif_info->addr_info;
	get_device(data->dev);

	return 0;
}

/*
 * Instead of having our own timer to periodically check the message
 * flags, we let the message handler drive us.
 */
static void request_events(void *send_info)
{
	struct ssif_info *ssif_info = (struct ssif_info *) send_info;
	unsigned long oflags, *flags;

	if (!ssif_info->has_event_buffer)
		return;

	flags = ipmi_ssif_lock_cond(ssif_info, &oflags);
	/*
	 * Request flags first, not events, because the lower layer
	 * doesn't have a way to send an attention.  But make sure
	 * event checking still happens.
	 */
	ssif_info->req_events = true;
	if (SSIF_IDLE(ssif_info))
		start_flag_fetch(ssif_info, flags);
	else {
		ssif_info->req_flags = true;
		ipmi_ssif_unlock_cond(ssif_info, flags);
	}
}

static int inc_usecount(void *send_info)
{
	struct ssif_info *ssif_info = send_info;

	if (!i2c_get_adapter(ssif_info->client->adapter->nr))
		return -ENODEV;

	i2c_use_client(ssif_info->client);
	return 0;
}

static void dec_usecount(void *send_info)
{
	struct ssif_info *ssif_info = send_info;

	i2c_release_client(ssif_info->client);
	i2c_put_adapter(ssif_info->client->adapter);
}

static int ssif_start_processing(void *send_info,
				 ipmi_smi_t intf)
{
	struct ssif_info *ssif_info = send_info;

	ssif_info->intf = intf;

	return 0;
}

#define MAX_SSIF_BMCS 4

static unsigned short addr[MAX_SSIF_BMCS];
static int num_addrs;
module_param_array(addr, ushort, &num_addrs, 0);
MODULE_PARM_DESC(addr, "The addresses to scan for IPMI BMCs on the SSIFs.");

static char *adapter_name[MAX_SSIF_BMCS];
static int num_adapter_names;
module_param_array(adapter_name, charp, &num_adapter_names, 0);
MODULE_PARM_DESC(adapter_name, "The string name of the I2C device that has the BMC.  By default all devices are scanned.");

static int slave_addrs[MAX_SSIF_BMCS];
static int num_slave_addrs;
module_param_array(slave_addrs, int, &num_slave_addrs, 0);
MODULE_PARM_DESC(slave_addrs,
		 "The default IPMB slave address for the controller.");

/*
 * Bit 0 enables message debugging, bit 1 enables state debugging, and
 * bit 2 enables timing debugging.  This is an array indexed by
 * interface number"
 */
static int dbg[MAX_SSIF_BMCS];
static int num_dbg;
module_param_array(dbg, int, &num_dbg, 0);
MODULE_PARM_DESC(dbg, "Turn on debugging.");

static bool ssif_dbg_probe;
module_param_named(dbg_probe, ssif_dbg_probe, bool, 0);
MODULE_PARM_DESC(dbg_probe, "Enable debugging of probing of adapters.");

static int use_thread;
module_param(use_thread, int, 0);
MODULE_PARM_DESC(use_thread, "Use the thread interface.");

static bool ssif_tryacpi = 1;
module_param_named(tryacpi, ssif_tryacpi, bool, 0);
MODULE_PARM_DESC(tryacpi, "Setting this to zero will disable the default scan of the interfaces identified via ACPI");

static bool ssif_trydmi = 1;
module_param_named(trydmi, ssif_trydmi, bool, 0);
MODULE_PARM_DESC(trydmi, "Setting this to zero will disable the default scan of the interfaces identified via DMI (SMBIOS)");

static DEFINE_MUTEX(ssif_infos_mutex);
static LIST_HEAD(ssif_infos);

static int ssif_remove(struct i2c_client *client)
{
	struct ssif_info *ssif_info = i2c_get_clientdata(client);
	int rv;

	if (!ssif_info)
		return 0;

	/*
	 * After this point, we won't deliver anything asychronously
	 * to the message handler.  We can unregister ourself.
	 */
	rv = ipmi_unregister_smi(ssif_info->intf);
	if (rv) {
		pr_err(PFX "Unable to unregister device: errno=%d\n", rv);
		return rv;
	}
	ssif_info->intf = NULL;

	/* make sure the driver is not looking for flags any more. */
	while (ssif_info->ssif_state != SSIF_NORMAL)
		schedule_timeout(1);

	ssif_info->stopping = true;
	del_timer_sync(&ssif_info->retry_timer);
	if (ssif_info->thread) {
		complete(&ssif_info->wake_thread);
		kthread_stop(ssif_info->thread);
	}

	/*
	 * No message can be outstanding now, we have removed the
	 * upper layer and it permitted us to do so.
	 */
	kfree(ssif_info);
	return 0;
}

static int do_cmd(struct i2c_client *client, int len, unsigned char *msg,
		  int *resp_len, unsigned char *resp)
{
	int retry_cnt;
	int ret;

	retry_cnt = SSIF_SEND_RETRIES;
 retry1:
	ret = i2c_smbus_write_block_data(client, SSIF_IPMI_REQUEST, len, msg);
	if (ret) {
		retry_cnt--;
		if (retry_cnt > 0)
			goto retry1;
		return -ENODEV;
	}

	ret = -ENODEV;
	retry_cnt = SSIF_RECV_RETRIES;
	while (retry_cnt > 0) {
		ret = i2c_smbus_read_block_data(client, SSIF_IPMI_RESPONSE,
						resp);
		if (ret > 0)
			break;
		msleep(SSIF_MSG_MSEC);
		retry_cnt--;
		if (retry_cnt <= 0)
			break;
	}

	if (ret > 0) {
		/* Validate that the response is correct. */
		if (ret < 3 ||
		    (resp[0] != (msg[0] | (1 << 2))) ||
		    (resp[1] != msg[1]))
			ret = -EINVAL;
		else {
			*resp_len = ret;
			ret = 0;
		}
	}

	return ret;
}

static int ssif_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	unsigned char *resp;
	unsigned char msg[3];
	int           rv;
	int           len;

	resp = kmalloc(IPMI_MAX_MSG_LENGTH, GFP_KERNEL);
	if (!resp)
		return -ENOMEM;

	/* Do a Get Device ID command, since it is required. */
	msg[0] = IPMI_NETFN_APP_REQUEST << 2;
	msg[1] = IPMI_GET_DEVICE_ID_CMD;
	rv = do_cmd(client, 2, msg, &len, resp);
	if (rv)
		rv = -ENODEV;
	else
		strlcpy(info->type, DEVICE_NAME, I2C_NAME_SIZE);
	kfree(resp);
	return rv;
}

static int smi_type_proc_show(struct seq_file *m, void *v)
{
	seq_puts(m, "ssif\n");

	return 0;
}

static int smi_type_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, smi_type_proc_show, inode->i_private);
}

static const struct file_operations smi_type_proc_ops = {
	.open		= smi_type_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int smi_stats_proc_show(struct seq_file *m, void *v)
{
	struct ssif_info *ssif_info = m->private;

	seq_printf(m, "sent_messages:          %u\n",
		   ssif_get_stat(ssif_info, sent_messages));
	seq_printf(m, "sent_messages_parts:    %u\n",
		   ssif_get_stat(ssif_info, sent_messages_parts));
	seq_printf(m, "send_retries:           %u\n",
		   ssif_get_stat(ssif_info, send_retries));
	seq_printf(m, "send_errors:            %u\n",
		   ssif_get_stat(ssif_info, send_errors));
	seq_printf(m, "received_messages:      %u\n",
		   ssif_get_stat(ssif_info, received_messages));
	seq_printf(m, "received_message_parts: %u\n",
		   ssif_get_stat(ssif_info, received_message_parts));
	seq_printf(m, "receive_retries:        %u\n",
		   ssif_get_stat(ssif_info, receive_retries));
	seq_printf(m, "receive_errors:         %u\n",
		   ssif_get_stat(ssif_info, receive_errors));
	seq_printf(m, "flag_fetches:           %u\n",
		   ssif_get_stat(ssif_info, flag_fetches));
	seq_printf(m, "hosed:                  %u\n",
		   ssif_get_stat(ssif_info, hosed));
	seq_printf(m, "events:                 %u\n",
		   ssif_get_stat(ssif_info, events));
	seq_printf(m, "watchdog_pretimeouts:   %u\n",
		   ssif_get_stat(ssif_info, watchdog_pretimeouts));
	seq_printf(m, "alerts:                 %u\n",
		   ssif_get_stat(ssif_info, alerts));
	return 0;
}

static int smi_stats_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, smi_stats_proc_show, PDE_DATA(inode));
}

static const struct file_operations smi_stats_proc_ops = {
	.open		= smi_stats_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int strcmp_nospace(char *s1, char *s2)
{
	while (*s1 && *s2) {
		while (isspace(*s1))
			s1++;
		while (isspace(*s2))
			s2++;
		if (*s1 > *s2)
			return 1;
		if (*s1 < *s2)
			return -1;
		s1++;
		s2++;
	}
	return 0;
}

static struct ssif_addr_info *ssif_info_find(unsigned short addr,
					     char *adapter_name,
					     bool match_null_name)
{
	struct ssif_addr_info *info, *found = NULL;

restart:
	list_for_each_entry(info, &ssif_infos, link) {
		if (info->binfo.addr == addr) {
			if (info->adapter_name || adapter_name) {
				if (!info->adapter_name != !adapter_name) {
					/* One is NULL and one is not */
					continue;
				}
				if (adapter_name &&
				    strcmp_nospace(info->adapter_name,
						   adapter_name))
					/* Names do not match */
					continue;
			}
			found = info;
			break;
		}
	}

	if (!found && match_null_name) {
		/* Try to get an exact match first, then try with a NULL name */
		adapter_name = NULL;
		match_null_name = false;
		goto restart;
	}

	return found;
}

static bool check_acpi(struct ssif_info *ssif_info, struct device *dev)
{
#ifdef CONFIG_ACPI
	acpi_handle acpi_handle;

	acpi_handle = ACPI_HANDLE(dev);
	if (acpi_handle) {
		ssif_info->addr_source = SI_ACPI;
		ssif_info->addr_info.acpi_info.acpi_handle = acpi_handle;
		return true;
	}
#endif
	return false;
}

/*
 * Global enables we care about.
 */
#define GLOBAL_ENABLES_MASK (IPMI_BMC_EVT_MSG_BUFF | IPMI_BMC_RCV_MSG_INTR | \
			     IPMI_BMC_EVT_MSG_INTR)

static int ssif_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	unsigned char     msg[3];
	unsigned char     *resp;
	struct ssif_info   *ssif_info;
	int               rv = 0;
	int               len;
	int               i;
	u8		  slave_addr = 0;
	struct ssif_addr_info *addr_info = NULL;


	resp = kmalloc(IPMI_MAX_MSG_LENGTH, GFP_KERNEL);
	if (!resp)
		return -ENOMEM;

	ssif_info = kzalloc(sizeof(*ssif_info), GFP_KERNEL);
	if (!ssif_info) {
		kfree(resp);
		return -ENOMEM;
	}

	if (!check_acpi(ssif_info, &client->dev)) {
		addr_info = ssif_info_find(client->addr, client->adapter->name,
					   true);
		if (!addr_info) {
			/* Must have come in through sysfs. */
			ssif_info->addr_source = SI_HOTMOD;
		} else {
			ssif_info->addr_source = addr_info->addr_src;
			ssif_info->ssif_debug = addr_info->debug;
			ssif_info->addr_info = addr_info->addr_info;
			slave_addr = addr_info->slave_addr;
		}
	}

	pr_info(PFX "Trying %s-specified SSIF interface at i2c address 0x%x, adapter %s, slave address 0x%x\n",
	       ipmi_addr_src_to_str(ssif_info->addr_source),
	       client->addr, client->adapter->name, slave_addr);

	/*
	 * Do a Get Device ID command, since it comes back with some
	 * useful info.
	 */
	msg[0] = IPMI_NETFN_APP_REQUEST << 2;
	msg[1] = IPMI_GET_DEVICE_ID_CMD;
	rv = do_cmd(client, 2, msg, &len, resp);
	if (rv)
		goto out;

	rv = ipmi_demangle_device_id(resp, len, &ssif_info->device_id);
	if (rv)
		goto out;

	ssif_info->client = client;
	i2c_set_clientdata(client, ssif_info);

	/* Now check for system interface capabilities */
	msg[0] = IPMI_NETFN_APP_REQUEST << 2;
	msg[1] = IPMI_GET_SYSTEM_INTERFACE_CAPABILITIES_CMD;
	msg[2] = 0; /* SSIF */
	rv = do_cmd(client, 3, msg, &len, resp);
	if (!rv && (len >= 3) && (resp[2] == 0)) {
		if (len < 7) {
			if (ssif_dbg_probe)
				pr_info(PFX "SSIF info too short: %d\n", len);
			goto no_support;
		}

		/* Got a good SSIF response, handle it. */
		ssif_info->max_xmit_msg_size = resp[5];
		ssif_info->max_recv_msg_size = resp[6];
		ssif_info->multi_support = (resp[4] >> 6) & 0x3;
		ssif_info->supports_pec = (resp[4] >> 3) & 0x1;

		/* Sanitize the data */
		switch (ssif_info->multi_support) {
		case SSIF_NO_MULTI:
			if (ssif_info->max_xmit_msg_size > 32)
				ssif_info->max_xmit_msg_size = 32;
			if (ssif_info->max_recv_msg_size > 32)
				ssif_info->max_recv_msg_size = 32;
			break;

		case SSIF_MULTI_2_PART:
			if (ssif_info->max_xmit_msg_size > 63)
				ssif_info->max_xmit_msg_size = 63;
			if (ssif_info->max_recv_msg_size > 62)
				ssif_info->max_recv_msg_size = 62;
			break;

		case SSIF_MULTI_n_PART:
			/*
			 * The specification is rather confusing at
			 * this point, but I think I understand what
			 * is meant.  At least I have a workable
			 * solution.  With multi-part messages, you
			 * cannot send a message that is a multiple of
			 * 32-bytes in length, because the start and
			 * middle messages are 32-bytes and the end
			 * message must be at least one byte.  You
			 * can't fudge on an extra byte, that would
			 * screw up things like fru data writes.  So
			 * we limit the length to 63 bytes.  That way
			 * a 32-byte message gets sent as a single
			 * part.  A larger message will be a 32-byte
			 * start and the next message is always going
			 * to be 1-31 bytes in length.  Not ideal, but
			 * it should work.
			 */
			if (ssif_info->max_xmit_msg_size > 63)
				ssif_info->max_xmit_msg_size = 63;
			break;

		default:
			/* Data is not sane, just give up. */
			goto no_support;
		}
	} else {
 no_support:
		/* Assume no multi-part or PEC support */
		pr_info(PFX "Error fetching SSIF: %d %d %2.2x, your system probably doesn't support this command so using defaults\n",
		       rv, len, resp[2]);

		ssif_info->max_xmit_msg_size = 32;
		ssif_info->max_recv_msg_size = 32;
		ssif_info->multi_support = SSIF_NO_MULTI;
		ssif_info->supports_pec = 0;
	}

	/* Make sure the NMI timeout is cleared. */
	msg[0] = IPMI_NETFN_APP_REQUEST << 2;
	msg[1] = IPMI_CLEAR_MSG_FLAGS_CMD;
	msg[2] = WDT_PRE_TIMEOUT_INT;
	rv = do_cmd(client, 3, msg, &len, resp);
	if (rv || (len < 3) || (resp[2] != 0))
		pr_warn(PFX "Unable to clear message flags: %d %d %2.2x\n",
			rv, len, resp[2]);

	/* Attempt to enable the event buffer. */
	msg[0] = IPMI_NETFN_APP_REQUEST << 2;
	msg[1] = IPMI_GET_BMC_GLOBAL_ENABLES_CMD;
	rv = do_cmd(client, 2, msg, &len, resp);
	if (rv || (len < 4) || (resp[2] != 0)) {
		pr_warn(PFX "Error getting global enables: %d %d %2.2x\n",
			rv, len, resp[2]);
		rv = 0; /* Not fatal */
		goto found;
	}

	ssif_info->global_enables = resp[3];

	if (resp[3] & IPMI_BMC_EVT_MSG_BUFF) {
		ssif_info->has_event_buffer = true;
		/* buffer is already enabled, nothing to do. */
		goto found;
	}

	msg[0] = IPMI_NETFN_APP_REQUEST << 2;
	msg[1] = IPMI_SET_BMC_GLOBAL_ENABLES_CMD;
	msg[2] = ssif_info->global_enables | IPMI_BMC_EVT_MSG_BUFF;
	rv = do_cmd(client, 3, msg, &len, resp);
	if (rv || (len < 2)) {
		pr_warn(PFX "Error setting global enables: %d %d %2.2x\n",
			rv, len, resp[2]);
		rv = 0; /* Not fatal */
		goto found;
	}

	if (resp[2] == 0) {
		/* A successful return means the event buffer is supported. */
		ssif_info->has_event_buffer = true;
		ssif_info->global_enables |= IPMI_BMC_EVT_MSG_BUFF;
	}

	msg[0] = IPMI_NETFN_APP_REQUEST << 2;
	msg[1] = IPMI_SET_BMC_GLOBAL_ENABLES_CMD;
	msg[2] = ssif_info->global_enables | IPMI_BMC_RCV_MSG_INTR;
	rv = do_cmd(client, 3, msg, &len, resp);
	if (rv || (len < 2)) {
		pr_warn(PFX "Error setting global enables: %d %d %2.2x\n",
			rv, len, resp[2]);
		rv = 0; /* Not fatal */
		goto found;
	}

	if (resp[2] == 0) {
		/* A successful return means the alert is supported. */
		ssif_info->supports_alert = true;
		ssif_info->global_enables |= IPMI_BMC_RCV_MSG_INTR;
	}

 found:
	ssif_info->intf_num = atomic_inc_return(&next_intf);

	if (ssif_dbg_probe) {
		pr_info("ssif_probe: i2c_probe found device at i2c address %x\n",
			client->addr);
	}

	spin_lock_init(&ssif_info->lock);
	ssif_info->ssif_state = SSIF_NORMAL;
	init_timer(&ssif_info->retry_timer);
	ssif_info->retry_timer.data = (unsigned long) ssif_info;
	ssif_info->retry_timer.function = retry_timeout;

	for (i = 0; i < SSIF_NUM_STATS; i++)
		atomic_set(&ssif_info->stats[i], 0);

	if (ssif_info->supports_pec)
		ssif_info->client->flags |= I2C_CLIENT_PEC;

	ssif_info->handlers.owner = THIS_MODULE;
	ssif_info->handlers.start_processing = ssif_start_processing;
	ssif_info->handlers.get_smi_info = get_smi_info;
	ssif_info->handlers.sender = sender;
	ssif_info->handlers.request_events = request_events;
	ssif_info->handlers.inc_usecount = inc_usecount;
	ssif_info->handlers.dec_usecount = dec_usecount;

	{
		unsigned int thread_num;

		thread_num = ((ssif_info->client->adapter->nr << 8) |
			      ssif_info->client->addr);
		init_completion(&ssif_info->wake_thread);
		ssif_info->thread = kthread_run(ipmi_ssif_thread, ssif_info,
					       "kssif%4.4x", thread_num);
		if (IS_ERR(ssif_info->thread)) {
			rv = PTR_ERR(ssif_info->thread);
			dev_notice(&ssif_info->client->dev,
				   "Could not start kernel thread: error %d\n",
				   rv);
			goto out;
		}
	}

	rv = ipmi_register_smi(&ssif_info->handlers,
			       ssif_info,
			       &ssif_info->device_id,
			       &ssif_info->client->dev,
			       slave_addr);
	 if (rv) {
		pr_err(PFX "Unable to register device: error %d\n", rv);
		goto out;
	}

	rv = ipmi_smi_add_proc_entry(ssif_info->intf, "type",
				     &smi_type_proc_ops,
				     ssif_info);
	if (rv) {
		pr_err(PFX "Unable to create proc entry: %d\n", rv);
		goto out_err_unreg;
	}

	rv = ipmi_smi_add_proc_entry(ssif_info->intf, "ssif_stats",
				     &smi_stats_proc_ops,
				     ssif_info);
	if (rv) {
		pr_err(PFX "Unable to create proc entry: %d\n", rv);
		goto out_err_unreg;
	}

 out:
	if (rv)
		kfree(ssif_info);
	kfree(resp);
	return rv;

 out_err_unreg:
	ipmi_unregister_smi(ssif_info->intf);
	goto out;
}

static int ssif_adapter_handler(struct device *adev, void *opaque)
{
	struct ssif_addr_info *addr_info = opaque;

	if (adev->type != &i2c_adapter_type)
		return 0;

	i2c_new_device(to_i2c_adapter(adev), &addr_info->binfo);

	if (!addr_info->adapter_name)
		return 1; /* Only try the first I2C adapter by default. */
	return 0;
}

static int new_ssif_client(int addr, char *adapter_name,
			   int debug, int slave_addr,
			   enum ipmi_addr_src addr_src)
{
	struct ssif_addr_info *addr_info;
	int rv = 0;

	mutex_lock(&ssif_infos_mutex);
	if (ssif_info_find(addr, adapter_name, false)) {
		rv = -EEXIST;
		goto out_unlock;
	}

	addr_info = kzalloc(sizeof(*addr_info), GFP_KERNEL);
	if (!addr_info) {
		rv = -ENOMEM;
		goto out_unlock;
	}

	if (adapter_name) {
		addr_info->adapter_name = kstrdup(adapter_name, GFP_KERNEL);
		if (!addr_info->adapter_name) {
			kfree(addr_info);
			rv = -ENOMEM;
			goto out_unlock;
		}
	}

	strncpy(addr_info->binfo.type, DEVICE_NAME,
		sizeof(addr_info->binfo.type));
	addr_info->binfo.addr = addr;
	addr_info->binfo.platform_data = addr_info;
	addr_info->debug = debug;
	addr_info->slave_addr = slave_addr;
	addr_info->addr_src = addr_src;

	list_add_tail(&addr_info->link, &ssif_infos);

	if (initialized)
		i2c_for_each_dev(addr_info, ssif_adapter_handler);
	/* Otherwise address list will get it */

out_unlock:
	mutex_unlock(&ssif_infos_mutex);
	return rv;
}

static void free_ssif_clients(void)
{
	struct ssif_addr_info *info, *tmp;

	mutex_lock(&ssif_infos_mutex);
	list_for_each_entry_safe(info, tmp, &ssif_infos, link) {
		list_del(&info->link);
		kfree(info->adapter_name);
		kfree(info);
	}
	mutex_unlock(&ssif_infos_mutex);
}

static unsigned short *ssif_address_list(void)
{
	struct ssif_addr_info *info;
	unsigned int count = 0, i;
	unsigned short *address_list;

	list_for_each_entry(info, &ssif_infos, link)
		count++;

	address_list = kzalloc(sizeof(*address_list) * (count + 1), GFP_KERNEL);
	if (!address_list)
		return NULL;

	i = 0;
	list_for_each_entry(info, &ssif_infos, link) {
		unsigned short addr = info->binfo.addr;
		int j;

		for (j = 0; j < i; j++) {
			if (address_list[j] == addr)
				goto skip_addr;
		}
		address_list[i] = addr;
skip_addr:
		i++;
	}
	address_list[i] = I2C_CLIENT_END;

	return address_list;
}

#ifdef CONFIG_ACPI
static struct acpi_device_id ssif_acpi_match[] = {
	{ "IPI0001", 0 },
	{ },
};
MODULE_DEVICE_TABLE(acpi, ssif_acpi_match);

/*
 * Once we get an ACPI failure, we don't try any more, because we go
 * through the tables sequentially.  Once we don't find a table, there
 * are no more.
 */
static int acpi_failure;

/*
 * Defined in the IPMI 2.0 spec.
 */
struct SPMITable {
	s8	Signature[4];
	u32	Length;
	u8	Revision;
	u8	Checksum;
	s8	OEMID[6];
	s8	OEMTableID[8];
	s8	OEMRevision[4];
	s8	CreatorID[4];
	s8	CreatorRevision[4];
	u8	InterfaceType;
	u8	IPMIlegacy;
	s16	SpecificationRevision;

	/*
	 * Bit 0 - SCI interrupt supported
	 * Bit 1 - I/O APIC/SAPIC
	 */
	u8	InterruptType;

	/*
	 * If bit 0 of InterruptType is set, then this is the SCI
	 * interrupt in the GPEx_STS register.
	 */
	u8	GPE;

	s16	Reserved;

	/*
	 * If bit 1 of InterruptType is set, then this is the I/O
	 * APIC/SAPIC interrupt.
	 */
	u32	GlobalSystemInterrupt;

	/* The actual register address. */
	struct acpi_generic_address addr;

	u8	UID[4];

	s8      spmi_id[1]; /* A '\0' terminated array starts here. */
};

static int try_init_spmi(struct SPMITable *spmi)
{
	unsigned short myaddr;

	if (num_addrs >= MAX_SSIF_BMCS)
		return -1;

	if (spmi->IPMIlegacy != 1) {
		pr_warn("IPMI: Bad SPMI legacy: %d\n", spmi->IPMIlegacy);
		return -ENODEV;
	}

	if (spmi->InterfaceType != 4)
		return -ENODEV;

	if (spmi->addr.space_id != ACPI_ADR_SPACE_SMBUS) {
		pr_warn(PFX "Invalid ACPI SSIF I/O Address type: %d\n",
			spmi->addr.space_id);
		return -EIO;
	}

	myaddr = spmi->addr.address >> 1;

	return new_ssif_client(myaddr, NULL, 0, 0, SI_SPMI);
}

static void spmi_find_bmc(void)
{
	acpi_status      status;
	struct SPMITable *spmi;
	int              i;

	if (acpi_disabled)
		return;

	if (acpi_failure)
		return;

	for (i = 0; ; i++) {
		status = acpi_get_table(ACPI_SIG_SPMI, i+1,
					(struct acpi_table_header **)&spmi);
		if (status != AE_OK)
			return;

		try_init_spmi(spmi);
	}
}
#else
static void spmi_find_bmc(void) { }
#endif

#ifdef CONFIG_DMI
static int decode_dmi(const struct dmi_device *dmi_dev)
{
	struct dmi_header *dm = dmi_dev->device_data;
	u8             *data = (u8 *) dm;
	u8             len = dm->length;
	unsigned short myaddr;
	int            slave_addr;

	if (num_addrs >= MAX_SSIF_BMCS)
		return -1;

	if (len < 9)
		return -1;

	if (data[0x04] != 4) /* Not SSIF */
		return -1;

	if ((data[8] >> 1) == 0) {
		/*
		 * Some broken systems put the I2C address in
		 * the slave address field.  We try to
		 * accommodate them here.
		 */
		myaddr = data[6] >> 1;
		slave_addr = 0;
	} else {
		myaddr = data[8] >> 1;
		slave_addr = data[6];
	}

	return new_ssif_client(myaddr, NULL, 0, 0, SI_SMBIOS);
}

static void dmi_iterator(void)
{
	const struct dmi_device *dev = NULL;

	while ((dev = dmi_find_device(DMI_DEV_TYPE_IPMI, NULL, dev)))
		decode_dmi(dev);
}
#else
static void dmi_iterator(void) { }
#endif

static const struct i2c_device_id ssif_id[] = {
	{ DEVICE_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ssif_id);

static struct i2c_driver ssif_i2c_driver = {
	.class		= I2C_CLASS_HWMON,
	.driver		= {
		.owner			= THIS_MODULE,
		.name			= DEVICE_NAME
	},
	.probe		= ssif_probe,
	.remove		= ssif_remove,
	.alert		= ssif_alert,
	.id_table	= ssif_id,
	.detect		= ssif_detect
};

static int init_ipmi_ssif(void)
{
	int i;
	int rv;

	if (initialized)
		return 0;

	pr_info("IPMI SSIF Interface driver\n");

	/* build list for i2c from addr list */
	for (i = 0; i < num_addrs; i++) {
		rv = new_ssif_client(addr[i], adapter_name[i],
				     dbg[i], slave_addrs[i],
				     SI_HARDCODED);
		if (rv)
			pr_err(PFX
			       "Couldn't add hardcoded device at addr 0x%x\n",
			       addr[i]);
	}

	if (ssif_tryacpi)
		ssif_i2c_driver.driver.acpi_match_table	=
			ACPI_PTR(ssif_acpi_match);
	if (ssif_trydmi)
		dmi_iterator();
	if (ssif_tryacpi)
		spmi_find_bmc();

	ssif_i2c_driver.address_list = ssif_address_list();

	rv = i2c_add_driver(&ssif_i2c_driver);
	if (!rv)
		initialized = true;

	return rv;
}
module_init(init_ipmi_ssif);

static void cleanup_ipmi_ssif(void)
{
	if (!initialized)
		return;

	initialized = false;

	i2c_del_driver(&ssif_i2c_driver);

	free_ssif_clients();
}
module_exit(cleanup_ipmi_ssif);

MODULE_AUTHOR("Todd C Davis <todd.c.davis@intel.com>, Corey Minyard <minyard@acm.org>");
MODULE_DESCRIPTION("IPMI driver for management controllers on a SMBus");
MODULE_LICENSE("GPL");
