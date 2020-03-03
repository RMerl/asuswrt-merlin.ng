/*
 * Copyright (c) 2009, Microsoft Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307 USA.
 *
 * Authors:
 *   Haiyang Zhang <haiyangz@microsoft.com>
 *   Hank Janssen  <hjanssen@microsoft.com>
 *   K. Y. Srinivasan <kys@microsoft.com>
 */

#include <linux/kernel.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/completion.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/hyperv.h>
#include <linux/blkdev.h>
#include <scsi/scsi.h>
#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_host.h>
#include <scsi/scsi_device.h>
#include <scsi/scsi_tcq.h>
#include <scsi/scsi_eh.h>
#include <scsi/scsi_devinfo.h>
#include <scsi/scsi_dbg.h>

/*
 * All wire protocol details (storage protocol between the guest and the host)
 * are consolidated here.
 *
 * Begin protocol definitions.
 */

/*
 * Version history:
 * V1 Beta: 0.1
 * V1 RC < 2008/1/31: 1.0
 * V1 RC > 2008/1/31:  2.0
 * Win7: 4.2
 * Win8: 5.1
 */


#define VMSTOR_WIN7_MAJOR 4
#define VMSTOR_WIN7_MINOR 2

#define VMSTOR_WIN8_MAJOR 5
#define VMSTOR_WIN8_MINOR 1


/*  Packet structure describing virtual storage requests. */
enum vstor_packet_operation {
	VSTOR_OPERATION_COMPLETE_IO		= 1,
	VSTOR_OPERATION_REMOVE_DEVICE		= 2,
	VSTOR_OPERATION_EXECUTE_SRB		= 3,
	VSTOR_OPERATION_RESET_LUN		= 4,
	VSTOR_OPERATION_RESET_ADAPTER		= 5,
	VSTOR_OPERATION_RESET_BUS		= 6,
	VSTOR_OPERATION_BEGIN_INITIALIZATION	= 7,
	VSTOR_OPERATION_END_INITIALIZATION	= 8,
	VSTOR_OPERATION_QUERY_PROTOCOL_VERSION	= 9,
	VSTOR_OPERATION_QUERY_PROPERTIES	= 10,
	VSTOR_OPERATION_ENUMERATE_BUS		= 11,
	VSTOR_OPERATION_FCHBA_DATA              = 12,
	VSTOR_OPERATION_CREATE_SUB_CHANNELS     = 13,
	VSTOR_OPERATION_MAXIMUM                 = 13
};

/*
 * WWN packet for Fibre Channel HBA
 */

struct hv_fc_wwn_packet {
	bool	primary_active;
	u8	reserved1;
	u8	reserved2;
	u8	primary_port_wwn[8];
	u8	primary_node_wwn[8];
	u8	secondary_port_wwn[8];
	u8	secondary_node_wwn[8];
};



/*
 * SRB Flag Bits
 */

#define SRB_FLAGS_QUEUE_ACTION_ENABLE		0x00000002
#define SRB_FLAGS_DISABLE_DISCONNECT		0x00000004
#define SRB_FLAGS_DISABLE_SYNCH_TRANSFER	0x00000008
#define SRB_FLAGS_BYPASS_FROZEN_QUEUE		0x00000010
#define SRB_FLAGS_DISABLE_AUTOSENSE		0x00000020
#define SRB_FLAGS_DATA_IN			0x00000040
#define SRB_FLAGS_DATA_OUT			0x00000080
#define SRB_FLAGS_NO_DATA_TRANSFER		0x00000000
#define SRB_FLAGS_UNSPECIFIED_DIRECTION	(SRB_FLAGS_DATA_IN | SRB_FLAGS_DATA_OUT)
#define SRB_FLAGS_NO_QUEUE_FREEZE		0x00000100
#define SRB_FLAGS_ADAPTER_CACHE_ENABLE		0x00000200
#define SRB_FLAGS_FREE_SENSE_BUFFER		0x00000400

/*
 * This flag indicates the request is part of the workflow for processing a D3.
 */
#define SRB_FLAGS_D3_PROCESSING			0x00000800
#define SRB_FLAGS_IS_ACTIVE			0x00010000
#define SRB_FLAGS_ALLOCATED_FROM_ZONE		0x00020000
#define SRB_FLAGS_SGLIST_FROM_POOL		0x00040000
#define SRB_FLAGS_BYPASS_LOCKED_QUEUE		0x00080000
#define SRB_FLAGS_NO_KEEP_AWAKE			0x00100000
#define SRB_FLAGS_PORT_DRIVER_ALLOCSENSE	0x00200000
#define SRB_FLAGS_PORT_DRIVER_SENSEHASPORT	0x00400000
#define SRB_FLAGS_DONT_START_NEXT_PACKET	0x00800000
#define SRB_FLAGS_PORT_DRIVER_RESERVED		0x0F000000
#define SRB_FLAGS_CLASS_DRIVER_RESERVED		0xF0000000

#define SP_UNTAGGED			((unsigned char) ~0)
#define SRB_SIMPLE_TAG_REQUEST		0x20

/*
 * Platform neutral description of a scsi request -
 * this remains the same across the write regardless of 32/64 bit
 * note: it's patterned off the SCSI_PASS_THROUGH structure
 */
#define STORVSC_MAX_CMD_LEN			0x10

#define POST_WIN7_STORVSC_SENSE_BUFFER_SIZE	0x14
#define PRE_WIN8_STORVSC_SENSE_BUFFER_SIZE	0x12

#define STORVSC_SENSE_BUFFER_SIZE		0x14
#define STORVSC_MAX_BUF_LEN_WITH_PADDING	0x14

/*
 * Sense buffer size changed in win8; have a run-time
 * variable to track the size we should use.
 */
static int sense_buffer_size;

/*
 * The size of the vmscsi_request has changed in win8. The
 * additional size is because of new elements added to the
 * structure. These elements are valid only when we are talking
 * to a win8 host.
 * Track the correction to size we need to apply.
 */

static int vmscsi_size_delta;
static int vmstor_current_major;
static int vmstor_current_minor;

struct vmscsi_win8_extension {
	/*
	 * The following were added in Windows 8
	 */
	u16 reserve;
	u8  queue_tag;
	u8  queue_action;
	u32 srb_flags;
	u32 time_out_value;
	u32 queue_sort_ey;
} __packed;

struct vmscsi_request {
	u16 length;
	u8 srb_status;
	u8 scsi_status;

	u8  port_number;
	u8  path_id;
	u8  target_id;
	u8  lun;

	u8  cdb_length;
	u8  sense_info_length;
	u8  data_in;
	u8  reserved;

	u32 data_transfer_length;

	union {
		u8 cdb[STORVSC_MAX_CMD_LEN];
		u8 sense_data[STORVSC_SENSE_BUFFER_SIZE];
		u8 reserved_array[STORVSC_MAX_BUF_LEN_WITH_PADDING];
	};
	/*
	 * The following was added in win8.
	 */
	struct vmscsi_win8_extension win8_extension;

} __attribute((packed));


/*
 * This structure is sent during the intialization phase to get the different
 * properties of the channel.
 */

#define STORAGE_CHANNEL_SUPPORTS_MULTI_CHANNEL		0x1

struct vmstorage_channel_properties {
	u32 reserved;
	u16 max_channel_cnt;
	u16 reserved1;

	u32 flags;
	u32   max_transfer_bytes;

	u64  reserved2;
} __packed;

/*  This structure is sent during the storage protocol negotiations. */
struct vmstorage_protocol_version {
	/* Major (MSW) and minor (LSW) version numbers. */
	u16 major_minor;

	/*
	 * Revision number is auto-incremented whenever this file is changed
	 * (See FILL_VMSTOR_REVISION macro above).  Mismatch does not
	 * definitely indicate incompatibility--but it does indicate mismatched
	 * builds.
	 * This is only used on the windows side. Just set it to 0.
	 */
	u16 revision;
} __packed;

/* Channel Property Flags */
#define STORAGE_CHANNEL_REMOVABLE_FLAG		0x1
#define STORAGE_CHANNEL_EMULATED_IDE_FLAG	0x2

struct vstor_packet {
	/* Requested operation type */
	enum vstor_packet_operation operation;

	/*  Flags - see below for values */
	u32 flags;

	/* Status of the request returned from the server side. */
	u32 status;

	/* Data payload area */
	union {
		/*
		 * Structure used to forward SCSI commands from the
		 * client to the server.
		 */
		struct vmscsi_request vm_srb;

		/* Structure used to query channel properties. */
		struct vmstorage_channel_properties storage_channel_properties;

		/* Used during version negotiations. */
		struct vmstorage_protocol_version version;

		/* Fibre channel address packet */
		struct hv_fc_wwn_packet wwn_packet;

		/* Number of sub-channels to create */
		u16 sub_channel_count;

		/* This will be the maximum of the union members */
		u8  buffer[0x34];
	};
} __packed;

/*
 * Packet Flags:
 *
 * This flag indicates that the server should send back a completion for this
 * packet.
 */

#define REQUEST_COMPLETION_FLAG	0x1

/* Matches Windows-end */
enum storvsc_request_type {
	WRITE_TYPE = 0,
	READ_TYPE,
	UNKNOWN_TYPE,
};

/*
 * SRB status codes and masks; a subset of the codes used here.
 */

#define SRB_STATUS_AUTOSENSE_VALID	0x80
#define SRB_STATUS_INVALID_LUN	0x20
#define SRB_STATUS_SUCCESS	0x01
#define SRB_STATUS_ABORTED	0x02
#define SRB_STATUS_ERROR	0x04

/*
 * This is the end of Protocol specific defines.
 */

static int storvsc_ringbuffer_size = (256 * PAGE_SIZE);
static u32 max_outstanding_req_per_channel;

static int storvsc_vcpus_per_sub_channel = 4;

module_param(storvsc_ringbuffer_size, int, S_IRUGO);
MODULE_PARM_DESC(storvsc_ringbuffer_size, "Ring buffer size (bytes)");

module_param(storvsc_vcpus_per_sub_channel, int, S_IRUGO);
MODULE_PARM_DESC(vcpus_per_sub_channel, "Ratio of VCPUs to subchannels");
/*
 * Timeout in seconds for all devices managed by this driver.
 */
static int storvsc_timeout = 180;


static void storvsc_on_channel_callback(void *context);

#define STORVSC_MAX_LUNS_PER_TARGET			255
#define STORVSC_MAX_TARGETS				2
#define STORVSC_MAX_CHANNELS				8

#define STORVSC_FC_MAX_LUNS_PER_TARGET			255
#define STORVSC_FC_MAX_TARGETS				128
#define STORVSC_FC_MAX_CHANNELS				8

#define STORVSC_IDE_MAX_LUNS_PER_TARGET			64
#define STORVSC_IDE_MAX_TARGETS				1
#define STORVSC_IDE_MAX_CHANNELS			1

struct storvsc_cmd_request {
	struct scsi_cmnd *cmd;

	unsigned int bounce_sgl_count;
	struct scatterlist *bounce_sgl;

	struct hv_device *device;

	/* Synchronize the request/response if needed */
	struct completion wait_event;

	struct vmbus_channel_packet_multipage_buffer mpb;
	struct vmbus_packet_mpb_array *payload;
	u32 payload_sz;

	struct vstor_packet vstor_packet;
};


/* A storvsc device is a device object that contains a vmbus channel */
struct storvsc_device {
	struct hv_device *device;

	bool	 destroy;
	bool	 drain_notify;
	bool	 open_sub_channel;
	atomic_t num_outstanding_req;
	struct Scsi_Host *host;

	wait_queue_head_t waiting_to_drain;

	/*
	 * Each unique Port/Path/Target represents 1 channel ie scsi
	 * controller. In reality, the pathid, targetid is always 0
	 * and the port is set by us
	 */
	unsigned int port_number;
	unsigned char path_id;
	unsigned char target_id;

	/*
	 * Max I/O, the device can support.
	 */
	u32   max_transfer_bytes;
	/* Used for vsc/vsp channel reset process */
	struct storvsc_cmd_request init_request;
	struct storvsc_cmd_request reset_request;
};

struct hv_host_device {
	struct hv_device *dev;
	unsigned int port;
	unsigned char path;
	unsigned char target;
};

struct storvsc_scan_work {
	struct work_struct work;
	struct Scsi_Host *host;
	uint lun;
};

static void storvsc_device_scan(struct work_struct *work)
{
	struct storvsc_scan_work *wrk;
	uint lun;
	struct scsi_device *sdev;

	wrk = container_of(work, struct storvsc_scan_work, work);
	lun = wrk->lun;

	sdev = scsi_device_lookup(wrk->host, 0, 0, lun);
	if (!sdev)
		goto done;
	scsi_rescan_device(&sdev->sdev_gendev);
	scsi_device_put(sdev);

done:
	kfree(wrk);
}

static void storvsc_host_scan(struct work_struct *work)
{
	struct storvsc_scan_work *wrk;
	struct Scsi_Host *host;
	struct scsi_device *sdev;
	unsigned long flags;

	wrk = container_of(work, struct storvsc_scan_work, work);
	host = wrk->host;

	/*
	 * Before scanning the host, first check to see if any of the
	 * currrently known devices have been hot removed. We issue a
	 * "unit ready" command against all currently known devices.
	 * This I/O will result in an error for devices that have been
	 * removed. As part of handling the I/O error, we remove the device.
	 *
	 * When a LUN is added or removed, the host sends us a signal to
	 * scan the host. Thus we are forced to discover the LUNs that
	 * may have been removed this way.
	 */
	mutex_lock(&host->scan_mutex);
	spin_lock_irqsave(host->host_lock, flags);
	list_for_each_entry(sdev, &host->__devices, siblings) {
		spin_unlock_irqrestore(host->host_lock, flags);
		scsi_test_unit_ready(sdev, 1, 1, NULL);
		spin_lock_irqsave(host->host_lock, flags);
		continue;
	}
	spin_unlock_irqrestore(host->host_lock, flags);
	mutex_unlock(&host->scan_mutex);
	/*
	 * Now scan the host to discover LUNs that may have been added.
	 */
	scsi_scan_host(host);

	kfree(wrk);
}

static void storvsc_remove_lun(struct work_struct *work)
{
	struct storvsc_scan_work *wrk;
	struct scsi_device *sdev;

	wrk = container_of(work, struct storvsc_scan_work, work);
	if (!scsi_host_get(wrk->host))
		goto done;

	sdev = scsi_device_lookup(wrk->host, 0, 0, wrk->lun);

	if (sdev) {
		scsi_remove_device(sdev);
		scsi_device_put(sdev);
	}
	scsi_host_put(wrk->host);

done:
	kfree(wrk);
}

/*
 * Major/minor macros.  Minor version is in LSB, meaning that earlier flat
 * version numbers will be interpreted as "0.x" (i.e., 1 becomes 0.1).
 */

static inline u16 storvsc_get_version(u8 major, u8 minor)
{
	u16 version;

	version = ((major << 8) | minor);
	return version;
}

/*
 * We can get incoming messages from the host that are not in response to
 * messages that we have sent out. An example of this would be messages
 * received by the guest to notify dynamic addition/removal of LUNs. To
 * deal with potential race conditions where the driver may be in the
 * midst of being unloaded when we might receive an unsolicited message
 * from the host, we have implemented a mechanism to gurantee sequential
 * consistency:
 *
 * 1) Once the device is marked as being destroyed, we will fail all
 *    outgoing messages.
 * 2) We permit incoming messages when the device is being destroyed,
 *    only to properly account for messages already sent out.
 */

static inline struct storvsc_device *get_out_stor_device(
					struct hv_device *device)
{
	struct storvsc_device *stor_device;

	stor_device = hv_get_drvdata(device);

	if (stor_device && stor_device->destroy)
		stor_device = NULL;

	return stor_device;
}


static inline void storvsc_wait_to_drain(struct storvsc_device *dev)
{
	dev->drain_notify = true;
	wait_event(dev->waiting_to_drain,
		   atomic_read(&dev->num_outstanding_req) == 0);
	dev->drain_notify = false;
}

static inline struct storvsc_device *get_in_stor_device(
					struct hv_device *device)
{
	struct storvsc_device *stor_device;

	stor_device = hv_get_drvdata(device);

	if (!stor_device)
		goto get_in_err;

	/*
	 * If the device is being destroyed; allow incoming
	 * traffic only to cleanup outstanding requests.
	 */

	if (stor_device->destroy  &&
		(atomic_read(&stor_device->num_outstanding_req) == 0))
		stor_device = NULL;

get_in_err:
	return stor_device;

}

static void destroy_bounce_buffer(struct scatterlist *sgl,
				  unsigned int sg_count)
{
	int i;
	struct page *page_buf;

	for (i = 0; i < sg_count; i++) {
		page_buf = sg_page((&sgl[i]));
		if (page_buf != NULL)
			__free_page(page_buf);
	}

	kfree(sgl);
}

static int do_bounce_buffer(struct scatterlist *sgl, unsigned int sg_count)
{
	int i;

	/* No need to check */
	if (sg_count < 2)
		return -1;

	/* We have at least 2 sg entries */
	for (i = 0; i < sg_count; i++) {
		if (i == 0) {
			/* make sure 1st one does not have hole */
			if (sgl->offset + sgl->length != PAGE_SIZE)
				return i;
		} else if (i == sg_count - 1) {
			/* make sure last one does not have hole */
			if (sgl->offset != 0)
				return i;
		} else {
			/* make sure no hole in the middle */
			if (sgl->length != PAGE_SIZE || sgl->offset != 0)
				return i;
		}
		sgl = sg_next(sgl);
	}
	return -1;
}

static struct scatterlist *create_bounce_buffer(struct scatterlist *sgl,
						unsigned int sg_count,
						unsigned int len,
						int write)
{
	int i;
	int num_pages;
	struct scatterlist *bounce_sgl;
	struct page *page_buf;
	unsigned int buf_len = ((write == WRITE_TYPE) ? 0 : PAGE_SIZE);

	num_pages = ALIGN(len, PAGE_SIZE) >> PAGE_SHIFT;

	bounce_sgl = kcalloc(num_pages, sizeof(struct scatterlist), GFP_ATOMIC);
	if (!bounce_sgl)
		return NULL;

	sg_init_table(bounce_sgl, num_pages);
	for (i = 0; i < num_pages; i++) {
		page_buf = alloc_page(GFP_ATOMIC);
		if (!page_buf)
			goto cleanup;
		sg_set_page(&bounce_sgl[i], page_buf, buf_len, 0);
	}

	return bounce_sgl;

cleanup:
	destroy_bounce_buffer(bounce_sgl, num_pages);
	return NULL;
}

/* Assume the original sgl has enough room */
static unsigned int copy_from_bounce_buffer(struct scatterlist *orig_sgl,
					    struct scatterlist *bounce_sgl,
					    unsigned int orig_sgl_count,
					    unsigned int bounce_sgl_count)
{
	int i;
	int j = 0;
	unsigned long src, dest;
	unsigned int srclen, destlen, copylen;
	unsigned int total_copied = 0;
	unsigned long bounce_addr = 0;
	unsigned long dest_addr = 0;
	unsigned long flags;
	struct scatterlist *cur_dest_sgl;
	struct scatterlist *cur_src_sgl;

	local_irq_save(flags);
	cur_dest_sgl = orig_sgl;
	cur_src_sgl = bounce_sgl;
	for (i = 0; i < orig_sgl_count; i++) {
		dest_addr = (unsigned long)
				kmap_atomic(sg_page(cur_dest_sgl)) +
				cur_dest_sgl->offset;
		dest = dest_addr;
		destlen = cur_dest_sgl->length;

		if (bounce_addr == 0)
			bounce_addr = (unsigned long)kmap_atomic(
							sg_page(cur_src_sgl));

		while (destlen) {
			src = bounce_addr + cur_src_sgl->offset;
			srclen = cur_src_sgl->length - cur_src_sgl->offset;

			copylen = min(srclen, destlen);
			memcpy((void *)dest, (void *)src, copylen);

			total_copied += copylen;
			cur_src_sgl->offset += copylen;
			destlen -= copylen;
			dest += copylen;

			if (cur_src_sgl->offset == cur_src_sgl->length) {
				/* full */
				kunmap_atomic((void *)bounce_addr);
				j++;

				/*
				 * It is possible that the number of elements
				 * in the bounce buffer may not be equal to
				 * the number of elements in the original
				 * scatter list. Handle this correctly.
				 */

				if (j == bounce_sgl_count) {
					/*
					 * We are done; cleanup and return.
					 */
					kunmap_atomic((void *)(dest_addr -
						cur_dest_sgl->offset));
					local_irq_restore(flags);
					return total_copied;
				}

				/* if we need to use another bounce buffer */
				if (destlen || i != orig_sgl_count - 1) {
					cur_src_sgl = sg_next(cur_src_sgl);
					bounce_addr = (unsigned long)
							kmap_atomic(
							sg_page(cur_src_sgl));
				}
			} else if (destlen == 0 && i == orig_sgl_count - 1) {
				/* unmap the last bounce that is < PAGE_SIZE */
				kunmap_atomic((void *)bounce_addr);
			}
		}

		kunmap_atomic((void *)(dest_addr - cur_dest_sgl->offset));
		cur_dest_sgl = sg_next(cur_dest_sgl);
	}

	local_irq_restore(flags);

	return total_copied;
}

/* Assume the bounce_sgl has enough room ie using the create_bounce_buffer() */
static unsigned int copy_to_bounce_buffer(struct scatterlist *orig_sgl,
					  struct scatterlist *bounce_sgl,
					  unsigned int orig_sgl_count)
{
	int i;
	int j = 0;
	unsigned long src, dest;
	unsigned int srclen, destlen, copylen;
	unsigned int total_copied = 0;
	unsigned long bounce_addr = 0;
	unsigned long src_addr = 0;
	unsigned long flags;
	struct scatterlist *cur_src_sgl;
	struct scatterlist *cur_dest_sgl;

	local_irq_save(flags);

	cur_src_sgl = orig_sgl;
	cur_dest_sgl = bounce_sgl;

	for (i = 0; i < orig_sgl_count; i++) {
		src_addr = (unsigned long)
				kmap_atomic(sg_page(cur_src_sgl)) +
				cur_src_sgl->offset;
		src = src_addr;
		srclen = cur_src_sgl->length;

		if (bounce_addr == 0)
			bounce_addr = (unsigned long)
					kmap_atomic(sg_page(cur_dest_sgl));

		while (srclen) {
			/* assume bounce offset always == 0 */
			dest = bounce_addr + cur_dest_sgl->length;
			destlen = PAGE_SIZE - cur_dest_sgl->length;

			copylen = min(srclen, destlen);
			memcpy((void *)dest, (void *)src, copylen);

			total_copied += copylen;
			cur_dest_sgl->length += copylen;
			srclen -= copylen;
			src += copylen;

			if (cur_dest_sgl->length == PAGE_SIZE) {
				/* full..move to next entry */
				kunmap_atomic((void *)bounce_addr);
				bounce_addr = 0;
				j++;
			}

			/* if we need to use another bounce buffer */
			if (srclen && bounce_addr == 0) {
				cur_dest_sgl = sg_next(cur_dest_sgl);
				bounce_addr = (unsigned long)
						kmap_atomic(
						sg_page(cur_dest_sgl));
			}

		}

		kunmap_atomic((void *)(src_addr - cur_src_sgl->offset));
		cur_src_sgl = sg_next(cur_src_sgl);
	}

	if (bounce_addr)
		kunmap_atomic((void *)bounce_addr);

	local_irq_restore(flags);

	return total_copied;
}

static void handle_sc_creation(struct vmbus_channel *new_sc)
{
	struct hv_device *device = new_sc->primary_channel->device_obj;
	struct storvsc_device *stor_device;
	struct vmstorage_channel_properties props;

	stor_device = get_out_stor_device(device);
	if (!stor_device)
		return;

	if (stor_device->open_sub_channel == false)
		return;

	memset(&props, 0, sizeof(struct vmstorage_channel_properties));

	vmbus_open(new_sc,
		   storvsc_ringbuffer_size,
		   storvsc_ringbuffer_size,
		   (void *)&props,
		   sizeof(struct vmstorage_channel_properties),
		   storvsc_on_channel_callback, new_sc);
}

static void  handle_multichannel_storage(struct hv_device *device, int max_chns)
{
	struct storvsc_device *stor_device;
	int num_cpus = num_online_cpus();
	int num_sc;
	struct storvsc_cmd_request *request;
	struct vstor_packet *vstor_packet;
	int ret, t;

	num_sc = ((max_chns > num_cpus) ? num_cpus : max_chns);
	stor_device = get_out_stor_device(device);
	if (!stor_device)
		return;

	request = &stor_device->init_request;
	vstor_packet = &request->vstor_packet;

	stor_device->open_sub_channel = true;
	/*
	 * Establish a handler for dealing with subchannels.
	 */
	vmbus_set_sc_create_callback(device->channel, handle_sc_creation);

	/*
	 * Check to see if sub-channels have already been created. This
	 * can happen when this driver is re-loaded after unloading.
	 */

	if (vmbus_are_subchannels_present(device->channel))
		return;

	stor_device->open_sub_channel = false;
	/*
	 * Request the host to create sub-channels.
	 */
	memset(request, 0, sizeof(struct storvsc_cmd_request));
	init_completion(&request->wait_event);
	vstor_packet->operation = VSTOR_OPERATION_CREATE_SUB_CHANNELS;
	vstor_packet->flags = REQUEST_COMPLETION_FLAG;
	vstor_packet->sub_channel_count = num_sc;

	ret = vmbus_sendpacket(device->channel, vstor_packet,
			       (sizeof(struct vstor_packet) -
			       vmscsi_size_delta),
			       (unsigned long)request,
			       VM_PKT_DATA_INBAND,
			       VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);

	if (ret != 0)
		return;

	t = wait_for_completion_timeout(&request->wait_event, 10*HZ);
	if (t == 0)
		return;

	if (vstor_packet->operation != VSTOR_OPERATION_COMPLETE_IO ||
	    vstor_packet->status != 0)
		return;

	/*
	 * Now that we created the sub-channels, invoke the check; this
	 * may trigger the callback.
	 */
	stor_device->open_sub_channel = true;
	vmbus_are_subchannels_present(device->channel);
}

static int storvsc_channel_init(struct hv_device *device)
{
	struct storvsc_device *stor_device;
	struct storvsc_cmd_request *request;
	struct vstor_packet *vstor_packet;
	int ret, t;
	int max_chns;
	bool process_sub_channels = false;

	stor_device = get_out_stor_device(device);
	if (!stor_device)
		return -ENODEV;

	request = &stor_device->init_request;
	vstor_packet = &request->vstor_packet;

	/*
	 * Now, initiate the vsc/vsp initialization protocol on the open
	 * channel
	 */
	memset(request, 0, sizeof(struct storvsc_cmd_request));
	init_completion(&request->wait_event);
	vstor_packet->operation = VSTOR_OPERATION_BEGIN_INITIALIZATION;
	vstor_packet->flags = REQUEST_COMPLETION_FLAG;

	ret = vmbus_sendpacket(device->channel, vstor_packet,
			       (sizeof(struct vstor_packet) -
			       vmscsi_size_delta),
			       (unsigned long)request,
			       VM_PKT_DATA_INBAND,
			       VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);
	if (ret != 0)
		goto cleanup;

	t = wait_for_completion_timeout(&request->wait_event, 5*HZ);
	if (t == 0) {
		ret = -ETIMEDOUT;
		goto cleanup;
	}

	if (vstor_packet->operation != VSTOR_OPERATION_COMPLETE_IO ||
	    vstor_packet->status != 0)
		goto cleanup;


	/* reuse the packet for version range supported */
	memset(vstor_packet, 0, sizeof(struct vstor_packet));
	vstor_packet->operation = VSTOR_OPERATION_QUERY_PROTOCOL_VERSION;
	vstor_packet->flags = REQUEST_COMPLETION_FLAG;

	vstor_packet->version.major_minor =
		storvsc_get_version(vmstor_current_major, vmstor_current_minor);

	/*
	 * The revision number is only used in Windows; set it to 0.
	 */
	vstor_packet->version.revision = 0;

	ret = vmbus_sendpacket(device->channel, vstor_packet,
			       (sizeof(struct vstor_packet) -
				vmscsi_size_delta),
			       (unsigned long)request,
			       VM_PKT_DATA_INBAND,
			       VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);
	if (ret != 0)
		goto cleanup;

	t = wait_for_completion_timeout(&request->wait_event, 5*HZ);
	if (t == 0) {
		ret = -ETIMEDOUT;
		goto cleanup;
	}

	if (vstor_packet->operation != VSTOR_OPERATION_COMPLETE_IO ||
	    vstor_packet->status != 0)
		goto cleanup;


	memset(vstor_packet, 0, sizeof(struct vstor_packet));
	vstor_packet->operation = VSTOR_OPERATION_QUERY_PROPERTIES;
	vstor_packet->flags = REQUEST_COMPLETION_FLAG;

	ret = vmbus_sendpacket(device->channel, vstor_packet,
			       (sizeof(struct vstor_packet) -
				vmscsi_size_delta),
			       (unsigned long)request,
			       VM_PKT_DATA_INBAND,
			       VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);

	if (ret != 0)
		goto cleanup;

	t = wait_for_completion_timeout(&request->wait_event, 5*HZ);
	if (t == 0) {
		ret = -ETIMEDOUT;
		goto cleanup;
	}

	if (vstor_packet->operation != VSTOR_OPERATION_COMPLETE_IO ||
	    vstor_packet->status != 0)
		goto cleanup;

	/*
	 * Check to see if multi-channel support is there.
	 * Hosts that implement protocol version of 5.1 and above
	 * support multi-channel.
	 */
	max_chns = vstor_packet->storage_channel_properties.max_channel_cnt;
	if ((vmbus_proto_version != VERSION_WIN7) &&
	   (vmbus_proto_version != VERSION_WS2008))  {
		if (vstor_packet->storage_channel_properties.flags &
		    STORAGE_CHANNEL_SUPPORTS_MULTI_CHANNEL)
			process_sub_channels = true;
	}
	stor_device->max_transfer_bytes =
		vstor_packet->storage_channel_properties.max_transfer_bytes;

	memset(vstor_packet, 0, sizeof(struct vstor_packet));
	vstor_packet->operation = VSTOR_OPERATION_END_INITIALIZATION;
	vstor_packet->flags = REQUEST_COMPLETION_FLAG;

	ret = vmbus_sendpacket(device->channel, vstor_packet,
			       (sizeof(struct vstor_packet) -
				vmscsi_size_delta),
			       (unsigned long)request,
			       VM_PKT_DATA_INBAND,
			       VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);

	if (ret != 0)
		goto cleanup;

	t = wait_for_completion_timeout(&request->wait_event, 5*HZ);
	if (t == 0) {
		ret = -ETIMEDOUT;
		goto cleanup;
	}

	if (vstor_packet->operation != VSTOR_OPERATION_COMPLETE_IO ||
	    vstor_packet->status != 0)
		goto cleanup;

	if (process_sub_channels)
		handle_multichannel_storage(device, max_chns);


cleanup:
	return ret;
}

static void storvsc_handle_error(struct vmscsi_request *vm_srb,
				struct scsi_cmnd *scmnd,
				struct Scsi_Host *host,
				u8 asc, u8 ascq)
{
	struct storvsc_scan_work *wrk;
	void (*process_err_fn)(struct work_struct *work);
	bool do_work = false;

	switch (vm_srb->srb_status) {
	case SRB_STATUS_ERROR:
		/*
		 * Let upper layer deal with error when
		 * sense message is present.
		 */

		if (vm_srb->srb_status & SRB_STATUS_AUTOSENSE_VALID)
			break;
		/*
		 * If there is an error; offline the device since all
		 * error recovery strategies would have already been
		 * deployed on the host side. However, if the command
		 * were a pass-through command deal with it appropriately.
		 */
		switch (scmnd->cmnd[0]) {
		case ATA_16:
		case ATA_12:
			set_host_byte(scmnd, DID_PASSTHROUGH);
			break;
		/*
		 * On Some Windows hosts TEST_UNIT_READY command can return
		 * SRB_STATUS_ERROR, let the upper level code deal with it
		 * based on the sense information.
		 */
		case TEST_UNIT_READY:
			break;
		default:
			set_host_byte(scmnd, DID_ERROR);
		}
		break;
	case SRB_STATUS_INVALID_LUN:
		set_host_byte(scmnd, DID_NO_CONNECT);
		do_work = true;
		process_err_fn = storvsc_remove_lun;
		break;
	case (SRB_STATUS_ABORTED | SRB_STATUS_AUTOSENSE_VALID):
		if ((asc == 0x2a) && (ascq == 0x9)) {
			do_work = true;
			process_err_fn = storvsc_device_scan;
			/*
			 * Retry the I/O that trigerred this.
			 */
			set_host_byte(scmnd, DID_REQUEUE);
		}
		break;
	}

	if (!do_work)
		return;

	/*
	 * We need to schedule work to process this error; schedule it.
	 */
	wrk = kmalloc(sizeof(struct storvsc_scan_work), GFP_ATOMIC);
	if (!wrk) {
		set_host_byte(scmnd, DID_TARGET_FAILURE);
		return;
	}

	wrk->host = host;
	wrk->lun = vm_srb->lun;
	INIT_WORK(&wrk->work, process_err_fn);
	schedule_work(&wrk->work);
}


static void storvsc_command_completion(struct storvsc_cmd_request *cmd_request)
{
	struct scsi_cmnd *scmnd = cmd_request->cmd;
	struct hv_host_device *host_dev = shost_priv(scmnd->device->host);
	struct scsi_sense_hdr sense_hdr;
	struct vmscsi_request *vm_srb;
	struct Scsi_Host *host;
	struct storvsc_device *stor_dev;
	struct hv_device *dev = host_dev->dev;
	u32 payload_sz = cmd_request->payload_sz;
	void *payload = cmd_request->payload;

	stor_dev = get_in_stor_device(dev);
	host = stor_dev->host;

	vm_srb = &cmd_request->vstor_packet.vm_srb;
	if (cmd_request->bounce_sgl_count) {
		if (vm_srb->data_in == READ_TYPE)
			copy_from_bounce_buffer(scsi_sglist(scmnd),
					cmd_request->bounce_sgl,
					scsi_sg_count(scmnd),
					cmd_request->bounce_sgl_count);
		destroy_bounce_buffer(cmd_request->bounce_sgl,
					cmd_request->bounce_sgl_count);
	}

	scmnd->result = vm_srb->scsi_status;

	if (scmnd->result) {
		if (scsi_normalize_sense(scmnd->sense_buffer,
				SCSI_SENSE_BUFFERSIZE, &sense_hdr))
			scsi_print_sense_hdr(scmnd->device, "storvsc",
					     &sense_hdr);
	}

	if (vm_srb->srb_status != SRB_STATUS_SUCCESS)
		storvsc_handle_error(vm_srb, scmnd, host, sense_hdr.asc,
					 sense_hdr.ascq);

	scsi_set_resid(scmnd,
		cmd_request->payload->range.len -
		vm_srb->data_transfer_length);

	scmnd->scsi_done(scmnd);

	if (payload_sz >
		sizeof(struct vmbus_channel_packet_multipage_buffer))
		kfree(payload);
}

static void storvsc_on_io_completion(struct hv_device *device,
				  struct vstor_packet *vstor_packet,
				  struct storvsc_cmd_request *request)
{
	struct storvsc_device *stor_device;
	struct vstor_packet *stor_pkt;

	stor_device = hv_get_drvdata(device);
	stor_pkt = &request->vstor_packet;

	/*
	 * The current SCSI handling on the host side does
	 * not correctly handle:
	 * INQUIRY command with page code parameter set to 0x80
	 * MODE_SENSE command with cmd[2] == 0x1c
	 *
	 * Setup srb and scsi status so this won't be fatal.
	 * We do this so we can distinguish truly fatal failues
	 * (srb status == 0x4) and off-line the device in that case.
	 */

	if ((stor_pkt->vm_srb.cdb[0] == INQUIRY) ||
	   (stor_pkt->vm_srb.cdb[0] == MODE_SENSE)) {
		vstor_packet->vm_srb.scsi_status = 0;
		vstor_packet->vm_srb.srb_status = SRB_STATUS_SUCCESS;
	}


	/* Copy over the status...etc */
	stor_pkt->vm_srb.scsi_status = vstor_packet->vm_srb.scsi_status;
	stor_pkt->vm_srb.srb_status = vstor_packet->vm_srb.srb_status;
	stor_pkt->vm_srb.sense_info_length =
	vstor_packet->vm_srb.sense_info_length;


	if ((vstor_packet->vm_srb.scsi_status & 0xFF) == 0x02) {
		/* CHECK_CONDITION */
		if (vstor_packet->vm_srb.srb_status &
			SRB_STATUS_AUTOSENSE_VALID) {
			/* autosense data available */

			memcpy(request->cmd->sense_buffer,
			       vstor_packet->vm_srb.sense_data,
			       vstor_packet->vm_srb.sense_info_length);

		}
	}

	stor_pkt->vm_srb.data_transfer_length =
	vstor_packet->vm_srb.data_transfer_length;

	storvsc_command_completion(request);

	if (atomic_dec_and_test(&stor_device->num_outstanding_req) &&
		stor_device->drain_notify)
		wake_up(&stor_device->waiting_to_drain);


}

static void storvsc_on_receive(struct hv_device *device,
			     struct vstor_packet *vstor_packet,
			     struct storvsc_cmd_request *request)
{
	struct storvsc_scan_work *work;
	struct storvsc_device *stor_device;

	switch (vstor_packet->operation) {
	case VSTOR_OPERATION_COMPLETE_IO:
		storvsc_on_io_completion(device, vstor_packet, request);
		break;

	case VSTOR_OPERATION_REMOVE_DEVICE:
	case VSTOR_OPERATION_ENUMERATE_BUS:
		stor_device = get_in_stor_device(device);
		work = kmalloc(sizeof(struct storvsc_scan_work), GFP_ATOMIC);
		if (!work)
			return;

		INIT_WORK(&work->work, storvsc_host_scan);
		work->host = stor_device->host;
		schedule_work(&work->work);
		break;

	default:
		break;
	}
}

static void storvsc_on_channel_callback(void *context)
{
	struct vmbus_channel *channel = (struct vmbus_channel *)context;
	struct hv_device *device;
	struct storvsc_device *stor_device;
	u32 bytes_recvd;
	u64 request_id;
	unsigned char packet[ALIGN(sizeof(struct vstor_packet), 8)];
	struct storvsc_cmd_request *request;
	int ret;

	if (channel->primary_channel != NULL)
		device = channel->primary_channel->device_obj;
	else
		device = channel->device_obj;

	stor_device = get_in_stor_device(device);
	if (!stor_device)
		return;

	do {
		ret = vmbus_recvpacket(channel, packet,
				       ALIGN((sizeof(struct vstor_packet) -
					     vmscsi_size_delta), 8),
				       &bytes_recvd, &request_id);
		if (ret == 0 && bytes_recvd > 0) {

			request = (struct storvsc_cmd_request *)
					(unsigned long)request_id;

			if ((request == &stor_device->init_request) ||
			    (request == &stor_device->reset_request)) {

				memcpy(&request->vstor_packet, packet,
				       (sizeof(struct vstor_packet) -
					vmscsi_size_delta));
				complete(&request->wait_event);
			} else {
				storvsc_on_receive(device,
						(struct vstor_packet *)packet,
						request);
			}
		} else {
			break;
		}
	} while (1);

	return;
}

static int storvsc_connect_to_vsp(struct hv_device *device, u32 ring_size)
{
	struct vmstorage_channel_properties props;
	int ret;

	memset(&props, 0, sizeof(struct vmstorage_channel_properties));

	ret = vmbus_open(device->channel,
			 ring_size,
			 ring_size,
			 (void *)&props,
			 sizeof(struct vmstorage_channel_properties),
			 storvsc_on_channel_callback, device->channel);

	if (ret != 0)
		return ret;

	ret = storvsc_channel_init(device);

	return ret;
}

static int storvsc_dev_remove(struct hv_device *device)
{
	struct storvsc_device *stor_device;
	unsigned long flags;

	stor_device = hv_get_drvdata(device);

	spin_lock_irqsave(&device->channel->inbound_lock, flags);
	stor_device->destroy = true;
	spin_unlock_irqrestore(&device->channel->inbound_lock, flags);

	/*
	 * At this point, all outbound traffic should be disable. We
	 * only allow inbound traffic (responses) to proceed so that
	 * outstanding requests can be completed.
	 */

	storvsc_wait_to_drain(stor_device);

	/*
	 * Since we have already drained, we don't need to busy wait
	 * as was done in final_release_stor_device()
	 * Note that we cannot set the ext pointer to NULL until
	 * we have drained - to drain the outgoing packets, we need to
	 * allow incoming packets.
	 */
	spin_lock_irqsave(&device->channel->inbound_lock, flags);
	hv_set_drvdata(device, NULL);
	spin_unlock_irqrestore(&device->channel->inbound_lock, flags);

	/* Close the channel */
	vmbus_close(device->channel);

	kfree(stor_device);
	return 0;
}

static int storvsc_do_io(struct hv_device *device,
			 struct storvsc_cmd_request *request)
{
	struct storvsc_device *stor_device;
	struct vstor_packet *vstor_packet;
	struct vmbus_channel *outgoing_channel;
	int ret = 0;

	vstor_packet = &request->vstor_packet;
	stor_device = get_out_stor_device(device);

	if (!stor_device)
		return -ENODEV;


	request->device  = device;
	/*
	 * Select an an appropriate channel to send the request out.
	 */

	outgoing_channel = vmbus_get_outgoing_channel(device->channel);


	vstor_packet->flags |= REQUEST_COMPLETION_FLAG;

	vstor_packet->vm_srb.length = (sizeof(struct vmscsi_request) -
					vmscsi_size_delta);


	vstor_packet->vm_srb.sense_info_length = sense_buffer_size;


	vstor_packet->vm_srb.data_transfer_length =
	request->payload->range.len;

	vstor_packet->operation = VSTOR_OPERATION_EXECUTE_SRB;

	if (request->payload->range.len) {

		ret = vmbus_sendpacket_mpb_desc(outgoing_channel,
				request->payload, request->payload_sz,
				vstor_packet,
				(sizeof(struct vstor_packet) -
				vmscsi_size_delta),
				(unsigned long)request);
	} else {
		ret = vmbus_sendpacket(outgoing_channel, vstor_packet,
			       (sizeof(struct vstor_packet) -
				vmscsi_size_delta),
			       (unsigned long)request,
			       VM_PKT_DATA_INBAND,
			       VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);
	}

	if (ret != 0)
		return ret;

	atomic_inc(&stor_device->num_outstanding_req);

	return ret;
}

static int storvsc_device_alloc(struct scsi_device *sdevice)
{
	/*
	 * Set blist flag to permit the reading of the VPD pages even when
	 * the target may claim SPC-2 compliance. MSFT targets currently
	 * claim SPC-2 compliance while they implement post SPC-2 features.
	 * With this flag we can correctly handle WRITE_SAME_16 issues.
	 *
	 * Hypervisor reports SCSI_UNKNOWN type for DVD ROM device but
	 * still supports REPORT LUN.
	 */
	sdevice->sdev_bflags = BLIST_REPORTLUN2 | BLIST_TRY_VPD_PAGES;

	return 0;
}

static int storvsc_device_configure(struct scsi_device *sdevice)
{

	blk_queue_max_segment_size(sdevice->request_queue, PAGE_SIZE);

	blk_queue_bounce_limit(sdevice->request_queue, BLK_BOUNCE_ANY);

	blk_queue_rq_timeout(sdevice->request_queue, (storvsc_timeout * HZ));

	sdevice->no_write_same = 1;

	/*
	 * If the host is WIN8 or WIN8 R2, claim conformance to SPC-3
	 * if the device is a MSFT virtual device.
	 */
	if (!strncmp(sdevice->vendor, "Msft", 4)) {
		switch (vmbus_proto_version) {
		case VERSION_WIN8:
		case VERSION_WIN8_1:
			sdevice->scsi_level = SCSI_SPC_3;
			break;
		}
	}

	return 0;
}

static int storvsc_get_chs(struct scsi_device *sdev, struct block_device * bdev,
			   sector_t capacity, int *info)
{
	sector_t nsect = capacity;
	sector_t cylinders = nsect;
	int heads, sectors_pt;

	/*
	 * We are making up these values; let us keep it simple.
	 */
	heads = 0xff;
	sectors_pt = 0x3f;      /* Sectors per track */
	sector_div(cylinders, heads * sectors_pt);
	if ((sector_t)(cylinders + 1) * heads * sectors_pt < nsect)
		cylinders = 0xffff;

	info[0] = heads;
	info[1] = sectors_pt;
	info[2] = (int)cylinders;

	return 0;
}

static int storvsc_host_reset_handler(struct scsi_cmnd *scmnd)
{
	struct hv_host_device *host_dev = shost_priv(scmnd->device->host);
	struct hv_device *device = host_dev->dev;

	struct storvsc_device *stor_device;
	struct storvsc_cmd_request *request;
	struct vstor_packet *vstor_packet;
	int ret, t;


	stor_device = get_out_stor_device(device);
	if (!stor_device)
		return FAILED;

	request = &stor_device->reset_request;
	vstor_packet = &request->vstor_packet;

	init_completion(&request->wait_event);

	vstor_packet->operation = VSTOR_OPERATION_RESET_BUS;
	vstor_packet->flags = REQUEST_COMPLETION_FLAG;
	vstor_packet->vm_srb.path_id = stor_device->path_id;

	ret = vmbus_sendpacket(device->channel, vstor_packet,
			       (sizeof(struct vstor_packet) -
				vmscsi_size_delta),
			       (unsigned long)&stor_device->reset_request,
			       VM_PKT_DATA_INBAND,
			       VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);
	if (ret != 0)
		return FAILED;

	t = wait_for_completion_timeout(&request->wait_event, 5*HZ);
	if (t == 0)
		return TIMEOUT_ERROR;


	/*
	 * At this point, all outstanding requests in the adapter
	 * should have been flushed out and return to us
	 * There is a potential race here where the host may be in
	 * the process of responding when we return from here.
	 * Just wait for all in-transit packets to be accounted for
	 * before we return from here.
	 */
	storvsc_wait_to_drain(stor_device);

	return SUCCESS;
}

/*
 * The host guarantees to respond to each command, although I/O latencies might
 * be unbounded on Azure.  Reset the timer unconditionally to give the host a
 * chance to perform EH.
 */
static enum blk_eh_timer_return storvsc_eh_timed_out(struct scsi_cmnd *scmnd)
{
	return BLK_EH_RESET_TIMER;
}

static bool storvsc_scsi_cmd_ok(struct scsi_cmnd *scmnd)
{
	bool allowed = true;
	u8 scsi_op = scmnd->cmnd[0];

	switch (scsi_op) {
	/* the host does not handle WRITE_SAME, log accident usage */
	case WRITE_SAME:
	/*
	 * smartd sends this command and the host does not handle
	 * this. So, don't send it.
	 */
	case SET_WINDOW:
		scmnd->result = ILLEGAL_REQUEST << 16;
		allowed = false;
		break;
	default:
		break;
	}
	return allowed;
}

static int storvsc_queuecommand(struct Scsi_Host *host, struct scsi_cmnd *scmnd)
{
	int ret;
	struct hv_host_device *host_dev = shost_priv(host);
	struct hv_device *dev = host_dev->dev;
	struct storvsc_cmd_request *cmd_request = scsi_cmd_priv(scmnd);
	int i;
	struct scatterlist *sgl;
	unsigned int sg_count = 0;
	struct vmscsi_request *vm_srb;
	struct scatterlist *cur_sgl;
	struct vmbus_packet_mpb_array  *payload;
	u32 payload_sz;
	u32 length;

	if (vmstor_current_major <= VMSTOR_WIN8_MAJOR) {
		/*
		 * On legacy hosts filter unimplemented commands.
		 * Future hosts are expected to correctly handle
		 * unsupported commands. Furthermore, it is
		 * possible that some of the currently
		 * unsupported commands maybe supported in
		 * future versions of the host.
		 */
		if (!storvsc_scsi_cmd_ok(scmnd)) {
			scmnd->scsi_done(scmnd);
			return 0;
		}
	}

	/* Setup the cmd request */
	cmd_request->cmd = scmnd;

	vm_srb = &cmd_request->vstor_packet.vm_srb;
	vm_srb->win8_extension.time_out_value = 60;

	vm_srb->win8_extension.srb_flags |=
		(SRB_FLAGS_QUEUE_ACTION_ENABLE |
		SRB_FLAGS_DISABLE_SYNCH_TRANSFER);

	if (scmnd->device->tagged_supported) {
		vm_srb->win8_extension.srb_flags |=
		(SRB_FLAGS_QUEUE_ACTION_ENABLE | SRB_FLAGS_NO_QUEUE_FREEZE);
		vm_srb->win8_extension.queue_tag = SP_UNTAGGED;
		vm_srb->win8_extension.queue_action = SRB_SIMPLE_TAG_REQUEST;
	}

	/* Build the SRB */
	switch (scmnd->sc_data_direction) {
	case DMA_TO_DEVICE:
		vm_srb->data_in = WRITE_TYPE;
		vm_srb->win8_extension.srb_flags |= SRB_FLAGS_DATA_OUT;
		break;
	case DMA_FROM_DEVICE:
		vm_srb->data_in = READ_TYPE;
		vm_srb->win8_extension.srb_flags |= SRB_FLAGS_DATA_IN;
		break;
	default:
		vm_srb->data_in = UNKNOWN_TYPE;
		vm_srb->win8_extension.srb_flags |= SRB_FLAGS_NO_DATA_TRANSFER;
		break;
	}


	vm_srb->port_number = host_dev->port;
	vm_srb->path_id = scmnd->device->channel;
	vm_srb->target_id = scmnd->device->id;
	vm_srb->lun = scmnd->device->lun;

	vm_srb->cdb_length = scmnd->cmd_len;

	memcpy(vm_srb->cdb, scmnd->cmnd, vm_srb->cdb_length);

	sgl = (struct scatterlist *)scsi_sglist(scmnd);
	sg_count = scsi_sg_count(scmnd);

	length = scsi_bufflen(scmnd);
	payload = (struct vmbus_packet_mpb_array *)&cmd_request->mpb;
	payload_sz = sizeof(cmd_request->mpb);

	if (sg_count) {
		/* check if we need to bounce the sgl */
		if (do_bounce_buffer(sgl, scsi_sg_count(scmnd)) != -1) {
			cmd_request->bounce_sgl =
				create_bounce_buffer(sgl, sg_count,
						     length,
						     vm_srb->data_in);
			if (!cmd_request->bounce_sgl)
				return SCSI_MLQUEUE_HOST_BUSY;

			cmd_request->bounce_sgl_count =
				ALIGN(length, PAGE_SIZE) >> PAGE_SHIFT;

			if (vm_srb->data_in == WRITE_TYPE)
				copy_to_bounce_buffer(sgl,
					cmd_request->bounce_sgl, sg_count);

			sgl = cmd_request->bounce_sgl;
			sg_count = cmd_request->bounce_sgl_count;
		}


		if (sg_count > MAX_PAGE_BUFFER_COUNT) {

			payload_sz = (sg_count * sizeof(void *) +
				      sizeof(struct vmbus_packet_mpb_array));
			payload = kmalloc(payload_sz, GFP_ATOMIC);
			if (!payload) {
				if (cmd_request->bounce_sgl_count)
					destroy_bounce_buffer(
					cmd_request->bounce_sgl,
					cmd_request->bounce_sgl_count);

					return SCSI_MLQUEUE_DEVICE_BUSY;
			}
		}

		payload->range.len = length;
		payload->range.offset = sgl[0].offset;

		cur_sgl = sgl;
		for (i = 0; i < sg_count; i++) {
			payload->range.pfn_array[i] =
				page_to_pfn(sg_page((cur_sgl)));
			cur_sgl = sg_next(cur_sgl);
		}

	} else if (scsi_sglist(scmnd)) {
		payload->range.len = length;
		payload->range.offset =
			virt_to_phys(scsi_sglist(scmnd)) & (PAGE_SIZE-1);
		payload->range.pfn_array[0] =
			virt_to_phys(scsi_sglist(scmnd)) >> PAGE_SHIFT;
	}

	cmd_request->payload = payload;
	cmd_request->payload_sz = payload_sz;

	/* Invokes the vsc to start an IO */
	ret = storvsc_do_io(dev, cmd_request);

	if (ret == -EAGAIN) {
		if (payload_sz > sizeof(cmd_request->mpb))
			kfree(payload);
		/* no more space */

		if (cmd_request->bounce_sgl_count)
			destroy_bounce_buffer(cmd_request->bounce_sgl,
					cmd_request->bounce_sgl_count);

		return SCSI_MLQUEUE_DEVICE_BUSY;
	}

	return 0;
}

static struct scsi_host_template scsi_driver = {
	.module	=		THIS_MODULE,
	.name =			"storvsc_host_t",
	.cmd_size =             sizeof(struct storvsc_cmd_request),
	.bios_param =		storvsc_get_chs,
	.queuecommand =		storvsc_queuecommand,
	.eh_host_reset_handler =	storvsc_host_reset_handler,
	.proc_name =		"storvsc_host",
	.eh_timed_out =		storvsc_eh_timed_out,
	.slave_alloc =		storvsc_device_alloc,
	.slave_configure =	storvsc_device_configure,
	.cmd_per_lun =		255,
	.this_id =		-1,
	.use_clustering =	ENABLE_CLUSTERING,
	/* Make sure we dont get a sg segment crosses a page boundary */
	.dma_boundary =		PAGE_SIZE-1,
	.no_write_same =	1,
};

enum {
	SCSI_GUID,
	IDE_GUID,
	SFC_GUID,
};

static const struct hv_vmbus_device_id id_table[] = {
	/* SCSI guid */
	{ HV_SCSI_GUID,
	  .driver_data = SCSI_GUID
	},
	/* IDE guid */
	{ HV_IDE_GUID,
	  .driver_data = IDE_GUID
	},
	/* Fibre Channel GUID */
	{
	  HV_SYNTHFC_GUID,
	  .driver_data = SFC_GUID
	},
	{ },
};

MODULE_DEVICE_TABLE(vmbus, id_table);

static int storvsc_probe(struct hv_device *device,
			const struct hv_vmbus_device_id *dev_id)
{
	int ret;
	int num_cpus = num_online_cpus();
	struct Scsi_Host *host;
	struct hv_host_device *host_dev;
	bool dev_is_ide = ((dev_id->driver_data == IDE_GUID) ? true : false);
	int target = 0;
	struct storvsc_device *stor_device;
	int max_luns_per_target;
	int max_targets;
	int max_channels;
	int max_sub_channels = 0;

	/*
	 * Based on the windows host we are running on,
	 * set state to properly communicate with the host.
	 */

	switch (vmbus_proto_version) {
	case VERSION_WS2008:
	case VERSION_WIN7:
		sense_buffer_size = PRE_WIN8_STORVSC_SENSE_BUFFER_SIZE;
		vmscsi_size_delta = sizeof(struct vmscsi_win8_extension);
		vmstor_current_major = VMSTOR_WIN7_MAJOR;
		vmstor_current_minor = VMSTOR_WIN7_MINOR;
		max_luns_per_target = STORVSC_IDE_MAX_LUNS_PER_TARGET;
		max_targets = STORVSC_IDE_MAX_TARGETS;
		max_channels = STORVSC_IDE_MAX_CHANNELS;
		break;
	default:
		sense_buffer_size = POST_WIN7_STORVSC_SENSE_BUFFER_SIZE;
		vmscsi_size_delta = 0;
		vmstor_current_major = VMSTOR_WIN8_MAJOR;
		vmstor_current_minor = VMSTOR_WIN8_MINOR;
		max_luns_per_target = STORVSC_MAX_LUNS_PER_TARGET;
		max_targets = STORVSC_MAX_TARGETS;
		max_channels = STORVSC_MAX_CHANNELS;
		/*
		 * On Windows8 and above, we support sub-channels for storage.
		 * The number of sub-channels offerred is based on the number of
		 * VCPUs in the guest.
		 */
		max_sub_channels = (num_cpus / storvsc_vcpus_per_sub_channel);
		break;
	}

	scsi_driver.can_queue = (max_outstanding_req_per_channel *
				 (max_sub_channels + 1));

	host = scsi_host_alloc(&scsi_driver,
			       sizeof(struct hv_host_device));
	if (!host)
		return -ENOMEM;

	host_dev = shost_priv(host);
	memset(host_dev, 0, sizeof(struct hv_host_device));

	host_dev->port = host->host_no;
	host_dev->dev = device;


	stor_device = kzalloc(sizeof(struct storvsc_device), GFP_KERNEL);
	if (!stor_device) {
		ret = -ENOMEM;
		goto err_out0;
	}

	stor_device->destroy = false;
	stor_device->open_sub_channel = false;
	init_waitqueue_head(&stor_device->waiting_to_drain);
	stor_device->device = device;
	stor_device->host = host;
	hv_set_drvdata(device, stor_device);

	stor_device->port_number = host->host_no;
	ret = storvsc_connect_to_vsp(device, storvsc_ringbuffer_size);
	if (ret)
		goto err_out1;

	host_dev->path = stor_device->path_id;
	host_dev->target = stor_device->target_id;

	switch (dev_id->driver_data) {
	case SFC_GUID:
		host->max_lun = STORVSC_FC_MAX_LUNS_PER_TARGET;
		host->max_id = STORVSC_FC_MAX_TARGETS;
		host->max_channel = STORVSC_FC_MAX_CHANNELS - 1;
		break;

	case SCSI_GUID:
		host->max_lun = max_luns_per_target;
		host->max_id = max_targets;
		host->max_channel = max_channels - 1;
		break;

	default:
		host->max_lun = STORVSC_IDE_MAX_LUNS_PER_TARGET;
		host->max_id = STORVSC_IDE_MAX_TARGETS;
		host->max_channel = STORVSC_IDE_MAX_CHANNELS - 1;
		break;
	}
	/* max cmd length */
	host->max_cmd_len = STORVSC_MAX_CMD_LEN;

	/*
	 * set the table size based on the info we got
	 * from the host.
	 */
	host->sg_tablesize = (stor_device->max_transfer_bytes >> PAGE_SHIFT);

	/* Register the HBA and start the scsi bus scan */
	ret = scsi_add_host(host, &device->device);
	if (ret != 0)
		goto err_out2;

	if (!dev_is_ide) {
		scsi_scan_host(host);
	} else {
		target = (device->dev_instance.b[5] << 8 |
			 device->dev_instance.b[4]);
		ret = scsi_add_device(host, 0, target, 0);
		if (ret) {
			scsi_remove_host(host);
			goto err_out2;
		}
	}
	return 0;

err_out2:
	/*
	 * Once we have connected with the host, we would need to
	 * to invoke storvsc_dev_remove() to rollback this state and
	 * this call also frees up the stor_device; hence the jump around
	 * err_out1 label.
	 */
	storvsc_dev_remove(device);
	goto err_out0;

err_out1:
	kfree(stor_device);

err_out0:
	scsi_host_put(host);
	return ret;
}

static int storvsc_remove(struct hv_device *dev)
{
	struct storvsc_device *stor_device = hv_get_drvdata(dev);
	struct Scsi_Host *host = stor_device->host;

	scsi_remove_host(host);
	storvsc_dev_remove(dev);
	scsi_host_put(host);

	return 0;
}

static struct hv_driver storvsc_drv = {
	.name = KBUILD_MODNAME,
	.id_table = id_table,
	.probe = storvsc_probe,
	.remove = storvsc_remove,
};

static int __init storvsc_drv_init(void)
{

	/*
	 * Divide the ring buffer data size (which is 1 page less
	 * than the ring buffer size since that page is reserved for
	 * the ring buffer indices) by the max request size (which is
	 * vmbus_channel_packet_multipage_buffer + struct vstor_packet + u64)
	 */
	max_outstanding_req_per_channel =
		((storvsc_ringbuffer_size - PAGE_SIZE) /
		ALIGN(MAX_MULTIPAGE_BUFFER_PACKET +
		sizeof(struct vstor_packet) + sizeof(u64) -
		vmscsi_size_delta,
		sizeof(u64)));

	return vmbus_driver_register(&storvsc_drv);
}

static void __exit storvsc_drv_exit(void)
{
	vmbus_driver_unregister(&storvsc_drv);
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Microsoft Hyper-V virtual storage driver");
module_init(storvsc_drv_init);
module_exit(storvsc_drv_exit);
