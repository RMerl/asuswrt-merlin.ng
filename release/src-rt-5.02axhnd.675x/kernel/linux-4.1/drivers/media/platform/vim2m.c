/*
 * A virtual v4l2-mem2mem example device.
 *
 * This is a virtual device driver for testing mem-to-mem videobuf framework.
 * It simulates a device that uses memory buffers for both source and
 * destination, processes the data and issues an "irq" (simulated by a timer).
 * The device is capable of multi-instance, multi-buffer-per-transaction
 * operation (via the mem2mem framework).
 *
 * Copyright (c) 2009-2010 Samsung Electronics Co., Ltd.
 * Pawel Osciak, <pawel@osciak.com>
 * Marek Szyprowski, <m.szyprowski@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version
 */
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <linux/slab.h>

#include <linux/platform_device.h>
#include <media/v4l2-mem2mem.h>
#include <media/v4l2-device.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-event.h>
#include <media/videobuf2-vmalloc.h>

MODULE_DESCRIPTION("Virtual device for mem2mem framework testing");
MODULE_AUTHOR("Pawel Osciak, <pawel@osciak.com>");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1.1");
MODULE_ALIAS("mem2mem_testdev");

static unsigned debug;
module_param(debug, uint, 0644);
MODULE_PARM_DESC(debug, "activates debug info");

#define MIN_W 32
#define MIN_H 32
#define MAX_W 640
#define MAX_H 480
#define DIM_ALIGN_MASK 7 /* 8-byte alignment for line length */

/* Flags that indicate a format can be used for capture/output */
#define MEM2MEM_CAPTURE	(1 << 0)
#define MEM2MEM_OUTPUT	(1 << 1)

#define MEM2MEM_NAME		"vim2m"

/* Per queue */
#define MEM2MEM_DEF_NUM_BUFS	VIDEO_MAX_FRAME
/* In bytes, per queue */
#define MEM2MEM_VID_MEM_LIMIT	(16 * 1024 * 1024)

/* Default transaction time in msec */
#define MEM2MEM_DEF_TRANSTIME	40
#define MEM2MEM_COLOR_STEP	(0xff >> 4)
#define MEM2MEM_NUM_TILES	8

/* Flags that indicate processing mode */
#define MEM2MEM_HFLIP	(1 << 0)
#define MEM2MEM_VFLIP	(1 << 1)

#define dprintk(dev, fmt, arg...) \
	v4l2_dbg(1, debug, &dev->v4l2_dev, "%s: " fmt, __func__, ## arg)


static void vim2m_dev_release(struct device *dev)
{}

static struct platform_device vim2m_pdev = {
	.name		= MEM2MEM_NAME,
	.dev.release	= vim2m_dev_release,
};

struct vim2m_fmt {
	char	*name;
	u32	fourcc;
	int	depth;
	/* Types the format can be used for */
	u32	types;
};

static struct vim2m_fmt formats[] = {
	{
		.name	= "RGB565 (BE)",
		.fourcc	= V4L2_PIX_FMT_RGB565X, /* rrrrrggg gggbbbbb */
		.depth	= 16,
		/* Both capture and output format */
		.types	= MEM2MEM_CAPTURE | MEM2MEM_OUTPUT,
	},
	{
		.name	= "4:2:2, packed, YUYV",
		.fourcc	= V4L2_PIX_FMT_YUYV,
		.depth	= 16,
		/* Output-only format */
		.types	= MEM2MEM_OUTPUT,
	},
};

#define NUM_FORMATS ARRAY_SIZE(formats)

/* Per-queue, driver-specific private data */
struct vim2m_q_data {
	unsigned int		width;
	unsigned int		height;
	unsigned int		sizeimage;
	unsigned int		sequence;
	struct vim2m_fmt	*fmt;
};

enum {
	V4L2_M2M_SRC = 0,
	V4L2_M2M_DST = 1,
};

#define V4L2_CID_TRANS_TIME_MSEC	(V4L2_CID_USER_BASE + 0x1000)
#define V4L2_CID_TRANS_NUM_BUFS		(V4L2_CID_USER_BASE + 0x1001)

static struct vim2m_fmt *find_format(struct v4l2_format *f)
{
	struct vim2m_fmt *fmt;
	unsigned int k;

	for (k = 0; k < NUM_FORMATS; k++) {
		fmt = &formats[k];
		if (fmt->fourcc == f->fmt.pix.pixelformat)
			break;
	}

	if (k == NUM_FORMATS)
		return NULL;

	return &formats[k];
}

struct vim2m_dev {
	struct v4l2_device	v4l2_dev;
	struct video_device	vfd;

	atomic_t		num_inst;
	struct mutex		dev_mutex;
	spinlock_t		irqlock;

	struct timer_list	timer;

	struct v4l2_m2m_dev	*m2m_dev;
};

struct vim2m_ctx {
	struct v4l2_fh		fh;
	struct vim2m_dev	*dev;

	struct v4l2_ctrl_handler hdl;

	/* Processed buffers in this transaction */
	u8			num_processed;

	/* Transaction length (i.e. how many buffers per transaction) */
	u32			translen;
	/* Transaction time (i.e. simulated processing time) in milliseconds */
	u32			transtime;

	/* Abort requested by m2m */
	int			aborting;

	/* Processing mode */
	int			mode;

	enum v4l2_colorspace	colorspace;

	/* Source and destination queue data */
	struct vim2m_q_data   q_data[2];
};

static inline struct vim2m_ctx *file2ctx(struct file *file)
{
	return container_of(file->private_data, struct vim2m_ctx, fh);
}

static struct vim2m_q_data *get_q_data(struct vim2m_ctx *ctx,
					 enum v4l2_buf_type type)
{
	switch (type) {
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
		return &ctx->q_data[V4L2_M2M_SRC];
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		return &ctx->q_data[V4L2_M2M_DST];
	default:
		BUG();
	}
	return NULL;
}


static int device_process(struct vim2m_ctx *ctx,
			  struct vb2_buffer *in_vb,
			  struct vb2_buffer *out_vb)
{
	struct vim2m_dev *dev = ctx->dev;
	struct vim2m_q_data *q_data;
	u8 *p_in, *p_out;
	int x, y, t, w;
	int tile_w, bytes_left;
	int width, height, bytesperline;

	q_data = get_q_data(ctx, V4L2_BUF_TYPE_VIDEO_OUTPUT);

	width	= q_data->width;
	height	= q_data->height;
	bytesperline	= (q_data->width * q_data->fmt->depth) >> 3;

	p_in = vb2_plane_vaddr(in_vb, 0);
	p_out = vb2_plane_vaddr(out_vb, 0);
	if (!p_in || !p_out) {
		v4l2_err(&dev->v4l2_dev,
			 "Acquiring kernel pointers to buffers failed\n");
		return -EFAULT;
	}

	if (vb2_plane_size(in_vb, 0) > vb2_plane_size(out_vb, 0)) {
		v4l2_err(&dev->v4l2_dev, "Output buffer is too small\n");
		return -EINVAL;
	}

	tile_w = (width * (q_data[V4L2_M2M_DST].fmt->depth >> 3))
		/ MEM2MEM_NUM_TILES;
	bytes_left = bytesperline - tile_w * MEM2MEM_NUM_TILES;
	w = 0;

	out_vb->v4l2_buf.sequence = get_q_data(ctx, V4L2_BUF_TYPE_VIDEO_CAPTURE)->sequence++;
	in_vb->v4l2_buf.sequence = q_data->sequence++;
	memcpy(&out_vb->v4l2_buf.timestamp,
			&in_vb->v4l2_buf.timestamp,
			sizeof(struct timeval));
	if (in_vb->v4l2_buf.flags & V4L2_BUF_FLAG_TIMECODE)
		memcpy(&out_vb->v4l2_buf.timecode, &in_vb->v4l2_buf.timecode,
			sizeof(struct v4l2_timecode));
	out_vb->v4l2_buf.field = in_vb->v4l2_buf.field;
	out_vb->v4l2_buf.flags = in_vb->v4l2_buf.flags &
		(V4L2_BUF_FLAG_TIMECODE |
		 V4L2_BUF_FLAG_KEYFRAME |
		 V4L2_BUF_FLAG_PFRAME |
		 V4L2_BUF_FLAG_BFRAME |
		 V4L2_BUF_FLAG_TSTAMP_SRC_MASK);

	switch (ctx->mode) {
	case MEM2MEM_HFLIP | MEM2MEM_VFLIP:
		p_out += bytesperline * height - bytes_left;
		for (y = 0; y < height; ++y) {
			for (t = 0; t < MEM2MEM_NUM_TILES; ++t) {
				if (w & 0x1) {
					for (x = 0; x < tile_w; ++x)
						*--p_out = *p_in++ +
							MEM2MEM_COLOR_STEP;
				} else {
					for (x = 0; x < tile_w; ++x)
						*--p_out = *p_in++ -
							MEM2MEM_COLOR_STEP;
				}
				++w;
			}
			p_in += bytes_left;
			p_out -= bytes_left;
		}
		break;

	case MEM2MEM_HFLIP:
		for (y = 0; y < height; ++y) {
			p_out += MEM2MEM_NUM_TILES * tile_w;
			for (t = 0; t < MEM2MEM_NUM_TILES; ++t) {
				if (w & 0x01) {
					for (x = 0; x < tile_w; ++x)
						*--p_out = *p_in++ +
							MEM2MEM_COLOR_STEP;
				} else {
					for (x = 0; x < tile_w; ++x)
						*--p_out = *p_in++ -
							MEM2MEM_COLOR_STEP;
				}
				++w;
			}
			p_in += bytes_left;
			p_out += bytesperline;
		}
		break;

	case MEM2MEM_VFLIP:
		p_out += bytesperline * (height - 1);
		for (y = 0; y < height; ++y) {
			for (t = 0; t < MEM2MEM_NUM_TILES; ++t) {
				if (w & 0x1) {
					for (x = 0; x < tile_w; ++x)
						*p_out++ = *p_in++ +
							MEM2MEM_COLOR_STEP;
				} else {
					for (x = 0; x < tile_w; ++x)
						*p_out++ = *p_in++ -
							MEM2MEM_COLOR_STEP;
				}
				++w;
			}
			p_in += bytes_left;
			p_out += bytes_left - 2 * bytesperline;
		}
		break;

	default:
		for (y = 0; y < height; ++y) {
			for (t = 0; t < MEM2MEM_NUM_TILES; ++t) {
				if (w & 0x1) {
					for (x = 0; x < tile_w; ++x)
						*p_out++ = *p_in++ +
							MEM2MEM_COLOR_STEP;
				} else {
					for (x = 0; x < tile_w; ++x)
						*p_out++ = *p_in++ -
							MEM2MEM_COLOR_STEP;
				}
				++w;
			}
			p_in += bytes_left;
			p_out += bytes_left;
		}
	}

	return 0;
}

static void schedule_irq(struct vim2m_dev *dev, int msec_timeout)
{
	dprintk(dev, "Scheduling a simulated irq\n");
	mod_timer(&dev->timer, jiffies + msecs_to_jiffies(msec_timeout));
}

/*
 * mem2mem callbacks
 */

/**
 * job_ready() - check whether an instance is ready to be scheduled to run
 */
static int job_ready(void *priv)
{
	struct vim2m_ctx *ctx = priv;

	if (v4l2_m2m_num_src_bufs_ready(ctx->fh.m2m_ctx) < ctx->translen
	    || v4l2_m2m_num_dst_bufs_ready(ctx->fh.m2m_ctx) < ctx->translen) {
		dprintk(ctx->dev, "Not enough buffers available\n");
		return 0;
	}

	return 1;
}

static void job_abort(void *priv)
{
	struct vim2m_ctx *ctx = priv;

	/* Will cancel the transaction in the next interrupt handler */
	ctx->aborting = 1;
}

/* device_run() - prepares and starts the device
 *
 * This simulates all the immediate preparations required before starting
 * a device. This will be called by the framework when it decides to schedule
 * a particular instance.
 */
static void device_run(void *priv)
{
	struct vim2m_ctx *ctx = priv;
	struct vim2m_dev *dev = ctx->dev;
	struct vb2_buffer *src_buf, *dst_buf;

	src_buf = v4l2_m2m_next_src_buf(ctx->fh.m2m_ctx);
	dst_buf = v4l2_m2m_next_dst_buf(ctx->fh.m2m_ctx);

	device_process(ctx, src_buf, dst_buf);

	/* Run a timer, which simulates a hardware irq  */
	schedule_irq(dev, ctx->transtime);
}

static void device_isr(unsigned long priv)
{
	struct vim2m_dev *vim2m_dev = (struct vim2m_dev *)priv;
	struct vim2m_ctx *curr_ctx;
	struct vb2_buffer *src_vb, *dst_vb;
	unsigned long flags;

	curr_ctx = v4l2_m2m_get_curr_priv(vim2m_dev->m2m_dev);

	if (NULL == curr_ctx) {
		pr_err("Instance released before the end of transaction\n");
		return;
	}

	src_vb = v4l2_m2m_src_buf_remove(curr_ctx->fh.m2m_ctx);
	dst_vb = v4l2_m2m_dst_buf_remove(curr_ctx->fh.m2m_ctx);

	curr_ctx->num_processed++;

	spin_lock_irqsave(&vim2m_dev->irqlock, flags);
	v4l2_m2m_buf_done(src_vb, VB2_BUF_STATE_DONE);
	v4l2_m2m_buf_done(dst_vb, VB2_BUF_STATE_DONE);
	spin_unlock_irqrestore(&vim2m_dev->irqlock, flags);

	if (curr_ctx->num_processed == curr_ctx->translen
	    || curr_ctx->aborting) {
		dprintk(curr_ctx->dev, "Finishing transaction\n");
		curr_ctx->num_processed = 0;
		v4l2_m2m_job_finish(vim2m_dev->m2m_dev, curr_ctx->fh.m2m_ctx);
	} else {
		device_run(curr_ctx);
	}
}

/*
 * video ioctls
 */
static int vidioc_querycap(struct file *file, void *priv,
			   struct v4l2_capability *cap)
{
	strncpy(cap->driver, MEM2MEM_NAME, sizeof(cap->driver) - 1);
	strncpy(cap->card, MEM2MEM_NAME, sizeof(cap->card) - 1);
	snprintf(cap->bus_info, sizeof(cap->bus_info),
			"platform:%s", MEM2MEM_NAME);
	cap->device_caps = V4L2_CAP_VIDEO_M2M | V4L2_CAP_STREAMING;
	cap->capabilities = cap->device_caps | V4L2_CAP_DEVICE_CAPS;
	return 0;
}

static int enum_fmt(struct v4l2_fmtdesc *f, u32 type)
{
	int i, num;
	struct vim2m_fmt *fmt;

	num = 0;

	for (i = 0; i < NUM_FORMATS; ++i) {
		if (formats[i].types & type) {
			/* index-th format of type type found ? */
			if (num == f->index)
				break;
			/* Correct type but haven't reached our index yet,
			 * just increment per-type index */
			++num;
		}
	}

	if (i < NUM_FORMATS) {
		/* Format found */
		fmt = &formats[i];
		strncpy(f->description, fmt->name, sizeof(f->description) - 1);
		f->pixelformat = fmt->fourcc;
		return 0;
	}

	/* Format not found */
	return -EINVAL;
}

static int vidioc_enum_fmt_vid_cap(struct file *file, void *priv,
				   struct v4l2_fmtdesc *f)
{
	return enum_fmt(f, MEM2MEM_CAPTURE);
}

static int vidioc_enum_fmt_vid_out(struct file *file, void *priv,
				   struct v4l2_fmtdesc *f)
{
	return enum_fmt(f, MEM2MEM_OUTPUT);
}

static int vidioc_g_fmt(struct vim2m_ctx *ctx, struct v4l2_format *f)
{
	struct vb2_queue *vq;
	struct vim2m_q_data *q_data;

	vq = v4l2_m2m_get_vq(ctx->fh.m2m_ctx, f->type);
	if (!vq)
		return -EINVAL;

	q_data = get_q_data(ctx, f->type);

	f->fmt.pix.width	= q_data->width;
	f->fmt.pix.height	= q_data->height;
	f->fmt.pix.field	= V4L2_FIELD_NONE;
	f->fmt.pix.pixelformat	= q_data->fmt->fourcc;
	f->fmt.pix.bytesperline	= (q_data->width * q_data->fmt->depth) >> 3;
	f->fmt.pix.sizeimage	= q_data->sizeimage;
	f->fmt.pix.colorspace	= ctx->colorspace;

	return 0;
}

static int vidioc_g_fmt_vid_out(struct file *file, void *priv,
				struct v4l2_format *f)
{
	return vidioc_g_fmt(file2ctx(file), f);
}

static int vidioc_g_fmt_vid_cap(struct file *file, void *priv,
				struct v4l2_format *f)
{
	return vidioc_g_fmt(file2ctx(file), f);
}

static int vidioc_try_fmt(struct v4l2_format *f, struct vim2m_fmt *fmt)
{
	/* V4L2 specification suggests the driver corrects the format struct
	 * if any of the dimensions is unsupported */
	if (f->fmt.pix.height < MIN_H)
		f->fmt.pix.height = MIN_H;
	else if (f->fmt.pix.height > MAX_H)
		f->fmt.pix.height = MAX_H;

	if (f->fmt.pix.width < MIN_W)
		f->fmt.pix.width = MIN_W;
	else if (f->fmt.pix.width > MAX_W)
		f->fmt.pix.width = MAX_W;

	f->fmt.pix.width &= ~DIM_ALIGN_MASK;
	f->fmt.pix.bytesperline = (f->fmt.pix.width * fmt->depth) >> 3;
	f->fmt.pix.sizeimage = f->fmt.pix.height * f->fmt.pix.bytesperline;
	f->fmt.pix.field = V4L2_FIELD_NONE;

	return 0;
}

static int vidioc_try_fmt_vid_cap(struct file *file, void *priv,
				  struct v4l2_format *f)
{
	struct vim2m_fmt *fmt;
	struct vim2m_ctx *ctx = file2ctx(file);

	fmt = find_format(f);
	if (!fmt) {
		f->fmt.pix.pixelformat = formats[0].fourcc;
		fmt = find_format(f);
	}
	if (!(fmt->types & MEM2MEM_CAPTURE)) {
		v4l2_err(&ctx->dev->v4l2_dev,
			 "Fourcc format (0x%08x) invalid.\n",
			 f->fmt.pix.pixelformat);
		return -EINVAL;
	}
	f->fmt.pix.colorspace = ctx->colorspace;

	return vidioc_try_fmt(f, fmt);
}

static int vidioc_try_fmt_vid_out(struct file *file, void *priv,
				  struct v4l2_format *f)
{
	struct vim2m_fmt *fmt;
	struct vim2m_ctx *ctx = file2ctx(file);

	fmt = find_format(f);
	if (!fmt) {
		f->fmt.pix.pixelformat = formats[0].fourcc;
		fmt = find_format(f);
	}
	if (!(fmt->types & MEM2MEM_OUTPUT)) {
		v4l2_err(&ctx->dev->v4l2_dev,
			 "Fourcc format (0x%08x) invalid.\n",
			 f->fmt.pix.pixelformat);
		return -EINVAL;
	}
	if (!f->fmt.pix.colorspace)
		f->fmt.pix.colorspace = V4L2_COLORSPACE_REC709;

	return vidioc_try_fmt(f, fmt);
}

static int vidioc_s_fmt(struct vim2m_ctx *ctx, struct v4l2_format *f)
{
	struct vim2m_q_data *q_data;
	struct vb2_queue *vq;

	vq = v4l2_m2m_get_vq(ctx->fh.m2m_ctx, f->type);
	if (!vq)
		return -EINVAL;

	q_data = get_q_data(ctx, f->type);
	if (!q_data)
		return -EINVAL;

	if (vb2_is_busy(vq)) {
		v4l2_err(&ctx->dev->v4l2_dev, "%s queue busy\n", __func__);
		return -EBUSY;
	}

	q_data->fmt		= find_format(f);
	q_data->width		= f->fmt.pix.width;
	q_data->height		= f->fmt.pix.height;
	q_data->sizeimage	= q_data->width * q_data->height
				* q_data->fmt->depth >> 3;

	dprintk(ctx->dev,
		"Setting format for type %d, wxh: %dx%d, fmt: %d\n",
		f->type, q_data->width, q_data->height, q_data->fmt->fourcc);

	return 0;
}

static int vidioc_s_fmt_vid_cap(struct file *file, void *priv,
				struct v4l2_format *f)
{
	int ret;

	ret = vidioc_try_fmt_vid_cap(file, priv, f);
	if (ret)
		return ret;

	return vidioc_s_fmt(file2ctx(file), f);
}

static int vidioc_s_fmt_vid_out(struct file *file, void *priv,
				struct v4l2_format *f)
{
	struct vim2m_ctx *ctx = file2ctx(file);
	int ret;

	ret = vidioc_try_fmt_vid_out(file, priv, f);
	if (ret)
		return ret;

	ret = vidioc_s_fmt(file2ctx(file), f);
	if (!ret)
		ctx->colorspace = f->fmt.pix.colorspace;
	return ret;
}

static int vim2m_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct vim2m_ctx *ctx =
		container_of(ctrl->handler, struct vim2m_ctx, hdl);

	switch (ctrl->id) {
	case V4L2_CID_HFLIP:
		if (ctrl->val)
			ctx->mode |= MEM2MEM_HFLIP;
		else
			ctx->mode &= ~MEM2MEM_HFLIP;
		break;

	case V4L2_CID_VFLIP:
		if (ctrl->val)
			ctx->mode |= MEM2MEM_VFLIP;
		else
			ctx->mode &= ~MEM2MEM_VFLIP;
		break;

	case V4L2_CID_TRANS_TIME_MSEC:
		ctx->transtime = ctrl->val;
		break;

	case V4L2_CID_TRANS_NUM_BUFS:
		ctx->translen = ctrl->val;
		break;

	default:
		v4l2_err(&ctx->dev->v4l2_dev, "Invalid control\n");
		return -EINVAL;
	}

	return 0;
}

static const struct v4l2_ctrl_ops vim2m_ctrl_ops = {
	.s_ctrl = vim2m_s_ctrl,
};


static const struct v4l2_ioctl_ops vim2m_ioctl_ops = {
	.vidioc_querycap	= vidioc_querycap,

	.vidioc_enum_fmt_vid_cap = vidioc_enum_fmt_vid_cap,
	.vidioc_g_fmt_vid_cap	= vidioc_g_fmt_vid_cap,
	.vidioc_try_fmt_vid_cap	= vidioc_try_fmt_vid_cap,
	.vidioc_s_fmt_vid_cap	= vidioc_s_fmt_vid_cap,

	.vidioc_enum_fmt_vid_out = vidioc_enum_fmt_vid_out,
	.vidioc_g_fmt_vid_out	= vidioc_g_fmt_vid_out,
	.vidioc_try_fmt_vid_out	= vidioc_try_fmt_vid_out,
	.vidioc_s_fmt_vid_out	= vidioc_s_fmt_vid_out,

	.vidioc_reqbufs		= v4l2_m2m_ioctl_reqbufs,
	.vidioc_querybuf	= v4l2_m2m_ioctl_querybuf,
	.vidioc_qbuf		= v4l2_m2m_ioctl_qbuf,
	.vidioc_dqbuf		= v4l2_m2m_ioctl_dqbuf,
	.vidioc_expbuf		= v4l2_m2m_ioctl_expbuf,

	.vidioc_streamon	= v4l2_m2m_ioctl_streamon,
	.vidioc_streamoff	= v4l2_m2m_ioctl_streamoff,

	.vidioc_subscribe_event = v4l2_ctrl_subscribe_event,
	.vidioc_unsubscribe_event = v4l2_event_unsubscribe,
};


/*
 * Queue operations
 */

static int vim2m_queue_setup(struct vb2_queue *vq,
				const struct v4l2_format *fmt,
				unsigned int *nbuffers, unsigned int *nplanes,
				unsigned int sizes[], void *alloc_ctxs[])
{
	struct vim2m_ctx *ctx = vb2_get_drv_priv(vq);
	struct vim2m_q_data *q_data;
	unsigned int size, count = *nbuffers;

	q_data = get_q_data(ctx, vq->type);

	size = q_data->width * q_data->height * q_data->fmt->depth >> 3;

	while (size * count > MEM2MEM_VID_MEM_LIMIT)
		(count)--;

	*nplanes = 1;
	*nbuffers = count;
	sizes[0] = size;

	/*
	 * videobuf2-vmalloc allocator is context-less so no need to set
	 * alloc_ctxs array.
	 */

	dprintk(ctx->dev, "get %d buffer(s) of size %d each.\n", count, size);

	return 0;
}

static int vim2m_buf_prepare(struct vb2_buffer *vb)
{
	struct vim2m_ctx *ctx = vb2_get_drv_priv(vb->vb2_queue);
	struct vim2m_q_data *q_data;

	dprintk(ctx->dev, "type: %d\n", vb->vb2_queue->type);

	q_data = get_q_data(ctx, vb->vb2_queue->type);
	if (V4L2_TYPE_IS_OUTPUT(vb->vb2_queue->type)) {
		if (vb->v4l2_buf.field == V4L2_FIELD_ANY)
			vb->v4l2_buf.field = V4L2_FIELD_NONE;
		if (vb->v4l2_buf.field != V4L2_FIELD_NONE) {
			dprintk(ctx->dev, "%s field isn't supported\n",
					__func__);
			return -EINVAL;
		}
	}

	if (vb2_plane_size(vb, 0) < q_data->sizeimage) {
		dprintk(ctx->dev, "%s data will not fit into plane (%lu < %lu)\n",
				__func__, vb2_plane_size(vb, 0), (long)q_data->sizeimage);
		return -EINVAL;
	}

	vb2_set_plane_payload(vb, 0, q_data->sizeimage);

	return 0;
}

static void vim2m_buf_queue(struct vb2_buffer *vb)
{
	struct vim2m_ctx *ctx = vb2_get_drv_priv(vb->vb2_queue);

	v4l2_m2m_buf_queue(ctx->fh.m2m_ctx, vb);
}

static int vim2m_start_streaming(struct vb2_queue *q, unsigned count)
{
	struct vim2m_ctx *ctx = vb2_get_drv_priv(q);
	struct vim2m_q_data *q_data = get_q_data(ctx, q->type);

	q_data->sequence = 0;
	return 0;
}

static void vim2m_stop_streaming(struct vb2_queue *q)
{
	struct vim2m_ctx *ctx = vb2_get_drv_priv(q);
	struct vb2_buffer *vb;
	unsigned long flags;

	for (;;) {
		if (V4L2_TYPE_IS_OUTPUT(q->type))
			vb = v4l2_m2m_src_buf_remove(ctx->fh.m2m_ctx);
		else
			vb = v4l2_m2m_dst_buf_remove(ctx->fh.m2m_ctx);
		if (vb == NULL)
			return;
		spin_lock_irqsave(&ctx->dev->irqlock, flags);
		v4l2_m2m_buf_done(vb, VB2_BUF_STATE_ERROR);
		spin_unlock_irqrestore(&ctx->dev->irqlock, flags);
	}
}

static struct vb2_ops vim2m_qops = {
	.queue_setup	 = vim2m_queue_setup,
	.buf_prepare	 = vim2m_buf_prepare,
	.buf_queue	 = vim2m_buf_queue,
	.start_streaming = vim2m_start_streaming,
	.stop_streaming  = vim2m_stop_streaming,
	.wait_prepare	 = vb2_ops_wait_prepare,
	.wait_finish	 = vb2_ops_wait_finish,
};

static int queue_init(void *priv, struct vb2_queue *src_vq, struct vb2_queue *dst_vq)
{
	struct vim2m_ctx *ctx = priv;
	int ret;

	src_vq->type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	src_vq->io_modes = VB2_MMAP | VB2_USERPTR | VB2_DMABUF;
	src_vq->drv_priv = ctx;
	src_vq->buf_struct_size = sizeof(struct v4l2_m2m_buffer);
	src_vq->ops = &vim2m_qops;
	src_vq->mem_ops = &vb2_vmalloc_memops;
	src_vq->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_COPY;
	src_vq->lock = &ctx->dev->dev_mutex;

	ret = vb2_queue_init(src_vq);
	if (ret)
		return ret;

	dst_vq->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	dst_vq->io_modes = VB2_MMAP | VB2_USERPTR | VB2_DMABUF;
	dst_vq->drv_priv = ctx;
	dst_vq->buf_struct_size = sizeof(struct v4l2_m2m_buffer);
	dst_vq->ops = &vim2m_qops;
	dst_vq->mem_ops = &vb2_vmalloc_memops;
	dst_vq->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_COPY;
	dst_vq->lock = &ctx->dev->dev_mutex;

	return vb2_queue_init(dst_vq);
}

static const struct v4l2_ctrl_config vim2m_ctrl_trans_time_msec = {
	.ops = &vim2m_ctrl_ops,
	.id = V4L2_CID_TRANS_TIME_MSEC,
	.name = "Transaction Time (msec)",
	.type = V4L2_CTRL_TYPE_INTEGER,
	.def = MEM2MEM_DEF_TRANSTIME,
	.min = 1,
	.max = 10001,
	.step = 1,
};

static const struct v4l2_ctrl_config vim2m_ctrl_trans_num_bufs = {
	.ops = &vim2m_ctrl_ops,
	.id = V4L2_CID_TRANS_NUM_BUFS,
	.name = "Buffers Per Transaction",
	.type = V4L2_CTRL_TYPE_INTEGER,
	.def = 1,
	.min = 1,
	.max = MEM2MEM_DEF_NUM_BUFS,
	.step = 1,
};

/*
 * File operations
 */
static int vim2m_open(struct file *file)
{
	struct vim2m_dev *dev = video_drvdata(file);
	struct vim2m_ctx *ctx = NULL;
	struct v4l2_ctrl_handler *hdl;
	int rc = 0;

	if (mutex_lock_interruptible(&dev->dev_mutex))
		return -ERESTARTSYS;
	ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
	if (!ctx) {
		rc = -ENOMEM;
		goto open_unlock;
	}

	v4l2_fh_init(&ctx->fh, video_devdata(file));
	file->private_data = &ctx->fh;
	ctx->dev = dev;
	hdl = &ctx->hdl;
	v4l2_ctrl_handler_init(hdl, 4);
	v4l2_ctrl_new_std(hdl, &vim2m_ctrl_ops, V4L2_CID_HFLIP, 0, 1, 1, 0);
	v4l2_ctrl_new_std(hdl, &vim2m_ctrl_ops, V4L2_CID_VFLIP, 0, 1, 1, 0);
	v4l2_ctrl_new_custom(hdl, &vim2m_ctrl_trans_time_msec, NULL);
	v4l2_ctrl_new_custom(hdl, &vim2m_ctrl_trans_num_bufs, NULL);
	if (hdl->error) {
		rc = hdl->error;
		v4l2_ctrl_handler_free(hdl);
		goto open_unlock;
	}
	ctx->fh.ctrl_handler = hdl;
	v4l2_ctrl_handler_setup(hdl);

	ctx->q_data[V4L2_M2M_SRC].fmt = &formats[0];
	ctx->q_data[V4L2_M2M_SRC].width = 640;
	ctx->q_data[V4L2_M2M_SRC].height = 480;
	ctx->q_data[V4L2_M2M_SRC].sizeimage =
		ctx->q_data[V4L2_M2M_SRC].width *
		ctx->q_data[V4L2_M2M_SRC].height *
		(ctx->q_data[V4L2_M2M_SRC].fmt->depth >> 3);
	ctx->q_data[V4L2_M2M_DST] = ctx->q_data[V4L2_M2M_SRC];
	ctx->colorspace = V4L2_COLORSPACE_REC709;

	ctx->fh.m2m_ctx = v4l2_m2m_ctx_init(dev->m2m_dev, ctx, &queue_init);

	if (IS_ERR(ctx->fh.m2m_ctx)) {
		rc = PTR_ERR(ctx->fh.m2m_ctx);

		v4l2_ctrl_handler_free(hdl);
		kfree(ctx);
		goto open_unlock;
	}

	v4l2_fh_add(&ctx->fh);
	atomic_inc(&dev->num_inst);

	dprintk(dev, "Created instance: %p, m2m_ctx: %p\n",
		ctx, ctx->fh.m2m_ctx);

open_unlock:
	mutex_unlock(&dev->dev_mutex);
	return rc;
}

static int vim2m_release(struct file *file)
{
	struct vim2m_dev *dev = video_drvdata(file);
	struct vim2m_ctx *ctx = file2ctx(file);

	dprintk(dev, "Releasing instance %p\n", ctx);

	v4l2_fh_del(&ctx->fh);
	v4l2_fh_exit(&ctx->fh);
	v4l2_ctrl_handler_free(&ctx->hdl);
	mutex_lock(&dev->dev_mutex);
	v4l2_m2m_ctx_release(ctx->fh.m2m_ctx);
	mutex_unlock(&dev->dev_mutex);
	kfree(ctx);

	atomic_dec(&dev->num_inst);

	return 0;
}

static const struct v4l2_file_operations vim2m_fops = {
	.owner		= THIS_MODULE,
	.open		= vim2m_open,
	.release	= vim2m_release,
	.poll		= v4l2_m2m_fop_poll,
	.unlocked_ioctl	= video_ioctl2,
	.mmap		= v4l2_m2m_fop_mmap,
};

static struct video_device vim2m_videodev = {
	.name		= MEM2MEM_NAME,
	.vfl_dir	= VFL_DIR_M2M,
	.fops		= &vim2m_fops,
	.ioctl_ops	= &vim2m_ioctl_ops,
	.minor		= -1,
	.release	= video_device_release_empty,
};

static struct v4l2_m2m_ops m2m_ops = {
	.device_run	= device_run,
	.job_ready	= job_ready,
	.job_abort	= job_abort,
};

static int vim2m_probe(struct platform_device *pdev)
{
	struct vim2m_dev *dev;
	struct video_device *vfd;
	int ret;

	dev = devm_kzalloc(&pdev->dev, sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	spin_lock_init(&dev->irqlock);

	ret = v4l2_device_register(&pdev->dev, &dev->v4l2_dev);
	if (ret)
		return ret;

	atomic_set(&dev->num_inst, 0);
	mutex_init(&dev->dev_mutex);

	dev->vfd = vim2m_videodev;
	vfd = &dev->vfd;
	vfd->lock = &dev->dev_mutex;
	vfd->v4l2_dev = &dev->v4l2_dev;

	ret = video_register_device(vfd, VFL_TYPE_GRABBER, 0);
	if (ret) {
		v4l2_err(&dev->v4l2_dev, "Failed to register video device\n");
		goto unreg_dev;
	}

	video_set_drvdata(vfd, dev);
	snprintf(vfd->name, sizeof(vfd->name), "%s", vim2m_videodev.name);
	v4l2_info(&dev->v4l2_dev,
			"Device registered as /dev/video%d\n", vfd->num);

	setup_timer(&dev->timer, device_isr, (long)dev);
	platform_set_drvdata(pdev, dev);

	dev->m2m_dev = v4l2_m2m_init(&m2m_ops);
	if (IS_ERR(dev->m2m_dev)) {
		v4l2_err(&dev->v4l2_dev, "Failed to init mem2mem device\n");
		ret = PTR_ERR(dev->m2m_dev);
		goto err_m2m;
	}

	return 0;

err_m2m:
	v4l2_m2m_release(dev->m2m_dev);
	video_unregister_device(&dev->vfd);
unreg_dev:
	v4l2_device_unregister(&dev->v4l2_dev);

	return ret;
}

static int vim2m_remove(struct platform_device *pdev)
{
	struct vim2m_dev *dev = platform_get_drvdata(pdev);

	v4l2_info(&dev->v4l2_dev, "Removing " MEM2MEM_NAME);
	v4l2_m2m_release(dev->m2m_dev);
	del_timer_sync(&dev->timer);
	video_unregister_device(&dev->vfd);
	v4l2_device_unregister(&dev->v4l2_dev);

	return 0;
}

static struct platform_driver vim2m_pdrv = {
	.probe		= vim2m_probe,
	.remove		= vim2m_remove,
	.driver		= {
		.name	= MEM2MEM_NAME,
	},
};

static void __exit vim2m_exit(void)
{
	platform_driver_unregister(&vim2m_pdrv);
	platform_device_unregister(&vim2m_pdev);
}

static int __init vim2m_init(void)
{
	int ret;

	ret = platform_device_register(&vim2m_pdev);
	if (ret)
		return ret;

	ret = platform_driver_register(&vim2m_pdrv);
	if (ret)
		platform_device_unregister(&vim2m_pdev);

	return 0;
}

module_init(vim2m_init);
module_exit(vim2m_exit);
