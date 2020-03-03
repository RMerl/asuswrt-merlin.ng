/* virthba.c
 *
 * Copyright (C) 2010 - 2013 UNISYS CORPORATION
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, GOOD TITLE or
 * NON INFRINGEMENT.  See the GNU General Public License for more
 * details.
 */

#define EXPORT_SYMTAB

/* if you want to turn on some debugging of write device data or read
 * device data, define these two undefs.  You will probably want to
 * customize the code which is here since it was written assuming
 * reading and writing a specific data file df.64M.txt which is a
 * 64Megabyte file created by Art Nilson using a scritp I wrote called
 * cr_test_data.pl.  The data file consists of 256 byte lines of text
 * which start with an 8 digit sequence number, a colon, and then
 * letters after that */

#include <linux/kernel.h>
#ifdef CONFIG_MODVERSIONS
#include <config/modversions.h>
#endif

#include "diagnostics/appos_subsystems.h"
#include "uisutils.h"
#include "uisqueue.h"
#include "uisthread.h"

#include <linux/module.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/spinlock.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <scsi/scsi.h>
#include <scsi/scsi_host.h>
#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_device.h>
#include <asm/param.h>
#include <linux/debugfs.h>
#include <linux/types.h>

#include "virthba.h"
#include "virtpci.h"
#include "visorchipset.h"
#include "version.h"
#include "guestlinuxdebug.h"
/* this is shorter than using __FILE__ (full path name) in
 * debug/info/error messages
 */
#define CURRENT_FILE_PC VIRT_HBA_PC_virthba_c
#define __MYFILE__ "virthba.c"

/* NOTE:  L1_CACHE_BYTES >=128 */
#define DEVICE_ATTRIBUTE struct device_attribute

 /* MAX_BUF = 6 lines x 10 MAXVHBA x 80 characters
 *         = 4800 bytes ~ 2^13 = 8192 bytes
 */
#define MAX_BUF 8192

/*****************************************************/
/* Forward declarations                              */
/*****************************************************/
static int virthba_probe(struct virtpci_dev *dev,
			 const struct pci_device_id *id);
static void virthba_remove(struct virtpci_dev *dev);
static int virthba_abort_handler(struct scsi_cmnd *scsicmd);
static int virthba_bus_reset_handler(struct scsi_cmnd *scsicmd);
static int virthba_device_reset_handler(struct scsi_cmnd *scsicmd);
static int virthba_host_reset_handler(struct scsi_cmnd *scsicmd);
static const char *virthba_get_info(struct Scsi_Host *shp);
static int virthba_ioctl(struct scsi_device *dev, int cmd, void __user *arg);
static int virthba_queue_command_lck(struct scsi_cmnd *scsicmd,
				     void (*virthba_cmnd_done)
					   (struct scsi_cmnd *));

static const struct x86_cpu_id unisys_spar_ids[] = {
	{ X86_VENDOR_INTEL, 6, 62, X86_FEATURE_ANY },
	{}
};

/* Autoload */
MODULE_DEVICE_TABLE(x86cpu, unisys_spar_ids);

#ifdef DEF_SCSI_QCMD
static DEF_SCSI_QCMD(virthba_queue_command)
#else
#define virthba_queue_command virthba_queue_command_lck
#endif

static int virthba_slave_alloc(struct scsi_device *scsidev);
static int virthba_slave_configure(struct scsi_device *scsidev);
static void virthba_slave_destroy(struct scsi_device *scsidev);
static int process_incoming_rsps(void *);
static int virthba_serverup(struct virtpci_dev *virtpcidev);
static int virthba_serverdown(struct virtpci_dev *virtpcidev, u32 state);
static void do_disk_add_remove(struct work_struct *work);
static void virthba_serverdown_complete(struct work_struct *work);
static ssize_t info_debugfs_read(struct file *file, char __user *buf,
				 size_t len, loff_t *offset);
static ssize_t enable_ints_write(struct file *file,
				 const char __user *buffer, size_t count,
				 loff_t *ppos);

/*****************************************************/
/* Globals                                           */
/*****************************************************/

static int rsltq_wait_usecs = 4000;	/* Default 4ms */
static unsigned int max_buff_len;

/* Module options */
static char *virthba_options = "NONE";

static const struct pci_device_id virthba_id_table[] = {
	{PCI_DEVICE(PCI_VENDOR_ID_UNISYS, PCI_DEVICE_ID_VIRTHBA)},
	{0},
};

/* export virthba_id_table */
MODULE_DEVICE_TABLE(pci, virthba_id_table);

static struct workqueue_struct *virthba_serverdown_workqueue;

static struct virtpci_driver virthba_driver = {
	.name = "uisvirthba",
	.version = VERSION,
	.vertag = NULL,
	.id_table = virthba_id_table,
	.probe = virthba_probe,
	.remove = virthba_remove,
	.resume = virthba_serverup,
	.suspend = virthba_serverdown
};

/* The Send and Recive Buffers of the IO Queue may both be full */
#define MAX_PENDING_REQUESTS (MIN_NUMSIGNALS*2)
#define INTERRUPT_VECTOR_MASK 0x3F

struct scsipending {
	char cmdtype;		/* Type of pointer that is being stored */
	void *sent;		/* The Data being tracked */
	/* struct scsi_cmnd *type for virthba_queue_command */
	/* struct uiscmdrsp *type for management commands */
};

#define VIRTHBA_ERROR_COUNT 30
#define IOS_ERROR_THRESHOLD 1000
struct virtdisk_info {
	u32 valid;
	u32 channel, id, lun;	/* Disk Path */
	atomic_t ios_threshold;
	atomic_t error_count;
	struct virtdisk_info *next;
};

/* Each Scsi_Host has a host_data area that contains this struct. */
struct virthba_info {
	struct Scsi_Host *scsihost;
	struct virtpci_dev *virtpcidev;
	struct list_head dev_info_list;
	struct chaninfo chinfo;
	struct irq_info intr;		/* use recvInterrupt info to receive
					   interrupts when IOs complete */
	int interrupt_vector;
	struct scsipending pending[MAX_PENDING_REQUESTS]; /* Tracks the requests
							     that have been */
	/* forwarded to the IOVM and haven't returned yet */
	unsigned int nextinsert;	/* Start search for next pending
					   free slot here */
	spinlock_t privlock;
	bool serverdown;
	bool serverchangingstate;
	unsigned long long acquire_failed_cnt;
	unsigned long long interrupts_rcvd;
	unsigned long long interrupts_notme;
	unsigned long long interrupts_disabled;
	struct work_struct serverdown_completion;
	u64 __iomem *flags_addr;
	atomic_t interrupt_rcvd;
	wait_queue_head_t rsp_queue;
	struct virtdisk_info head;
};

/* Work Data for dar_work_queue */
struct diskaddremove {
	u8 add;			/* 0-remove, 1-add */
	struct Scsi_Host *shost; /* Scsi Host for this virthba instance */
	u32 channel, id, lun;	/* Disk Path */
	struct diskaddremove *next;
};

#define virtpci_dev_to_virthba_virthba_get_info(d) \
	container_of(d, struct virthba_info, virtpcidev)

static DEVICE_ATTRIBUTE *virthba_shost_attrs[];
static struct scsi_host_template virthba_driver_template = {
	.name = "Unisys Virtual HBA",
	.info = virthba_get_info,
	.ioctl = virthba_ioctl,
	.queuecommand = virthba_queue_command,
	.eh_abort_handler = virthba_abort_handler,
	.eh_device_reset_handler = virthba_device_reset_handler,
	.eh_bus_reset_handler = virthba_bus_reset_handler,
	.eh_host_reset_handler = virthba_host_reset_handler,
	.shost_attrs = virthba_shost_attrs,

#define VIRTHBA_MAX_CMNDS 128
	.can_queue = VIRTHBA_MAX_CMNDS,
	.sg_tablesize = 64,	/* largest number of address/length pairs */
	.this_id = -1,
	.slave_alloc = virthba_slave_alloc,
	.slave_configure = virthba_slave_configure,
	.slave_destroy = virthba_slave_destroy,
	.use_clustering = ENABLE_CLUSTERING,
};

struct virthba_devices_open {
	struct virthba_info *virthbainfo;
};

static const struct file_operations debugfs_info_fops = {
	.read = info_debugfs_read,
};

static const struct file_operations debugfs_enable_ints_fops = {
	.write = enable_ints_write,
};

/*****************************************************/
/* Structs                                           */
/*****************************************************/

#define VIRTHBASOPENMAX 1
/* array of open devices maintained by open() and close(); */
static struct virthba_devices_open virthbas_open[VIRTHBASOPENMAX];
static struct dentry *virthba_debugfs_dir;

/*****************************************************/
/* Local Functions				     */
/*****************************************************/
static int
add_scsipending_entry(struct virthba_info *vhbainfo, char cmdtype, void *new)
{
	unsigned long flags;
	int insert_location;

	spin_lock_irqsave(&vhbainfo->privlock, flags);
	insert_location = vhbainfo->nextinsert;
	while (vhbainfo->pending[insert_location].sent) {
		insert_location = (insert_location + 1) % MAX_PENDING_REQUESTS;
		if (insert_location == (int)vhbainfo->nextinsert) {
			spin_unlock_irqrestore(&vhbainfo->privlock, flags);
			return -1;
		}
	}

	vhbainfo->pending[insert_location].cmdtype = cmdtype;
	vhbainfo->pending[insert_location].sent = new;
	vhbainfo->nextinsert = (insert_location + 1) % MAX_PENDING_REQUESTS;
	spin_unlock_irqrestore(&vhbainfo->privlock, flags);

	return insert_location;
}

static unsigned int
add_scsipending_entry_with_wait(struct virthba_info *vhbainfo, char cmdtype,
				void *new)
{
	int insert_location = add_scsipending_entry(vhbainfo, cmdtype, new);

	while (insert_location == -1) {
		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(msecs_to_jiffies(10));
		insert_location = add_scsipending_entry(vhbainfo, cmdtype, new);
	}

	return (unsigned int)insert_location;
}

static void *
del_scsipending_entry(struct virthba_info *vhbainfo, uintptr_t del)
{
	unsigned long flags;
	void *sent = NULL;

	if (del < MAX_PENDING_REQUESTS) {
		spin_lock_irqsave(&vhbainfo->privlock, flags);
		sent = vhbainfo->pending[del].sent;

		vhbainfo->pending[del].cmdtype = 0;
		vhbainfo->pending[del].sent = NULL;
		spin_unlock_irqrestore(&vhbainfo->privlock, flags);
	}

	return sent;
}

/* dar_work_queue (Disk Add/Remove) */
static struct work_struct dar_work_queue;
static struct diskaddremove *dar_work_queue_head;
static spinlock_t dar_work_queue_lock;
static unsigned short dar_work_queue_sched;
#define QUEUE_DISKADDREMOVE(dar) { \
	spin_lock_irqsave(&dar_work_queue_lock, flags); \
	if (!dar_work_queue_head) { \
		dar_work_queue_head = dar; \
		dar->next = NULL; \
	} \
	else { \
		dar->next = dar_work_queue_head; \
		dar_work_queue_head = dar; \
	} \
	if (!dar_work_queue_sched) { \
		schedule_work(&dar_work_queue); \
		dar_work_queue_sched = 1; \
	} \
	spin_unlock_irqrestore(&dar_work_queue_lock, flags); \
}

static inline void
send_disk_add_remove(struct diskaddremove *dar)
{
	struct scsi_device *sdev;
	int error;

	sdev = scsi_device_lookup(dar->shost, dar->channel, dar->id, dar->lun);
	if (sdev) {
		if (!(dar->add))
			scsi_remove_device(sdev);
	} else if (dar->add) {
		error =
		    scsi_add_device(dar->shost, dar->channel, dar->id,
				    dar->lun);
	}
	kfree(dar);
}

/*****************************************************/
/* dar_work_queue Handler Thread                     */
/*****************************************************/
static void
do_disk_add_remove(struct work_struct *work)
{
	struct diskaddremove *dar;
	struct diskaddremove *tmphead;
	int i = 0;
	unsigned long flags;

	spin_lock_irqsave(&dar_work_queue_lock, flags);
	tmphead = dar_work_queue_head;
	dar_work_queue_head = NULL;
	dar_work_queue_sched = 0;
	spin_unlock_irqrestore(&dar_work_queue_lock, flags);
	while (tmphead) {
		dar = tmphead;
		tmphead = dar->next;
		send_disk_add_remove(dar);
		i++;
	}
}

/*****************************************************/
/* Routine to add entry to dar_work_queue            */
/*****************************************************/
static void
process_disk_notify(struct Scsi_Host *shost, struct uiscmdrsp *cmdrsp)
{
	struct diskaddremove *dar;
	unsigned long flags;

	dar = kzalloc(sizeof(*dar), GFP_ATOMIC);
	if (dar) {
		dar->add = cmdrsp->disknotify.add;
		dar->shost = shost;
		dar->channel = cmdrsp->disknotify.channel;
		dar->id = cmdrsp->disknotify.id;
		dar->lun = cmdrsp->disknotify.lun;
		QUEUE_DISKADDREMOVE(dar);
	}
}

/*****************************************************/
/* Probe Remove Functions                            */
/*****************************************************/
static irqreturn_t
virthba_isr(int irq, void *dev_id)
{
	struct virthba_info *virthbainfo = (struct virthba_info *)dev_id;
	struct channel_header __iomem *channel_header;
	struct signal_queue_header __iomem *pqhdr;
	u64 mask;
	unsigned long long rc1;

	if (!virthbainfo)
		return IRQ_NONE;
	virthbainfo->interrupts_rcvd++;
	channel_header = virthbainfo->chinfo.queueinfo->chan;
	if (((readq(&channel_header->features)
	      & ULTRA_IO_IOVM_IS_OK_WITH_DRIVER_DISABLING_INTS) != 0) &&
	     ((readq(&channel_header->features) &
		 ULTRA_IO_DRIVER_DISABLES_INTS) !=
		0)) {
		virthbainfo->interrupts_disabled++;
		mask = ~ULTRA_CHANNEL_ENABLE_INTS;
		rc1 = uisqueue_interlocked_and(virthbainfo->flags_addr, mask);
	}
	if (spar_signalqueue_empty(channel_header, IOCHAN_FROM_IOPART)) {
		virthbainfo->interrupts_notme++;
		return IRQ_NONE;
	}
	pqhdr = (struct signal_queue_header __iomem *)
		((char __iomem *)channel_header +
		 readq(&channel_header->ch_space_offset)) + IOCHAN_FROM_IOPART;
	writeq(readq(&pqhdr->num_irq_received) + 1,
	       &pqhdr->num_irq_received);
	atomic_set(&virthbainfo->interrupt_rcvd, 1);
	wake_up_interruptible(&virthbainfo->rsp_queue);
	return IRQ_HANDLED;
}

static int
virthba_probe(struct virtpci_dev *virtpcidev, const struct pci_device_id *id)
{
	int error;
	struct Scsi_Host *scsihost;
	struct virthba_info *virthbainfo;
	int rsp;
	int i;
	irq_handler_t handler = virthba_isr;
	struct channel_header __iomem *channel_header;
	struct signal_queue_header __iomem *pqhdr;
	u64 mask;

	POSTCODE_LINUX_2(VHBA_PROBE_ENTRY_PC, POSTCODE_SEVERITY_INFO);
	/* call scsi_host_alloc to register a scsi host adapter
	 * instance - this virthba that has just been created is an
	 * instance of a scsi host adapter. This scsi_host_alloc
	 * function allocates a new Scsi_Host struct & performs basic
	 * initialization.  The host is not published to the scsi
	 * midlayer until scsi_add_host is called.
	 */

	/* arg 2 passed in length of extra space we want allocated
	 * with scsi_host struct for our own use scsi_host_alloc
	 * assign host_no
	 */
	scsihost = scsi_host_alloc(&virthba_driver_template,
				   sizeof(struct virthba_info));
	if (!scsihost)
		return -ENODEV;

	scsihost->this_id = UIS_MAGIC_VHBA;
	/* linux treats max-channel differently than max-id & max-lun.
	 * In the latter cases, those two values result in 0 to max-1
	 * (inclusive) being scanned. But in the case of channels, the
	 * scan is 0 to max (inclusive); so we will subtract one from
	 * the max-channel value.
	 */
	scsihost->max_channel = (unsigned)virtpcidev->scsi.max.max_channel;
	scsihost->max_id = (unsigned)virtpcidev->scsi.max.max_id;
	scsihost->max_lun = (unsigned)virtpcidev->scsi.max.max_lun;
	scsihost->cmd_per_lun = (unsigned)virtpcidev->scsi.max.cmd_per_lun;
	scsihost->max_sectors =
	    (unsigned short)(virtpcidev->scsi.max.max_io_size >> 9);
	scsihost->sg_tablesize =
	    (unsigned short)(virtpcidev->scsi.max.max_io_size / PAGE_SIZE);
	if (scsihost->sg_tablesize > MAX_PHYS_INFO)
		scsihost->sg_tablesize = MAX_PHYS_INFO;

	/* this creates "host%d" in sysfs.  If 2nd argument is NULL,
	 * then this generic /sys/devices/platform/host?  device is
	 * created and /sys/scsi_host/host? ->
	 * /sys/devices/platform/host?  If 2nd argument is not NULL,
	 * then this generic /sys/devices/<path>/host? is created and
	 * host? points to that device instead.
	 */
	error = scsi_add_host(scsihost, &virtpcidev->generic_dev);
	if (error) {
		POSTCODE_LINUX_2(VHBA_PROBE_FAILURE_PC, POSTCODE_SEVERITY_ERR);
		/* decr refcount on scsihost which was incremented by
		 * scsi_add_host so the scsi_host gets deleted
		 */
		scsi_host_put(scsihost);
		return -ENODEV;
	}

	virthbainfo = (struct virthba_info *)scsihost->hostdata;
	memset(virthbainfo, 0, sizeof(struct virthba_info));
	for (i = 0; i < VIRTHBASOPENMAX; i++) {
		if (!virthbas_open[i].virthbainfo) {
			virthbas_open[i].virthbainfo = virthbainfo;
			break;
		}
	}
	virthbainfo->interrupt_vector = -1;
	virthbainfo->chinfo.queueinfo = &virtpcidev->queueinfo;
	virthbainfo->virtpcidev = virtpcidev;
	spin_lock_init(&virthbainfo->chinfo.insertlock);

	init_waitqueue_head(&virthbainfo->rsp_queue);
	spin_lock_init(&virthbainfo->privlock);
	memset(&virthbainfo->pending, 0, sizeof(virthbainfo->pending));
	virthbainfo->serverdown = false;
	virthbainfo->serverchangingstate = false;

	virthbainfo->intr = virtpcidev->intr;
	/* save of host within virthba_info */
	virthbainfo->scsihost = scsihost;

	/* save of host within virtpci_dev */
	virtpcidev->scsi.scsihost = scsihost;

	/* Setup workqueue for serverdown messages */
	INIT_WORK(&virthbainfo->serverdown_completion,
		  virthba_serverdown_complete);

	writeq(readq(&virthbainfo->chinfo.queueinfo->chan->features) |
	       ULTRA_IO_CHANNEL_IS_POLLING,
	       &virthbainfo->chinfo.queueinfo->chan->features);
	/* start thread that will receive scsicmnd responses */

	channel_header = virthbainfo->chinfo.queueinfo->chan;
	pqhdr = (struct signal_queue_header __iomem *)
		((char __iomem *)channel_header +
		 readq(&channel_header->ch_space_offset)) + IOCHAN_FROM_IOPART;
	virthbainfo->flags_addr = &pqhdr->features;

	if (!uisthread_start(&virthbainfo->chinfo.threadinfo,
			     process_incoming_rsps,
			     virthbainfo, "vhba_incoming")) {
		/* decr refcount on scsihost which was incremented by
		 * scsi_add_host so the scsi_host gets deleted
		 */
		POSTCODE_LINUX_2(VHBA_PROBE_FAILURE_PC, POSTCODE_SEVERITY_ERR);
		scsi_host_put(scsihost);
		return -ENODEV;
	}
	virthbainfo->interrupt_vector =
	    virthbainfo->intr.recv_irq_handle & INTERRUPT_VECTOR_MASK;
	rsp = request_irq(virthbainfo->interrupt_vector, handler, IRQF_SHARED,
			  scsihost->hostt->name, virthbainfo);
	if (rsp != 0) {
		virthbainfo->interrupt_vector = -1;
		POSTCODE_LINUX_2(VHBA_PROBE_FAILURE_PC, POSTCODE_SEVERITY_ERR);
	} else {
		u64 __iomem *features_addr =
		    &virthbainfo->chinfo.queueinfo->chan->features;
		mask = ~(ULTRA_IO_CHANNEL_IS_POLLING |
			 ULTRA_IO_DRIVER_DISABLES_INTS);
		uisqueue_interlocked_and(features_addr, mask);
		mask = ULTRA_IO_DRIVER_ENABLES_INTS;
		uisqueue_interlocked_or(features_addr, mask);
		rsltq_wait_usecs = 4000000;
	}

	scsi_scan_host(scsihost);

	POSTCODE_LINUX_2(VHBA_PROBE_EXIT_PC, POSTCODE_SEVERITY_INFO);
	return 0;
}

static void
virthba_remove(struct virtpci_dev *virtpcidev)
{
	struct virthba_info *virthbainfo;
	struct Scsi_Host *scsihost =
	    (struct Scsi_Host *)virtpcidev->scsi.scsihost;

	virthbainfo = (struct virthba_info *)scsihost->hostdata;
	if (virthbainfo->interrupt_vector != -1)
		free_irq(virthbainfo->interrupt_vector, virthbainfo);

	scsi_remove_host(scsihost);

	uisthread_stop(&virthbainfo->chinfo.threadinfo);

	/* decr refcount on scsihost which was incremented by
	 * scsi_add_host so the scsi_host gets deleted
	 */
	scsi_host_put(scsihost);
}

static int
forward_vdiskmgmt_command(enum vdisk_mgmt_types vdiskcmdtype,
			  struct Scsi_Host *scsihost,
			  struct uisscsi_dest *vdest)
{
	struct uiscmdrsp *cmdrsp;
	struct virthba_info *virthbainfo =
	    (struct virthba_info *)scsihost->hostdata;
	int notifyresult = 0xffff;
	wait_queue_head_t notifyevent;

	if (virthbainfo->serverdown || virthbainfo->serverchangingstate)
		return FAILED;

	cmdrsp = kzalloc(SIZEOF_CMDRSP, GFP_ATOMIC);
	if (!cmdrsp)
		return FAILED;  /* reject */

	init_waitqueue_head(&notifyevent);

	/* issue VDISK_MGMT_CMD
	 * set type to command - as opposed to task mgmt
	 */
	cmdrsp->cmdtype = CMD_VDISKMGMT_TYPE;
	/* specify the event that has to be triggered when this cmd is
	 * complete
	 */
	cmdrsp->vdiskmgmt.notify = (void *)&notifyevent;
	cmdrsp->vdiskmgmt.notifyresult = (void *)&notifyresult;

	/* save destination */
	cmdrsp->vdiskmgmt.vdisktype = vdiskcmdtype;
	cmdrsp->vdiskmgmt.vdest.channel = vdest->channel;
	cmdrsp->vdiskmgmt.vdest.id = vdest->id;
	cmdrsp->vdiskmgmt.vdest.lun = vdest->lun;
	cmdrsp->vdiskmgmt.scsicmd =
	    (void *)(uintptr_t)
		add_scsipending_entry_with_wait(virthbainfo, CMD_VDISKMGMT_TYPE,
						(void *)cmdrsp);

	uisqueue_put_cmdrsp_with_lock_client(virthbainfo->chinfo.queueinfo,
					     cmdrsp, IOCHAN_TO_IOPART,
					     &virthbainfo->chinfo.insertlock,
					     DONT_ISSUE_INTERRUPT, (u64)NULL,
					     OK_TO_WAIT, "vhba");
	wait_event(notifyevent, notifyresult != 0xffff);
	kfree(cmdrsp);
	return SUCCESS;
}

/*****************************************************/
/* Scsi Host support functions                       */
/*****************************************************/

static int
forward_taskmgmt_command(enum task_mgmt_types tasktype,
			 struct scsi_device *scsidev)
{
	struct uiscmdrsp *cmdrsp;
	struct virthba_info *virthbainfo =
	    (struct virthba_info *)scsidev->host->hostdata;
	int notifyresult = 0xffff;
	wait_queue_head_t notifyevent;

	if (virthbainfo->serverdown || virthbainfo->serverchangingstate)
		return FAILED;

	cmdrsp = kzalloc(SIZEOF_CMDRSP, GFP_ATOMIC);
	if (!cmdrsp)
		return FAILED;	/* reject */

	init_waitqueue_head(&notifyevent);

	/* issue TASK_MGMT_ABORT_TASK */
	/* set type to command - as opposed to task mgmt */
	cmdrsp->cmdtype = CMD_SCSITASKMGMT_TYPE;
	/* specify the event that has to be triggered when this */
	/* cmd is complete */
	cmdrsp->scsitaskmgmt.notify = (void *)&notifyevent;
	cmdrsp->scsitaskmgmt.notifyresult = (void *)&notifyresult;

	/* save destination */
	cmdrsp->scsitaskmgmt.tasktype = tasktype;
	cmdrsp->scsitaskmgmt.vdest.channel = scsidev->channel;
	cmdrsp->scsitaskmgmt.vdest.id = scsidev->id;
	cmdrsp->scsitaskmgmt.vdest.lun = scsidev->lun;
	cmdrsp->scsitaskmgmt.scsicmd =
	    (void *)(uintptr_t)
		add_scsipending_entry_with_wait(virthbainfo,
						CMD_SCSITASKMGMT_TYPE,
						(void *)cmdrsp);

	uisqueue_put_cmdrsp_with_lock_client(virthbainfo->chinfo.queueinfo,
					     cmdrsp, IOCHAN_TO_IOPART,
					     &virthbainfo->chinfo.insertlock,
					     DONT_ISSUE_INTERRUPT, (u64)NULL,
					     OK_TO_WAIT, "vhba");
	wait_event(notifyevent, notifyresult != 0xffff);
	kfree(cmdrsp);
	return SUCCESS;
}

/* The abort handler returns SUCCESS if it has succeeded to make LLDD
 * and all related hardware forget about the scmd.
 */
static int
virthba_abort_handler(struct scsi_cmnd *scsicmd)
{
	/* issue TASK_MGMT_ABORT_TASK */
	struct scsi_device *scsidev;
	struct virtdisk_info *vdisk;

	scsidev = scsicmd->device;
	for (vdisk = &((struct virthba_info *)scsidev->host->hostdata)->head;
	     vdisk->next; vdisk = vdisk->next) {
		if ((scsidev->channel == vdisk->channel) &&
		    (scsidev->id == vdisk->id) &&
		    (scsidev->lun == vdisk->lun)) {
			if (atomic_read(&vdisk->error_count) <
			    VIRTHBA_ERROR_COUNT) {
				atomic_inc(&vdisk->error_count);
				POSTCODE_LINUX_2(VHBA_COMMAND_HANDLER_PC,
						 POSTCODE_SEVERITY_INFO);
			} else
				atomic_set(&vdisk->ios_threshold,
					   IOS_ERROR_THRESHOLD);
		}
	}
	return forward_taskmgmt_command(TASK_MGMT_ABORT_TASK, scsicmd->device);
}

static int
virthba_bus_reset_handler(struct scsi_cmnd *scsicmd)
{
	/* issue TASK_MGMT_TARGET_RESET for each target on the bus */
	struct scsi_device *scsidev;
	struct virtdisk_info *vdisk;

	scsidev = scsicmd->device;
	for (vdisk = &((struct virthba_info *)scsidev->host->hostdata)->head;
	     vdisk->next; vdisk = vdisk->next) {
		if ((scsidev->channel == vdisk->channel) &&
		    (scsidev->id == vdisk->id) &&
		    (scsidev->lun == vdisk->lun)) {
			if (atomic_read(&vdisk->error_count) <
			    VIRTHBA_ERROR_COUNT) {
				atomic_inc(&vdisk->error_count);
				POSTCODE_LINUX_2(VHBA_COMMAND_HANDLER_PC,
						 POSTCODE_SEVERITY_INFO);
			} else
				atomic_set(&vdisk->ios_threshold,
					   IOS_ERROR_THRESHOLD);
		}
	}
	return forward_taskmgmt_command(TASK_MGMT_BUS_RESET, scsicmd->device);
}

static int
virthba_device_reset_handler(struct scsi_cmnd *scsicmd)
{
	/* issue TASK_MGMT_LUN_RESET */
	struct scsi_device *scsidev;
	struct virtdisk_info *vdisk;

	scsidev = scsicmd->device;
	for (vdisk = &((struct virthba_info *)scsidev->host->hostdata)->head;
	     vdisk->next; vdisk = vdisk->next) {
		if ((scsidev->channel == vdisk->channel) &&
		    (scsidev->id == vdisk->id) &&
		    (scsidev->lun == vdisk->lun)) {
			if (atomic_read(&vdisk->error_count) <
			    VIRTHBA_ERROR_COUNT) {
				atomic_inc(&vdisk->error_count);
				POSTCODE_LINUX_2(VHBA_COMMAND_HANDLER_PC,
						 POSTCODE_SEVERITY_INFO);
			} else
				atomic_set(&vdisk->ios_threshold,
					   IOS_ERROR_THRESHOLD);
		}
	}
	return forward_taskmgmt_command(TASK_MGMT_LUN_RESET, scsicmd->device);
}

static int
virthba_host_reset_handler(struct scsi_cmnd *scsicmd)
{
	/* issue TASK_MGMT_TARGET_RESET for each target on each bus for host */
	return SUCCESS;
}

static char virthba_get_info_str[256];

static const char *
virthba_get_info(struct Scsi_Host *shp)
{
	/* Return version string */
	sprintf(virthba_get_info_str, "virthba, version %s\n", VIRTHBA_VERSION);
	return virthba_get_info_str;
}

static int
virthba_ioctl(struct scsi_device *dev, int cmd, void __user *arg)
{
	return -EINVAL;
}

/* This returns SCSI_MLQUEUE_DEVICE_BUSY if the signal queue to IOpart
 * is full.
 */
static int
virthba_queue_command_lck(struct scsi_cmnd *scsicmd,
			  void (*virthba_cmnd_done)(struct scsi_cmnd *))
{
	struct scsi_device *scsidev = scsicmd->device;
	int insert_location;
	unsigned char op;
	unsigned char *cdb = scsicmd->cmnd;
	struct Scsi_Host *scsihost = scsidev->host;
	struct uiscmdrsp *cmdrsp;
	unsigned int i;
	struct virthba_info *virthbainfo =
	    (struct virthba_info *)scsihost->hostdata;
	struct scatterlist *sg = NULL;
	struct scatterlist *sgl = NULL;
	int sg_failed = 0;

	if (virthbainfo->serverdown || virthbainfo->serverchangingstate)
		return SCSI_MLQUEUE_DEVICE_BUSY;
	cmdrsp = kzalloc(SIZEOF_CMDRSP, GFP_ATOMIC);
	if (!cmdrsp)
		return 1;	/* reject the command */

	/* now saving everything we need from scsi_cmd into cmdrsp
	 * before we queue cmdrsp set type to command - as opposed to
	 * task mgmt
	 */
	cmdrsp->cmdtype = CMD_SCSI_TYPE;
	/* save the pending insertion location.  Deletion from pending
	 * will return the scsicmd pointer for completion
	 */
	insert_location =
	    add_scsipending_entry(virthbainfo, CMD_SCSI_TYPE, (void *)scsicmd);
	if (insert_location != -1) {
		cmdrsp->scsi.scsicmd = (void *)(uintptr_t)insert_location;
	} else {
		kfree(cmdrsp);
		return SCSI_MLQUEUE_DEVICE_BUSY;
	}
	/* save done function that we have call when cmd is complete */
	scsicmd->scsi_done = virthba_cmnd_done;
	/* save destination */
	cmdrsp->scsi.vdest.channel = scsidev->channel;
	cmdrsp->scsi.vdest.id = scsidev->id;
	cmdrsp->scsi.vdest.lun = scsidev->lun;
	/* save datadir */
	cmdrsp->scsi.data_dir = scsicmd->sc_data_direction;
	memcpy(cmdrsp->scsi.cmnd, cdb, MAX_CMND_SIZE);

	cmdrsp->scsi.bufflen = scsi_bufflen(scsicmd);

	/* keep track of the max buffer length so far. */
	if (cmdrsp->scsi.bufflen > max_buff_len)
		max_buff_len = cmdrsp->scsi.bufflen;

	if (scsi_sg_count(scsicmd) > MAX_PHYS_INFO) {
		del_scsipending_entry(virthbainfo, (uintptr_t)insert_location);
		kfree(cmdrsp);
		return 1;	/* reject the command */
	}

	/* This is what we USED to do when we assumed we were running */
	/* uissd & virthba on the same Linux system. */
	/* cmdrsp->scsi.buffer = scsicmd->request_buffer; */
	/* The following code does NOT make that assumption. */
	/* convert buffer to phys information */
	if (scsi_sg_count(scsicmd) == 0) {
		if (scsi_bufflen(scsicmd) > 0) {
			BUG_ON(scsi_sg_count(scsicmd) == 0);
		}
	} else {
		/* buffer is scatterlist - copy it out */
		sgl = scsi_sglist(scsicmd);

		for_each_sg(sgl, sg, scsi_sg_count(scsicmd), i) {
			cmdrsp->scsi.gpi_list[i].address = sg_phys(sg);
			cmdrsp->scsi.gpi_list[i].length = sg->length;
		}

		if (sg_failed) {
			/* BUG(); ***** For now, let it fail in uissd
			 * if it is a problem, as it might just
			 * work
			 */
		}

		cmdrsp->scsi.guest_phys_entries = scsi_sg_count(scsicmd);
	}

	op = cdb[0];
	i = uisqueue_put_cmdrsp_with_lock_client(virthbainfo->chinfo.queueinfo,
						 cmdrsp, IOCHAN_TO_IOPART,
						 &virthbainfo->chinfo.
						 insertlock,
						 DONT_ISSUE_INTERRUPT,
						 (u64)NULL, DONT_WAIT, "vhba");
	if (i == 0) {
		/* queue must be full - and we said don't wait - return busy */
		kfree(cmdrsp);
		del_scsipending_entry(virthbainfo, (uintptr_t)insert_location);
		return SCSI_MLQUEUE_DEVICE_BUSY;
	}

	/* we're done with cmdrsp space - data from it has been copied
	 * into channel - free it now.
	 */
	kfree(cmdrsp);
	return 0;		/* non-zero implies host/device is busy */
}

static int
virthba_slave_alloc(struct scsi_device *scsidev)
{
	/* this called by the midlayer before scan for new devices -
	 * LLD can alloc any struct & do init if needed.
	 */
	struct virtdisk_info *vdisk;
	struct virtdisk_info *tmpvdisk;
	struct virthba_info *virthbainfo;
	struct Scsi_Host *scsihost = (struct Scsi_Host *)scsidev->host;

	virthbainfo = (struct virthba_info *)scsihost->hostdata;
	if (!virthbainfo)
		return 0;	/* even though we errored, treat as success */

	for (vdisk = &virthbainfo->head; vdisk->next; vdisk = vdisk->next) {
		if (vdisk->next->valid &&
		    (vdisk->next->channel == scsidev->channel) &&
		    (vdisk->next->id == scsidev->id) &&
		    (vdisk->next->lun == scsidev->lun))
			return 0;
	}
	tmpvdisk = kzalloc(sizeof(*tmpvdisk), GFP_ATOMIC);
	if (!tmpvdisk)
		return 0;

	tmpvdisk->channel = scsidev->channel;
	tmpvdisk->id = scsidev->id;
	tmpvdisk->lun = scsidev->lun;
	tmpvdisk->valid = 1;
	vdisk->next = tmpvdisk;
	return 0;		/* success */
}

static int
virthba_slave_configure(struct scsi_device *scsidev)
{
	return 0;		/* success */
}

static void
virthba_slave_destroy(struct scsi_device *scsidev)
{
	/* midlevel calls this after device has been quiesced and
	 * before it is to be deleted.
	 */
	struct virtdisk_info *vdisk, *delvdisk;
	struct virthba_info *virthbainfo;
	struct Scsi_Host *scsihost = (struct Scsi_Host *)scsidev->host;

	virthbainfo = (struct virthba_info *)scsihost->hostdata;
	for (vdisk = &virthbainfo->head; vdisk->next; vdisk = vdisk->next) {
		if (vdisk->next->valid &&
		    (vdisk->next->channel == scsidev->channel) &&
		    (vdisk->next->id == scsidev->id) &&
		    (vdisk->next->lun == scsidev->lun)) {
			delvdisk = vdisk->next;
			vdisk->next = vdisk->next->next;
			kfree(delvdisk);
			return;
		}
	}
}

/*****************************************************/
/* Scsi Cmnd support thread                          */
/*****************************************************/

static void
do_scsi_linuxstat(struct uiscmdrsp *cmdrsp, struct scsi_cmnd *scsicmd)
{
	struct virtdisk_info *vdisk;
	struct scsi_device *scsidev;
	struct sense_data *sd;

	scsidev = scsicmd->device;
	memcpy(scsicmd->sense_buffer, cmdrsp->scsi.sensebuf, MAX_SENSE_SIZE);
	sd = (struct sense_data *)scsicmd->sense_buffer;

	/* Do not log errors for disk-not-present inquiries */
	if ((cmdrsp->scsi.cmnd[0] == INQUIRY) &&
	    (host_byte(cmdrsp->scsi.linuxstat) == DID_NO_CONNECT) &&
	    (cmdrsp->scsi.addlstat == ADDL_SEL_TIMEOUT))
		return;

	/* Okay see what our error_count is here.... */
	for (vdisk = &((struct virthba_info *)scsidev->host->hostdata)->head;
	     vdisk->next; vdisk = vdisk->next) {
		if ((scsidev->channel != vdisk->channel) ||
		    (scsidev->id != vdisk->id) ||
		    (scsidev->lun != vdisk->lun))
			continue;

		if (atomic_read(&vdisk->error_count) < VIRTHBA_ERROR_COUNT) {
			atomic_inc(&vdisk->error_count);
			atomic_set(&vdisk->ios_threshold, IOS_ERROR_THRESHOLD);
		}
	}
}

static void
do_scsi_nolinuxstat(struct uiscmdrsp *cmdrsp, struct scsi_cmnd *scsicmd)
{
	struct scsi_device *scsidev;
	unsigned char buf[36];
	struct scatterlist *sg;
	unsigned int i;
	char *thispage;
	char *thispage_orig;
	int bufind = 0;
	struct virtdisk_info *vdisk;

	scsidev = scsicmd->device;
	if ((cmdrsp->scsi.cmnd[0] == INQUIRY) &&
	    (cmdrsp->scsi.bufflen >= MIN_INQUIRY_RESULT_LEN)) {
		if (cmdrsp->scsi.no_disk_result == 0)
			return;

		/* Linux scsi code is weird; it wants
		 * a device at Lun 0 to issue report
		 * luns, but we don't want a disk
		 * there so we'll present a processor
		 * there. */
		SET_NO_DISK_INQUIRY_RESULT(buf, cmdrsp->scsi.bufflen,
					   scsidev->lun,
					   DEV_DISK_CAPABLE_NOT_PRESENT,
					   DEV_NOT_CAPABLE);

		if (scsi_sg_count(scsicmd) == 0) {
			if (scsi_bufflen(scsicmd) > 0) {
				BUG_ON(scsi_sg_count(scsicmd) ==
				       0);
			}
			memcpy(scsi_sglist(scsicmd), buf,
			       cmdrsp->scsi.bufflen);
			return;
		}

		sg = scsi_sglist(scsicmd);
		for (i = 0; i < scsi_sg_count(scsicmd); i++) {
			thispage_orig = kmap_atomic(sg_page(sg + i));
			thispage = (void *)((unsigned long)thispage_orig |
					     sg[i].offset);
			memcpy(thispage, buf + bufind, sg[i].length);
			kunmap_atomic(thispage_orig);
			bufind += sg[i].length;
		}
	} else {
		vdisk = &((struct virthba_info *)scsidev->host->hostdata)->head;
		for ( ; vdisk->next; vdisk = vdisk->next) {
			if ((scsidev->channel != vdisk->channel) ||
			    (scsidev->id != vdisk->id) ||
			    (scsidev->lun != vdisk->lun))
				continue;

			if (atomic_read(&vdisk->ios_threshold) > 0) {
				atomic_dec(&vdisk->ios_threshold);
				if (atomic_read(&vdisk->ios_threshold) == 0) {
					atomic_set(&vdisk->error_count, 0);
				}
			}
		}
	}
}

static void
complete_scsi_command(struct uiscmdrsp *cmdrsp, struct scsi_cmnd *scsicmd)
{
	/* take what we need out of cmdrsp and complete the scsicmd */
	scsicmd->result = cmdrsp->scsi.linuxstat;
	if (cmdrsp->scsi.linuxstat)
		do_scsi_linuxstat(cmdrsp, scsicmd);
	else
		do_scsi_nolinuxstat(cmdrsp, scsicmd);

	if (scsicmd->scsi_done)
		scsicmd->scsi_done(scsicmd);
}

static inline void
complete_vdiskmgmt_command(struct uiscmdrsp *cmdrsp)
{
	/* copy the result of the taskmgmt and */
	/* wake up the error handler that is waiting for this */
	*(int *)cmdrsp->vdiskmgmt.notifyresult = cmdrsp->vdiskmgmt.result;
	wake_up_all((wait_queue_head_t *)cmdrsp->vdiskmgmt.notify);
}

static inline void
complete_taskmgmt_command(struct uiscmdrsp *cmdrsp)
{
	/* copy the result of the taskmgmt and */
	/* wake up the error handler that is waiting for this */
	*(int *)cmdrsp->scsitaskmgmt.notifyresult =
	    cmdrsp->scsitaskmgmt.result;
	wake_up_all((wait_queue_head_t *)cmdrsp->scsitaskmgmt.notify);
}

static void
drain_queue(struct virthba_info *virthbainfo, struct chaninfo *dc,
	    struct uiscmdrsp *cmdrsp)
{
	unsigned long flags;
	int qrslt = 0;
	struct scsi_cmnd *scsicmd;
	struct Scsi_Host *shost = virthbainfo->scsihost;

	while (1) {
		spin_lock_irqsave(&virthbainfo->chinfo.insertlock, flags);
		if (!spar_channel_client_acquire_os(dc->queueinfo->chan,
						    "vhba")) {
			spin_unlock_irqrestore(&virthbainfo->chinfo.insertlock,
					       flags);
			virthbainfo->acquire_failed_cnt++;
			break;
		}
		qrslt = uisqueue_get_cmdrsp(dc->queueinfo, cmdrsp,
					    IOCHAN_FROM_IOPART);
		spar_channel_client_release_os(dc->queueinfo->chan, "vhba");
		spin_unlock_irqrestore(&virthbainfo->chinfo.insertlock, flags);
		if (qrslt == 0)
			break;
		if (cmdrsp->cmdtype == CMD_SCSI_TYPE) {
			/* scsicmd location is returned by the
			 * deletion
			 */
			scsicmd = del_scsipending_entry(virthbainfo,
							(uintptr_t)
							 cmdrsp->scsi.scsicmd);
			if (!scsicmd)
				break;
			/* complete the orig cmd */
			complete_scsi_command(cmdrsp, scsicmd);
		} else if (cmdrsp->cmdtype == CMD_SCSITASKMGMT_TYPE) {
			if (!del_scsipending_entry(virthbainfo,
				   (uintptr_t)cmdrsp->scsitaskmgmt.scsicmd))
				break;
			complete_taskmgmt_command(cmdrsp);
		} else if (cmdrsp->cmdtype == CMD_NOTIFYGUEST_TYPE) {
			/* The vHba pointer has no meaning in
			 * a Client/Guest Partition. Let's be
			 * safe and set it to NULL now.  Do
			 * not use it here! */
			cmdrsp->disknotify.v_hba = NULL;
			process_disk_notify(shost, cmdrsp);
		} else if (cmdrsp->cmdtype == CMD_VDISKMGMT_TYPE) {
			if (!del_scsipending_entry(virthbainfo,
						   (uintptr_t)
						    cmdrsp->vdiskmgmt.scsicmd))
				break;
			complete_vdiskmgmt_command(cmdrsp);
		}
		/* cmdrsp is now available for reuse */
	}
}

/* main function for the thread that waits for scsi commands to arrive
 * in a specified queue
 */
static int
process_incoming_rsps(void *v)
{
	struct virthba_info *virthbainfo = v;
	struct chaninfo *dc = &virthbainfo->chinfo;
	struct uiscmdrsp *cmdrsp = NULL;
	const int SZ = sizeof(struct uiscmdrsp);
	u64 mask;
	unsigned long long rc1;

	UIS_DAEMONIZE("vhba_incoming");
	/* alloc once and reuse */
	cmdrsp = kmalloc(SZ, GFP_ATOMIC);
	if (!cmdrsp) {
		complete_and_exit(&dc->threadinfo.has_stopped, 0);
		return 0;
	}
	mask = ULTRA_CHANNEL_ENABLE_INTS;
	while (1) {
		if (kthread_should_stop())
			break;
		wait_event_interruptible_timeout(virthbainfo->rsp_queue,
			 (atomic_read(&virthbainfo->interrupt_rcvd) == 1),
				      usecs_to_jiffies(rsltq_wait_usecs));
		atomic_set(&virthbainfo->interrupt_rcvd, 0);
		/* drain queue */
		drain_queue(virthbainfo, dc, cmdrsp);
		rc1 = uisqueue_interlocked_or(virthbainfo->flags_addr, mask);
	}

	kfree(cmdrsp);

	complete_and_exit(&dc->threadinfo.has_stopped, 0);
}

/*****************************************************/
/* Debugfs filesystem functions                      */
/*****************************************************/

static ssize_t info_debugfs_read(struct file *file,
				 char __user *buf, size_t len, loff_t *offset)
{
	ssize_t bytes_read = 0;
	int str_pos = 0;
	u64 phys_flags_addr;
	int i;
	struct virthba_info *virthbainfo;
	char *vbuf;

	if (len > MAX_BUF)
		len = MAX_BUF;
	vbuf = kzalloc(len, GFP_KERNEL);
	if (!vbuf)
		return -ENOMEM;

	for (i = 0; i < VIRTHBASOPENMAX; i++) {
		if (!virthbas_open[i].virthbainfo)
			continue;

		virthbainfo = virthbas_open[i].virthbainfo;

		str_pos += scnprintf(vbuf + str_pos,
				len - str_pos, "max_buff_len:%u\n",
				max_buff_len);

		str_pos += scnprintf(vbuf + str_pos, len - str_pos,
				"\nvirthba result queue poll wait:%d usecs.\n",
				rsltq_wait_usecs);
		str_pos += scnprintf(vbuf + str_pos, len - str_pos,
				"\ninterrupts_rcvd = %llu, interrupts_disabled = %llu\n",
				virthbainfo->interrupts_rcvd,
				virthbainfo->interrupts_disabled);
		str_pos += scnprintf(vbuf + str_pos,
				len - str_pos, "\ninterrupts_notme = %llu,\n",
				virthbainfo->interrupts_notme);
		phys_flags_addr = virt_to_phys((__force  void *)
					       virthbainfo->flags_addr);
		str_pos += scnprintf(vbuf + str_pos, len - str_pos,
				"flags_addr = %p, phys_flags_addr=0x%016llx, FeatureFlags=%llu\n",
				virthbainfo->flags_addr, phys_flags_addr,
				(__le64)readq(virthbainfo->flags_addr));
		str_pos += scnprintf(vbuf + str_pos,
			len - str_pos, "acquire_failed_cnt:%llu\n",
			virthbainfo->acquire_failed_cnt);
		str_pos += scnprintf(vbuf + str_pos, len - str_pos, "\n");
	}

	bytes_read = simple_read_from_buffer(buf, len, offset, vbuf, str_pos);
	kfree(vbuf);
	return bytes_read;
}

static ssize_t enable_ints_write(struct file *file, const char __user *buffer,
				 size_t count, loff_t *ppos)
{
	char buf[4];
	int i, new_value;
	struct virthba_info *virthbainfo;

	u64 __iomem *features_addr;
	u64 mask;

	if (count >= ARRAY_SIZE(buf))
		return -EINVAL;

	buf[count] = '\0';
	if (copy_from_user(buf, buffer, count))
		return -EFAULT;

	i = kstrtoint(buf, 10, &new_value);

	if (i != 0)
		return -EFAULT;

	/* set all counts to new_value usually 0 */
	for (i = 0; i < VIRTHBASOPENMAX; i++) {
		if (virthbas_open[i].virthbainfo) {
			virthbainfo = virthbas_open[i].virthbainfo;
			features_addr =
				&virthbainfo->chinfo.queueinfo->chan->features;
			if (new_value == 1) {
				mask = ~(ULTRA_IO_CHANNEL_IS_POLLING |
					 ULTRA_IO_DRIVER_DISABLES_INTS);
				uisqueue_interlocked_and(features_addr, mask);
				mask = ULTRA_IO_DRIVER_ENABLES_INTS;
				uisqueue_interlocked_or(features_addr, mask);
				rsltq_wait_usecs = 4000000;
			} else {
				mask = ~(ULTRA_IO_DRIVER_ENABLES_INTS |
					 ULTRA_IO_DRIVER_DISABLES_INTS);
				uisqueue_interlocked_and(features_addr, mask);
				mask = ULTRA_IO_CHANNEL_IS_POLLING;
				uisqueue_interlocked_or(features_addr, mask);
				rsltq_wait_usecs = 4000;
			}
		}
	}
	return count;
}

/* As per VirtpciFunc returns 1 for success and 0 for failure */
static int
virthba_serverup(struct virtpci_dev *virtpcidev)
{
	struct virthba_info *virthbainfo =
	    (struct virthba_info *)((struct Scsi_Host *)virtpcidev->scsi.
				     scsihost)->hostdata;

	if (!virthbainfo->serverdown)
		return 1;

	if (virthbainfo->serverchangingstate)
		return 0;

	virthbainfo->serverchangingstate = true;
	/* Must transition channel to ATTACHED state BEFORE we
	 * can start using the device again
	 */
	SPAR_CHANNEL_CLIENT_TRANSITION(virthbainfo->chinfo.queueinfo->chan,
				       dev_name(&virtpcidev->generic_dev),
				       CHANNELCLI_ATTACHED, NULL);

	/* Start Processing the IOVM Response Queue Again */
	if (!uisthread_start(&virthbainfo->chinfo.threadinfo,
			     process_incoming_rsps,
			     virthbainfo, "vhba_incoming")) {
		return 0;
	}
	virthbainfo->serverdown = false;
	virthbainfo->serverchangingstate = false;

	return 1;
}

static void
virthba_serverdown_complete(struct work_struct *work)
{
	struct virthba_info *virthbainfo;
	struct virtpci_dev *virtpcidev;
	int i;
	struct scsipending *pendingdel = NULL;
	struct scsi_cmnd *scsicmd = NULL;
	struct uiscmdrsp *cmdrsp;
	unsigned long flags;

	virthbainfo = container_of(work, struct virthba_info,
				   serverdown_completion);

	/* Stop Using the IOVM Response Queue (queue should be drained
	 * by the end)
	 */
	uisthread_stop(&virthbainfo->chinfo.threadinfo);

	/* Fail Commands that weren't completed */
	spin_lock_irqsave(&virthbainfo->privlock, flags);
	for (i = 0; i < MAX_PENDING_REQUESTS; i++) {
		pendingdel = &virthbainfo->pending[i];
		switch (pendingdel->cmdtype) {
		case CMD_SCSI_TYPE:
			scsicmd = (struct scsi_cmnd *)pendingdel->sent;
			scsicmd->result = DID_RESET << 16;
			if (scsicmd->scsi_done)
				scsicmd->scsi_done(scsicmd);
			break;
		case CMD_SCSITASKMGMT_TYPE:
			cmdrsp = (struct uiscmdrsp *)pendingdel->sent;
			wake_up_all((wait_queue_head_t *)
				    cmdrsp->scsitaskmgmt.notify);
			*(int *)cmdrsp->scsitaskmgmt.notifyresult =
				TASK_MGMT_FAILED;
			break;
		case CMD_VDISKMGMT_TYPE:
			cmdrsp = (struct uiscmdrsp *)pendingdel->sent;
			*(int *)cmdrsp->vdiskmgmt.notifyresult =
			    VDISK_MGMT_FAILED;
			wake_up_all((wait_queue_head_t *)
				    cmdrsp->vdiskmgmt.notify);
			break;
		default:
			break;
		}
		pendingdel->cmdtype = 0;
		pendingdel->sent = NULL;
	}
	spin_unlock_irqrestore(&virthbainfo->privlock, flags);

	virtpcidev = virthbainfo->virtpcidev;

	virthbainfo->serverdown = true;
	virthbainfo->serverchangingstate = false;
	/* Return the ServerDown response to Command */
	visorchipset_device_pause_response(virtpcidev->bus_no,
					   virtpcidev->device_no, 0);
}

/* As per VirtpciFunc returns 1 for success and 0 for failure */
static int
virthba_serverdown(struct virtpci_dev *virtpcidev, u32 state)
{
	int stat = 1;

	struct virthba_info *virthbainfo =
	    (struct virthba_info *)((struct Scsi_Host *)virtpcidev->scsi.
				     scsihost)->hostdata;

	if (!virthbainfo->serverdown && !virthbainfo->serverchangingstate) {
		virthbainfo->serverchangingstate = true;
		queue_work(virthba_serverdown_workqueue,
			   &virthbainfo->serverdown_completion);
	} else if (virthbainfo->serverchangingstate) {
		stat = 0;
	}

	return stat;
}

/*****************************************************/
/* Module Init & Exit functions                      */
/*****************************************************/

static int __init
virthba_parse_line(char *str)
{
	return 1;
}

static void __init
virthba_parse_options(char *line)
{
	char *next = line;

	POSTCODE_LINUX_2(VHBA_CREATE_ENTRY_PC, POSTCODE_SEVERITY_INFO);
	if (!line || !*line)
		return;
	while ((line = next)) {
		next = strchr(line, ' ');
		if (next)
			*next++ = 0;
		virthba_parse_line(line);
	}

	POSTCODE_LINUX_2(VHBA_CREATE_EXIT_PC, POSTCODE_SEVERITY_INFO);
}

static int __init
virthba_mod_init(void)
{
	int error;
	int i;

	if (!unisys_spar_platform)
		return -ENODEV;

	POSTCODE_LINUX_2(VHBA_CREATE_ENTRY_PC, POSTCODE_SEVERITY_INFO);
	virthba_parse_options(virthba_options);

	error = virtpci_register_driver(&virthba_driver);
	if (error < 0) {
		POSTCODE_LINUX_3(VHBA_CREATE_FAILURE_PC, error,
				 POSTCODE_SEVERITY_ERR);
	} else {
		/* create the debugfs directories and entries */
		virthba_debugfs_dir = debugfs_create_dir("virthba", NULL);
		debugfs_create_file("info", S_IRUSR, virthba_debugfs_dir,
				    NULL, &debugfs_info_fops);
		debugfs_create_u32("rqwait_usecs", S_IRUSR | S_IWUSR,
				   virthba_debugfs_dir, &rsltq_wait_usecs);
		debugfs_create_file("enable_ints", S_IWUSR,
				    virthba_debugfs_dir, NULL,
				    &debugfs_enable_ints_fops);
		/* Initialize dar_work_queue */
		INIT_WORK(&dar_work_queue, do_disk_add_remove);
		spin_lock_init(&dar_work_queue_lock);

		/* clear out array */
		for (i = 0; i < VIRTHBASOPENMAX; i++)
			virthbas_open[i].virthbainfo = NULL;
		/* Initialize the serverdown workqueue */
		virthba_serverdown_workqueue =
		    create_singlethread_workqueue("virthba_serverdown");
		if (!virthba_serverdown_workqueue) {
			POSTCODE_LINUX_2(VHBA_CREATE_FAILURE_PC,
					 POSTCODE_SEVERITY_ERR);
			error = -1;
		}
	}

	POSTCODE_LINUX_2(VHBA_CREATE_EXIT_PC, POSTCODE_SEVERITY_INFO);
	return error;
}

static ssize_t
virthba_acquire_lun(struct device *cdev, struct device_attribute *attr,
		    const char *buf, size_t count)
{
	struct uisscsi_dest vdest;
	struct Scsi_Host *shost = class_to_shost(cdev);
	int i;

	i = sscanf(buf, "%d-%d-%d", &vdest.channel, &vdest.id, &vdest.lun);
	if (i != 3)
		return i;

	return forward_vdiskmgmt_command(VDISK_MGMT_ACQUIRE, shost, &vdest);
}

static ssize_t
virthba_release_lun(struct device *cdev, struct device_attribute *attr,
		    const char *buf, size_t count)
{
	struct uisscsi_dest vdest;
	struct Scsi_Host *shost = class_to_shost(cdev);
	int i;

	i = sscanf(buf, "%d-%d-%d", &vdest.channel, &vdest.id, &vdest.lun);
	if (i != 3)
		return i;

	return forward_vdiskmgmt_command(VDISK_MGMT_RELEASE, shost, &vdest);
}

#define CLASS_DEVICE_ATTR(_name, _mode, _show, _store)      \
	struct device_attribute class_device_attr_##_name =   \
		__ATTR(_name, _mode, _show, _store)

static CLASS_DEVICE_ATTR(acquire_lun, S_IWUSR, NULL, virthba_acquire_lun);
static CLASS_DEVICE_ATTR(release_lun, S_IWUSR, NULL, virthba_release_lun);

static DEVICE_ATTRIBUTE *virthba_shost_attrs[] = {
	&class_device_attr_acquire_lun,
	&class_device_attr_release_lun,
	NULL
};

static void __exit
virthba_mod_exit(void)
{
	virtpci_unregister_driver(&virthba_driver);
	/* unregister is going to call virthba_remove */
	/* destroy serverdown completion workqueue */
	if (virthba_serverdown_workqueue) {
		destroy_workqueue(virthba_serverdown_workqueue);
		virthba_serverdown_workqueue = NULL;
	}

	debugfs_remove_recursive(virthba_debugfs_dir);
}

/* specify function to be run at module insertion time */
module_init(virthba_mod_init);

/* specify function to be run when module is removed */
module_exit(virthba_mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Usha Srinivasan");
MODULE_ALIAS("uisvirthba");
	/* this is extracted during depmod and kept in modules.dep */
/* module parameter */
module_param(virthba_options, charp, S_IRUGO);
