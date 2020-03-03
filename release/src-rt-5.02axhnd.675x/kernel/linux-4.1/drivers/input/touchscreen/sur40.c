/*
 * Surface2.0/SUR40/PixelSense input driver
 *
 * Copyright (c) 2014 by Florian 'floe' Echtler <floe@butterbrot.org>
 *
 * Derived from the USB Skeleton driver 1.1,
 * Copyright (c) 2003 Greg Kroah-Hartman (greg@kroah.com)
 *
 * and from the Apple USB BCM5974 multitouch driver,
 * Copyright (c) 2008 Henrik Rydberg (rydberg@euromail.se)
 *
 * and from the generic hid-multitouch driver,
 * Copyright (c) 2010-2012 Stephane Chatty <chatty@enac.fr>
 *
 * and from the v4l2-pci-skeleton driver,
 * Copyright (c) Copyright 2014 Cisco Systems, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/completion.h>
#include <linux/uaccess.h>
#include <linux/usb.h>
#include <linux/printk.h>
#include <linux/input-polldev.h>
#include <linux/input/mt.h>
#include <linux/usb/input.h>
#include <linux/videodev2.h>
#include <media/v4l2-device.h>
#include <media/v4l2-dev.h>
#include <media/v4l2-ioctl.h>
#include <media/videobuf2-dma-sg.h>

/* read 512 bytes from endpoint 0x86 -> get header + blobs */
struct sur40_header {

	__le16 type;       /* always 0x0001 */
	__le16 count;      /* count of blobs (if 0: continue prev. packet) */

	__le32 packet_id;  /* unique ID for all packets in one frame */

	__le32 timestamp;  /* milliseconds (inc. by 16 or 17 each frame) */
	__le32 unknown;    /* "epoch?" always 02/03 00 00 00 */

} __packed;

struct sur40_blob {

	__le16 blob_id;

	u8 action;         /* 0x02 = enter/exit, 0x03 = update (?) */
	u8 unknown;        /* always 0x01 or 0x02 (no idea what this is?) */

	__le16 bb_pos_x;   /* upper left corner of bounding box */
	__le16 bb_pos_y;

	__le16 bb_size_x;  /* size of bounding box */
	__le16 bb_size_y;

	__le16 pos_x;      /* finger tip position */
	__le16 pos_y;

	__le16 ctr_x;      /* centroid position */
	__le16 ctr_y;

	__le16 axis_x;     /* somehow related to major/minor axis, mostly: */
	__le16 axis_y;     /* axis_x == bb_size_y && axis_y == bb_size_x */

	__le32 angle;      /* orientation in radians relative to x axis -
	                      actually an IEEE754 float, don't use in kernel */

	__le32 area;       /* size in pixels/pressure (?) */

	u8 padding[32];

} __packed;

/* combined header/blob data */
struct sur40_data {
	struct sur40_header header;
	struct sur40_blob   blobs[];
} __packed;

/* read 512 bytes from endpoint 0x82 -> get header below
 * continue reading 16k blocks until header.size bytes read */
struct sur40_image_header {
	__le32 magic;     /* "SUBF" */
	__le32 packet_id;
	__le32 size;      /* always 0x0007e900 = 960x540 */
	__le32 timestamp; /* milliseconds (increases by 16 or 17 each frame) */
	__le32 unknown;   /* "epoch?" always 02/03 00 00 00 */
} __packed;

/* version information */
#define DRIVER_SHORT   "sur40"
#define DRIVER_LONG    "Samsung SUR40"
#define DRIVER_AUTHOR  "Florian 'floe' Echtler <floe@butterbrot.org>"
#define DRIVER_DESC    "Surface2.0/SUR40/PixelSense input driver"

/* vendor and device IDs */
#define ID_MICROSOFT 0x045e
#define ID_SUR40     0x0775

/* sensor resolution */
#define SENSOR_RES_X 1920
#define SENSOR_RES_Y 1080

/* touch data endpoint */
#define TOUCH_ENDPOINT 0x86

/* video data endpoint */
#define VIDEO_ENDPOINT 0x82

/* video header fields */
#define VIDEO_HEADER_MAGIC 0x46425553
#define VIDEO_PACKET_SIZE  16384

/* polling interval (ms) */
#define POLL_INTERVAL 10

/* maximum number of contacts FIXME: this is a guess? */
#define MAX_CONTACTS 64

/* control commands */
#define SUR40_GET_VERSION 0xb0 /* 12 bytes string    */
#define SUR40_UNKNOWN1    0xb3 /*  5 bytes           */
#define SUR40_UNKNOWN2    0xc1 /* 24 bytes           */

#define SUR40_GET_STATE   0xc5 /*  4 bytes state (?) */
#define SUR40_GET_SENSORS 0xb1 /*  8 bytes sensors   */

/* master device state */
struct sur40_state {

	struct usb_device *usbdev;
	struct device *dev;
	struct input_polled_dev *input;

	struct v4l2_device v4l2;
	struct video_device vdev;
	struct mutex lock;

	struct vb2_queue queue;
	struct vb2_alloc_ctx *alloc_ctx;
	struct list_head buf_list;
	spinlock_t qlock;
	int sequence;

	struct sur40_data *bulk_in_buffer;
	size_t bulk_in_size;
	u8 bulk_in_epaddr;

	char phys[64];
};

struct sur40_buffer {
	struct vb2_buffer vb;
	struct list_head list;
};

/* forward declarations */
static const struct video_device sur40_video_device;
static const struct v4l2_pix_format sur40_video_format;
static const struct vb2_queue sur40_queue;
static void sur40_process_video(struct sur40_state *sur40);

/*
 * Note: an earlier, non-public version of this driver used USB_RECIP_ENDPOINT
 * here by mistake which is very likely to have corrupted the firmware EEPROM
 * on two separate SUR40 devices. Thanks to Alan Stern who spotted this bug.
 * Should you ever run into a similar problem, the background story to this
 * incident and instructions on how to fix the corrupted EEPROM are available
 * at https://floe.butterbrot.org/matrix/hacking/surface/brick.html
*/

/* command wrapper */
static int sur40_command(struct sur40_state *dev,
			 u8 command, u16 index, void *buffer, u16 size)
{
	return usb_control_msg(dev->usbdev, usb_rcvctrlpipe(dev->usbdev, 0),
			       command,
			       USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_DIR_IN,
			       0x00, index, buffer, size, 1000);
}

/* Initialization routine, called from sur40_open */
static int sur40_init(struct sur40_state *dev)
{
	int result;
	u8 buffer[24];

	/* stupidly replay the original MS driver init sequence */
	result = sur40_command(dev, SUR40_GET_VERSION, 0x00, buffer, 12);
	if (result < 0)
		return result;

	result = sur40_command(dev, SUR40_GET_VERSION, 0x01, buffer, 12);
	if (result < 0)
		return result;

	result = sur40_command(dev, SUR40_GET_VERSION, 0x02, buffer, 12);
	if (result < 0)
		return result;

	result = sur40_command(dev, SUR40_UNKNOWN2,    0x00, buffer, 24);
	if (result < 0)
		return result;

	result = sur40_command(dev, SUR40_UNKNOWN1,    0x00, buffer,  5);
	if (result < 0)
		return result;

	result = sur40_command(dev, SUR40_GET_VERSION, 0x03, buffer, 12);

	/*
	 * Discard the result buffer - no known data inside except
	 * some version strings, maybe extract these sometime...
	 */

	return result;
}

/*
 * Callback routines from input_polled_dev
 */

/* Enable the device, polling will now start. */
static void sur40_open(struct input_polled_dev *polldev)
{
	struct sur40_state *sur40 = polldev->private;

	dev_dbg(sur40->dev, "open\n");
	sur40_init(sur40);
}

/* Disable device, polling has stopped. */
static void sur40_close(struct input_polled_dev *polldev)
{
	struct sur40_state *sur40 = polldev->private;

	dev_dbg(sur40->dev, "close\n");
	/*
	 * There is no known way to stop the device, so we simply
	 * stop polling.
	 */
}

/*
 * This function is called when a whole contact has been processed,
 * so that it can assign it to a slot and store the data there.
 */
static void sur40_report_blob(struct sur40_blob *blob, struct input_dev *input)
{
	int wide, major, minor;

	int bb_size_x = le16_to_cpu(blob->bb_size_x);
	int bb_size_y = le16_to_cpu(blob->bb_size_y);

	int pos_x = le16_to_cpu(blob->pos_x);
	int pos_y = le16_to_cpu(blob->pos_y);

	int ctr_x = le16_to_cpu(blob->ctr_x);
	int ctr_y = le16_to_cpu(blob->ctr_y);

	int slotnum = input_mt_get_slot_by_key(input, blob->blob_id);
	if (slotnum < 0 || slotnum >= MAX_CONTACTS)
		return;

	input_mt_slot(input, slotnum);
	input_mt_report_slot_state(input, MT_TOOL_FINGER, 1);
	wide = (bb_size_x > bb_size_y);
	major = max(bb_size_x, bb_size_y);
	minor = min(bb_size_x, bb_size_y);

	input_report_abs(input, ABS_MT_POSITION_X, pos_x);
	input_report_abs(input, ABS_MT_POSITION_Y, pos_y);
	input_report_abs(input, ABS_MT_TOOL_X, ctr_x);
	input_report_abs(input, ABS_MT_TOOL_Y, ctr_y);

	/* TODO: use a better orientation measure */
	input_report_abs(input, ABS_MT_ORIENTATION, wide);
	input_report_abs(input, ABS_MT_TOUCH_MAJOR, major);
	input_report_abs(input, ABS_MT_TOUCH_MINOR, minor);
}

/* core function: poll for new input data */
static void sur40_poll(struct input_polled_dev *polldev)
{
	struct sur40_state *sur40 = polldev->private;
	struct input_dev *input = polldev->input;
	int result, bulk_read, need_blobs, packet_blobs, i;
	u32 uninitialized_var(packet_id);

	struct sur40_header *header = &sur40->bulk_in_buffer->header;
	struct sur40_blob *inblob = &sur40->bulk_in_buffer->blobs[0];

	dev_dbg(sur40->dev, "poll\n");

	need_blobs = -1;

	do {

		/* perform a blocking bulk read to get data from the device */
		result = usb_bulk_msg(sur40->usbdev,
			usb_rcvbulkpipe(sur40->usbdev, sur40->bulk_in_epaddr),
			sur40->bulk_in_buffer, sur40->bulk_in_size,
			&bulk_read, 1000);

		dev_dbg(sur40->dev, "received %d bytes\n", bulk_read);

		if (result < 0) {
			dev_err(sur40->dev, "error in usb_bulk_read\n");
			return;
		}

		result = bulk_read - sizeof(struct sur40_header);

		if (result % sizeof(struct sur40_blob) != 0) {
			dev_err(sur40->dev, "transfer size mismatch\n");
			return;
		}

		/* first packet? */
		if (need_blobs == -1) {
			need_blobs = le16_to_cpu(header->count);
			dev_dbg(sur40->dev, "need %d blobs\n", need_blobs);
			packet_id = le32_to_cpu(header->packet_id);
		}

		/*
		 * Sanity check. when video data is also being retrieved, the
		 * packet ID will usually increase in the middle of a series
		 * instead of at the end.
		 */
		if (packet_id != header->packet_id)
			dev_warn(sur40->dev, "packet ID mismatch\n");

		packet_blobs = result / sizeof(struct sur40_blob);
		dev_dbg(sur40->dev, "received %d blobs\n", packet_blobs);

		/* packets always contain at least 4 blobs, even if empty */
		if (packet_blobs > need_blobs)
			packet_blobs = need_blobs;

		for (i = 0; i < packet_blobs; i++) {
			need_blobs--;
			dev_dbg(sur40->dev, "processing blob\n");
			sur40_report_blob(&(inblob[i]), input);
		}

	} while (need_blobs > 0);

	input_mt_sync_frame(input);
	input_sync(input);

	sur40_process_video(sur40);
}

/* deal with video data */
static void sur40_process_video(struct sur40_state *sur40)
{

	struct sur40_image_header *img = (void *)(sur40->bulk_in_buffer);
	struct sur40_buffer *new_buf;
	struct usb_sg_request sgr;
	struct sg_table *sgt;
	int result, bulk_read;

	if (!vb2_start_streaming_called(&sur40->queue))
		return;

	/* get a new buffer from the list */
	spin_lock(&sur40->qlock);
	if (list_empty(&sur40->buf_list)) {
		dev_dbg(sur40->dev, "buffer queue empty\n");
		spin_unlock(&sur40->qlock);
		return;
	}
	new_buf = list_entry(sur40->buf_list.next, struct sur40_buffer, list);
	list_del(&new_buf->list);
	spin_unlock(&sur40->qlock);

	/* retrieve data via bulk read */
	result = usb_bulk_msg(sur40->usbdev,
			usb_rcvbulkpipe(sur40->usbdev, VIDEO_ENDPOINT),
			sur40->bulk_in_buffer, sur40->bulk_in_size,
			&bulk_read, 1000);

	if (result < 0) {
		dev_err(sur40->dev, "error in usb_bulk_read\n");
		goto err_poll;
	}

	if (bulk_read != sizeof(struct sur40_image_header)) {
		dev_err(sur40->dev, "received %d bytes (%zd expected)\n",
			bulk_read, sizeof(struct sur40_image_header));
		goto err_poll;
	}

	if (le32_to_cpu(img->magic) != VIDEO_HEADER_MAGIC) {
		dev_err(sur40->dev, "image magic mismatch\n");
		goto err_poll;
	}

	if (le32_to_cpu(img->size) != sur40_video_format.sizeimage) {
		dev_err(sur40->dev, "image size mismatch\n");
		goto err_poll;
	}

	sgt = vb2_dma_sg_plane_desc(&new_buf->vb, 0);

	result = usb_sg_init(&sgr, sur40->usbdev,
		usb_rcvbulkpipe(sur40->usbdev, VIDEO_ENDPOINT), 0,
		sgt->sgl, sgt->nents, sur40_video_format.sizeimage, 0);
	if (result < 0) {
		dev_err(sur40->dev, "error %d in usb_sg_init\n", result);
		goto err_poll;
	}

	usb_sg_wait(&sgr);
	if (sgr.status < 0) {
		dev_err(sur40->dev, "error %d in usb_sg_wait\n", sgr.status);
		goto err_poll;
	}

	/* mark as finished */
	v4l2_get_timestamp(&new_buf->vb.v4l2_buf.timestamp);
	new_buf->vb.v4l2_buf.sequence = sur40->sequence++;
	new_buf->vb.v4l2_buf.field = V4L2_FIELD_NONE;
	vb2_buffer_done(&new_buf->vb, VB2_BUF_STATE_DONE);
	return;

err_poll:
	vb2_buffer_done(&new_buf->vb, VB2_BUF_STATE_ERROR);
}

/* Initialize input device parameters. */
static void sur40_input_setup(struct input_dev *input_dev)
{
	__set_bit(EV_KEY, input_dev->evbit);
	__set_bit(EV_ABS, input_dev->evbit);

	input_set_abs_params(input_dev, ABS_MT_POSITION_X,
			     0, SENSOR_RES_X, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y,
			     0, SENSOR_RES_Y, 0, 0);

	input_set_abs_params(input_dev, ABS_MT_TOOL_X,
			     0, SENSOR_RES_X, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOOL_Y,
			     0, SENSOR_RES_Y, 0, 0);

	/* max value unknown, but major/minor axis
	 * can never be larger than screen */
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR,
			     0, SENSOR_RES_X, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MINOR,
			     0, SENSOR_RES_Y, 0, 0);

	input_set_abs_params(input_dev, ABS_MT_ORIENTATION, 0, 1, 0, 0);

	input_mt_init_slots(input_dev, MAX_CONTACTS,
			    INPUT_MT_DIRECT | INPUT_MT_DROP_UNUSED);
}

/* Check candidate USB interface. */
static int sur40_probe(struct usb_interface *interface,
		       const struct usb_device_id *id)
{
	struct usb_device *usbdev = interface_to_usbdev(interface);
	struct sur40_state *sur40;
	struct usb_host_interface *iface_desc;
	struct usb_endpoint_descriptor *endpoint;
	struct input_polled_dev *poll_dev;
	int error;

	/* Check if we really have the right interface. */
	iface_desc = &interface->altsetting[0];
	if (iface_desc->desc.bInterfaceClass != 0xFF)
		return -ENODEV;

	if (iface_desc->desc.bNumEndpoints < 5)
		return -ENODEV;

	/* Use endpoint #4 (0x86). */
	endpoint = &iface_desc->endpoint[4].desc;
	if (endpoint->bEndpointAddress != TOUCH_ENDPOINT)
		return -ENODEV;

	/* Allocate memory for our device state and initialize it. */
	sur40 = kzalloc(sizeof(struct sur40_state), GFP_KERNEL);
	if (!sur40)
		return -ENOMEM;

	poll_dev = input_allocate_polled_device();
	if (!poll_dev) {
		error = -ENOMEM;
		goto err_free_dev;
	}

	/* initialize locks/lists */
	INIT_LIST_HEAD(&sur40->buf_list);
	spin_lock_init(&sur40->qlock);
	mutex_init(&sur40->lock);

	/* Set up polled input device control structure */
	poll_dev->private = sur40;
	poll_dev->poll_interval = POLL_INTERVAL;
	poll_dev->open = sur40_open;
	poll_dev->poll = sur40_poll;
	poll_dev->close = sur40_close;

	/* Set up regular input device structure */
	sur40_input_setup(poll_dev->input);

	poll_dev->input->name = DRIVER_LONG;
	usb_to_input_id(usbdev, &poll_dev->input->id);
	usb_make_path(usbdev, sur40->phys, sizeof(sur40->phys));
	strlcat(sur40->phys, "/input0", sizeof(sur40->phys));
	poll_dev->input->phys = sur40->phys;
	poll_dev->input->dev.parent = &interface->dev;

	sur40->usbdev = usbdev;
	sur40->dev = &interface->dev;
	sur40->input = poll_dev;

	/* use the bulk-in endpoint tested above */
	sur40->bulk_in_size = usb_endpoint_maxp(endpoint);
	sur40->bulk_in_epaddr = endpoint->bEndpointAddress;
	sur40->bulk_in_buffer = kmalloc(sur40->bulk_in_size, GFP_KERNEL);
	if (!sur40->bulk_in_buffer) {
		dev_err(&interface->dev, "Unable to allocate input buffer.");
		error = -ENOMEM;
		goto err_free_polldev;
	}

	/* register the polled input device */
	error = input_register_polled_device(poll_dev);
	if (error) {
		dev_err(&interface->dev,
			"Unable to register polled input device.");
		goto err_free_buffer;
	}

	/* register the video master device */
	snprintf(sur40->v4l2.name, sizeof(sur40->v4l2.name), "%s", DRIVER_LONG);
	error = v4l2_device_register(sur40->dev, &sur40->v4l2);
	if (error) {
		dev_err(&interface->dev,
			"Unable to register video master device.");
		goto err_unreg_v4l2;
	}

	/* initialize the lock and subdevice */
	sur40->queue = sur40_queue;
	sur40->queue.drv_priv = sur40;
	sur40->queue.lock = &sur40->lock;

	/* initialize the queue */
	error = vb2_queue_init(&sur40->queue);
	if (error)
		goto err_unreg_v4l2;

	sur40->alloc_ctx = vb2_dma_sg_init_ctx(sur40->dev);
	if (IS_ERR(sur40->alloc_ctx)) {
		dev_err(sur40->dev, "Can't allocate buffer context");
		goto err_unreg_v4l2;
	}

	sur40->vdev = sur40_video_device;
	sur40->vdev.v4l2_dev = &sur40->v4l2;
	sur40->vdev.lock = &sur40->lock;
	sur40->vdev.queue = &sur40->queue;
	video_set_drvdata(&sur40->vdev, sur40);

	error = video_register_device(&sur40->vdev, VFL_TYPE_GRABBER, -1);
	if (error) {
		dev_err(&interface->dev,
			"Unable to register video subdevice.");
		goto err_unreg_video;
	}

	/* we can register the device now, as it is ready */
	usb_set_intfdata(interface, sur40);
	dev_dbg(&interface->dev, "%s is now attached\n", DRIVER_DESC);

	return 0;

err_unreg_video:
	video_unregister_device(&sur40->vdev);
err_unreg_v4l2:
	v4l2_device_unregister(&sur40->v4l2);
err_free_buffer:
	kfree(sur40->bulk_in_buffer);
err_free_polldev:
	input_free_polled_device(sur40->input);
err_free_dev:
	kfree(sur40);

	return error;
}

/* Unregister device & clean up. */
static void sur40_disconnect(struct usb_interface *interface)
{
	struct sur40_state *sur40 = usb_get_intfdata(interface);

	video_unregister_device(&sur40->vdev);
	v4l2_device_unregister(&sur40->v4l2);
	vb2_dma_sg_cleanup_ctx(sur40->alloc_ctx);

	input_unregister_polled_device(sur40->input);
	input_free_polled_device(sur40->input);
	kfree(sur40->bulk_in_buffer);
	kfree(sur40);

	usb_set_intfdata(interface, NULL);
	dev_dbg(&interface->dev, "%s is now disconnected\n", DRIVER_DESC);
}

/*
 * Setup the constraints of the queue: besides setting the number of planes
 * per buffer and the size and allocation context of each plane, it also
 * checks if sufficient buffers have been allocated. Usually 3 is a good
 * minimum number: many DMA engines need a minimum of 2 buffers in the
 * queue and you need to have another available for userspace processing.
 */
static int sur40_queue_setup(struct vb2_queue *q, const struct v4l2_format *fmt,
		       unsigned int *nbuffers, unsigned int *nplanes,
		       unsigned int sizes[], void *alloc_ctxs[])
{
	struct sur40_state *sur40 = vb2_get_drv_priv(q);

	if (q->num_buffers + *nbuffers < 3)
		*nbuffers = 3 - q->num_buffers;

	if (fmt && fmt->fmt.pix.sizeimage < sur40_video_format.sizeimage)
		return -EINVAL;

	*nplanes = 1;
	sizes[0] = fmt ? fmt->fmt.pix.sizeimage : sur40_video_format.sizeimage;
	alloc_ctxs[0] = sur40->alloc_ctx;

	return 0;
}

/*
 * Prepare the buffer for queueing to the DMA engine: check and set the
 * payload size.
 */
static int sur40_buffer_prepare(struct vb2_buffer *vb)
{
	struct sur40_state *sur40 = vb2_get_drv_priv(vb->vb2_queue);
	unsigned long size = sur40_video_format.sizeimage;

	if (vb2_plane_size(vb, 0) < size) {
		dev_err(&sur40->usbdev->dev, "buffer too small (%lu < %lu)\n",
			 vb2_plane_size(vb, 0), size);
		return -EINVAL;
	}

	vb2_set_plane_payload(vb, 0, size);
	return 0;
}

/*
 * Queue this buffer to the DMA engine.
 */
static void sur40_buffer_queue(struct vb2_buffer *vb)
{
	struct sur40_state *sur40 = vb2_get_drv_priv(vb->vb2_queue);
	struct sur40_buffer *buf = (struct sur40_buffer *)vb;

	spin_lock(&sur40->qlock);
	list_add_tail(&buf->list, &sur40->buf_list);
	spin_unlock(&sur40->qlock);
}

static void return_all_buffers(struct sur40_state *sur40,
			       enum vb2_buffer_state state)
{
	struct sur40_buffer *buf, *node;

	spin_lock(&sur40->qlock);
	list_for_each_entry_safe(buf, node, &sur40->buf_list, list) {
		vb2_buffer_done(&buf->vb, state);
		list_del(&buf->list);
	}
	spin_unlock(&sur40->qlock);
}

/*
 * Start streaming. First check if the minimum number of buffers have been
 * queued. If not, then return -ENOBUFS and the vb2 framework will call
 * this function again the next time a buffer has been queued until enough
 * buffers are available to actually start the DMA engine.
 */
static int sur40_start_streaming(struct vb2_queue *vq, unsigned int count)
{
	struct sur40_state *sur40 = vb2_get_drv_priv(vq);

	sur40->sequence = 0;
	return 0;
}

/*
 * Stop the DMA engine. Any remaining buffers in the DMA queue are dequeued
 * and passed on to the vb2 framework marked as STATE_ERROR.
 */
static void sur40_stop_streaming(struct vb2_queue *vq)
{
	struct sur40_state *sur40 = vb2_get_drv_priv(vq);

	/* Release all active buffers */
	return_all_buffers(sur40, VB2_BUF_STATE_ERROR);
}

/* V4L ioctl */
static int sur40_vidioc_querycap(struct file *file, void *priv,
				 struct v4l2_capability *cap)
{
	struct sur40_state *sur40 = video_drvdata(file);

	strlcpy(cap->driver, DRIVER_SHORT, sizeof(cap->driver));
	strlcpy(cap->card, DRIVER_LONG, sizeof(cap->card));
	usb_make_path(sur40->usbdev, cap->bus_info, sizeof(cap->bus_info));
	cap->device_caps = V4L2_CAP_VIDEO_CAPTURE |
		V4L2_CAP_READWRITE |
		V4L2_CAP_STREAMING;
	cap->capabilities = cap->device_caps | V4L2_CAP_DEVICE_CAPS;
	return 0;
}

static int sur40_vidioc_enum_input(struct file *file, void *priv,
				   struct v4l2_input *i)
{
	if (i->index != 0)
		return -EINVAL;
	i->type = V4L2_INPUT_TYPE_CAMERA;
	i->std = V4L2_STD_UNKNOWN;
	strlcpy(i->name, "In-Cell Sensor", sizeof(i->name));
	i->capabilities = 0;
	return 0;
}

static int sur40_vidioc_s_input(struct file *file, void *priv, unsigned int i)
{
	return (i == 0) ? 0 : -EINVAL;
}

static int sur40_vidioc_g_input(struct file *file, void *priv, unsigned int *i)
{
	*i = 0;
	return 0;
}

static int sur40_vidioc_fmt(struct file *file, void *priv,
			    struct v4l2_format *f)
{
	f->fmt.pix = sur40_video_format;
	return 0;
}

static int sur40_vidioc_enum_fmt(struct file *file, void *priv,
				 struct v4l2_fmtdesc *f)
{
	if (f->index != 0)
		return -EINVAL;
	strlcpy(f->description, "8-bit greyscale", sizeof(f->description));
	f->pixelformat = V4L2_PIX_FMT_GREY;
	f->flags = 0;
	return 0;
}

static const struct usb_device_id sur40_table[] = {
	{ USB_DEVICE(ID_MICROSOFT, ID_SUR40) },  /* Samsung SUR40 */
	{ }                                      /* terminating null entry */
};
MODULE_DEVICE_TABLE(usb, sur40_table);

/* V4L2 structures */
static const struct vb2_ops sur40_queue_ops = {
	.queue_setup		= sur40_queue_setup,
	.buf_prepare		= sur40_buffer_prepare,
	.buf_queue		= sur40_buffer_queue,
	.start_streaming	= sur40_start_streaming,
	.stop_streaming		= sur40_stop_streaming,
	.wait_prepare		= vb2_ops_wait_prepare,
	.wait_finish		= vb2_ops_wait_finish,
};

static const struct vb2_queue sur40_queue = {
	.type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
	/*
	 * VB2_USERPTR in currently not enabled: passing a user pointer to
	 * dma-sg will result in segment sizes that are not a multiple of
	 * 512 bytes, which is required by the host controller.
	*/
	.io_modes = VB2_MMAP | VB2_READ | VB2_DMABUF,
	.buf_struct_size = sizeof(struct sur40_buffer),
	.ops = &sur40_queue_ops,
	.mem_ops = &vb2_dma_sg_memops,
	.timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC,
	.min_buffers_needed = 3,
};

static const struct v4l2_file_operations sur40_video_fops = {
	.owner = THIS_MODULE,
	.open = v4l2_fh_open,
	.release = vb2_fop_release,
	.unlocked_ioctl = video_ioctl2,
	.read = vb2_fop_read,
	.mmap = vb2_fop_mmap,
	.poll = vb2_fop_poll,
};

static const struct v4l2_ioctl_ops sur40_video_ioctl_ops = {

	.vidioc_querycap	= sur40_vidioc_querycap,

	.vidioc_enum_fmt_vid_cap = sur40_vidioc_enum_fmt,
	.vidioc_try_fmt_vid_cap	= sur40_vidioc_fmt,
	.vidioc_s_fmt_vid_cap	= sur40_vidioc_fmt,
	.vidioc_g_fmt_vid_cap	= sur40_vidioc_fmt,

	.vidioc_enum_input	= sur40_vidioc_enum_input,
	.vidioc_g_input		= sur40_vidioc_g_input,
	.vidioc_s_input		= sur40_vidioc_s_input,

	.vidioc_reqbufs		= vb2_ioctl_reqbufs,
	.vidioc_create_bufs	= vb2_ioctl_create_bufs,
	.vidioc_querybuf	= vb2_ioctl_querybuf,
	.vidioc_qbuf		= vb2_ioctl_qbuf,
	.vidioc_dqbuf		= vb2_ioctl_dqbuf,
	.vidioc_expbuf		= vb2_ioctl_expbuf,

	.vidioc_streamon	= vb2_ioctl_streamon,
	.vidioc_streamoff	= vb2_ioctl_streamoff,
};

static const struct video_device sur40_video_device = {
	.name = DRIVER_LONG,
	.fops = &sur40_video_fops,
	.ioctl_ops = &sur40_video_ioctl_ops,
	.release = video_device_release_empty,
};

static const struct v4l2_pix_format sur40_video_format = {
	.pixelformat = V4L2_PIX_FMT_GREY,
	.width  = SENSOR_RES_X / 2,
	.height = SENSOR_RES_Y / 2,
	.field = V4L2_FIELD_NONE,
	.colorspace = V4L2_COLORSPACE_SRGB,
	.bytesperline = SENSOR_RES_X / 2,
	.sizeimage = (SENSOR_RES_X/2) * (SENSOR_RES_Y/2),
};

/* USB-specific object needed to register this driver with the USB subsystem. */
static struct usb_driver sur40_driver = {
	.name = DRIVER_SHORT,
	.probe = sur40_probe,
	.disconnect = sur40_disconnect,
	.id_table = sur40_table,
};

module_usb_driver(sur40_driver);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");
