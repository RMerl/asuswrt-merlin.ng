/*
 * Copyright (c) 2011 Atmel Corporation
 * Josh Wu, <josh.wu@atmel.com>
 *
 * Based on previous work by Lars Haring, <lars.haring@atmel.com>
 * and Sedji Gaouaou
 * Based on the bttv driver for Bt848 with respective copyright holders
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/clk.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include <media/atmel-isi.h>
#include <media/soc_camera.h>
#include <media/soc_mediabus.h>
#include <media/v4l2-of.h>
#include <media/videobuf2-dma-contig.h>

#define MAX_BUFFER_NUM			32
#define MAX_SUPPORT_WIDTH		2048
#define MAX_SUPPORT_HEIGHT		2048
#define VID_LIMIT_BYTES			(16 * 1024 * 1024)
#define MIN_FRAME_RATE			15
#define FRAME_INTERVAL_MILLI_SEC	(1000 / MIN_FRAME_RATE)
#define ISI_DEFAULT_MCLK_FREQ		25000000

/* Frame buffer descriptor */
struct fbd {
	/* Physical address of the frame buffer */
	u32 fb_address;
	/* DMA Control Register(only in HISI2) */
	u32 dma_ctrl;
	/* Physical address of the next fbd */
	u32 next_fbd_address;
};

static void set_dma_ctrl(struct fbd *fb_desc, u32 ctrl)
{
	fb_desc->dma_ctrl = ctrl;
}

struct isi_dma_desc {
	struct list_head list;
	struct fbd *p_fbd;
	dma_addr_t fbd_phys;
};

/* Frame buffer data */
struct frame_buffer {
	struct vb2_buffer vb;
	struct isi_dma_desc *p_dma_desc;
	struct list_head list;
};

struct atmel_isi {
	/* Protects the access of variables shared with the ISR */
	spinlock_t			lock;
	void __iomem			*regs;

	int				sequence;

	struct vb2_alloc_ctx		*alloc_ctx;

	/* Allocate descriptors for dma buffer use */
	struct fbd			*p_fb_descriptors;
	dma_addr_t			fb_descriptors_phys;
	struct				list_head dma_desc_head;
	struct isi_dma_desc		dma_desc[MAX_BUFFER_NUM];

	struct completion		complete;
	/* ISI peripherial clock */
	struct clk			*pclk;
	/* ISI_MCK, feed to camera sensor to generate pixel clock */
	struct clk			*mck;
	unsigned int			irq;

	struct isi_platform_data	pdata;
	u16				width_flags;	/* max 12 bits */

	struct list_head		video_buffer_list;
	struct frame_buffer		*active;

	struct soc_camera_host		soc_host;
};

static void isi_writel(struct atmel_isi *isi, u32 reg, u32 val)
{
	writel(val, isi->regs + reg);
}
static u32 isi_readl(struct atmel_isi *isi, u32 reg)
{
	return readl(isi->regs + reg);
}

static int configure_geometry(struct atmel_isi *isi, u32 width,
			u32 height, u32 code)
{
	u32 cfg2, cr;

	switch (code) {
	/* YUV, including grey */
	case MEDIA_BUS_FMT_Y8_1X8:
		cr = ISI_CFG2_GRAYSCALE;
		break;
	case MEDIA_BUS_FMT_VYUY8_2X8:
		cr = ISI_CFG2_YCC_SWAP_MODE_3;
		break;
	case MEDIA_BUS_FMT_UYVY8_2X8:
		cr = ISI_CFG2_YCC_SWAP_MODE_2;
		break;
	case MEDIA_BUS_FMT_YVYU8_2X8:
		cr = ISI_CFG2_YCC_SWAP_MODE_1;
		break;
	case MEDIA_BUS_FMT_YUYV8_2X8:
		cr = ISI_CFG2_YCC_SWAP_DEFAULT;
		break;
	/* RGB, TODO */
	default:
		return -EINVAL;
	}

	isi_writel(isi, ISI_CTRL, ISI_CTRL_DIS);

	cfg2 = isi_readl(isi, ISI_CFG2);
	/* Set YCC swap mode */
	cfg2 &= ~ISI_CFG2_YCC_SWAP_MODE_MASK;
	cfg2 |= cr;
	/* Set width */
	cfg2 &= ~(ISI_CFG2_IM_HSIZE_MASK);
	cfg2 |= ((width - 1) << ISI_CFG2_IM_HSIZE_OFFSET) &
			ISI_CFG2_IM_HSIZE_MASK;
	/* Set height */
	cfg2 &= ~(ISI_CFG2_IM_VSIZE_MASK);
	cfg2 |= ((height - 1) << ISI_CFG2_IM_VSIZE_OFFSET)
			& ISI_CFG2_IM_VSIZE_MASK;
	isi_writel(isi, ISI_CFG2, cfg2);

	return 0;
}

static irqreturn_t atmel_isi_handle_streaming(struct atmel_isi *isi)
{
	if (isi->active) {
		struct vb2_buffer *vb = &isi->active->vb;
		struct frame_buffer *buf = isi->active;

		list_del_init(&buf->list);
		v4l2_get_timestamp(&vb->v4l2_buf.timestamp);
		vb->v4l2_buf.sequence = isi->sequence++;
		vb2_buffer_done(vb, VB2_BUF_STATE_DONE);
	}

	if (list_empty(&isi->video_buffer_list)) {
		isi->active = NULL;
	} else {
		/* start next dma frame. */
		isi->active = list_entry(isi->video_buffer_list.next,
					struct frame_buffer, list);
		isi_writel(isi, ISI_DMA_C_DSCR,
			(u32)isi->active->p_dma_desc->fbd_phys);
		isi_writel(isi, ISI_DMA_C_CTRL,
			ISI_DMA_CTRL_FETCH | ISI_DMA_CTRL_DONE);
		isi_writel(isi, ISI_DMA_CHER, ISI_DMA_CHSR_C_CH);
	}
	return IRQ_HANDLED;
}

/* ISI interrupt service routine */
static irqreturn_t isi_interrupt(int irq, void *dev_id)
{
	struct atmel_isi *isi = dev_id;
	u32 status, mask, pending;
	irqreturn_t ret = IRQ_NONE;

	spin_lock(&isi->lock);

	status = isi_readl(isi, ISI_STATUS);
	mask = isi_readl(isi, ISI_INTMASK);
	pending = status & mask;

	if (pending & ISI_CTRL_SRST) {
		complete(&isi->complete);
		isi_writel(isi, ISI_INTDIS, ISI_CTRL_SRST);
		ret = IRQ_HANDLED;
	} else if (pending & ISI_CTRL_DIS) {
		complete(&isi->complete);
		isi_writel(isi, ISI_INTDIS, ISI_CTRL_DIS);
		ret = IRQ_HANDLED;
	} else {
		if (likely(pending & ISI_SR_CXFR_DONE))
			ret = atmel_isi_handle_streaming(isi);
	}

	spin_unlock(&isi->lock);
	return ret;
}

#define	WAIT_ISI_RESET		1
#define	WAIT_ISI_DISABLE	0
static int atmel_isi_wait_status(struct atmel_isi *isi, int wait_reset)
{
	unsigned long timeout;
	/*
	 * The reset or disable will only succeed if we have a
	 * pixel clock from the camera.
	 */
	init_completion(&isi->complete);

	if (wait_reset) {
		isi_writel(isi, ISI_INTEN, ISI_CTRL_SRST);
		isi_writel(isi, ISI_CTRL, ISI_CTRL_SRST);
	} else {
		isi_writel(isi, ISI_INTEN, ISI_CTRL_DIS);
		isi_writel(isi, ISI_CTRL, ISI_CTRL_DIS);
	}

	timeout = wait_for_completion_timeout(&isi->complete,
			msecs_to_jiffies(100));
	if (timeout == 0)
		return -ETIMEDOUT;

	return 0;
}

/* ------------------------------------------------------------------
	Videobuf operations
   ------------------------------------------------------------------*/
static int queue_setup(struct vb2_queue *vq, const struct v4l2_format *fmt,
				unsigned int *nbuffers, unsigned int *nplanes,
				unsigned int sizes[], void *alloc_ctxs[])
{
	struct soc_camera_device *icd = soc_camera_from_vb2q(vq);
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct atmel_isi *isi = ici->priv;
	unsigned long size;

	size = icd->sizeimage;

	if (!*nbuffers || *nbuffers > MAX_BUFFER_NUM)
		*nbuffers = MAX_BUFFER_NUM;

	if (size * *nbuffers > VID_LIMIT_BYTES)
		*nbuffers = VID_LIMIT_BYTES / size;

	*nplanes = 1;
	sizes[0] = size;
	alloc_ctxs[0] = isi->alloc_ctx;

	isi->sequence = 0;
	isi->active = NULL;

	dev_dbg(icd->parent, "%s, count=%d, size=%ld\n", __func__,
		*nbuffers, size);

	return 0;
}

static int buffer_init(struct vb2_buffer *vb)
{
	struct frame_buffer *buf = container_of(vb, struct frame_buffer, vb);

	buf->p_dma_desc = NULL;
	INIT_LIST_HEAD(&buf->list);

	return 0;
}

static int buffer_prepare(struct vb2_buffer *vb)
{
	struct soc_camera_device *icd = soc_camera_from_vb2q(vb->vb2_queue);
	struct frame_buffer *buf = container_of(vb, struct frame_buffer, vb);
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct atmel_isi *isi = ici->priv;
	unsigned long size;
	struct isi_dma_desc *desc;

	size = icd->sizeimage;

	if (vb2_plane_size(vb, 0) < size) {
		dev_err(icd->parent, "%s data will not fit into plane (%lu < %lu)\n",
				__func__, vb2_plane_size(vb, 0), size);
		return -EINVAL;
	}

	vb2_set_plane_payload(&buf->vb, 0, size);

	if (!buf->p_dma_desc) {
		if (list_empty(&isi->dma_desc_head)) {
			dev_err(icd->parent, "Not enough dma descriptors.\n");
			return -EINVAL;
		} else {
			/* Get an available descriptor */
			desc = list_entry(isi->dma_desc_head.next,
						struct isi_dma_desc, list);
			/* Delete the descriptor since now it is used */
			list_del_init(&desc->list);

			/* Initialize the dma descriptor */
			desc->p_fbd->fb_address =
					vb2_dma_contig_plane_dma_addr(vb, 0);
			desc->p_fbd->next_fbd_address = 0;
			set_dma_ctrl(desc->p_fbd, ISI_DMA_CTRL_WB);

			buf->p_dma_desc = desc;
		}
	}
	return 0;
}

static void buffer_cleanup(struct vb2_buffer *vb)
{
	struct soc_camera_device *icd = soc_camera_from_vb2q(vb->vb2_queue);
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct atmel_isi *isi = ici->priv;
	struct frame_buffer *buf = container_of(vb, struct frame_buffer, vb);

	/* This descriptor is available now and we add to head list */
	if (buf->p_dma_desc)
		list_add(&buf->p_dma_desc->list, &isi->dma_desc_head);
}

static void start_dma(struct atmel_isi *isi, struct frame_buffer *buffer)
{
	u32 ctrl, cfg1;

	cfg1 = isi_readl(isi, ISI_CFG1);
	/* Enable irq: cxfr for the codec path, pxfr for the preview path */
	isi_writel(isi, ISI_INTEN,
			ISI_SR_CXFR_DONE | ISI_SR_PXFR_DONE);

	/* Check if already in a frame */
	if (isi_readl(isi, ISI_STATUS) & ISI_CTRL_CDC) {
		dev_err(isi->soc_host.icd->parent, "Already in frame handling.\n");
		return;
	}

	isi_writel(isi, ISI_DMA_C_DSCR, (u32)buffer->p_dma_desc->fbd_phys);
	isi_writel(isi, ISI_DMA_C_CTRL, ISI_DMA_CTRL_FETCH | ISI_DMA_CTRL_DONE);
	isi_writel(isi, ISI_DMA_CHER, ISI_DMA_CHSR_C_CH);

	cfg1 &= ~ISI_CFG1_FRATE_DIV_MASK;
	/* Enable linked list */
	cfg1 |= isi->pdata.frate | ISI_CFG1_DISCR;

	/* Enable codec path and ISI */
	ctrl = ISI_CTRL_CDC | ISI_CTRL_EN;
	isi_writel(isi, ISI_CTRL, ctrl);
	isi_writel(isi, ISI_CFG1, cfg1);
}

static void buffer_queue(struct vb2_buffer *vb)
{
	struct soc_camera_device *icd = soc_camera_from_vb2q(vb->vb2_queue);
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct atmel_isi *isi = ici->priv;
	struct frame_buffer *buf = container_of(vb, struct frame_buffer, vb);
	unsigned long flags = 0;

	spin_lock_irqsave(&isi->lock, flags);
	list_add_tail(&buf->list, &isi->video_buffer_list);

	if (isi->active == NULL) {
		isi->active = buf;
		if (vb2_is_streaming(vb->vb2_queue))
			start_dma(isi, buf);
	}
	spin_unlock_irqrestore(&isi->lock, flags);
}

static int start_streaming(struct vb2_queue *vq, unsigned int count)
{
	struct soc_camera_device *icd = soc_camera_from_vb2q(vq);
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct atmel_isi *isi = ici->priv;
	int ret;

	/* Reset ISI */
	ret = atmel_isi_wait_status(isi, WAIT_ISI_RESET);
	if (ret < 0) {
		dev_err(icd->parent, "Reset ISI timed out\n");
		return ret;
	}
	/* Disable all interrupts */
	isi_writel(isi, ISI_INTDIS, (u32)~0UL);

	spin_lock_irq(&isi->lock);
	/* Clear any pending interrupt */
	isi_readl(isi, ISI_STATUS);

	if (count)
		start_dma(isi, isi->active);
	spin_unlock_irq(&isi->lock);

	return 0;
}

/* abort streaming and wait for last buffer */
static void stop_streaming(struct vb2_queue *vq)
{
	struct soc_camera_device *icd = soc_camera_from_vb2q(vq);
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct atmel_isi *isi = ici->priv;
	struct frame_buffer *buf, *node;
	int ret = 0;
	unsigned long timeout;

	spin_lock_irq(&isi->lock);
	isi->active = NULL;
	/* Release all active buffers */
	list_for_each_entry_safe(buf, node, &isi->video_buffer_list, list) {
		list_del_init(&buf->list);
		vb2_buffer_done(&buf->vb, VB2_BUF_STATE_ERROR);
	}
	spin_unlock_irq(&isi->lock);

	timeout = jiffies + FRAME_INTERVAL_MILLI_SEC * HZ;
	/* Wait until the end of the current frame. */
	while ((isi_readl(isi, ISI_STATUS) & ISI_CTRL_CDC) &&
			time_before(jiffies, timeout))
		msleep(1);

	if (time_after(jiffies, timeout)) {
		dev_err(icd->parent,
			"Timeout waiting for finishing codec request\n");
		return;
	}

	/* Disable interrupts */
	isi_writel(isi, ISI_INTDIS,
			ISI_SR_CXFR_DONE | ISI_SR_PXFR_DONE);

	/* Disable ISI and wait for it is done */
	ret = atmel_isi_wait_status(isi, WAIT_ISI_DISABLE);
	if (ret < 0)
		dev_err(icd->parent, "Disable ISI timed out\n");
}

static struct vb2_ops isi_video_qops = {
	.queue_setup		= queue_setup,
	.buf_init		= buffer_init,
	.buf_prepare		= buffer_prepare,
	.buf_cleanup		= buffer_cleanup,
	.buf_queue		= buffer_queue,
	.start_streaming	= start_streaming,
	.stop_streaming		= stop_streaming,
	.wait_prepare		= vb2_ops_wait_prepare,
	.wait_finish		= vb2_ops_wait_finish,
};

/* ------------------------------------------------------------------
	SOC camera operations for the device
   ------------------------------------------------------------------*/
static int isi_camera_init_videobuf(struct vb2_queue *q,
				     struct soc_camera_device *icd)
{
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);

	q->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	q->io_modes = VB2_MMAP;
	q->drv_priv = icd;
	q->buf_struct_size = sizeof(struct frame_buffer);
	q->ops = &isi_video_qops;
	q->mem_ops = &vb2_dma_contig_memops;
	q->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC;
	q->lock = &ici->host_lock;

	return vb2_queue_init(q);
}

static int isi_camera_set_fmt(struct soc_camera_device *icd,
			      struct v4l2_format *f)
{
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct atmel_isi *isi = ici->priv;
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	const struct soc_camera_format_xlate *xlate;
	struct v4l2_pix_format *pix = &f->fmt.pix;
	struct v4l2_mbus_framefmt mf;
	int ret;

	xlate = soc_camera_xlate_by_fourcc(icd, pix->pixelformat);
	if (!xlate) {
		dev_warn(icd->parent, "Format %x not found\n",
			 pix->pixelformat);
		return -EINVAL;
	}

	dev_dbg(icd->parent, "Plan to set format %dx%d\n",
			pix->width, pix->height);

	mf.width	= pix->width;
	mf.height	= pix->height;
	mf.field	= pix->field;
	mf.colorspace	= pix->colorspace;
	mf.code		= xlate->code;

	ret = v4l2_subdev_call(sd, video, s_mbus_fmt, &mf);
	if (ret < 0)
		return ret;

	if (mf.code != xlate->code)
		return -EINVAL;

	ret = configure_geometry(isi, pix->width, pix->height, xlate->code);
	if (ret < 0)
		return ret;

	pix->width		= mf.width;
	pix->height		= mf.height;
	pix->field		= mf.field;
	pix->colorspace		= mf.colorspace;
	icd->current_fmt	= xlate;

	dev_dbg(icd->parent, "Finally set format %dx%d\n",
		pix->width, pix->height);

	return ret;
}

static int isi_camera_try_fmt(struct soc_camera_device *icd,
			      struct v4l2_format *f)
{
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	const struct soc_camera_format_xlate *xlate;
	struct v4l2_pix_format *pix = &f->fmt.pix;
	struct v4l2_mbus_framefmt mf;
	u32 pixfmt = pix->pixelformat;
	int ret;

	xlate = soc_camera_xlate_by_fourcc(icd, pixfmt);
	if (pixfmt && !xlate) {
		dev_warn(icd->parent, "Format %x not found\n", pixfmt);
		return -EINVAL;
	}

	/* limit to Atmel ISI hardware capabilities */
	if (pix->height > MAX_SUPPORT_HEIGHT)
		pix->height = MAX_SUPPORT_HEIGHT;
	if (pix->width > MAX_SUPPORT_WIDTH)
		pix->width = MAX_SUPPORT_WIDTH;

	/* limit to sensor capabilities */
	mf.width	= pix->width;
	mf.height	= pix->height;
	mf.field	= pix->field;
	mf.colorspace	= pix->colorspace;
	mf.code		= xlate->code;

	ret = v4l2_subdev_call(sd, video, try_mbus_fmt, &mf);
	if (ret < 0)
		return ret;

	pix->width	= mf.width;
	pix->height	= mf.height;
	pix->colorspace	= mf.colorspace;

	switch (mf.field) {
	case V4L2_FIELD_ANY:
		pix->field = V4L2_FIELD_NONE;
		break;
	case V4L2_FIELD_NONE:
		break;
	default:
		dev_err(icd->parent, "Field type %d unsupported.\n",
			mf.field);
		ret = -EINVAL;
	}

	return ret;
}

static const struct soc_mbus_pixelfmt isi_camera_formats[] = {
	{
		.fourcc			= V4L2_PIX_FMT_YUYV,
		.name			= "Packed YUV422 16 bit",
		.bits_per_sample	= 8,
		.packing		= SOC_MBUS_PACKING_2X8_PADHI,
		.order			= SOC_MBUS_ORDER_LE,
		.layout			= SOC_MBUS_LAYOUT_PACKED,
	},
};

/* This will be corrected as we get more formats */
static bool isi_camera_packing_supported(const struct soc_mbus_pixelfmt *fmt)
{
	return	fmt->packing == SOC_MBUS_PACKING_NONE ||
		(fmt->bits_per_sample == 8 &&
		 fmt->packing == SOC_MBUS_PACKING_2X8_PADHI) ||
		(fmt->bits_per_sample > 8 &&
		 fmt->packing == SOC_MBUS_PACKING_EXTEND16);
}

#define ISI_BUS_PARAM (V4L2_MBUS_MASTER |	\
		V4L2_MBUS_HSYNC_ACTIVE_HIGH |	\
		V4L2_MBUS_HSYNC_ACTIVE_LOW |	\
		V4L2_MBUS_VSYNC_ACTIVE_HIGH |	\
		V4L2_MBUS_VSYNC_ACTIVE_LOW |	\
		V4L2_MBUS_PCLK_SAMPLE_RISING |	\
		V4L2_MBUS_PCLK_SAMPLE_FALLING |	\
		V4L2_MBUS_DATA_ACTIVE_HIGH)

static int isi_camera_try_bus_param(struct soc_camera_device *icd,
				    unsigned char buswidth)
{
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct atmel_isi *isi = ici->priv;
	struct v4l2_mbus_config cfg = {.type = V4L2_MBUS_PARALLEL,};
	unsigned long common_flags;
	int ret;

	ret = v4l2_subdev_call(sd, video, g_mbus_config, &cfg);
	if (!ret) {
		common_flags = soc_mbus_config_compatible(&cfg,
							  ISI_BUS_PARAM);
		if (!common_flags) {
			dev_warn(icd->parent,
				 "Flags incompatible: camera 0x%x, host 0x%x\n",
				 cfg.flags, ISI_BUS_PARAM);
			return -EINVAL;
		}
	} else if (ret != -ENOIOCTLCMD) {
		return ret;
	}

	if ((1 << (buswidth - 1)) & isi->width_flags)
		return 0;
	return -EINVAL;
}


static int isi_camera_get_formats(struct soc_camera_device *icd,
				  unsigned int idx,
				  struct soc_camera_format_xlate *xlate)
{
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	int formats = 0, ret;
	/* sensor format */
	u32 code;
	/* soc camera host format */
	const struct soc_mbus_pixelfmt *fmt;

	ret = v4l2_subdev_call(sd, video, enum_mbus_fmt, idx, &code);
	if (ret < 0)
		/* No more formats */
		return 0;

	fmt = soc_mbus_get_fmtdesc(code);
	if (!fmt) {
		dev_err(icd->parent,
			"Invalid format code #%u: %d\n", idx, code);
		return 0;
	}

	/* This also checks support for the requested bits-per-sample */
	ret = isi_camera_try_bus_param(icd, fmt->bits_per_sample);
	if (ret < 0) {
		dev_err(icd->parent,
			"Fail to try the bus parameters.\n");
		return 0;
	}

	switch (code) {
	case MEDIA_BUS_FMT_UYVY8_2X8:
	case MEDIA_BUS_FMT_VYUY8_2X8:
	case MEDIA_BUS_FMT_YUYV8_2X8:
	case MEDIA_BUS_FMT_YVYU8_2X8:
		formats++;
		if (xlate) {
			xlate->host_fmt	= &isi_camera_formats[0];
			xlate->code	= code;
			xlate++;
			dev_dbg(icd->parent, "Providing format %s using code %d\n",
				isi_camera_formats[0].name, code);
		}
		break;
	default:
		if (!isi_camera_packing_supported(fmt))
			return 0;
		if (xlate)
			dev_dbg(icd->parent,
				"Providing format %s in pass-through mode\n",
				fmt->name);
	}

	/* Generic pass-through */
	formats++;
	if (xlate) {
		xlate->host_fmt	= fmt;
		xlate->code	= code;
		xlate++;
	}

	return formats;
}

static int isi_camera_add_device(struct soc_camera_device *icd)
{
	dev_dbg(icd->parent, "Atmel ISI Camera driver attached to camera %d\n",
		 icd->devnum);

	return 0;
}

static void isi_camera_remove_device(struct soc_camera_device *icd)
{
	dev_dbg(icd->parent, "Atmel ISI Camera driver detached from camera %d\n",
		 icd->devnum);
}

/* Called with .host_lock held */
static int isi_camera_clock_start(struct soc_camera_host *ici)
{
	struct atmel_isi *isi = ici->priv;
	int ret;

	ret = clk_prepare_enable(isi->pclk);
	if (ret)
		return ret;

	if (!IS_ERR(isi->mck)) {
		ret = clk_prepare_enable(isi->mck);
		if (ret) {
			clk_disable_unprepare(isi->pclk);
			return ret;
		}
	}

	return 0;
}

/* Called with .host_lock held */
static void isi_camera_clock_stop(struct soc_camera_host *ici)
{
	struct atmel_isi *isi = ici->priv;

	if (!IS_ERR(isi->mck))
		clk_disable_unprepare(isi->mck);
	clk_disable_unprepare(isi->pclk);
}

static unsigned int isi_camera_poll(struct file *file, poll_table *pt)
{
	struct soc_camera_device *icd = file->private_data;

	return vb2_poll(&icd->vb2_vidq, file, pt);
}

static int isi_camera_querycap(struct soc_camera_host *ici,
			       struct v4l2_capability *cap)
{
	strcpy(cap->driver, "atmel-isi");
	strcpy(cap->card, "Atmel Image Sensor Interface");
	cap->device_caps = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING;
	cap->capabilities = cap->device_caps | V4L2_CAP_DEVICE_CAPS;

	return 0;
}

static int isi_camera_set_bus_param(struct soc_camera_device *icd)
{
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct atmel_isi *isi = ici->priv;
	struct v4l2_mbus_config cfg = {.type = V4L2_MBUS_PARALLEL,};
	unsigned long common_flags;
	int ret;
	u32 cfg1 = 0;

	ret = v4l2_subdev_call(sd, video, g_mbus_config, &cfg);
	if (!ret) {
		common_flags = soc_mbus_config_compatible(&cfg,
							  ISI_BUS_PARAM);
		if (!common_flags) {
			dev_warn(icd->parent,
				 "Flags incompatible: camera 0x%x, host 0x%x\n",
				 cfg.flags, ISI_BUS_PARAM);
			return -EINVAL;
		}
	} else if (ret != -ENOIOCTLCMD) {
		return ret;
	} else {
		common_flags = ISI_BUS_PARAM;
	}
	dev_dbg(icd->parent, "Flags cam: 0x%x host: 0x%x common: 0x%lx\n",
		cfg.flags, ISI_BUS_PARAM, common_flags);

	/* Make choises, based on platform preferences */
	if ((common_flags & V4L2_MBUS_HSYNC_ACTIVE_HIGH) &&
	    (common_flags & V4L2_MBUS_HSYNC_ACTIVE_LOW)) {
		if (isi->pdata.hsync_act_low)
			common_flags &= ~V4L2_MBUS_HSYNC_ACTIVE_HIGH;
		else
			common_flags &= ~V4L2_MBUS_HSYNC_ACTIVE_LOW;
	}

	if ((common_flags & V4L2_MBUS_VSYNC_ACTIVE_HIGH) &&
	    (common_flags & V4L2_MBUS_VSYNC_ACTIVE_LOW)) {
		if (isi->pdata.vsync_act_low)
			common_flags &= ~V4L2_MBUS_VSYNC_ACTIVE_HIGH;
		else
			common_flags &= ~V4L2_MBUS_VSYNC_ACTIVE_LOW;
	}

	if ((common_flags & V4L2_MBUS_PCLK_SAMPLE_RISING) &&
	    (common_flags & V4L2_MBUS_PCLK_SAMPLE_FALLING)) {
		if (isi->pdata.pclk_act_falling)
			common_flags &= ~V4L2_MBUS_PCLK_SAMPLE_RISING;
		else
			common_flags &= ~V4L2_MBUS_PCLK_SAMPLE_FALLING;
	}

	cfg.flags = common_flags;
	ret = v4l2_subdev_call(sd, video, s_mbus_config, &cfg);
	if (ret < 0 && ret != -ENOIOCTLCMD) {
		dev_dbg(icd->parent, "camera s_mbus_config(0x%lx) returned %d\n",
			common_flags, ret);
		return ret;
	}

	/* set bus param for ISI */
	if (common_flags & V4L2_MBUS_HSYNC_ACTIVE_LOW)
		cfg1 |= ISI_CFG1_HSYNC_POL_ACTIVE_LOW;
	if (common_flags & V4L2_MBUS_VSYNC_ACTIVE_LOW)
		cfg1 |= ISI_CFG1_VSYNC_POL_ACTIVE_LOW;
	if (common_flags & V4L2_MBUS_PCLK_SAMPLE_FALLING)
		cfg1 |= ISI_CFG1_PIXCLK_POL_ACTIVE_FALLING;

	if (isi->pdata.has_emb_sync)
		cfg1 |= ISI_CFG1_EMB_SYNC;
	if (isi->pdata.full_mode)
		cfg1 |= ISI_CFG1_FULL_MODE;

	cfg1 |= ISI_CFG1_THMASK_BEATS_16;

	isi_writel(isi, ISI_CTRL, ISI_CTRL_DIS);
	isi_writel(isi, ISI_CFG1, cfg1);

	return 0;
}

static struct soc_camera_host_ops isi_soc_camera_host_ops = {
	.owner		= THIS_MODULE,
	.add		= isi_camera_add_device,
	.remove		= isi_camera_remove_device,
	.clock_start	= isi_camera_clock_start,
	.clock_stop	= isi_camera_clock_stop,
	.set_fmt	= isi_camera_set_fmt,
	.try_fmt	= isi_camera_try_fmt,
	.get_formats	= isi_camera_get_formats,
	.init_videobuf2	= isi_camera_init_videobuf,
	.poll		= isi_camera_poll,
	.querycap	= isi_camera_querycap,
	.set_bus_param	= isi_camera_set_bus_param,
};

/* -----------------------------------------------------------------------*/
static int atmel_isi_remove(struct platform_device *pdev)
{
	struct soc_camera_host *soc_host = to_soc_camera_host(&pdev->dev);
	struct atmel_isi *isi = container_of(soc_host,
					struct atmel_isi, soc_host);

	soc_camera_host_unregister(soc_host);
	vb2_dma_contig_cleanup_ctx(isi->alloc_ctx);
	dma_free_coherent(&pdev->dev,
			sizeof(struct fbd) * MAX_BUFFER_NUM,
			isi->p_fb_descriptors,
			isi->fb_descriptors_phys);

	return 0;
}

static int atmel_isi_probe_dt(struct atmel_isi *isi,
			struct platform_device *pdev)
{
	struct device_node *np= pdev->dev.of_node;
	struct v4l2_of_endpoint ep;
	int err;

	/* Default settings for ISI */
	isi->pdata.full_mode = 1;
	isi->pdata.mck_hz = ISI_DEFAULT_MCLK_FREQ;
	isi->pdata.frate = ISI_CFG1_FRATE_CAPTURE_ALL;

	np = of_graph_get_next_endpoint(np, NULL);
	if (!np) {
		dev_err(&pdev->dev, "Could not find the endpoint\n");
		return -EINVAL;
	}

	err = v4l2_of_parse_endpoint(np, &ep);
	if (err) {
		dev_err(&pdev->dev, "Could not parse the endpoint\n");
		goto err_probe_dt;
	}

	switch (ep.bus.parallel.bus_width) {
	case 8:
		isi->pdata.data_width_flags = ISI_DATAWIDTH_8;
		break;
	case 10:
		isi->pdata.data_width_flags =
				ISI_DATAWIDTH_8 | ISI_DATAWIDTH_10;
		break;
	default:
		dev_err(&pdev->dev, "Unsupported bus width: %d\n",
				ep.bus.parallel.bus_width);
		err = -EINVAL;
		goto err_probe_dt;
	}

err_probe_dt:
	of_node_put(np);

	return err;
}

static int atmel_isi_probe(struct platform_device *pdev)
{
	unsigned int irq;
	struct atmel_isi *isi;
	struct resource *regs;
	int ret, i;
	struct device *dev = &pdev->dev;
	struct soc_camera_host *soc_host;
	struct isi_platform_data *pdata;

	pdata = dev->platform_data;
	if ((!pdata || !pdata->data_width_flags) && !pdev->dev.of_node) {
		dev_err(&pdev->dev,
			"No config available for Atmel ISI\n");
		return -EINVAL;
	}

	isi = devm_kzalloc(&pdev->dev, sizeof(struct atmel_isi), GFP_KERNEL);
	if (!isi) {
		dev_err(&pdev->dev, "Can't allocate interface!\n");
		return -ENOMEM;
	}

	isi->pclk = devm_clk_get(&pdev->dev, "isi_clk");
	if (IS_ERR(isi->pclk))
		return PTR_ERR(isi->pclk);

	if (pdata) {
		memcpy(&isi->pdata, pdata, sizeof(isi->pdata));
	} else {
		ret = atmel_isi_probe_dt(isi, pdev);
		if (ret)
			return ret;
	}

	isi->active = NULL;
	spin_lock_init(&isi->lock);
	INIT_LIST_HEAD(&isi->video_buffer_list);
	INIT_LIST_HEAD(&isi->dma_desc_head);

	/* ISI_MCK is the sensor master clock. It should be handled by the
	 * sensor driver directly, as the ISI has no use for that clock. Make
	 * the clock optional here while platforms transition to the correct
	 * model.
	 */
	isi->mck = devm_clk_get(dev, "isi_mck");
	if (!IS_ERR(isi->mck)) {
		/* Set ISI_MCK's frequency, it should be faster than pixel
		 * clock.
		 */
		ret = clk_set_rate(isi->mck, isi->pdata.mck_hz);
		if (ret < 0)
			return ret;
	}

	isi->p_fb_descriptors = dma_alloc_coherent(&pdev->dev,
				sizeof(struct fbd) * MAX_BUFFER_NUM,
				&isi->fb_descriptors_phys,
				GFP_KERNEL);
	if (!isi->p_fb_descriptors) {
		dev_err(&pdev->dev, "Can't allocate descriptors!\n");
		return -ENOMEM;
	}

	for (i = 0; i < MAX_BUFFER_NUM; i++) {
		isi->dma_desc[i].p_fbd = isi->p_fb_descriptors + i;
		isi->dma_desc[i].fbd_phys = isi->fb_descriptors_phys +
					i * sizeof(struct fbd);
		list_add(&isi->dma_desc[i].list, &isi->dma_desc_head);
	}

	isi->alloc_ctx = vb2_dma_contig_init_ctx(&pdev->dev);
	if (IS_ERR(isi->alloc_ctx)) {
		ret = PTR_ERR(isi->alloc_ctx);
		goto err_alloc_ctx;
	}

	regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	isi->regs = devm_ioremap_resource(&pdev->dev, regs);
	if (IS_ERR(isi->regs)) {
		ret = PTR_ERR(isi->regs);
		goto err_ioremap;
	}

	if (isi->pdata.data_width_flags & ISI_DATAWIDTH_8)
		isi->width_flags = 1 << 7;
	if (isi->pdata.data_width_flags & ISI_DATAWIDTH_10)
		isi->width_flags |= 1 << 9;

	isi_writel(isi, ISI_CTRL, ISI_CTRL_DIS);

	irq = platform_get_irq(pdev, 0);
	if (IS_ERR_VALUE(irq)) {
		ret = irq;
		goto err_req_irq;
	}

	ret = devm_request_irq(&pdev->dev, irq, isi_interrupt, 0, "isi", isi);
	if (ret) {
		dev_err(&pdev->dev, "Unable to request irq %d\n", irq);
		goto err_req_irq;
	}
	isi->irq = irq;

	soc_host		= &isi->soc_host;
	soc_host->drv_name	= "isi-camera";
	soc_host->ops		= &isi_soc_camera_host_ops;
	soc_host->priv		= isi;
	soc_host->v4l2_dev.dev	= &pdev->dev;
	soc_host->nr		= pdev->id;

	if (isi->pdata.asd_sizes) {
		soc_host->asd = isi->pdata.asd;
		soc_host->asd_sizes = isi->pdata.asd_sizes;
	}

	ret = soc_camera_host_register(soc_host);
	if (ret) {
		dev_err(&pdev->dev, "Unable to register soc camera host\n");
		goto err_register_soc_camera_host;
	}
	return 0;

err_register_soc_camera_host:
err_req_irq:
err_ioremap:
	vb2_dma_contig_cleanup_ctx(isi->alloc_ctx);
err_alloc_ctx:
	dma_free_coherent(&pdev->dev,
			sizeof(struct fbd) * MAX_BUFFER_NUM,
			isi->p_fb_descriptors,
			isi->fb_descriptors_phys);

	return ret;
}

static const struct of_device_id atmel_isi_of_match[] = {
	{ .compatible = "atmel,at91sam9g45-isi" },
	{ }
};
MODULE_DEVICE_TABLE(of, atmel_isi_of_match);

static struct platform_driver atmel_isi_driver = {
	.remove		= atmel_isi_remove,
	.driver		= {
		.name = "atmel_isi",
		.of_match_table = of_match_ptr(atmel_isi_of_match),
	},
};

module_platform_driver_probe(atmel_isi_driver, atmel_isi_probe);

MODULE_AUTHOR("Josh Wu <josh.wu@atmel.com>");
MODULE_DESCRIPTION("The V4L2 driver for Atmel Linux");
MODULE_LICENSE("GPL");
MODULE_SUPPORTED_DEVICE("video");
