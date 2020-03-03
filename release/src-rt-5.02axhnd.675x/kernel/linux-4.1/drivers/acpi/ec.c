/*
 *  ec.c - ACPI Embedded Controller Driver (v3)
 *
 *  Copyright (C) 2001-2015 Intel Corporation
 *    Author: 2014, 2015 Lv Zheng <lv.zheng@intel.com>
 *            2006, 2007 Alexey Starikovskiy <alexey.y.starikovskiy@intel.com>
 *            2006       Denis Sadykov <denis.m.sadykov@intel.com>
 *            2004       Luming Yu <luming.yu@intel.com>
 *            2001, 2002 Andy Grover <andrew.grover@intel.com>
 *            2001, 2002 Paul Diefenbaugh <paul.s.diefenbaugh@intel.com>
 *  Copyright (C) 2008      Alexey Starikovskiy <astarikovskiy@suse.de>
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or (at
 *  your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

/* Uncomment next line to get verbose printout */
/* #define DEBUG */
#define pr_fmt(fmt) "ACPI : EC: " fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/acpi.h>
#include <linux/dmi.h>
#include <asm/io.h>

#include "internal.h"

#define ACPI_EC_CLASS			"embedded_controller"
#define ACPI_EC_DEVICE_NAME		"Embedded Controller"
#define ACPI_EC_FILE_INFO		"info"

/* EC status register */
#define ACPI_EC_FLAG_OBF	0x01	/* Output buffer full */
#define ACPI_EC_FLAG_IBF	0x02	/* Input buffer full */
#define ACPI_EC_FLAG_CMD	0x08	/* Input buffer contains a command */
#define ACPI_EC_FLAG_BURST	0x10	/* burst mode */
#define ACPI_EC_FLAG_SCI	0x20	/* EC-SCI occurred */

/* EC commands */
enum ec_command {
	ACPI_EC_COMMAND_READ = 0x80,
	ACPI_EC_COMMAND_WRITE = 0x81,
	ACPI_EC_BURST_ENABLE = 0x82,
	ACPI_EC_BURST_DISABLE = 0x83,
	ACPI_EC_COMMAND_QUERY = 0x84,
};

#define ACPI_EC_DELAY		500	/* Wait 500ms max. during EC ops */
#define ACPI_EC_UDELAY_GLK	1000	/* Wait 1ms max. to get global lock */
#define ACPI_EC_MSI_UDELAY	550	/* Wait 550us for MSI EC */
#define ACPI_EC_UDELAY_POLL	1000	/* Wait 1ms for EC transaction polling */
#define ACPI_EC_CLEAR_MAX	100	/* Maximum number of events to query
					 * when trying to clear the EC */

enum {
	EC_FLAGS_QUERY_PENDING,		/* Query is pending */
	EC_FLAGS_HANDLERS_INSTALLED,	/* Handlers for GPE and
					 * OpReg are installed */
	EC_FLAGS_STARTED,		/* Driver is started */
	EC_FLAGS_STOPPED,		/* Driver is stopped */
	EC_FLAGS_COMMAND_STORM,		/* GPE storms occurred to the
					 * current command processing */
};

#define ACPI_EC_COMMAND_POLL		0x01 /* Available for command byte */
#define ACPI_EC_COMMAND_COMPLETE	0x02 /* Completed last byte */

/* ec.c is compiled in acpi namespace so this shows up as acpi.ec_delay param */
static unsigned int ec_delay __read_mostly = ACPI_EC_DELAY;
module_param(ec_delay, uint, 0644);
MODULE_PARM_DESC(ec_delay, "Timeout(ms) waited until an EC command completes");

/*
 * If the number of false interrupts per one transaction exceeds
 * this threshold, will think there is a GPE storm happened and
 * will disable the GPE for normal transaction.
 */
static unsigned int ec_storm_threshold  __read_mostly = 8;
module_param(ec_storm_threshold, uint, 0644);
MODULE_PARM_DESC(ec_storm_threshold, "Maxim false GPE numbers not considered as GPE storm");

struct acpi_ec_query_handler {
	struct list_head node;
	acpi_ec_query_func func;
	acpi_handle handle;
	void *data;
	u8 query_bit;
	struct kref kref;
};

struct transaction {
	const u8 *wdata;
	u8 *rdata;
	unsigned short irq_count;
	u8 command;
	u8 wi;
	u8 ri;
	u8 wlen;
	u8 rlen;
	u8 flags;
	unsigned long timestamp;
};

static int acpi_ec_query(struct acpi_ec *ec, u8 *data);
static void advance_transaction(struct acpi_ec *ec);

struct acpi_ec *boot_ec, *first_ec;
EXPORT_SYMBOL(first_ec);

static int EC_FLAGS_MSI; /* Out-of-spec MSI controller */
static int EC_FLAGS_VALIDATE_ECDT; /* ASUStec ECDTs need to be validated */
static int EC_FLAGS_SKIP_DSDT_SCAN; /* Not all BIOS survive early DSDT scan */
static int EC_FLAGS_CLEAR_ON_RESUME; /* Needs acpi_ec_clear() on boot/resume */
static int EC_FLAGS_QUERY_HANDSHAKE; /* Needs QR_EC issued when SCI_EVT set */

/* --------------------------------------------------------------------------
 *                           Logging/Debugging
 * -------------------------------------------------------------------------- */

/*
 * Splitters used by the developers to track the boundary of the EC
 * handling processes.
 */
#ifdef DEBUG
#define EC_DBG_SEP	" "
#define EC_DBG_DRV	"+++++"
#define EC_DBG_STM	"====="
#define EC_DBG_REQ	"*****"
#define EC_DBG_EVT	"#####"
#else
#define EC_DBG_SEP	""
#define EC_DBG_DRV
#define EC_DBG_STM
#define EC_DBG_REQ
#define EC_DBG_EVT
#endif

#define ec_log_raw(fmt, ...) \
	pr_info(fmt "\n", ##__VA_ARGS__)
#define ec_dbg_raw(fmt, ...) \
	pr_debug(fmt "\n", ##__VA_ARGS__)
#define ec_log(filter, fmt, ...) \
	ec_log_raw(filter EC_DBG_SEP fmt EC_DBG_SEP filter, ##__VA_ARGS__)
#define ec_dbg(filter, fmt, ...) \
	ec_dbg_raw(filter EC_DBG_SEP fmt EC_DBG_SEP filter, ##__VA_ARGS__)

#define ec_log_drv(fmt, ...) \
	ec_log(EC_DBG_DRV, fmt, ##__VA_ARGS__)
#define ec_dbg_drv(fmt, ...) \
	ec_dbg(EC_DBG_DRV, fmt, ##__VA_ARGS__)
#define ec_dbg_stm(fmt, ...) \
	ec_dbg(EC_DBG_STM, fmt, ##__VA_ARGS__)
#define ec_dbg_req(fmt, ...) \
	ec_dbg(EC_DBG_REQ, fmt, ##__VA_ARGS__)
#define ec_dbg_evt(fmt, ...) \
	ec_dbg(EC_DBG_EVT, fmt, ##__VA_ARGS__)
#define ec_dbg_ref(ec, fmt, ...) \
	ec_dbg_raw("%lu: " fmt, ec->reference_count, ## __VA_ARGS__)

/* --------------------------------------------------------------------------
 *                           Device Flags
 * -------------------------------------------------------------------------- */

static bool acpi_ec_started(struct acpi_ec *ec)
{
	return test_bit(EC_FLAGS_STARTED, &ec->flags) &&
	       !test_bit(EC_FLAGS_STOPPED, &ec->flags);
}

static bool acpi_ec_flushed(struct acpi_ec *ec)
{
	return ec->reference_count == 1;
}

/* --------------------------------------------------------------------------
 *                           EC Registers
 * -------------------------------------------------------------------------- */

static inline u8 acpi_ec_read_status(struct acpi_ec *ec)
{
	u8 x = inb(ec->command_addr);

	ec_dbg_raw("EC_SC(R) = 0x%2.2x "
		   "SCI_EVT=%d BURST=%d CMD=%d IBF=%d OBF=%d",
		   x,
		   !!(x & ACPI_EC_FLAG_SCI),
		   !!(x & ACPI_EC_FLAG_BURST),
		   !!(x & ACPI_EC_FLAG_CMD),
		   !!(x & ACPI_EC_FLAG_IBF),
		   !!(x & ACPI_EC_FLAG_OBF));
	return x;
}

static inline u8 acpi_ec_read_data(struct acpi_ec *ec)
{
	u8 x = inb(ec->data_addr);

	ec->curr->timestamp = jiffies;
	ec_dbg_raw("EC_DATA(R) = 0x%2.2x", x);
	return x;
}

static inline void acpi_ec_write_cmd(struct acpi_ec *ec, u8 command)
{
	ec_dbg_raw("EC_SC(W) = 0x%2.2x", command);
	outb(command, ec->command_addr);
	ec->curr->timestamp = jiffies;
}

static inline void acpi_ec_write_data(struct acpi_ec *ec, u8 data)
{
	ec_dbg_raw("EC_DATA(W) = 0x%2.2x", data);
	outb(data, ec->data_addr);
	ec->curr->timestamp = jiffies;
}

#ifdef DEBUG
static const char *acpi_ec_cmd_string(u8 cmd)
{
	switch (cmd) {
	case 0x80:
		return "RD_EC";
	case 0x81:
		return "WR_EC";
	case 0x82:
		return "BE_EC";
	case 0x83:
		return "BD_EC";
	case 0x84:
		return "QR_EC";
	}
	return "UNKNOWN";
}
#else
#define acpi_ec_cmd_string(cmd)		"UNDEF"
#endif

/* --------------------------------------------------------------------------
 *                           GPE Registers
 * -------------------------------------------------------------------------- */

static inline bool acpi_ec_is_gpe_raised(struct acpi_ec *ec)
{
	acpi_event_status gpe_status = 0;

	(void)acpi_get_gpe_status(NULL, ec->gpe, &gpe_status);
	return (gpe_status & ACPI_EVENT_FLAG_SET) ? true : false;
}

static inline void acpi_ec_enable_gpe(struct acpi_ec *ec, bool open)
{
	if (open)
		acpi_enable_gpe(NULL, ec->gpe);
	else {
		BUG_ON(ec->reference_count < 1);
		acpi_set_gpe(NULL, ec->gpe, ACPI_GPE_ENABLE);
	}
	if (acpi_ec_is_gpe_raised(ec)) {
		/*
		 * On some platforms, EN=1 writes cannot trigger GPE. So
		 * software need to manually trigger a pseudo GPE event on
		 * EN=1 writes.
		 */
		ec_dbg_raw("Polling quirk");
		advance_transaction(ec);
	}
}

static inline void acpi_ec_disable_gpe(struct acpi_ec *ec, bool close)
{
	if (close)
		acpi_disable_gpe(NULL, ec->gpe);
	else {
		BUG_ON(ec->reference_count < 1);
		acpi_set_gpe(NULL, ec->gpe, ACPI_GPE_DISABLE);
	}
}

static inline void acpi_ec_clear_gpe(struct acpi_ec *ec)
{
	/*
	 * GPE STS is a W1C register, which means:
	 * 1. Software can clear it without worrying about clearing other
	 *    GPEs' STS bits when the hardware sets them in parallel.
	 * 2. As long as software can ensure only clearing it when it is
	 *    set, hardware won't set it in parallel.
	 * So software can clear GPE in any contexts.
	 * Warning: do not move the check into advance_transaction() as the
	 * EC commands will be sent without GPE raised.
	 */
	if (!acpi_ec_is_gpe_raised(ec))
		return;
	acpi_clear_gpe(NULL, ec->gpe);
}

/* --------------------------------------------------------------------------
 *                           Transaction Management
 * -------------------------------------------------------------------------- */

static void acpi_ec_submit_request(struct acpi_ec *ec)
{
	ec->reference_count++;
	if (ec->reference_count == 1)
		acpi_ec_enable_gpe(ec, true);
}

static void acpi_ec_complete_request(struct acpi_ec *ec)
{
	bool flushed = false;

	ec->reference_count--;
	if (ec->reference_count == 0)
		acpi_ec_disable_gpe(ec, true);
	flushed = acpi_ec_flushed(ec);
	if (flushed)
		wake_up(&ec->wait);
}

static void acpi_ec_set_storm(struct acpi_ec *ec, u8 flag)
{
	if (!test_bit(flag, &ec->flags)) {
		acpi_ec_disable_gpe(ec, false);
		ec_dbg_drv("Polling enabled");
		set_bit(flag, &ec->flags);
	}
}

static void acpi_ec_clear_storm(struct acpi_ec *ec, u8 flag)
{
	if (test_bit(flag, &ec->flags)) {
		clear_bit(flag, &ec->flags);
		acpi_ec_enable_gpe(ec, false);
		ec_dbg_drv("Polling disabled");
	}
}

/*
 * acpi_ec_submit_flushable_request() - Increase the reference count unless
 *                                      the flush operation is not in
 *                                      progress
 * @ec: the EC device
 *
 * This function must be used before taking a new action that should hold
 * the reference count.  If this function returns false, then the action
 * must be discarded or it will prevent the flush operation from being
 * completed.
 */
static bool acpi_ec_submit_flushable_request(struct acpi_ec *ec)
{
	if (!acpi_ec_started(ec))
		return false;
	acpi_ec_submit_request(ec);
	return true;
}

static void acpi_ec_submit_query(struct acpi_ec *ec)
{
	if (!test_and_set_bit(EC_FLAGS_QUERY_PENDING, &ec->flags)) {
		ec_dbg_req("Event started");
		schedule_work(&ec->work);
	}
}

static void acpi_ec_complete_query(struct acpi_ec *ec)
{
	if (ec->curr->command == ACPI_EC_COMMAND_QUERY) {
		clear_bit(EC_FLAGS_QUERY_PENDING, &ec->flags);
		ec_dbg_req("Event stopped");
	}
}

static int ec_transaction_completed(struct acpi_ec *ec)
{
	unsigned long flags;
	int ret = 0;

	spin_lock_irqsave(&ec->lock, flags);
	if (ec->curr && (ec->curr->flags & ACPI_EC_COMMAND_COMPLETE))
		ret = 1;
	spin_unlock_irqrestore(&ec->lock, flags);
	return ret;
}

static void advance_transaction(struct acpi_ec *ec)
{
	struct transaction *t;
	u8 status;
	bool wakeup = false;

	ec_dbg_stm("%s (%d)", in_interrupt() ? "IRQ" : "TASK",
		   smp_processor_id());
	/*
	 * By always clearing STS before handling all indications, we can
	 * ensure a hardware STS 0->1 change after this clearing can always
	 * trigger a GPE interrupt.
	 */
	acpi_ec_clear_gpe(ec);
	status = acpi_ec_read_status(ec);
	t = ec->curr;
	if (!t)
		goto err;
	if (t->flags & ACPI_EC_COMMAND_POLL) {
		if (t->wlen > t->wi) {
			if ((status & ACPI_EC_FLAG_IBF) == 0)
				acpi_ec_write_data(ec, t->wdata[t->wi++]);
			else
				goto err;
		} else if (t->rlen > t->ri) {
			if ((status & ACPI_EC_FLAG_OBF) == 1) {
				t->rdata[t->ri++] = acpi_ec_read_data(ec);
				if (t->rlen == t->ri) {
					t->flags |= ACPI_EC_COMMAND_COMPLETE;
					if (t->command == ACPI_EC_COMMAND_QUERY)
						ec_dbg_req("Command(%s) hardware completion",
							   acpi_ec_cmd_string(t->command));
					wakeup = true;
				}
			} else
				goto err;
		} else if (t->wlen == t->wi &&
			   (status & ACPI_EC_FLAG_IBF) == 0) {
			t->flags |= ACPI_EC_COMMAND_COMPLETE;
			wakeup = true;
		}
		goto out;
	} else {
		if (EC_FLAGS_QUERY_HANDSHAKE &&
		    !(status & ACPI_EC_FLAG_SCI) &&
		    (t->command == ACPI_EC_COMMAND_QUERY)) {
			t->flags |= ACPI_EC_COMMAND_POLL;
			acpi_ec_complete_query(ec);
			t->rdata[t->ri++] = 0x00;
			t->flags |= ACPI_EC_COMMAND_COMPLETE;
			ec_dbg_req("Command(%s) software completion",
				   acpi_ec_cmd_string(t->command));
			wakeup = true;
		} else if ((status & ACPI_EC_FLAG_IBF) == 0) {
			acpi_ec_write_cmd(ec, t->command);
			t->flags |= ACPI_EC_COMMAND_POLL;
			acpi_ec_complete_query(ec);
		} else
			goto err;
		goto out;
	}
err:
	/*
	 * If SCI bit is set, then don't think it's a false IRQ
	 * otherwise will take a not handled IRQ as a false one.
	 */
	if (!(status & ACPI_EC_FLAG_SCI)) {
		if (in_interrupt() && t) {
			if (t->irq_count < ec_storm_threshold)
				++t->irq_count;
			/* Allow triggering on 0 threshold */
			if (t->irq_count == ec_storm_threshold)
				acpi_ec_set_storm(ec, EC_FLAGS_COMMAND_STORM);
		}
	}
out:
	if (status & ACPI_EC_FLAG_SCI)
		acpi_ec_submit_query(ec);
	if (wakeup && in_interrupt())
		wake_up(&ec->wait);
}

static void start_transaction(struct acpi_ec *ec)
{
	ec->curr->irq_count = ec->curr->wi = ec->curr->ri = 0;
	ec->curr->flags = 0;
	ec->curr->timestamp = jiffies;
	advance_transaction(ec);
}

static int ec_poll(struct acpi_ec *ec)
{
	unsigned long flags;
	int repeat = 5; /* number of command restarts */

	while (repeat--) {
		unsigned long delay = jiffies +
			msecs_to_jiffies(ec_delay);
		unsigned long usecs = ACPI_EC_UDELAY_POLL;
		do {
			/* don't sleep with disabled interrupts */
			if (EC_FLAGS_MSI || irqs_disabled()) {
				usecs = ACPI_EC_MSI_UDELAY;
				udelay(usecs);
				if (ec_transaction_completed(ec))
					return 0;
			} else {
				if (wait_event_timeout(ec->wait,
						ec_transaction_completed(ec),
						usecs_to_jiffies(usecs)))
					return 0;
			}
			spin_lock_irqsave(&ec->lock, flags);
			if (time_after(jiffies,
					ec->curr->timestamp +
					usecs_to_jiffies(usecs)))
				advance_transaction(ec);
			spin_unlock_irqrestore(&ec->lock, flags);
		} while (time_before(jiffies, delay));
		pr_debug("controller reset, restart transaction\n");
		spin_lock_irqsave(&ec->lock, flags);
		start_transaction(ec);
		spin_unlock_irqrestore(&ec->lock, flags);
	}
	return -ETIME;
}

static int acpi_ec_transaction_unlocked(struct acpi_ec *ec,
					struct transaction *t)
{
	unsigned long tmp;
	int ret = 0;

	if (EC_FLAGS_MSI)
		udelay(ACPI_EC_MSI_UDELAY);
	/* start transaction */
	spin_lock_irqsave(&ec->lock, tmp);
	/* Enable GPE for command processing (IBF=0/OBF=1) */
	if (!acpi_ec_submit_flushable_request(ec)) {
		ret = -EINVAL;
		goto unlock;
	}
	ec_dbg_ref(ec, "Increase command");
	/* following two actions should be kept atomic */
	ec->curr = t;
	ec_dbg_req("Command(%s) started", acpi_ec_cmd_string(t->command));
	start_transaction(ec);
	spin_unlock_irqrestore(&ec->lock, tmp);
	ret = ec_poll(ec);
	spin_lock_irqsave(&ec->lock, tmp);
	if (t->irq_count == ec_storm_threshold)
		acpi_ec_clear_storm(ec, EC_FLAGS_COMMAND_STORM);
	ec_dbg_req("Command(%s) stopped", acpi_ec_cmd_string(t->command));
	ec->curr = NULL;
	/* Disable GPE for command processing (IBF=0/OBF=1) */
	acpi_ec_complete_request(ec);
	ec_dbg_ref(ec, "Decrease command");
unlock:
	spin_unlock_irqrestore(&ec->lock, tmp);
	return ret;
}

static int acpi_ec_transaction(struct acpi_ec *ec, struct transaction *t)
{
	int status;
	u32 glk;

	if (!ec || (!t) || (t->wlen && !t->wdata) || (t->rlen && !t->rdata))
		return -EINVAL;
	if (t->rdata)
		memset(t->rdata, 0, t->rlen);
	mutex_lock(&ec->mutex);
	if (ec->global_lock) {
		status = acpi_acquire_global_lock(ACPI_EC_UDELAY_GLK, &glk);
		if (ACPI_FAILURE(status)) {
			status = -ENODEV;
			goto unlock;
		}
	}

	status = acpi_ec_transaction_unlocked(ec, t);

	if (test_bit(EC_FLAGS_COMMAND_STORM, &ec->flags))
		msleep(1);
	if (ec->global_lock)
		acpi_release_global_lock(glk);
unlock:
	mutex_unlock(&ec->mutex);
	return status;
}

static int acpi_ec_burst_enable(struct acpi_ec *ec)
{
	u8 d;
	struct transaction t = {.command = ACPI_EC_BURST_ENABLE,
				.wdata = NULL, .rdata = &d,
				.wlen = 0, .rlen = 1};

	return acpi_ec_transaction(ec, &t);
}

static int acpi_ec_burst_disable(struct acpi_ec *ec)
{
	struct transaction t = {.command = ACPI_EC_BURST_DISABLE,
				.wdata = NULL, .rdata = NULL,
				.wlen = 0, .rlen = 0};

	return (acpi_ec_read_status(ec) & ACPI_EC_FLAG_BURST) ?
				acpi_ec_transaction(ec, &t) : 0;
}

static int acpi_ec_read(struct acpi_ec *ec, u8 address, u8 *data)
{
	int result;
	u8 d;
	struct transaction t = {.command = ACPI_EC_COMMAND_READ,
				.wdata = &address, .rdata = &d,
				.wlen = 1, .rlen = 1};

	result = acpi_ec_transaction(ec, &t);
	*data = d;
	return result;
}

static int acpi_ec_write(struct acpi_ec *ec, u8 address, u8 data)
{
	u8 wdata[2] = { address, data };
	struct transaction t = {.command = ACPI_EC_COMMAND_WRITE,
				.wdata = wdata, .rdata = NULL,
				.wlen = 2, .rlen = 0};

	return acpi_ec_transaction(ec, &t);
}

int ec_read(u8 addr, u8 *val)
{
	int err;
	u8 temp_data;

	if (!first_ec)
		return -ENODEV;

	err = acpi_ec_read(first_ec, addr, &temp_data);

	if (!err) {
		*val = temp_data;
		return 0;
	}
	return err;
}
EXPORT_SYMBOL(ec_read);

int ec_write(u8 addr, u8 val)
{
	int err;

	if (!first_ec)
		return -ENODEV;

	err = acpi_ec_write(first_ec, addr, val);

	return err;
}
EXPORT_SYMBOL(ec_write);

int ec_transaction(u8 command,
		   const u8 *wdata, unsigned wdata_len,
		   u8 *rdata, unsigned rdata_len)
{
	struct transaction t = {.command = command,
				.wdata = wdata, .rdata = rdata,
				.wlen = wdata_len, .rlen = rdata_len};

	if (!first_ec)
		return -ENODEV;

	return acpi_ec_transaction(first_ec, &t);
}
EXPORT_SYMBOL(ec_transaction);

/* Get the handle to the EC device */
acpi_handle ec_get_handle(void)
{
	if (!first_ec)
		return NULL;
	return first_ec->handle;
}
EXPORT_SYMBOL(ec_get_handle);

/*
 * Process _Q events that might have accumulated in the EC.
 * Run with locked ec mutex.
 */
static void acpi_ec_clear(struct acpi_ec *ec)
{
	int i, status;
	u8 value = 0;

	for (i = 0; i < ACPI_EC_CLEAR_MAX; i++) {
		status = acpi_ec_query(ec, &value);
		if (status || !value)
			break;
	}

	if (unlikely(i == ACPI_EC_CLEAR_MAX))
		pr_warn("Warning: Maximum of %d stale EC events cleared\n", i);
	else
		pr_info("%d stale EC events cleared\n", i);
}

static void acpi_ec_start(struct acpi_ec *ec, bool resuming)
{
	unsigned long flags;

	spin_lock_irqsave(&ec->lock, flags);
	if (!test_and_set_bit(EC_FLAGS_STARTED, &ec->flags)) {
		ec_dbg_drv("Starting EC");
		/* Enable GPE for event processing (SCI_EVT=1) */
		if (!resuming) {
			acpi_ec_submit_request(ec);
			ec_dbg_ref(ec, "Increase driver");
		}
		ec_log_drv("EC started");
	}
	spin_unlock_irqrestore(&ec->lock, flags);
}

static bool acpi_ec_stopped(struct acpi_ec *ec)
{
	unsigned long flags;
	bool flushed;

	spin_lock_irqsave(&ec->lock, flags);
	flushed = acpi_ec_flushed(ec);
	spin_unlock_irqrestore(&ec->lock, flags);
	return flushed;
}

static void acpi_ec_stop(struct acpi_ec *ec, bool suspending)
{
	unsigned long flags;

	spin_lock_irqsave(&ec->lock, flags);
	if (acpi_ec_started(ec)) {
		ec_dbg_drv("Stopping EC");
		set_bit(EC_FLAGS_STOPPED, &ec->flags);
		spin_unlock_irqrestore(&ec->lock, flags);
		wait_event(ec->wait, acpi_ec_stopped(ec));
		spin_lock_irqsave(&ec->lock, flags);
		/* Disable GPE for event processing (SCI_EVT=1) */
		if (!suspending) {
			acpi_ec_complete_request(ec);
			ec_dbg_ref(ec, "Decrease driver");
		}
		clear_bit(EC_FLAGS_STARTED, &ec->flags);
		clear_bit(EC_FLAGS_STOPPED, &ec->flags);
		ec_log_drv("EC stopped");
	}
	spin_unlock_irqrestore(&ec->lock, flags);
}

void acpi_ec_block_transactions(void)
{
	struct acpi_ec *ec = first_ec;

	if (!ec)
		return;

	mutex_lock(&ec->mutex);
	/* Prevent transactions from being carried out */
	acpi_ec_stop(ec, true);
	mutex_unlock(&ec->mutex);
}

void acpi_ec_unblock_transactions(void)
{
	struct acpi_ec *ec = first_ec;

	if (!ec)
		return;

	/* Allow transactions to be carried out again */
	acpi_ec_start(ec, true);

	if (EC_FLAGS_CLEAR_ON_RESUME)
		acpi_ec_clear(ec);
}

void acpi_ec_unblock_transactions_early(void)
{
	/*
	 * Allow transactions to happen again (this function is called from
	 * atomic context during wakeup, so we don't need to acquire the mutex).
	 */
	if (first_ec)
		acpi_ec_start(first_ec, true);
}

/* --------------------------------------------------------------------------
                                Event Management
   -------------------------------------------------------------------------- */
static struct acpi_ec_query_handler *
acpi_ec_get_query_handler(struct acpi_ec_query_handler *handler)
{
	if (handler)
		kref_get(&handler->kref);
	return handler;
}

static void acpi_ec_query_handler_release(struct kref *kref)
{
	struct acpi_ec_query_handler *handler =
		container_of(kref, struct acpi_ec_query_handler, kref);

	kfree(handler);
}

static void acpi_ec_put_query_handler(struct acpi_ec_query_handler *handler)
{
	kref_put(&handler->kref, acpi_ec_query_handler_release);
}

int acpi_ec_add_query_handler(struct acpi_ec *ec, u8 query_bit,
			      acpi_handle handle, acpi_ec_query_func func,
			      void *data)
{
	struct acpi_ec_query_handler *handler =
	    kzalloc(sizeof(struct acpi_ec_query_handler), GFP_KERNEL);

	if (!handler)
		return -ENOMEM;

	handler->query_bit = query_bit;
	handler->handle = handle;
	handler->func = func;
	handler->data = data;
	mutex_lock(&ec->mutex);
	kref_init(&handler->kref);
	list_add(&handler->node, &ec->list);
	mutex_unlock(&ec->mutex);
	return 0;
}
EXPORT_SYMBOL_GPL(acpi_ec_add_query_handler);

void acpi_ec_remove_query_handler(struct acpi_ec *ec, u8 query_bit)
{
	struct acpi_ec_query_handler *handler, *tmp;
	LIST_HEAD(free_list);

	mutex_lock(&ec->mutex);
	list_for_each_entry_safe(handler, tmp, &ec->list, node) {
		if (query_bit == handler->query_bit) {
			list_del_init(&handler->node);
			list_add(&handler->node, &free_list);
		}
	}
	mutex_unlock(&ec->mutex);
	list_for_each_entry_safe(handler, tmp, &free_list, node)
		acpi_ec_put_query_handler(handler);
}
EXPORT_SYMBOL_GPL(acpi_ec_remove_query_handler);

static void acpi_ec_run(void *cxt)
{
	struct acpi_ec_query_handler *handler = cxt;

	if (!handler)
		return;
	ec_dbg_evt("Query(0x%02x) started", handler->query_bit);
	if (handler->func)
		handler->func(handler->data);
	else if (handler->handle)
		acpi_evaluate_object(handler->handle, NULL, NULL, NULL);
	ec_dbg_evt("Query(0x%02x) stopped", handler->query_bit);
	acpi_ec_put_query_handler(handler);
}

static int acpi_ec_query(struct acpi_ec *ec, u8 *data)
{
	u8 value = 0;
	int result;
	acpi_status status;
	struct acpi_ec_query_handler *handler;
	struct transaction t = {.command = ACPI_EC_COMMAND_QUERY,
				.wdata = NULL, .rdata = &value,
				.wlen = 0, .rlen = 1};

	/*
	 * Query the EC to find out which _Qxx method we need to evaluate.
	 * Note that successful completion of the query causes the ACPI_EC_SCI
	 * bit to be cleared (and thus clearing the interrupt source).
	 */
	result = acpi_ec_transaction(ec, &t);
	if (result)
		return result;
	if (data)
		*data = value;
	if (!value)
		return -ENODATA;

	mutex_lock(&ec->mutex);
	list_for_each_entry(handler, &ec->list, node) {
		if (value == handler->query_bit) {
			/* have custom handler for this bit */
			handler = acpi_ec_get_query_handler(handler);
			ec_dbg_evt("Query(0x%02x) scheduled",
				   handler->query_bit);
			status = acpi_os_execute((handler->func) ?
				OSL_NOTIFY_HANDLER : OSL_GPE_HANDLER,
				acpi_ec_run, handler);
			if (ACPI_FAILURE(status))
				result = -EBUSY;
			break;
		}
	}
	mutex_unlock(&ec->mutex);
	return result;
}

static void acpi_ec_gpe_poller(struct work_struct *work)
{
	struct acpi_ec *ec = container_of(work, struct acpi_ec, work);

	acpi_ec_query(ec, NULL);
}

static u32 acpi_ec_gpe_handler(acpi_handle gpe_device,
	u32 gpe_number, void *data)
{
	unsigned long flags;
	struct acpi_ec *ec = data;

	spin_lock_irqsave(&ec->lock, flags);
	advance_transaction(ec);
	spin_unlock_irqrestore(&ec->lock, flags);
	return ACPI_INTERRUPT_HANDLED;
}

/* --------------------------------------------------------------------------
 *                           Address Space Management
 * -------------------------------------------------------------------------- */

static acpi_status
acpi_ec_space_handler(u32 function, acpi_physical_address address,
		      u32 bits, u64 *value64,
		      void *handler_context, void *region_context)
{
	struct acpi_ec *ec = handler_context;
	int result = 0, i, bytes = bits / 8;
	u8 *value = (u8 *)value64;

	if ((address > 0xFF) || !value || !handler_context)
		return AE_BAD_PARAMETER;

	if (function != ACPI_READ && function != ACPI_WRITE)
		return AE_BAD_PARAMETER;

	if (EC_FLAGS_MSI || bits > 8)
		acpi_ec_burst_enable(ec);

	for (i = 0; i < bytes; ++i, ++address, ++value)
		result = (function == ACPI_READ) ?
			acpi_ec_read(ec, address, value) :
			acpi_ec_write(ec, address, *value);

	if (EC_FLAGS_MSI || bits > 8)
		acpi_ec_burst_disable(ec);

	switch (result) {
	case -EINVAL:
		return AE_BAD_PARAMETER;
	case -ENODEV:
		return AE_NOT_FOUND;
	case -ETIME:
		return AE_TIME;
	default:
		return AE_OK;
	}
}

/* --------------------------------------------------------------------------
 *                             Driver Interface
 * -------------------------------------------------------------------------- */

static acpi_status
ec_parse_io_ports(struct acpi_resource *resource, void *context);

static struct acpi_ec *make_acpi_ec(void)
{
	struct acpi_ec *ec = kzalloc(sizeof(struct acpi_ec), GFP_KERNEL);

	if (!ec)
		return NULL;
	ec->flags = 1 << EC_FLAGS_QUERY_PENDING;
	mutex_init(&ec->mutex);
	init_waitqueue_head(&ec->wait);
	INIT_LIST_HEAD(&ec->list);
	spin_lock_init(&ec->lock);
	INIT_WORK(&ec->work, acpi_ec_gpe_poller);
	return ec;
}

static acpi_status
acpi_ec_register_query_methods(acpi_handle handle, u32 level,
			       void *context, void **return_value)
{
	char node_name[5];
	struct acpi_buffer buffer = { sizeof(node_name), node_name };
	struct acpi_ec *ec = context;
	int value = 0;
	acpi_status status;

	status = acpi_get_name(handle, ACPI_SINGLE_NAME, &buffer);

	if (ACPI_SUCCESS(status) && sscanf(node_name, "_Q%x", &value) == 1)
		acpi_ec_add_query_handler(ec, value, handle, NULL, NULL);
	return AE_OK;
}

static acpi_status
ec_parse_device(acpi_handle handle, u32 Level, void *context, void **retval)
{
	acpi_status status;
	unsigned long long tmp = 0;
	struct acpi_ec *ec = context;

	/* clear addr values, ec_parse_io_ports depend on it */
	ec->command_addr = ec->data_addr = 0;

	status = acpi_walk_resources(handle, METHOD_NAME__CRS,
				     ec_parse_io_ports, ec);
	if (ACPI_FAILURE(status))
		return status;

	/* Get GPE bit assignment (EC events). */
	/* TODO: Add support for _GPE returning a package */
	status = acpi_evaluate_integer(handle, "_GPE", NULL, &tmp);
	if (ACPI_FAILURE(status))
		return status;
	ec->gpe = tmp;
	/* Use the global lock for all EC transactions? */
	tmp = 0;
	acpi_evaluate_integer(handle, "_GLK", NULL, &tmp);
	ec->global_lock = tmp;
	ec->handle = handle;
	return AE_CTRL_TERMINATE;
}

static int ec_install_handlers(struct acpi_ec *ec)
{
	acpi_status status;

	if (test_bit(EC_FLAGS_HANDLERS_INSTALLED, &ec->flags))
		return 0;
	status = acpi_install_gpe_raw_handler(NULL, ec->gpe,
				  ACPI_GPE_EDGE_TRIGGERED,
				  &acpi_ec_gpe_handler, ec);
	if (ACPI_FAILURE(status))
		return -ENODEV;

	acpi_ec_start(ec, false);
	status = acpi_install_address_space_handler(ec->handle,
						    ACPI_ADR_SPACE_EC,
						    &acpi_ec_space_handler,
						    NULL, ec);
	if (ACPI_FAILURE(status)) {
		if (status == AE_NOT_FOUND) {
			/*
			 * Maybe OS fails in evaluating the _REG object.
			 * The AE_NOT_FOUND error will be ignored and OS
			 * continue to initialize EC.
			 */
			pr_err("Fail in evaluating the _REG object"
				" of EC device. Broken bios is suspected.\n");
		} else {
			acpi_ec_stop(ec, false);
			acpi_remove_gpe_handler(NULL, ec->gpe,
				&acpi_ec_gpe_handler);
			return -ENODEV;
		}
	}

	set_bit(EC_FLAGS_HANDLERS_INSTALLED, &ec->flags);
	return 0;
}

static void ec_remove_handlers(struct acpi_ec *ec)
{
	if (!test_bit(EC_FLAGS_HANDLERS_INSTALLED, &ec->flags))
		return;
	acpi_ec_stop(ec, false);
	if (ACPI_FAILURE(acpi_remove_address_space_handler(ec->handle,
				ACPI_ADR_SPACE_EC, &acpi_ec_space_handler)))
		pr_err("failed to remove space handler\n");
	if (ACPI_FAILURE(acpi_remove_gpe_handler(NULL, ec->gpe,
				&acpi_ec_gpe_handler)))
		pr_err("failed to remove gpe handler\n");
	clear_bit(EC_FLAGS_HANDLERS_INSTALLED, &ec->flags);
}

static int acpi_ec_add(struct acpi_device *device)
{
	struct acpi_ec *ec = NULL;
	int ret;

	strcpy(acpi_device_name(device), ACPI_EC_DEVICE_NAME);
	strcpy(acpi_device_class(device), ACPI_EC_CLASS);

	/* Check for boot EC */
	if (boot_ec &&
	    (boot_ec->handle == device->handle ||
	     boot_ec->handle == ACPI_ROOT_OBJECT)) {
		ec = boot_ec;
		boot_ec = NULL;
	} else {
		ec = make_acpi_ec();
		if (!ec)
			return -ENOMEM;
	}
	if (ec_parse_device(device->handle, 0, ec, NULL) !=
		AE_CTRL_TERMINATE) {
			kfree(ec);
			return -EINVAL;
	}

	/* Find and register all query methods */
	acpi_walk_namespace(ACPI_TYPE_METHOD, ec->handle, 1,
			    acpi_ec_register_query_methods, NULL, ec, NULL);

	if (!first_ec)
		first_ec = ec;
	device->driver_data = ec;

	ret = !!request_region(ec->data_addr, 1, "EC data");
	WARN(!ret, "Could not request EC data io port 0x%lx", ec->data_addr);
	ret = !!request_region(ec->command_addr, 1, "EC cmd");
	WARN(!ret, "Could not request EC cmd io port 0x%lx", ec->command_addr);

	pr_info("GPE = 0x%lx, I/O: command/status = 0x%lx, data = 0x%lx\n",
			  ec->gpe, ec->command_addr, ec->data_addr);

	ret = ec_install_handlers(ec);

	/* Reprobe devices depending on the EC */
	acpi_walk_dep_device_list(ec->handle);

	/* EC is fully operational, allow queries */
	clear_bit(EC_FLAGS_QUERY_PENDING, &ec->flags);

	/* Clear stale _Q events if hardware might require that */
	if (EC_FLAGS_CLEAR_ON_RESUME)
		acpi_ec_clear(ec);
	return ret;
}

static int acpi_ec_remove(struct acpi_device *device)
{
	struct acpi_ec *ec;
	struct acpi_ec_query_handler *handler, *tmp;

	if (!device)
		return -EINVAL;

	ec = acpi_driver_data(device);
	ec_remove_handlers(ec);
	mutex_lock(&ec->mutex);
	list_for_each_entry_safe(handler, tmp, &ec->list, node) {
		list_del(&handler->node);
		kfree(handler);
	}
	mutex_unlock(&ec->mutex);
	release_region(ec->data_addr, 1);
	release_region(ec->command_addr, 1);
	device->driver_data = NULL;
	if (ec == first_ec)
		first_ec = NULL;
	kfree(ec);
	return 0;
}

static acpi_status
ec_parse_io_ports(struct acpi_resource *resource, void *context)
{
	struct acpi_ec *ec = context;

	if (resource->type != ACPI_RESOURCE_TYPE_IO)
		return AE_OK;

	/*
	 * The first address region returned is the data port, and
	 * the second address region returned is the status/command
	 * port.
	 */
	if (ec->data_addr == 0)
		ec->data_addr = resource->data.io.minimum;
	else if (ec->command_addr == 0)
		ec->command_addr = resource->data.io.minimum;
	else
		return AE_CTRL_TERMINATE;

	return AE_OK;
}

int __init acpi_boot_ec_enable(void)
{
	if (!boot_ec || test_bit(EC_FLAGS_HANDLERS_INSTALLED, &boot_ec->flags))
		return 0;
	if (!ec_install_handlers(boot_ec)) {
		first_ec = boot_ec;
		return 0;
	}
	return -EFAULT;
}

static const struct acpi_device_id ec_device_ids[] = {
	{"PNP0C09", 0},
	{"", 0},
};

/* Some BIOS do not survive early DSDT scan, skip it */
static int ec_skip_dsdt_scan(const struct dmi_system_id *id)
{
	EC_FLAGS_SKIP_DSDT_SCAN = 1;
	return 0;
}

/* ASUStek often supplies us with broken ECDT, validate it */
static int ec_validate_ecdt(const struct dmi_system_id *id)
{
	EC_FLAGS_VALIDATE_ECDT = 1;
	return 0;
}

/* MSI EC needs special treatment, enable it */
static int ec_flag_msi(const struct dmi_system_id *id)
{
	pr_debug("Detected MSI hardware, enabling workarounds.\n");
	EC_FLAGS_MSI = 1;
	EC_FLAGS_VALIDATE_ECDT = 1;
	return 0;
}

/*
 * Clevo M720 notebook actually works ok with IRQ mode, if we lifted
 * the GPE storm threshold back to 20
 */
static int ec_enlarge_storm_threshold(const struct dmi_system_id *id)
{
	pr_debug("Setting the EC GPE storm threshold to 20\n");
	ec_storm_threshold  = 20;
	return 0;
}

/*
 * Acer EC firmware refuses to respond QR_EC when SCI_EVT is not set, for
 * which case, we complete the QR_EC without issuing it to the firmware.
 * https://bugzilla.kernel.org/show_bug.cgi?id=86211
 */
static int ec_flag_query_handshake(const struct dmi_system_id *id)
{
	pr_debug("Detected the EC firmware requiring QR_EC issued when SCI_EVT set\n");
	EC_FLAGS_QUERY_HANDSHAKE = 1;
	return 0;
}

/*
 * On some hardware it is necessary to clear events accumulated by the EC during
 * sleep. These ECs stop reporting GPEs until they are manually polled, if too
 * many events are accumulated. (e.g. Samsung Series 5/9 notebooks)
 *
 * https://bugzilla.kernel.org/show_bug.cgi?id=44161
 *
 * Ideally, the EC should also be instructed NOT to accumulate events during
 * sleep (which Windows seems to do somehow), but the interface to control this
 * behaviour is not known at this time.
 *
 * Models known to be affected are Samsung 530Uxx/535Uxx/540Uxx/550Pxx/900Xxx,
 * however it is very likely that other Samsung models are affected.
 *
 * On systems which don't accumulate _Q events during sleep, this extra check
 * should be harmless.
 */
static int ec_clear_on_resume(const struct dmi_system_id *id)
{
	pr_debug("Detected system needing EC poll on resume.\n");
	EC_FLAGS_CLEAR_ON_RESUME = 1;
	return 0;
}

static struct dmi_system_id ec_dmi_table[] __initdata = {
	{
	ec_skip_dsdt_scan, "Compal JFL92", {
	DMI_MATCH(DMI_BIOS_VENDOR, "COMPAL"),
	DMI_MATCH(DMI_BOARD_NAME, "JFL92") }, NULL},
	{
	ec_flag_msi, "MSI hardware", {
	DMI_MATCH(DMI_BIOS_VENDOR, "Micro-Star")}, NULL},
	{
	ec_flag_msi, "MSI hardware", {
	DMI_MATCH(DMI_SYS_VENDOR, "Micro-Star")}, NULL},
	{
	ec_flag_msi, "MSI hardware", {
	DMI_MATCH(DMI_CHASSIS_VENDOR, "MICRO-Star")}, NULL},
	{
	ec_flag_msi, "MSI hardware", {
	DMI_MATCH(DMI_CHASSIS_VENDOR, "MICRO-STAR")}, NULL},
	{
	ec_flag_msi, "Quanta hardware", {
	DMI_MATCH(DMI_SYS_VENDOR, "Quanta"),
	DMI_MATCH(DMI_PRODUCT_NAME, "TW8/SW8/DW8"),}, NULL},
	{
	ec_flag_msi, "Quanta hardware", {
	DMI_MATCH(DMI_SYS_VENDOR, "Quanta"),
	DMI_MATCH(DMI_PRODUCT_NAME, "TW9/SW9"),}, NULL},
	{
	ec_flag_msi, "Clevo W350etq", {
	DMI_MATCH(DMI_SYS_VENDOR, "CLEVO CO."),
	DMI_MATCH(DMI_PRODUCT_NAME, "W35_37ET"),}, NULL},
	{
	ec_validate_ecdt, "ASUS hardware", {
	DMI_MATCH(DMI_BIOS_VENDOR, "ASUS") }, NULL},
	{
	ec_validate_ecdt, "ASUS hardware", {
	DMI_MATCH(DMI_BOARD_VENDOR, "ASUSTeK Computer Inc.") }, NULL},
	{
	ec_enlarge_storm_threshold, "CLEVO hardware", {
	DMI_MATCH(DMI_SYS_VENDOR, "CLEVO Co."),
	DMI_MATCH(DMI_PRODUCT_NAME, "M720T/M730T"),}, NULL},
	{
	ec_skip_dsdt_scan, "HP Folio 13", {
	DMI_MATCH(DMI_SYS_VENDOR, "Hewlett-Packard"),
	DMI_MATCH(DMI_PRODUCT_NAME, "HP Folio 13"),}, NULL},
	{
	ec_validate_ecdt, "ASUS hardware", {
	DMI_MATCH(DMI_SYS_VENDOR, "ASUSTek Computer Inc."),
	DMI_MATCH(DMI_PRODUCT_NAME, "L4R"),}, NULL},
	{
	ec_clear_on_resume, "Samsung hardware", {
	DMI_MATCH(DMI_SYS_VENDOR, "SAMSUNG ELECTRONICS CO., LTD.")}, NULL},
	{
	ec_flag_query_handshake, "Acer hardware", {
	DMI_MATCH(DMI_SYS_VENDOR, "Acer"), }, NULL},
	{},
};

int __init acpi_ec_ecdt_probe(void)
{
	acpi_status status;
	struct acpi_ec *saved_ec = NULL;
	struct acpi_table_ecdt *ecdt_ptr;

	boot_ec = make_acpi_ec();
	if (!boot_ec)
		return -ENOMEM;
	/*
	 * Generate a boot ec context
	 */
	dmi_check_system(ec_dmi_table);
	status = acpi_get_table(ACPI_SIG_ECDT, 1,
				(struct acpi_table_header **)&ecdt_ptr);
	if (ACPI_SUCCESS(status)) {
		pr_info("EC description table is found, configuring boot EC\n");
		boot_ec->command_addr = ecdt_ptr->control.address;
		boot_ec->data_addr = ecdt_ptr->data.address;
		boot_ec->gpe = ecdt_ptr->gpe;
		boot_ec->handle = ACPI_ROOT_OBJECT;
		acpi_get_handle(ACPI_ROOT_OBJECT, ecdt_ptr->id,
				&boot_ec->handle);
		/* Don't trust ECDT, which comes from ASUSTek */
		if (!EC_FLAGS_VALIDATE_ECDT)
			goto install;
		saved_ec = kmemdup(boot_ec, sizeof(struct acpi_ec), GFP_KERNEL);
		if (!saved_ec)
			return -ENOMEM;
	/* fall through */
	}

	if (EC_FLAGS_SKIP_DSDT_SCAN) {
		kfree(saved_ec);
		return -ENODEV;
	}

	/* This workaround is needed only on some broken machines,
	 * which require early EC, but fail to provide ECDT */
	pr_debug("Look up EC in DSDT\n");
	status = acpi_get_devices(ec_device_ids[0].id, ec_parse_device,
					boot_ec, NULL);
	/* Check that acpi_get_devices actually find something */
	if (ACPI_FAILURE(status) || !boot_ec->handle)
		goto error;
	if (saved_ec) {
		/* try to find good ECDT from ASUSTek */
		if (saved_ec->command_addr != boot_ec->command_addr ||
		    saved_ec->data_addr != boot_ec->data_addr ||
		    saved_ec->gpe != boot_ec->gpe ||
		    saved_ec->handle != boot_ec->handle)
			pr_info("ASUSTek keeps feeding us with broken "
			"ECDT tables, which are very hard to workaround. "
			"Trying to use DSDT EC info instead. Please send "
			"output of acpidump to linux-acpi@vger.kernel.org\n");
		kfree(saved_ec);
		saved_ec = NULL;
	} else {
		/* We really need to limit this workaround, the only ASUS,
		* which needs it, has fake EC._INI method, so use it as flag.
		* Keep boot_ec struct as it will be needed soon.
		*/
		if (!dmi_name_in_vendors("ASUS") ||
		    !acpi_has_method(boot_ec->handle, "_INI"))
			return -ENODEV;
	}
install:
	if (!ec_install_handlers(boot_ec)) {
		first_ec = boot_ec;
		return 0;
	}
error:
	kfree(boot_ec);
	kfree(saved_ec);
	boot_ec = NULL;
	return -ENODEV;
}

static struct acpi_driver acpi_ec_driver = {
	.name = "ec",
	.class = ACPI_EC_CLASS,
	.ids = ec_device_ids,
	.ops = {
		.add = acpi_ec_add,
		.remove = acpi_ec_remove,
		},
};

int __init acpi_ec_init(void)
{
	int result = 0;

	/* Now register the driver for the EC */
	result = acpi_bus_register_driver(&acpi_ec_driver);
	if (result < 0)
		return -ENODEV;

	return result;
}

/* EC driver currently not unloadable */
#if 0
static void __exit acpi_ec_exit(void)
{

	acpi_bus_unregister_driver(&acpi_ec_driver);
}
#endif	/* 0 */
