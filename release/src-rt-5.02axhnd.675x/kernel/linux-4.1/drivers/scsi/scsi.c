/*
 *  scsi.c Copyright (C) 1992 Drew Eckhardt
 *         Copyright (C) 1993, 1994, 1995, 1999 Eric Youngdale
 *         Copyright (C) 2002, 2003 Christoph Hellwig
 *
 *  generic mid-level SCSI driver
 *      Initial versions: Drew Eckhardt
 *      Subsequent revisions: Eric Youngdale
 *
 *  <drew@colorado.edu>
 *
 *  Bug correction thanks go to :
 *      Rik Faith <faith@cs.unc.edu>
 *      Tommy Thorn <tthorn>
 *      Thomas Wuensche <tw@fgb1.fgb.mw.tu-muenchen.de>
 *
 *  Modified by Eric Youngdale eric@andante.org or ericy@gnu.ai.mit.edu to
 *  add scatter-gather, multiple outstanding request, and other
 *  enhancements.
 *
 *  Native multichannel, wide scsi, /proc/scsi and hot plugging
 *  support added by Michael Neuffer <mike@i-connect.net>
 *
 *  Added request_module("scsi_hostadapter") for kerneld:
 *  (Put an "alias scsi_hostadapter your_hostadapter" in /etc/modprobe.conf)
 *  Bjorn Ekwall  <bj0rn@blox.se>
 *  (changed to kmod)
 *
 *  Major improvements to the timeout, abort, and reset processing,
 *  as well as performance modifications for large queue depths by
 *  Leonard N. Zubkoff <lnz@dandelion.com>
 *
 *  Converted cli() code to spinlocks, Ingo Molnar
 *
 *  Jiffies wrap fixes (host->resetting), 3 Dec 1998 Andrea Arcangeli
 *
 *  out_of_space hacks, D. Gilbert (dpg) 990608
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/blkdev.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/completion.h>
#include <linux/unistd.h>
#include <linux/spinlock.h>
#include <linux/kmod.h>
#include <linux/interrupt.h>
#include <linux/notifier.h>
#include <linux/cpu.h>
#include <linux/mutex.h>
#include <linux/async.h>
#include <asm/unaligned.h>

#include <scsi/scsi.h>
#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_dbg.h>
#include <scsi/scsi_device.h>
#include <scsi/scsi_driver.h>
#include <scsi/scsi_eh.h>
#include <scsi/scsi_host.h>
#include <scsi/scsi_tcq.h>

#include "scsi_priv.h"
#include "scsi_logging.h"

#define CREATE_TRACE_POINTS
#include <trace/events/scsi.h>

/*
 * Definitions and constants.
 */

/*
 * Note - the initial logging level can be set here to log events at boot time.
 * After the system is up, you may enable logging via the /proc interface.
 */
unsigned int scsi_logging_level;
#if defined(CONFIG_SCSI_LOGGING)
EXPORT_SYMBOL(scsi_logging_level);
#endif

/* sd, scsi core and power management need to coordinate flushing async actions */
ASYNC_DOMAIN(scsi_sd_probe_domain);
EXPORT_SYMBOL(scsi_sd_probe_domain);

/*
 * Separate domain (from scsi_sd_probe_domain) to maximize the benefit of
 * asynchronous system resume operations.  It is marked 'exclusive' to avoid
 * being included in the async_synchronize_full() that is invoked by
 * dpm_resume()
 */
ASYNC_DOMAIN_EXCLUSIVE(scsi_sd_pm_domain);
EXPORT_SYMBOL(scsi_sd_pm_domain);

/* NB: These are exposed through /proc/scsi/scsi and form part of the ABI.
 * You may not alter any existing entry (although adding new ones is
 * encouraged once assigned by ANSI/INCITS T10
 */
static const char *const scsi_device_types[] = {
	"Direct-Access    ",
	"Sequential-Access",
	"Printer          ",
	"Processor        ",
	"WORM             ",
	"CD-ROM           ",
	"Scanner          ",
	"Optical Device   ",
	"Medium Changer   ",
	"Communications   ",
	"ASC IT8          ",
	"ASC IT8          ",
	"RAID             ",
	"Enclosure        ",
	"Direct-Access-RBC",
	"Optical card     ",
	"Bridge controller",
	"Object storage   ",
	"Automation/Drive ",
	"Security Manager ",
	"Direct-Access-ZBC",
};

/**
 * scsi_device_type - Return 17 char string indicating device type.
 * @type: type number to look up
 */

const char * scsi_device_type(unsigned type)
{
	if (type == 0x1e)
		return "Well-known LUN   ";
	if (type == 0x1f)
		return "No Device        ";
	if (type >= ARRAY_SIZE(scsi_device_types))
		return "Unknown          ";
	return scsi_device_types[type];
}

EXPORT_SYMBOL(scsi_device_type);

struct scsi_host_cmd_pool {
	struct kmem_cache	*cmd_slab;
	struct kmem_cache	*sense_slab;
	unsigned int		users;
	char			*cmd_name;
	char			*sense_name;
	unsigned int		slab_flags;
	gfp_t			gfp_mask;
};

static struct scsi_host_cmd_pool scsi_cmd_pool = {
	.cmd_name	= "scsi_cmd_cache",
	.sense_name	= "scsi_sense_cache",
	.slab_flags	= SLAB_HWCACHE_ALIGN,
};

static struct scsi_host_cmd_pool scsi_cmd_dma_pool = {
	.cmd_name	= "scsi_cmd_cache(DMA)",
	.sense_name	= "scsi_sense_cache(DMA)",
	.slab_flags	= SLAB_HWCACHE_ALIGN|SLAB_CACHE_DMA,
	.gfp_mask	= __GFP_DMA,
};

static DEFINE_MUTEX(host_cmd_pool_mutex);

/**
 * scsi_host_free_command - internal function to release a command
 * @shost:	host to free the command for
 * @cmd:	command to release
 *
 * the command must previously have been allocated by
 * scsi_host_alloc_command.
 */
static void
scsi_host_free_command(struct Scsi_Host *shost, struct scsi_cmnd *cmd)
{
	struct scsi_host_cmd_pool *pool = shost->cmd_pool;

	if (cmd->prot_sdb)
		kmem_cache_free(scsi_sdb_cache, cmd->prot_sdb);
	kmem_cache_free(pool->sense_slab, cmd->sense_buffer);
	kmem_cache_free(pool->cmd_slab, cmd);
}

/**
 * scsi_host_alloc_command - internal function to allocate command
 * @shost:	SCSI host whose pool to allocate from
 * @gfp_mask:	mask for the allocation
 *
 * Returns a fully allocated command with sense buffer and protection
 * data buffer (where applicable) or NULL on failure
 */
static struct scsi_cmnd *
scsi_host_alloc_command(struct Scsi_Host *shost, gfp_t gfp_mask)
{
	struct scsi_host_cmd_pool *pool = shost->cmd_pool;
	struct scsi_cmnd *cmd;

	cmd = kmem_cache_zalloc(pool->cmd_slab, gfp_mask | pool->gfp_mask);
	if (!cmd)
		goto fail;

	cmd->sense_buffer = kmem_cache_alloc(pool->sense_slab,
					     gfp_mask | pool->gfp_mask);
	if (!cmd->sense_buffer)
		goto fail_free_cmd;

	if (scsi_host_get_prot(shost) >= SHOST_DIX_TYPE0_PROTECTION) {
		cmd->prot_sdb = kmem_cache_zalloc(scsi_sdb_cache, gfp_mask);
		if (!cmd->prot_sdb)
			goto fail_free_sense;
	}

	return cmd;

fail_free_sense:
	kmem_cache_free(pool->sense_slab, cmd->sense_buffer);
fail_free_cmd:
	kmem_cache_free(pool->cmd_slab, cmd);
fail:
	return NULL;
}

/**
 * __scsi_get_command - Allocate a struct scsi_cmnd
 * @shost: host to transmit command
 * @gfp_mask: allocation mask
 *
 * Description: allocate a struct scsi_cmd from host's slab, recycling from the
 *              host's free_list if necessary.
 */
static struct scsi_cmnd *
__scsi_get_command(struct Scsi_Host *shost, gfp_t gfp_mask)
{
	struct scsi_cmnd *cmd = scsi_host_alloc_command(shost, gfp_mask);

	if (unlikely(!cmd)) {
		unsigned long flags;

		spin_lock_irqsave(&shost->free_list_lock, flags);
		if (likely(!list_empty(&shost->free_list))) {
			cmd = list_entry(shost->free_list.next,
					 struct scsi_cmnd, list);
			list_del_init(&cmd->list);
		}
		spin_unlock_irqrestore(&shost->free_list_lock, flags);

		if (cmd) {
			void *buf, *prot;

			buf = cmd->sense_buffer;
			prot = cmd->prot_sdb;

			memset(cmd, 0, sizeof(*cmd));

			cmd->sense_buffer = buf;
			cmd->prot_sdb = prot;
		}
	}

	return cmd;
}

/**
 * scsi_get_command - Allocate and setup a scsi command block
 * @dev: parent scsi device
 * @gfp_mask: allocator flags
 *
 * Returns:	The allocated scsi command structure.
 */
struct scsi_cmnd *scsi_get_command(struct scsi_device *dev, gfp_t gfp_mask)
{
	struct scsi_cmnd *cmd = __scsi_get_command(dev->host, gfp_mask);
	unsigned long flags;

	if (unlikely(cmd == NULL))
		return NULL;

	cmd->device = dev;
	INIT_LIST_HEAD(&cmd->list);
	INIT_DELAYED_WORK(&cmd->abort_work, scmd_eh_abort_handler);
	spin_lock_irqsave(&dev->list_lock, flags);
	list_add_tail(&cmd->list, &dev->cmd_list);
	spin_unlock_irqrestore(&dev->list_lock, flags);
	cmd->jiffies_at_alloc = jiffies;
	return cmd;
}

/**
 * __scsi_put_command - Free a struct scsi_cmnd
 * @shost: dev->host
 * @cmd: Command to free
 */
static void __scsi_put_command(struct Scsi_Host *shost, struct scsi_cmnd *cmd)
{
	unsigned long flags;

	if (unlikely(list_empty(&shost->free_list))) {
		spin_lock_irqsave(&shost->free_list_lock, flags);
		if (list_empty(&shost->free_list)) {
			list_add(&cmd->list, &shost->free_list);
			cmd = NULL;
		}
		spin_unlock_irqrestore(&shost->free_list_lock, flags);
	}

	if (likely(cmd != NULL))
		scsi_host_free_command(shost, cmd);
}

/**
 * scsi_put_command - Free a scsi command block
 * @cmd: command block to free
 *
 * Returns:	Nothing.
 *
 * Notes:	The command must not belong to any lists.
 */
void scsi_put_command(struct scsi_cmnd *cmd)
{
	unsigned long flags;

	/* serious error if the command hasn't come from a device list */
	spin_lock_irqsave(&cmd->device->list_lock, flags);
	BUG_ON(list_empty(&cmd->list));
	list_del_init(&cmd->list);
	spin_unlock_irqrestore(&cmd->device->list_lock, flags);

	BUG_ON(delayed_work_pending(&cmd->abort_work));

	__scsi_put_command(cmd->device->host, cmd);
}

static struct scsi_host_cmd_pool *
scsi_find_host_cmd_pool(struct Scsi_Host *shost)
{
	if (shost->hostt->cmd_size)
		return shost->hostt->cmd_pool;
	if (shost->unchecked_isa_dma)
		return &scsi_cmd_dma_pool;
	return &scsi_cmd_pool;
}

static void
scsi_free_host_cmd_pool(struct scsi_host_cmd_pool *pool)
{
	kfree(pool->sense_name);
	kfree(pool->cmd_name);
	kfree(pool);
}

static struct scsi_host_cmd_pool *
scsi_alloc_host_cmd_pool(struct Scsi_Host *shost)
{
	struct scsi_host_template *hostt = shost->hostt;
	struct scsi_host_cmd_pool *pool;

	pool = kzalloc(sizeof(*pool), GFP_KERNEL);
	if (!pool)
		return NULL;

	pool->cmd_name = kasprintf(GFP_KERNEL, "%s_cmd", hostt->proc_name);
	pool->sense_name = kasprintf(GFP_KERNEL, "%s_sense", hostt->proc_name);
	if (!pool->cmd_name || !pool->sense_name) {
		scsi_free_host_cmd_pool(pool);
		return NULL;
	}

	pool->slab_flags = SLAB_HWCACHE_ALIGN;
	if (shost->unchecked_isa_dma) {
		pool->slab_flags |= SLAB_CACHE_DMA;
		pool->gfp_mask = __GFP_DMA;
	}

	if (hostt->cmd_size)
		hostt->cmd_pool = pool;

	return pool;
}

static struct scsi_host_cmd_pool *
scsi_get_host_cmd_pool(struct Scsi_Host *shost)
{
	struct scsi_host_template *hostt = shost->hostt;
	struct scsi_host_cmd_pool *retval = NULL, *pool;
	size_t cmd_size = sizeof(struct scsi_cmnd) + hostt->cmd_size;

	/*
	 * Select a command slab for this host and create it if not
	 * yet existent.
	 */
	mutex_lock(&host_cmd_pool_mutex);
	pool = scsi_find_host_cmd_pool(shost);
	if (!pool) {
		pool = scsi_alloc_host_cmd_pool(shost);
		if (!pool)
			goto out;
	}

	if (!pool->users) {
		pool->cmd_slab = kmem_cache_create(pool->cmd_name, cmd_size, 0,
						   pool->slab_flags, NULL);
		if (!pool->cmd_slab)
			goto out_free_pool;

		pool->sense_slab = kmem_cache_create(pool->sense_name,
						     SCSI_SENSE_BUFFERSIZE, 0,
						     pool->slab_flags, NULL);
		if (!pool->sense_slab)
			goto out_free_slab;
	}

	pool->users++;
	retval = pool;
out:
	mutex_unlock(&host_cmd_pool_mutex);
	return retval;

out_free_slab:
	kmem_cache_destroy(pool->cmd_slab);
out_free_pool:
	if (hostt->cmd_size) {
		scsi_free_host_cmd_pool(pool);
		hostt->cmd_pool = NULL;
	}
	goto out;
}

static void scsi_put_host_cmd_pool(struct Scsi_Host *shost)
{
	struct scsi_host_template *hostt = shost->hostt;
	struct scsi_host_cmd_pool *pool;

	mutex_lock(&host_cmd_pool_mutex);
	pool = scsi_find_host_cmd_pool(shost);

	/*
	 * This may happen if a driver has a mismatched get and put
	 * of the command pool; the driver should be implicated in
	 * the stack trace
	 */
	BUG_ON(pool->users == 0);

	if (!--pool->users) {
		kmem_cache_destroy(pool->cmd_slab);
		kmem_cache_destroy(pool->sense_slab);
		if (hostt->cmd_size) {
			scsi_free_host_cmd_pool(pool);
			hostt->cmd_pool = NULL;
		}
	}
	mutex_unlock(&host_cmd_pool_mutex);
}

/**
 * scsi_setup_command_freelist - Setup the command freelist for a scsi host.
 * @shost: host to allocate the freelist for.
 *
 * Description: The command freelist protects against system-wide out of memory
 * deadlock by preallocating one SCSI command structure for each host, so the
 * system can always write to a swap file on a device associated with that host.
 *
 * Returns:	Nothing.
 */
int scsi_setup_command_freelist(struct Scsi_Host *shost)
{
	const gfp_t gfp_mask = shost->unchecked_isa_dma ? GFP_DMA : GFP_KERNEL;
	struct scsi_cmnd *cmd;

	spin_lock_init(&shost->free_list_lock);
	INIT_LIST_HEAD(&shost->free_list);

	shost->cmd_pool = scsi_get_host_cmd_pool(shost);
	if (!shost->cmd_pool)
		return -ENOMEM;

	/*
	 * Get one backup command for this host.
	 */
	cmd = scsi_host_alloc_command(shost, gfp_mask);
	if (!cmd) {
		scsi_put_host_cmd_pool(shost);
		shost->cmd_pool = NULL;
		return -ENOMEM;
	}
	list_add(&cmd->list, &shost->free_list);
	return 0;
}

/**
 * scsi_destroy_command_freelist - Release the command freelist for a scsi host.
 * @shost: host whose freelist is going to be destroyed
 */
void scsi_destroy_command_freelist(struct Scsi_Host *shost)
{
	/*
	 * If cmd_pool is NULL the free list was not initialized, so
	 * do not attempt to release resources.
	 */
	if (!shost->cmd_pool)
		return;

	while (!list_empty(&shost->free_list)) {
		struct scsi_cmnd *cmd;

		cmd = list_entry(shost->free_list.next, struct scsi_cmnd, list);
		list_del_init(&cmd->list);
		scsi_host_free_command(shost, cmd);
	}
	shost->cmd_pool = NULL;
	scsi_put_host_cmd_pool(shost);
}

#ifdef CONFIG_SCSI_LOGGING
void scsi_log_send(struct scsi_cmnd *cmd)
{
	unsigned int level;

	/*
	 * If ML QUEUE log level is greater than or equal to:
	 *
	 * 1: nothing (match completion)
	 *
	 * 2: log opcode + command of all commands + cmd address
	 *
	 * 3: same as 2
	 *
	 * 4: same as 3
	 */
	if (unlikely(scsi_logging_level)) {
		level = SCSI_LOG_LEVEL(SCSI_LOG_MLQUEUE_SHIFT,
				       SCSI_LOG_MLQUEUE_BITS);
		if (level > 1) {
			scmd_printk(KERN_INFO, cmd,
				    "Send: scmd 0x%p\n", cmd);
			scsi_print_command(cmd);
		}
	}
}

void scsi_log_completion(struct scsi_cmnd *cmd, int disposition)
{
	unsigned int level;

	/*
	 * If ML COMPLETE log level is greater than or equal to:
	 *
	 * 1: log disposition, result, opcode + command, and conditionally
	 * sense data for failures or non SUCCESS dispositions.
	 *
	 * 2: same as 1 but for all command completions.
	 *
	 * 3: same as 2
	 *
	 * 4: same as 3 plus dump extra junk
	 */
	if (unlikely(scsi_logging_level)) {
		level = SCSI_LOG_LEVEL(SCSI_LOG_MLCOMPLETE_SHIFT,
				       SCSI_LOG_MLCOMPLETE_BITS);
		if (((level > 0) && (cmd->result || disposition != SUCCESS)) ||
		    (level > 1)) {
			scsi_print_result(cmd, "Done", disposition);
			scsi_print_command(cmd);
			if (status_byte(cmd->result) & CHECK_CONDITION)
				scsi_print_sense(cmd);
			if (level > 3)
				scmd_printk(KERN_INFO, cmd,
					    "scsi host busy %d failed %d\n",
					    atomic_read(&cmd->device->host->host_busy),
					    cmd->device->host->host_failed);
		}
	}
}
#endif

/**
 * scsi_cmd_get_serial - Assign a serial number to a command
 * @host: the scsi host
 * @cmd: command to assign serial number to
 *
 * Description: a serial number identifies a request for error recovery
 * and debugging purposes.  Protected by the Host_Lock of host.
 */
void scsi_cmd_get_serial(struct Scsi_Host *host, struct scsi_cmnd *cmd)
{
	cmd->serial_number = host->cmd_serial_number++;
	if (cmd->serial_number == 0) 
		cmd->serial_number = host->cmd_serial_number++;
}
EXPORT_SYMBOL(scsi_cmd_get_serial);

/**
 * scsi_finish_command - cleanup and pass command back to upper layer
 * @cmd: the command
 *
 * Description: Pass command off to upper layer for finishing of I/O
 *              request, waking processes that are waiting on results,
 *              etc.
 */
void scsi_finish_command(struct scsi_cmnd *cmd)
{
	struct scsi_device *sdev = cmd->device;
	struct scsi_target *starget = scsi_target(sdev);
	struct Scsi_Host *shost = sdev->host;
	struct scsi_driver *drv;
	unsigned int good_bytes;

	scsi_device_unbusy(sdev);

	/*
	 * Clear the flags that say that the device/target/host is no longer
	 * capable of accepting new commands.
	 */
	if (atomic_read(&shost->host_blocked))
		atomic_set(&shost->host_blocked, 0);
	if (atomic_read(&starget->target_blocked))
		atomic_set(&starget->target_blocked, 0);
	if (atomic_read(&sdev->device_blocked))
		atomic_set(&sdev->device_blocked, 0);

	/*
	 * If we have valid sense information, then some kind of recovery
	 * must have taken place.  Make a note of this.
	 */
	if (SCSI_SENSE_VALID(cmd))
		cmd->result |= (DRIVER_SENSE << 24);

	SCSI_LOG_MLCOMPLETE(4, sdev_printk(KERN_INFO, sdev,
				"Notifying upper driver of completion "
				"(result %x)\n", cmd->result));

	good_bytes = scsi_bufflen(cmd);
        if (cmd->request->cmd_type != REQ_TYPE_BLOCK_PC) {
		int old_good_bytes = good_bytes;
		drv = scsi_cmd_to_driver(cmd);
		if (drv->done)
			good_bytes = drv->done(cmd);
		/*
		 * USB may not give sense identifying bad sector and
		 * simply return a residue instead, so subtract off the
		 * residue if drv->done() error processing indicates no
		 * change to the completion length.
		 */
		if (good_bytes == old_good_bytes)
			good_bytes -= scsi_get_resid(cmd);
	}
	scsi_io_completion(cmd, good_bytes);
}

/**
 * scsi_change_queue_depth - change a device's queue depth
 * @sdev: SCSI Device in question
 * @depth: number of commands allowed to be queued to the driver
 *
 * Sets the device queue depth and returns the new value.
 */
int scsi_change_queue_depth(struct scsi_device *sdev, int depth)
{
	unsigned long flags;

	if (depth <= 0)
		goto out;

	spin_lock_irqsave(sdev->request_queue->queue_lock, flags);

	/*
	 * Check to see if the queue is managed by the block layer.
	 * If it is, and we fail to adjust the depth, exit.
	 *
	 * Do not resize the tag map if it is a host wide share bqt,
	 * because the size should be the hosts's can_queue. If there
	 * is more IO than the LLD's can_queue (so there are not enuogh
	 * tags) request_fn's host queue ready check will handle it.
	 */
	if (!shost_use_blk_mq(sdev->host) && !sdev->host->bqt) {
		if (blk_queue_tagged(sdev->request_queue) &&
		    blk_queue_resize_tags(sdev->request_queue, depth) != 0)
			goto out_unlock;
	}

	sdev->queue_depth = depth;
out_unlock:
	spin_unlock_irqrestore(sdev->request_queue->queue_lock, flags);
out:
	return sdev->queue_depth;
}
EXPORT_SYMBOL(scsi_change_queue_depth);

/**
 * scsi_track_queue_full - track QUEUE_FULL events to adjust queue depth
 * @sdev: SCSI Device in question
 * @depth: Current number of outstanding SCSI commands on this device,
 *         not counting the one returned as QUEUE_FULL.
 *
 * Description:	This function will track successive QUEUE_FULL events on a
 * 		specific SCSI device to determine if and when there is a
 * 		need to adjust the queue depth on the device.
 *
 * Returns:	0 - No change needed, >0 - Adjust queue depth to this new depth,
 * 		-1 - Drop back to untagged operation using host->cmd_per_lun
 * 			as the untagged command depth
 *
 * Lock Status:	None held on entry
 *
 * Notes:	Low level drivers may call this at any time and we will do
 * 		"The Right Thing."  We are interrupt context safe.
 */
int scsi_track_queue_full(struct scsi_device *sdev, int depth)
{

	/*
	 * Don't let QUEUE_FULLs on the same
	 * jiffies count, they could all be from
	 * same event.
	 */
	if ((jiffies >> 4) == (sdev->last_queue_full_time >> 4))
		return 0;

	sdev->last_queue_full_time = jiffies;
	if (sdev->last_queue_full_depth != depth) {
		sdev->last_queue_full_count = 1;
		sdev->last_queue_full_depth = depth;
	} else {
		sdev->last_queue_full_count++;
	}

	if (sdev->last_queue_full_count <= 10)
		return 0;

	return scsi_change_queue_depth(sdev, depth);
}
EXPORT_SYMBOL(scsi_track_queue_full);

/**
 * scsi_vpd_inquiry - Request a device provide us with a VPD page
 * @sdev: The device to ask
 * @buffer: Where to put the result
 * @page: Which Vital Product Data to return
 * @len: The length of the buffer
 *
 * This is an internal helper function.  You probably want to use
 * scsi_get_vpd_page instead.
 *
 * Returns size of the vpd page on success or a negative error number.
 */
static int scsi_vpd_inquiry(struct scsi_device *sdev, unsigned char *buffer,
							u8 page, unsigned len)
{
	int result;
	unsigned char cmd[16];

	if (len < 4)
		return -EINVAL;

	cmd[0] = INQUIRY;
	cmd[1] = 1;		/* EVPD */
	cmd[2] = page;
	cmd[3] = len >> 8;
	cmd[4] = len & 0xff;
	cmd[5] = 0;		/* Control byte */

	/*
	 * I'm not convinced we need to try quite this hard to get VPD, but
	 * all the existing users tried this hard.
	 */
	result = scsi_execute_req(sdev, cmd, DMA_FROM_DEVICE, buffer,
				  len, NULL, 30 * HZ, 3, NULL);
	if (result)
		return -EIO;

	/* Sanity check that we got the page back that we asked for */
	if (buffer[1] != page)
		return -EIO;

	return get_unaligned_be16(&buffer[2]) + 4;
}

/**
 * scsi_get_vpd_page - Get Vital Product Data from a SCSI device
 * @sdev: The device to ask
 * @page: Which Vital Product Data to return
 * @buf: where to store the VPD
 * @buf_len: number of bytes in the VPD buffer area
 *
 * SCSI devices may optionally supply Vital Product Data.  Each 'page'
 * of VPD is defined in the appropriate SCSI document (eg SPC, SBC).
 * If the device supports this VPD page, this routine returns a pointer
 * to a buffer containing the data from that page.  The caller is
 * responsible for calling kfree() on this pointer when it is no longer
 * needed.  If we cannot retrieve the VPD page this routine returns %NULL.
 */
int scsi_get_vpd_page(struct scsi_device *sdev, u8 page, unsigned char *buf,
		      int buf_len)
{
	int i, result;

	if (sdev->skip_vpd_pages)
		goto fail;

	/* Ask for all the pages supported by this device */
	result = scsi_vpd_inquiry(sdev, buf, 0, buf_len);
	if (result < 4)
		goto fail;

	/* If the user actually wanted this page, we can skip the rest */
	if (page == 0)
		return 0;

	for (i = 4; i < min(result, buf_len); i++)
		if (buf[i] == page)
			goto found;

	if (i < result && i >= buf_len)
		/* ran off the end of the buffer, give us benefit of doubt */
		goto found;
	/* The device claims it doesn't support the requested page */
	goto fail;

 found:
	result = scsi_vpd_inquiry(sdev, buf, page, buf_len);
	if (result < 0)
		goto fail;

	return 0;

 fail:
	return -EINVAL;
}
EXPORT_SYMBOL_GPL(scsi_get_vpd_page);

/**
 * scsi_attach_vpd - Attach Vital Product Data to a SCSI device structure
 * @sdev: The device to ask
 *
 * Attach the 'Device Identification' VPD page (0x83) and the
 * 'Unit Serial Number' VPD page (0x80) to a SCSI device
 * structure. This information can be used to identify the device
 * uniquely.
 */
void scsi_attach_vpd(struct scsi_device *sdev)
{
	int result, i;
	int vpd_len = SCSI_VPD_PG_LEN;
	int pg80_supported = 0;
	int pg83_supported = 0;
	unsigned char *vpd_buf;

	if (sdev->skip_vpd_pages)
		return;
retry_pg0:
	vpd_buf = kmalloc(vpd_len, GFP_KERNEL);
	if (!vpd_buf)
		return;

	/* Ask for all the pages supported by this device */
	result = scsi_vpd_inquiry(sdev, vpd_buf, 0, vpd_len);
	if (result < 0) {
		kfree(vpd_buf);
		return;
	}
	if (result > vpd_len) {
		vpd_len = result;
		kfree(vpd_buf);
		goto retry_pg0;
	}

	for (i = 4; i < result; i++) {
		if (vpd_buf[i] == 0x80)
			pg80_supported = 1;
		if (vpd_buf[i] == 0x83)
			pg83_supported = 1;
	}
	kfree(vpd_buf);
	vpd_len = SCSI_VPD_PG_LEN;

	if (pg80_supported) {
retry_pg80:
		vpd_buf = kmalloc(vpd_len, GFP_KERNEL);
		if (!vpd_buf)
			return;

		result = scsi_vpd_inquiry(sdev, vpd_buf, 0x80, vpd_len);
		if (result < 0) {
			kfree(vpd_buf);
			return;
		}
		if (result > vpd_len) {
			vpd_len = result;
			kfree(vpd_buf);
			goto retry_pg80;
		}
		sdev->vpd_pg80_len = result;
		sdev->vpd_pg80 = vpd_buf;
		vpd_len = SCSI_VPD_PG_LEN;
	}

	if (pg83_supported) {
retry_pg83:
		vpd_buf = kmalloc(vpd_len, GFP_KERNEL);
		if (!vpd_buf)
			return;

		result = scsi_vpd_inquiry(sdev, vpd_buf, 0x83, vpd_len);
		if (result < 0) {
			kfree(vpd_buf);
			return;
		}
		if (result > vpd_len) {
			vpd_len = result;
			kfree(vpd_buf);
			goto retry_pg83;
		}
		sdev->vpd_pg83_len = result;
		sdev->vpd_pg83 = vpd_buf;
	}
}

/**
 * scsi_report_opcode - Find out if a given command opcode is supported
 * @sdev:	scsi device to query
 * @buffer:	scratch buffer (must be at least 20 bytes long)
 * @len:	length of buffer
 * @opcode:	opcode for command to look up
 *
 * Uses the REPORT SUPPORTED OPERATION CODES to look up the given
 * opcode. Returns -EINVAL if RSOC fails, 0 if the command opcode is
 * unsupported and 1 if the device claims to support the command.
 */
int scsi_report_opcode(struct scsi_device *sdev, unsigned char *buffer,
		       unsigned int len, unsigned char opcode)
{
	unsigned char cmd[16];
	struct scsi_sense_hdr sshdr;
	int result;

	if (sdev->no_report_opcodes || sdev->scsi_level < SCSI_SPC_3)
		return -EINVAL;

	memset(cmd, 0, 16);
	cmd[0] = MAINTENANCE_IN;
	cmd[1] = MI_REPORT_SUPPORTED_OPERATION_CODES;
	cmd[2] = 1;		/* One command format */
	cmd[3] = opcode;
	put_unaligned_be32(len, &cmd[6]);
	memset(buffer, 0, len);

	result = scsi_execute_req(sdev, cmd, DMA_FROM_DEVICE, buffer, len,
				  &sshdr, 30 * HZ, 3, NULL);

	if (result && scsi_sense_valid(&sshdr) &&
	    sshdr.sense_key == ILLEGAL_REQUEST &&
	    (sshdr.asc == 0x20 || sshdr.asc == 0x24) && sshdr.ascq == 0x00)
		return -EINVAL;

	if ((buffer[1] & 3) == 3) /* Command supported */
		return 1;

	return 0;
}
EXPORT_SYMBOL(scsi_report_opcode);

/**
 * scsi_device_get  -  get an additional reference to a scsi_device
 * @sdev:	device to get a reference to
 *
 * Description: Gets a reference to the scsi_device and increments the use count
 * of the underlying LLDD module.  You must hold host_lock of the
 * parent Scsi_Host or already have a reference when calling this.
 *
 * This will fail if a device is deleted or cancelled, or when the LLD module
 * is in the process of being unloaded.
 */
int scsi_device_get(struct scsi_device *sdev)
{
	if (sdev->sdev_state == SDEV_DEL || sdev->sdev_state == SDEV_CANCEL)
		goto fail;
	if (!get_device(&sdev->sdev_gendev))
		goto fail;
	if (!try_module_get(sdev->host->hostt->module))
		goto fail_put_device;
	return 0;

fail_put_device:
	put_device(&sdev->sdev_gendev);
fail:
	return -ENXIO;
}
EXPORT_SYMBOL(scsi_device_get);

/**
 * scsi_device_put  -  release a reference to a scsi_device
 * @sdev:	device to release a reference on.
 *
 * Description: Release a reference to the scsi_device and decrements the use
 * count of the underlying LLDD module.  The device is freed once the last
 * user vanishes.
 */
void scsi_device_put(struct scsi_device *sdev)
{
	module_put(sdev->host->hostt->module);
	put_device(&sdev->sdev_gendev);
}
EXPORT_SYMBOL(scsi_device_put);

/* helper for shost_for_each_device, see that for documentation */
struct scsi_device *__scsi_iterate_devices(struct Scsi_Host *shost,
					   struct scsi_device *prev)
{
	struct list_head *list = (prev ? &prev->siblings : &shost->__devices);
	struct scsi_device *next = NULL;
	unsigned long flags;

	spin_lock_irqsave(shost->host_lock, flags);
	while (list->next != &shost->__devices) {
		next = list_entry(list->next, struct scsi_device, siblings);
		/* skip devices that we can't get a reference to */
		if (!scsi_device_get(next))
			break;
		next = NULL;
		list = list->next;
	}
	spin_unlock_irqrestore(shost->host_lock, flags);

	if (prev)
		scsi_device_put(prev);
	return next;
}
EXPORT_SYMBOL(__scsi_iterate_devices);

/**
 * starget_for_each_device  -  helper to walk all devices of a target
 * @starget:	target whose devices we want to iterate over.
 * @data:	Opaque passed to each function call.
 * @fn:		Function to call on each device
 *
 * This traverses over each device of @starget.  The devices have
 * a reference that must be released by scsi_host_put when breaking
 * out of the loop.
 */
void starget_for_each_device(struct scsi_target *starget, void *data,
		     void (*fn)(struct scsi_device *, void *))
{
	struct Scsi_Host *shost = dev_to_shost(starget->dev.parent);
	struct scsi_device *sdev;

	shost_for_each_device(sdev, shost) {
		if ((sdev->channel == starget->channel) &&
		    (sdev->id == starget->id))
			fn(sdev, data);
	}
}
EXPORT_SYMBOL(starget_for_each_device);

/**
 * __starget_for_each_device - helper to walk all devices of a target (UNLOCKED)
 * @starget:	target whose devices we want to iterate over.
 * @data:	parameter for callback @fn()
 * @fn:		callback function that is invoked for each device
 *
 * This traverses over each device of @starget.  It does _not_
 * take a reference on the scsi_device, so the whole loop must be
 * protected by shost->host_lock.
 *
 * Note:  The only reason why drivers would want to use this is because
 * they need to access the device list in irq context.  Otherwise you
 * really want to use starget_for_each_device instead.
 **/
void __starget_for_each_device(struct scsi_target *starget, void *data,
			       void (*fn)(struct scsi_device *, void *))
{
	struct Scsi_Host *shost = dev_to_shost(starget->dev.parent);
	struct scsi_device *sdev;

	__shost_for_each_device(sdev, shost) {
		if ((sdev->channel == starget->channel) &&
		    (sdev->id == starget->id))
			fn(sdev, data);
	}
}
EXPORT_SYMBOL(__starget_for_each_device);

/**
 * __scsi_device_lookup_by_target - find a device given the target (UNLOCKED)
 * @starget:	SCSI target pointer
 * @lun:	SCSI Logical Unit Number
 *
 * Description: Looks up the scsi_device with the specified @lun for a given
 * @starget.  The returned scsi_device does not have an additional
 * reference.  You must hold the host's host_lock over this call and
 * any access to the returned scsi_device. A scsi_device in state
 * SDEV_DEL is skipped.
 *
 * Note:  The only reason why drivers should use this is because
 * they need to access the device list in irq context.  Otherwise you
 * really want to use scsi_device_lookup_by_target instead.
 **/
struct scsi_device *__scsi_device_lookup_by_target(struct scsi_target *starget,
						   u64 lun)
{
	struct scsi_device *sdev;

	list_for_each_entry(sdev, &starget->devices, same_target_siblings) {
		if (sdev->sdev_state == SDEV_DEL)
			continue;
		if (sdev->lun ==lun)
			return sdev;
	}

	return NULL;
}
EXPORT_SYMBOL(__scsi_device_lookup_by_target);

/**
 * scsi_device_lookup_by_target - find a device given the target
 * @starget:	SCSI target pointer
 * @lun:	SCSI Logical Unit Number
 *
 * Description: Looks up the scsi_device with the specified @lun for a given
 * @starget.  The returned scsi_device has an additional reference that
 * needs to be released with scsi_device_put once you're done with it.
 **/
struct scsi_device *scsi_device_lookup_by_target(struct scsi_target *starget,
						 u64 lun)
{
	struct scsi_device *sdev;
	struct Scsi_Host *shost = dev_to_shost(starget->dev.parent);
	unsigned long flags;

	spin_lock_irqsave(shost->host_lock, flags);
	sdev = __scsi_device_lookup_by_target(starget, lun);
	if (sdev && scsi_device_get(sdev))
		sdev = NULL;
	spin_unlock_irqrestore(shost->host_lock, flags);

	return sdev;
}
EXPORT_SYMBOL(scsi_device_lookup_by_target);

/**
 * __scsi_device_lookup - find a device given the host (UNLOCKED)
 * @shost:	SCSI host pointer
 * @channel:	SCSI channel (zero if only one channel)
 * @id:		SCSI target number (physical unit number)
 * @lun:	SCSI Logical Unit Number
 *
 * Description: Looks up the scsi_device with the specified @channel, @id, @lun
 * for a given host. The returned scsi_device does not have an additional
 * reference.  You must hold the host's host_lock over this call and any access
 * to the returned scsi_device.
 *
 * Note:  The only reason why drivers would want to use this is because
 * they need to access the device list in irq context.  Otherwise you
 * really want to use scsi_device_lookup instead.
 **/
struct scsi_device *__scsi_device_lookup(struct Scsi_Host *shost,
		uint channel, uint id, u64 lun)
{
	struct scsi_device *sdev;

	list_for_each_entry(sdev, &shost->__devices, siblings) {
		if (sdev->channel == channel && sdev->id == id &&
				sdev->lun ==lun)
			return sdev;
	}

	return NULL;
}
EXPORT_SYMBOL(__scsi_device_lookup);

/**
 * scsi_device_lookup - find a device given the host
 * @shost:	SCSI host pointer
 * @channel:	SCSI channel (zero if only one channel)
 * @id:		SCSI target number (physical unit number)
 * @lun:	SCSI Logical Unit Number
 *
 * Description: Looks up the scsi_device with the specified @channel, @id, @lun
 * for a given host.  The returned scsi_device has an additional reference that
 * needs to be released with scsi_device_put once you're done with it.
 **/
struct scsi_device *scsi_device_lookup(struct Scsi_Host *shost,
		uint channel, uint id, u64 lun)
{
	struct scsi_device *sdev;
	unsigned long flags;

	spin_lock_irqsave(shost->host_lock, flags);
	sdev = __scsi_device_lookup(shost, channel, id, lun);
	if (sdev && scsi_device_get(sdev))
		sdev = NULL;
	spin_unlock_irqrestore(shost->host_lock, flags);

	return sdev;
}
EXPORT_SYMBOL(scsi_device_lookup);

MODULE_DESCRIPTION("SCSI core");
MODULE_LICENSE("GPL");

module_param(scsi_logging_level, int, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(scsi_logging_level, "a bit mask of logging levels");

#ifdef CONFIG_SCSI_MQ_DEFAULT
bool scsi_use_blk_mq = true;
#else
bool scsi_use_blk_mq = false;
#endif
module_param_named(use_blk_mq, scsi_use_blk_mq, bool, S_IWUSR | S_IRUGO);

static int __init init_scsi(void)
{
	int error;

	error = scsi_init_queue();
	if (error)
		return error;
	error = scsi_init_procfs();
	if (error)
		goto cleanup_queue;
	error = scsi_init_devinfo();
	if (error)
		goto cleanup_procfs;
	error = scsi_init_hosts();
	if (error)
		goto cleanup_devlist;
	error = scsi_init_sysctl();
	if (error)
		goto cleanup_hosts;
	error = scsi_sysfs_register();
	if (error)
		goto cleanup_sysctl;

	scsi_netlink_init();

	printk(KERN_NOTICE "SCSI subsystem initialized\n");
	return 0;

cleanup_sysctl:
	scsi_exit_sysctl();
cleanup_hosts:
	scsi_exit_hosts();
cleanup_devlist:
	scsi_exit_devinfo();
cleanup_procfs:
	scsi_exit_procfs();
cleanup_queue:
	scsi_exit_queue();
	printk(KERN_ERR "SCSI subsystem failed to initialize, error = %d\n",
	       -error);
	return error;
}

static void __exit exit_scsi(void)
{
	scsi_netlink_exit();
	scsi_sysfs_unregister();
	scsi_exit_sysctl();
	scsi_exit_hosts();
	scsi_exit_devinfo();
	scsi_exit_procfs();
	scsi_exit_queue();
	async_unregister_domain(&scsi_sd_probe_domain);
}

subsys_initcall(init_scsi);
module_exit(exit_scsi);
