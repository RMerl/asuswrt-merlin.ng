// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 - 2022, Xilinx Inc.
 * Copyright (C) 2022 - 2023, Advanced Micro Devices, Inc.
 *
 * Xilinx displayport(DP) Tx Subsytem driver
 */

#include <common.h>
#include <clk.h>
#include <cpu_func.h>
#include <dm.h>
#include <errno.h>
#include <generic-phy.h>
#include <stdlib.h>
#include <video.h>
#include <wait_bit.h>
#include <dm/device_compat.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <dm/device_compat.h>
#include <asm/global_data.h>

#include "zynqmp_dpsub.h"

DECLARE_GLOBAL_DATA_PTR;

/* Maximum supported resolution */
#define WIDTH				1024
#define HEIGHT				768

static struct dp_dma dp_dma;
static struct dp_dma_descriptor cur_desc __aligned(256);

static void dma_init_video_descriptor(struct udevice *dev)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	struct dp_dma_frame_buffer *frame_buffer = &dp_sub->frame_buffer;

	cur_desc.control = DPDMA_DESC_PREAMBLE | DPDMA_DESC_IGNR_DONE |
			   DPDMA_DESC_LAST_FRAME;
	cur_desc.dscr_id = 0;
	cur_desc.xfer_size = frame_buffer->size;
	cur_desc.line_size_stride = ((frame_buffer->stride >> 4) <<
				     DPDMA_DESCRIPTOR_LINE_SIZE_STRIDE_SHIFT) |
				     (frame_buffer->line_size);
	cur_desc.addr_ext = (((u32)(frame_buffer->address >>
			     DPDMA_DESCRIPTOR_SRC_ADDR_WIDTH) <<
			     DPDMA_DESCRIPTOR_ADDR_EXT_SRC_ADDR_EXT_SHIFT) |
			     (upper_32_bits((u64)&cur_desc)));
	cur_desc.next_desr = lower_32_bits((u64)&cur_desc);
	cur_desc.src_addr = lower_32_bits((u64)gd->fb_base);
}

static void dma_set_descriptor_address(struct udevice *dev)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);

	flush_dcache_range((u64)&cur_desc,
			   ALIGN(((u64)&cur_desc + sizeof(cur_desc)),
				 CONFIG_SYS_CACHELINE_SIZE));
	writel(upper_32_bits((u64)&cur_desc), dp_sub->dp_dma->base_addr +
	       DPDMA_CH3_DSCR_STRT_ADDRE);
	writel(lower_32_bits((u64)&cur_desc), dp_sub->dp_dma->base_addr +
	       DPDMA_CH3_DSCR_STRT_ADDR);
}

static void dma_setup_channel(struct udevice *dev)
{
	dma_init_video_descriptor(dev);
	dma_set_descriptor_address(dev);
}

static void dma_set_channel_state(struct udevice *dev)
{
	u32 mask = 0, regval = 0;
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);

	mask = DPDMA_CH_CNTL_EN_MASK | DPDMA_CH_CNTL_PAUSE_MASK;
	regval = DPDMA_CH_CNTL_EN_MASK;

	clrsetbits_le32(dp_sub->dp_dma->base_addr + DPDMA_CH3_CNTL,
			mask, regval);
}

static void dma_trigger(struct udevice *dev)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	u32 trigger;

	trigger = DPDMA_GBL_TRG_CH3_MASK;
	dp_sub->dp_dma->gfx.trigger_status = DPDMA_TRIGGER_DONE;
	writel(trigger, dp_sub->dp_dma->base_addr + DPDMA_GBL);
}

static void dma_vsync_intr_handler(struct udevice *dev)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);

	dma_setup_channel(dev);
	dma_set_channel_state(dev);
	dma_trigger(dev);

	/* Clear VSync Interrupt */
	writel(DPDMA_ISR_VSYNC_INT_MASK, dp_sub->dp_dma->base_addr + DPDMA_ISR);
}

/**
 * wait_phy_ready() - Wait for the DisplayPort PHY to come out of reset
 * @dev:  The DP device
 *
 * Return: 0 if wait succeeded, -ve if error occurred
 */
static int wait_phy_ready(struct udevice *dev)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	u32 timeout = 100, phy_status;
	u8 phy_ready_mask =  DP_PHY_STATUS_RESET_LANE_0_DONE_MASK |
			     DP_PHY_STATUS_GT_PLL_LOCK_MASK;

	/* Wait until the PHY is ready. */
	do {
		udelay(20);
		phy_status = readl(dp_sub->base_addr + DP_PHY_STATUS);
		phy_status &= phy_ready_mask;
		/* Protect against an infinite loop. */
		if (!timeout--)
			return -ETIMEDOUT;
	} while (phy_status != phy_ready_mask);

	return 0;
}

static int init_dp_tx(struct udevice *dev)
{
	u32 status, phyval, regval, rate;
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);

	phyval = readl(dp_sub->base_addr + DP_PHY_CONFIG);
	writel(DP_SOFT_RESET_EN, dp_sub->base_addr + DP_SOFT_RESET);
	status = readl(dp_sub->base_addr + DP_SOFT_RESET);
	writel(DP_DISABLE, dp_sub->base_addr + DP_ENABLE);

	regval = (readl(dp_sub->base_addr + DP_AUX_CLK_DIVIDER) &
		  ~DP_AUX_CLK_DIVIDER_VAL_MASK) |
		  (60 << 8) |
		  (dp_sub->clock / 1000000);
	writel(regval, dp_sub->base_addr + DP_AUX_CLK_DIVIDER);

	writel(DP_PHY_CLOCK_SELECT_540GBPS, dp_sub->base_addr + DP_PHY_CLOCK_SELECT);

	regval = phyval & ~DP_PHY_CONFIG_GT_ALL_RESET_MASK;
	writel(regval, dp_sub->base_addr + DP_PHY_CONFIG);
	status = wait_phy_ready(dev);
	if (status)
		return -EINVAL;

	writel(DP_ENABLE, dp_sub->base_addr + DP_ENABLE);

	rate = ~DP_INTR_HPD_PULSE_DETECTED_MASK & ~DP_INTR_HPD_EVENT_MASK
		& ~DP_INTR_HPD_IRQ_MASK;
	writel(rate, dp_sub->base_addr + DP_INTR_MASK);
	return 0;
}

static int set_nonlive_gfx_format(struct udevice *dev, enum av_buf_video_format format)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	struct av_buf_vid_attribute *ptr = (struct av_buf_vid_attribute *)avbuf_supported_formats;

	while (1) {
		dev_dbg(dev, "Format %d\n", ptr->video_format);

		if (!ptr->video_format)
			return -EINVAL;

		if (ptr->video_format == format) {
			dp_sub->non_live_graphics = ptr;
			break;
		}
		ptr++;
	}
	dev_dbg(dev, "Video format found. BPP %d\n", dp_sub->non_live_graphics->bpp);
	return 0;
}

/* DP dma setup */
static void set_qos(struct udevice *dev, u8 qos)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	u8 index;
	u32 regval = 0, mask;

	regval = (((u32)qos << DPDMA_CH_CNTL_QOS_DATA_RD_SHIFT) |
		 ((u32)qos << DPDMA_CH_CNTL_QOS_DSCR_RD_SHIFT) |
		 ((u32)qos << DPDMA_CH_CNTL_QOS_DSCR_WR_SHIFT));

	mask = DPDMA_CH_CNTL_QOS_DATA_RD_MASK |
	       DPDMA_CH_CNTL_QOS_DSCR_RD_MASK |
	       DPDMA_CH_CNTL_QOS_DSCR_WR_MASK;
	for (index = 0; index <= DPDMA_AUDIO_CHANNEL1; index++) {
		clrsetbits_le32(dp_sub->dp_dma->base_addr +
				DPDMA_CH0_CNTL +
				(DPDMA_CH_OFFSET * (u32)index),
				mask, regval);
	}
}

static void enable_gfx_buffers(struct udevice *dev, u8 enable)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	u32 regval = 0;

	regval = (0xF << AVBUF_CHBUF3_BURST_LEN_SHIFT) |
			 AVBUF_CHBUF3_FLUSH_MASK;
	writel(regval, dp_sub->base_addr + AVBUF_CHBUF3);
	if (enable) {
		regval = (0xF << AVBUF_CHBUF3_BURST_LEN_SHIFT) |
				 AVBUF_CHBUF0_EN_MASK;
		writel(regval, dp_sub->base_addr + AVBUF_CHBUF3);
	}
}

static void avbuf_video_select(struct udevice *dev, enum av_buf_video_stream vid_stream,
			       enum av_buf_gfx_stream gfx_stream)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);

	dp_sub->av_mode.video_src = vid_stream;
	dp_sub->av_mode.gfx_src = gfx_stream;

	clrsetbits_le32(dp_sub->base_addr +
			AVBUF_BUF_OUTPUT_AUD_VID_SELECT,
			AVBUF_BUF_OUTPUT_AUD_VID_SELECT_VID_STREAM2_SEL_MASK |
			AVBUF_BUF_OUTPUT_AUD_VID_SELECT_VID_STREAM1_SEL_MASK,
			vid_stream | gfx_stream);
}

static void config_gfx_pipeline(struct udevice *dev)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	u16 *csc_matrix, *offset_matrix;
	u32 regval = 0, index = 0, *scaling_factors = NULL;
	u16 rgb_coeffs[] = { 0x1000, 0x0000, 0x0000,
			     0x0000, 0x1000, 0x0000,
			     0x0000, 0x0000, 0x1000 };
	u16 rgb_offset[] = { 0x0000, 0x0000, 0x0000 };
	struct av_buf_vid_attribute *video = dp_sub->non_live_graphics;

	scaling_factors = video->sf;

	clrsetbits_le32(dp_sub->base_addr + AVBUF_BUF_FORMAT,
			AVBUF_BUF_FORMAT_NL_GRAPHX_FORMAT_MASK,
			(video->value) << AVBUF_BUF_FORMAT_NL_GRAPHX_FORMAT_SHIFT);

	for (index = 0; index < 3; index++) {
		writel(scaling_factors[index], dp_sub->base_addr +
		       AVBUF_BUF_GRAPHICS_COMP0_SCALE_FACTOR + (index * 4));
	}
	regval = (video->is_rgb << AVBUF_V_BLEND_LAYER0_CONTROL_RGB_MODE_SHIFT) |
								video->sampling_en;
	writel(regval, dp_sub->base_addr + AVBUF_V_BLEND_LAYER1_CONTROL);

	if (video->is_rgb) {
		csc_matrix = rgb_coeffs;
		offset_matrix = rgb_offset;
	}
	/* Program Colorspace conversion coefficients */
	for (index = 9; index < 12; index++) {
		writel(offset_matrix[index - 9], dp_sub->base_addr +
		       AVBUF_V_BLEND_IN2CSC_COEFF0 + (index * 4));
	}

	/* Program Colorspace conversion matrix */
	for (index = 0; index < 9; index++) {
		writel(csc_matrix[index], dp_sub->base_addr +
		       AVBUF_V_BLEND_IN2CSC_COEFF0 + (index * 4));
	}
}

static void set_blender_alpha(struct udevice *dev, u8 alpha, u8 enable)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	u32 regval;

	regval = enable;
	regval |= alpha << AVBUF_V_BLEND_SET_GLOBAL_ALPHA_REG_VALUE_SHIFT;
	writel(regval, dp_sub->base_addr +
	       AVBUF_V_BLEND_SET_GLOBAL_ALPHA_REG);
}

static void config_output_video(struct udevice *dev)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	u32 regval = 0, index;
	u16 rgb_coeffs[] = { 0x1000, 0x0000, 0x0000,
			     0x0000, 0x1000, 0x0000,
			     0x0000, 0x0000, 0x1000 };
	u16 rgb_offset[] = { 0x0000, 0x0000, 0x0000 };
	u16 *matrix_coeff = rgb_coeffs, *matrix_offset = rgb_offset;

	struct av_buf_vid_attribute *output_video = dp_sub->non_live_graphics;

	regval |= output_video->sampling_en <<
		  AVBUF_V_BLEND_OUTPUT_VID_FORMAT_EN_DOWNSAMPLE_SHIFT;
	regval |= output_video->value;
	writel(regval, dp_sub->base_addr + AVBUF_V_BLEND_OUTPUT_VID_FORMAT);

	for (index = 0; index < 9; index++) {
		writel(matrix_coeff[index], dp_sub->base_addr +
		       AVBUF_V_BLEND_RGB2YCBCR_COEFF0 + (index * 4));
	}

	for (index = 0; index < 3; index++) {
		writel((matrix_offset[index] <<
			AVBUF_V_BLEND_LUMA_IN1CSC_OFFSET_POST_OFFSET_SHIFT),
			dp_sub->base_addr +
			AVBUF_V_BLEND_LUMA_OUTCSC_OFFSET
			+ (index * 4));
	}

	set_blender_alpha(dev, 0, 0);
}

static void config_msa_sync_clk_mode(struct udevice *dev, u8 enable)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	struct main_stream_attributes *msa_config;

	msa_config = &dp_sub->msa_config;
	msa_config->synchronous_clock_mode = enable;

	if (enable == 1) {
		msa_config->misc0 |= (1 <<
				     DP_MAIN_STREAM_MISC0_COMPONENT_FORMAT_SHIFT);
	} else {
		msa_config->misc0 &= ~(1 <<
				      DP_MAIN_STREAM_MISC0_COMPONENT_FORMAT_SHIFT);
	}
}

static void av_buf_soft_reset(struct udevice *dev)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);

	writel(AVBUF_BUF_SRST_REG_VID_RST_MASK,
	       dp_sub->base_addr + AVBUF_BUF_SRST_REG);
	writel(0, dp_sub->base_addr + AVBUF_BUF_SRST_REG);
}

static void set_video_clk_source(struct udevice *dev, u8 video_clk, u8 audio_clk)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	u32 regval = 0;

	if (dp_sub->av_mode.video_src != AVBUF_VIDSTREAM1_LIVE &&
	    dp_sub->av_mode.gfx_src != AVBUF_VIDSTREAM2_LIVE_GFX) {
		regval = 1 << AVBUF_BUF_AUD_VID_CLK_SOURCE_VID_TIMING_SRC_SHIFT;
	} else if (dp_sub->av_mode.video_src == AVBUF_VIDSTREAM1_LIVE ||
		   dp_sub->av_mode.gfx_src == AVBUF_VIDSTREAM2_LIVE_GFX) {
		video_clk = AVBUF_PL_CLK;
	}

	regval |= (video_clk << AVBUF_BUF_AUD_VID_CLK_SOURCE_VID_CLK_SRC_SHIFT) |
		  (audio_clk << AVBUF_BUF_AUD_VID_CLK_SOURCE_AUD_CLK_SRC_SHIFT);
	writel(regval, dp_sub->base_addr + AVBUF_BUF_AUD_VID_CLK_SOURCE);

	av_buf_soft_reset(dev);
}

static int init_dpdma_subsys(struct udevice *dev)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);

	dp_sub->dp_dma->base_addr = DPDMA_BASE_ADDRESS;
	dp_sub->dp_dma->gfx.channel.cur = NULL;
	dp_sub->dp_dma->gfx.trigger_status = DPDMA_TRIGGER_DONE;

	set_qos(dev, 11);
	return 0;
}

/**
 * is_dp_connected() - Check if there is a connected RX device
 * @dev: The DP device
 *
 *
 * Return: true if a connected RX device was detected, false otherwise
 */
static bool is_dp_connected(struct udevice *dev)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	u32 status;
	u8 retries = 0;

	do {
		status = readl(dp_sub->base_addr +
				DP_INTERRUPT_SIG_STATE)
				& DP_INTERRUPT_SIG_STATE_HPD_STATE_MASK;

		if (retries > DP_IS_CONNECTED_MAX_TIMEOUT_COUNT)
			return 0;

		retries++;
		udelay(1000);
	} while (status == 0);

	return 1;
}

/**
 * aux_wait_ready() -  Wait until another request is no longer in progress
 * @dev: The DP device
 *
 * Return: 0 if wait succeeded, -ve if error occurred
 */
static int aux_wait_ready(struct udevice *dev)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	u32 status, timeout = 100;

	do {
		status = readl(dp_sub->base_addr +
			       DP_INTERRUPT_SIG_STATE);
		if (!timeout--)
			return -ETIMEDOUT;

		udelay(20);
	} while (status & DP_REPLY_STATUS_REPLY_IN_PROGRESS_MASK);

	return 0;
}

/**
 * aux_wait_reply() - Wait for reply on AUX channel
 * @dev: The DP device
 *
 * Wait for a reply indicating that the most recent AUX request
 * has been received by the RX device.
 *
 * Return: 0 if wait succeeded, -ve if error occurred
 */
static int aux_wait_reply(struct udevice *dev)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	u32 timeout = DP_AUX_MAX_WAIT, status;

	while (timeout > 0) {
		status = readl(dp_sub->base_addr + DP_REPLY_STATUS);
		if (status & DP_REPLY_STATUS_REPLY_ERROR_MASK)
			return -ETIMEDOUT;

		if ((status & DP_REPLY_STATUS_REPLY_RECEIVED_MASK) &&
		    !(status & DP_REPLY_STATUS_REQUEST_IN_PROGRESS_MASK) &&
		    !(status & DP_REPLY_STATUS_REPLY_IN_PROGRESS_MASK)) {
			return 0;
		}
		timeout--;
		udelay(20);
	}
	return -ETIMEDOUT;
}

/**
 * aux_request_send() - Send request on the AUX channel
 * @dev:     The DP device
 * @request: The request to send
 *
 * Submit the supplied AUX request to the RX device over the AUX
 * channel by writing the command, the destination address, (the write buffer
 * for write commands), and the data size to the DisplayPort TX core.
 *
 * This is the lower-level sending routine, which is called by aux_request().
 *
 * Return: 0 if request was sent successfully, -ve on error
 */
static int aux_request_send(struct udevice *dev, struct aux_transaction *request)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	u32 timeout_count = 0, status;
	u8 index;

	do {
		status = readl(dp_sub->base_addr +
			       DP_REPLY_STATUS);

		udelay(20);
		timeout_count++;
		if (timeout_count >= DP_AUX_MAX_TIMEOUT_COUNT)
			return -ETIMEDOUT;

	} while ((status & DP_REPLY_STATUS_REQUEST_IN_PROGRESS_MASK) ||
		(status & DP_REPLY_STATUS_REPLY_IN_PROGRESS_MASK));
	/* Set the address for the request. */
	writel(request->address, dp_sub->base_addr + DP_AUX_ADDRESS);

	if (request->cmd_code == DP_AUX_CMD_WRITE ||
	    request->cmd_code == DP_AUX_CMD_I2C_WRITE ||
	    request->cmd_code == DP_AUX_CMD_I2C_WRITE_MOT) {
		/* Feed write data into the DisplayPort TX core's write FIFO. */
		for (index = 0; index < request->num_bytes; index++) {
			writel(request->data[index],
			       dp_sub->base_addr +
			       DP_AUX_WRITE_FIFO);
		}
	}

	status = ((request->cmd_code << DP_AUX_CMD_SHIFT) |
		 ((request->num_bytes - 1) &
		 DP_AUX_CMD_NBYTES_TRANSFER_MASK));

	/* Submit the command and the data size. */
	writel(((request->cmd_code << DP_AUX_CMD_SHIFT) |
		((request->num_bytes - 1) & DP_AUX_CMD_NBYTES_TRANSFER_MASK)),
		dp_sub->base_addr + DP_AUX_CMD);

	/* Check for a reply from the RX device to the submitted request. */
	status = aux_wait_reply(dev);
	if (status)
		/* Waiting for a reply timed out. */
		return -ETIMEDOUT;

	/* Analyze the reply. */
	status = readl(dp_sub->base_addr + DP_AUX_REPLY_CODE);
	if (status == DP_AUX_REPLY_CODE_DEFER ||
	    status == DP_AUX_REPLY_CODE_I2C_DEFER) {
		/* The request was deferred. */
		return -EAGAIN;
	} else if (status == DP_AUX_REPLY_CODE_NACK ||
		   status == DP_AUX_REPLY_CODE_I2C_NACK) {
		/* The request was not acknowledged. */
		return -EIO;
	}

	/* The request was acknowledged. */
	if (request->cmd_code == DP_AUX_CMD_READ ||
	    request->cmd_code == DP_AUX_CMD_I2C_READ ||
	    request->cmd_code == DP_AUX_CMD_I2C_READ_MOT) {
		/* Wait until all data has been received. */
		timeout_count = 0;
		do {
			status = readl(dp_sub->base_addr +
				       DP_REPLY_DATA_COUNT);
			udelay(100);
			timeout_count++;
			if (timeout_count >= DP_AUX_MAX_TIMEOUT_COUNT)
				return -ETIMEDOUT;
		} while (status != request->num_bytes);

		/* Obtain the read data from the reply FIFO. */
		for (index = 0; index < request->num_bytes; index++) {
			request->data[index] = readl(dp_sub->base_addr +
						     DP_AUX_REPLY_DATA);
		}
	}
	return 0;
}

/**
 * aux_request() - Submit request on the AUX channel
 * @dev:     The DP device
 * @request: The request to submit
 *
 * Submit the supplied AUX request to the RX device over the AUX
 * channel. If waiting for a reply times out, or if the DisplayPort TX core
 * indicates that the request was deferred, the request is sent again (up to a
 * maximum specified by DP_AUX_MAX_DEFER_COUNT|DP_AUX_MAX_TIMEOUT_COUNT).
 *
 * Return: 0 if request was submitted successfully, -ve on error
 */
static int aux_request(struct udevice *dev, struct aux_transaction *request)
{
	u32 status, defer_count = 0, timeout_count = 0;

	do {
		status = aux_wait_ready(dev);
		if (status) {
			/* The RX device isn't ready yet. */
			timeout_count++;
			continue;
		}
		/* Send the request. */
		status = aux_request_send(dev, request);
		if (status == -EAGAIN) {
			/* The request was deferred. */
			defer_count++;
		} else if (status == -ETIMEDOUT) {
			/* Waiting for a reply timed out. */
			timeout_count++;
		} else {
			return status;
		}

		udelay(100);
	} while ((defer_count < DP_AUX_MAX_DEFER_COUNT) &&
		(timeout_count < DP_AUX_MAX_TIMEOUT_COUNT));

	/* The request was not successfully received by the RX device. */
	return -ETIMEDOUT;
}

/**
 * aux_common() - Common (read/write) AUX communication transmission
 * @dev:       The DP device
 * @cmd_type:  Command code of the transaction
 * @address:   The DPCD address of the transaction
 * @num_bytes: Number of bytes in the payload data
 * @data:      The payload data of the AUX command
 *
 * Common sequence of submitting an AUX command for AUX read, AUX write,
 * I2C-over-AUX read, and I2C-over-AUX write transactions. If required, the
 * reads and writes are split into multiple requests, each acting on a maximum
 * of 16 bytes.
 *
 * Return: 0 if OK, -ve on error
 */
static int aux_common(struct udevice *dev, u32 cmd_type, u32 address,
		      u32 num_bytes, u8 *data)
{
	u32 status, bytes_left;
	struct aux_transaction request;

	if (!is_dp_connected(dev))
		return -ENODEV;

	/*
	 * Set the start address for AUX transactions. For I2C transactions,
	 * this is the address of the I2C bus.
	 */
	request.address = address;
	bytes_left = num_bytes;
	while (bytes_left > 0) {
		request.cmd_code = cmd_type;

		if (cmd_type == DP_AUX_CMD_READ ||
		    cmd_type == DP_AUX_CMD_WRITE) {
			/* Increment address for normal AUX transactions. */
			request.address = address + (num_bytes - bytes_left);
		}

		/* Increment the pointer to the supplied data buffer. */
		request.data = &data[num_bytes - bytes_left];

		if (bytes_left > 16)
			request.num_bytes = 16;
		else
			request.num_bytes = bytes_left;

		bytes_left -= request.num_bytes;

		if (cmd_type == DP_AUX_CMD_I2C_READ && bytes_left > 0) {
			/*
			 * Middle of a transaction I2C read request. Override
			 * the command code that was set to CmdType.
			 */
			request.cmd_code = DP_AUX_CMD_I2C_READ_MOT;
		} else if (cmd_type == DP_AUX_CMD_I2C_WRITE && bytes_left > 0) {
			/*
			 * Middle of a transaction I2C write request. Override
			 * the command code that was set to CmdType.
			 */
			request.cmd_code = DP_AUX_CMD_I2C_WRITE_MOT;
		}

		status = aux_request(dev, &request);
		if (status)
			return status;
	}
	return 0;
}

/**
 * aux_write() - Issue AUX write request
 * @dev:            The DP device
 * @dpcd_address:   The DPCD address to write to
 * @bytes_to_write: Number of bytes to write
 * @write_data:     Buffer containig data to be written
 *
 * Issue a write request over the AUX channel that will write to
 * the RX device's DisplayPort Configuration data (DPCD) address space. The
 * write message will be divided into multiple transactions which write a
 * maximum of 16 bytes each.
 *
 * Return: 0 if write operation was successful, -ve on error
 */
static int aux_write(struct udevice *dev, u32 dpcd_address, u32 bytes_to_write,
		     void *write_data)
{
	return aux_common(dev, DP_AUX_CMD_WRITE, dpcd_address,
			  bytes_to_write, (u8 *)write_data);
}

/**
 * aux_read() - Issue AUX read request
 * @dev:           The DP device
 * @dpcd_address:  The DPCD address to read from
 * @bytes_to_read: Number of bytes to read
 * @read_data:     Buffer to receive the read data
 *
 * Issue a read request over the AUX channel that will read from the RX
 * device's DisplayPort Configuration data (DPCD) address space. The read
 * message will be divided into multiple transactions which read a maximum of
 * 16 bytes each.
 *
 * Return: 0 if read operation was successful, -ve on error
 */
static int aux_read(struct udevice *dev, u32 dpcd_address, u32 bytes_to_read, void *read_data)
{
	return aux_common(dev, DP_AUX_CMD_READ, dpcd_address,
			  bytes_to_read, (u8 *)read_data);
}

static int dp_tx_wakeup(struct udevice *dev)
{
	u32 status;
	u8 aux_data;

	aux_data = 0x1;
	status = aux_write(dev, DP_DPCD_SET_POWER_DP_PWR_VOLTAGE, 1, &aux_data);
	if (status)
		debug("! 1st power wake-up - AUX write failed.\n");
	status = aux_write(dev, DP_DPCD_SET_POWER_DP_PWR_VOLTAGE, 1, &aux_data);
	if (status)
		debug("! 2nd power wake-up - AUX write failed.\n");

	return status;
}

/**
 * enable_main_link() - Switch on main link for a device
 * @dev: The DP device
 */
static void enable_main_link(struct udevice *dev, u8 enable)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);

	/* Reset the scrambler. */
	writel(1, dp_sub->base_addr + DP_FORCE_SCRAMBLER_RESET);
	/* Enable the main stream. */
	writel(enable, dp_sub->base_addr + DP_ENABLE_MAIN_STREAM);
}

/**
 * get_rx_capabilities() - Check if capabilities of RX device are valid for TX
 *                         device
 * @dev: The DP device
 *
 * Return: 0 if the capabilities of the RX device are valid for the TX device,
 *         -ve if not, of an error occurred during capability determination
 */
static int get_rx_capabilities(struct udevice *dev)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	u8 rx_max_link_rate, rx_max_lane_count, *dpcd = NULL;
	u32 status;
	struct link_config *link_config = NULL;

	dpcd = dp_sub->dpcd_rx_caps;
	link_config = &dp_sub->link_config;

	status = aux_read(dev, DP_DPCD_RECEIVER_CAP_FIELD_START, 16, dpcd);
	if (status)
		return status;

	rx_max_link_rate = dpcd[DP_DPCD_MAX_LINK_RATE];
	rx_max_lane_count = dpcd[DP_DPCD_MAX_LANE_COUNT] & DP_DPCD_MAX_LANE_COUNT_MASK;
	link_config->max_link_rate = (rx_max_link_rate > DP_0_LINK_RATE) ?
				      DP_0_LINK_RATE : rx_max_link_rate;
	link_config->max_lane_count = (rx_max_lane_count > DP_0_LANE_COUNT) ?
				       DP_0_LANE_COUNT : rx_max_lane_count;
	link_config->support_enhanced_framing_mode = dpcd[DP_DPCD_MAX_LANE_COUNT] &
						     DP_DPCD_ENHANCED_FRAME_SUPPORT_MASK;
	link_config->support_downspread_control = dpcd[DP_DPCD_MAX_DOWNSPREAD] &
						  DP_DPCD_MAX_DOWNSPREAD_MASK;

	return 0;
}

/**
 * set_enhanced_frame_mode() - Enable/Disable enhanced frame mode
 * @dev:    The DP device
 * @enable: Flag to determine whether to enable (1) or disable (0) the enhanced
 *          frame mode
 *
 * Enable or disable the enhanced framing symbol sequence for
 * both the DisplayPort TX core and the RX device.
 *
 * Return: 0 if enabling/disabling the enhanced frame mode was successful, -ve
 *         on error
 */
static int set_enhanced_frame_mode(struct udevice *dev, u8 enable)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	u32 status;
	u8 regval;

	dp_sub->link_config.enhanced_framing_mode = enable;
	/* Write enhanced frame mode enable to the DisplayPort TX core. */
	writel(dp_sub->link_config.enhanced_framing_mode,
	       dp_sub->base_addr + DP_ENHANCED_FRAME_EN);

	/* Preserve the current RX device settings. */
	status = aux_read(dev, DP_DPCD_LANE_COUNT_SET, 0x1, &regval);
	if (status)
		return status;

	if (dp_sub->link_config.enhanced_framing_mode)
		regval |= DP_DPCD_ENHANCED_FRAME_EN_MASK;
	else
		regval &= ~DP_DPCD_ENHANCED_FRAME_EN_MASK;

	/* Write enhanced frame mode enable to the RX device. */
	return aux_write(dev, DP_DPCD_LANE_COUNT_SET, 0x1, &regval);
}

/**
 * set_lane_count() - Set the lane count
 * @dev:        The DP device
 * @lane_count: Lane count to set
 *
 * Set the number of lanes to be used by the main link for both
 * the DisplayPort TX core and the RX device.
 *
 * Return: 0 if setting the lane count was successful, -ve on error
 */
static int set_lane_count(struct udevice *dev, u8 lane_count)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	u32 status;
	u8 regval;

	dp_sub->link_config.lane_count = lane_count;
	/* Write the new lane count to the DisplayPort TX core. */
	writel(dp_sub->link_config.lane_count,
	       dp_sub->base_addr + DP_LANE_COUNT_SET);

	/* Preserve the current RX device settings. */
	status = aux_read(dev, DP_DPCD_LANE_COUNT_SET, 0x1, &regval);
	if (status)
		return status;

	regval &= ~DP_DPCD_LANE_COUNT_SET_MASK;
	regval |= dp_sub->link_config.lane_count;

	/* Write the new lane count to the RX device. */
	return aux_write(dev, DP_DPCD_LANE_COUNT_SET, 0x1, &regval);
}

/**
 * set_clk_speed() - Set DP phy clock speed
 * @dev:   The DP device
 * @speed: The clock frquency to set (one of PHY_CLOCK_SELECT_*)
 *
 * Set the clock frequency for the DisplayPort PHY corresponding to a desired
 * data rate.
 *
 * Return: 0 if setting the DP phy clock speed was successful, -ve on error
 */
static int set_clk_speed(struct udevice *dev, u32 speed)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	u32 regval;

	/* Disable the DisplayPort TX core first. */
	regval = readl(dp_sub->base_addr + DP_ENABLE);
	writel(0, dp_sub->base_addr + DP_ENABLE);

	/* Change speed of the feedback clock. */
	writel(speed, dp_sub->base_addr + DP_PHY_CLOCK_SELECT);

	/* Re-enable the DisplayPort TX core if it was previously enabled. */
	if (regval)
		writel(regval, dp_sub->base_addr + DP_ENABLE);

	/* Wait until the PHY is ready. */
	return wait_phy_ready(dev);
}

/**
 * set_link_rate() - Set the link rate
 * @dev:       The DP device
 * @link_rate: The link rate to set (one of LINK_BW_SET_*)
 *
 * Set the data rate to be used by the main link for both the DisplayPort TX
 * core and the RX device.
 *
 * Return: 0 if setting the link rate was successful, -ve on error
 */
static int set_link_rate(struct udevice *dev, u8 link_rate)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	u32 status;

	/* Write a corresponding clock frequency to the DisplayPort TX core. */
	switch (link_rate) {
	case DP_LINK_BW_SET_162GBPS:
		status = set_clk_speed(dev, DP_PHY_CLOCK_SELECT_162GBPS);
		break;
	case DP_LINK_BW_SET_270GBPS:
		status = set_clk_speed(dev, DP_PHY_CLOCK_SELECT_270GBPS);
		break;
	case DP_LINK_BW_SET_540GBPS:
		status = set_clk_speed(dev, DP_PHY_CLOCK_SELECT_540GBPS);
		break;
	default:
		status = -EINVAL;
		break;
	}
	if (status)
		return status;

	dp_sub->link_config.link_rate = link_rate;
	/* Write new link rate to the DisplayPort TX core. */
	writel(dp_sub->link_config.link_rate,
	       dp_sub->base_addr +
	       DP_LINK_BW_SET);

	/* Write new link rate to the RX device. */
	return aux_write(dev, DP_DPCD_LINK_BW_SET, 0x1,
			 &dp_sub->link_config.link_rate);
}

static int set_downspread(struct udevice *dev, u8 enable)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	u32 status;
	u8 regval;

	dp_sub->link_config.support_downspread_control = enable;
	/* Write downspread enable to the DisplayPort TX core. */
	writel(dp_sub->link_config.support_downspread_control,
	       dp_sub->base_addr + DP_DOWNSPREAD_CTRL);

	/* Preserve the current RX device settings. */
	status = aux_read(dev, DP_DPCD_DOWNSPREAD_CTRL, 0x1, &regval);
	if (status)
		return status;

	if (dp_sub->link_config.support_downspread_control)
		regval |= DP_DPCD_SPREAD_AMP_MASK;
	else
		regval &= ~DP_DPCD_SPREAD_AMP_MASK;

	/* Write downspread enable to the RX device. */
	return aux_write(dev, DP_DPCD_DOWNSPREAD_CTRL, 0x1, &regval);
}

static void set_serdes_vswing_preemp(struct udevice *dev)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	u8  index;
	u8  vs_level_rx = dp_sub->link_config.vs_level;
	u8  pe_level_rx = dp_sub->link_config.pe_level;

	for (index = 0; index < dp_sub->link_config.lane_count; index++) {
		/* Write new voltage swing levels to the TX registers. */
		writel(vs[pe_level_rx][vs_level_rx], (ulong)SERDES_BASEADDR +
			SERDES_L0_TX_MARGININGF + index * SERDES_LANE_OFFSET);
		/* Write new pre-emphasis levels to the TX registers. */
		writel(pe[pe_level_rx][vs_level_rx], (ulong)SERDES_BASEADDR +
			SERDES_L0_TX_DEEMPHASIS + index * SERDES_LANE_OFFSET);
	}
}

/**
 * set_vswing_preemp() - Build AUX data to set voltage swing and pre-emphasis
 * @dev:      The DP device
 * @aux_data: Buffer to receive the built AUX data
 *
 * Build AUX data to set current voltage swing and pre-emphasis level settings;
 * the necessary data is taken from the link_config structure.
 */
static void set_vswing_preemp(struct udevice *dev, u8 *aux_data)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	u8 data = 0;
	u8 vs_level_rx = dp_sub->link_config.vs_level;
	u8 pe_level_rx = dp_sub->link_config.pe_level;

	if (vs_level_rx >= DP_MAXIMUM_VS_LEVEL)
		data |= DP_DPCD_TRAINING_LANEX_SET_MAX_VS_MASK;

	/* The maximum pre-emphasis level has been reached. */
	if (pe_level_rx >= DP_MAXIMUM_PE_LEVEL)
		data |= DP_DPCD_TRAINING_LANEX_SET_MAX_PE_MASK;

	/* Set up the data buffer for writing to the RX device. */
	data |= (pe_level_rx << DP_DPCD_TRAINING_LANEX_SET_PE_SHIFT) |
		 vs_level_rx;
	memset(aux_data, data, 4);

	set_serdes_vswing_preemp(dev);
}

static int set_training_pattern(struct udevice *dev, u32 pattern)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	u8 aux_data[5];

	writel(pattern, dp_sub->base_addr + TRAINING_PATTERN_SET);

	aux_data[0] = pattern;
	switch (pattern) {
	case TRAINING_PATTERN_SET_OFF:
		writel(0, dp_sub->base_addr + SCRAMBLING_DISABLE);
		dp_sub->link_config.scrambler_en = 1;
		break;
	case TRAINING_PATTERN_SET_TP1:
	case TRAINING_PATTERN_SET_TP2:
	case TRAINING_PATTERN_SET_TP3:
		aux_data[0] |= DP_DPCD_TP_SET_SCRAMB_DIS_MASK;
		writel(1, dp_sub->base_addr + SCRAMBLING_DISABLE);
		dp_sub->link_config.scrambler_en = 0;
		break;
	default:
		break;
	}
	/*
	 * Make the adjustments to both the DisplayPort TX core and the RX
	 * device.
	 */
	set_vswing_preemp(dev, &aux_data[1]);
	/*
	 * Write the voltage swing and pre-emphasis levels for each lane to the
	 * RX device.
	 */
	if (pattern == TRAINING_PATTERN_SET_OFF)
		return aux_write(dev, DP_DPCD_TP_SET, 1, aux_data);
	else
		return aux_write(dev, DP_DPCD_TP_SET, 5, aux_data);
}

static int get_lane_status_adj_reqs(struct udevice *dev)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	u32 status;
	u8 aux_data[8];

	status = aux_read(dev, DP_DPCD_SINK_COUNT, 8, aux_data);
	if (status)
		return status;

	/* Save XDPPSU_DPCD_SINK_COUNT contents. */
	dp_sub->sink_count =
		((aux_data[0] & DP_DPCD_SINK_COUNT_HIGH_MASK) >>
		DP_DPCD_SINK_COUNT_HIGH_LOW_SHIFT) |
		(aux_data[0] & DP_DPCD_SINK_COUNT_LOW_MASK);
	memcpy(dp_sub->lane_status_ajd_reqs, &aux_data[2], 6);
	return 0;
}

/**
 * check_clock_recovery() - Check clock recovery success
 * @dev:        The LogiCore DP TX device in question
 * @lane_count: The number of lanes for which to check clock recovery success
 *
 * Check if the RX device's DisplayPort Configuration data (DPCD) indicates
 * that the clock recovery sequence during link training was successful - the
 * RX device's link clock and data recovery unit has realized and maintained
 * the frequency lock for all lanes currently in use.
 *
 * Return: 0 if clock recovery was successful on all lanes in question, -ve if
 *         not
 */
static int check_clock_recovery(struct udevice *dev, u8 lane_count)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	u8 *lane_status = dp_sub->lane_status_ajd_reqs;

	switch (lane_count) {
	case DP_LANE_COUNT_SET_2:
		if (!(lane_status[0] & DP_DPCD_STATUS_LANE_1_CR_DONE_MASK))
			return -EINVAL;
	case DP_LANE_COUNT_SET_1:
		if (!(lane_status[0] & DP_DPCD_STATUS_LANE_0_CR_DONE_MASK))
			return -EINVAL;
	default:
		/* All (LaneCount) lanes have achieved clock recovery. */
		break;
	}
	return 0;
}

/**
 * adj_vswing_preemp() - Adjust voltage swing and pre-emphasis
 * @dev: The DP device
 *
 * Set new voltage swing and pre-emphasis levels using the
 * adjustment requests obtained from the RX device.
 *
 * Return: 0 if voltage swing and pre-emphasis could be adjusted successfully,
 *         -ve on error
 */
static int adj_vswing_preemp(struct udevice *dev)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	u8 index, vs_level_adj_req[4], pe_level_adj_req[4];
	u8 aux_data[4];
	u8 *adj_reqs = &dp_sub->lane_status_ajd_reqs[4];

	/*
	 * Analyze the adjustment requests for changes in voltage swing and
	 * pre-emphasis levels.
	 */
	vs_level_adj_req[0] = adj_reqs[0] & DP_DPCD_ADJ_REQ_LANE_0_2_VS_MASK;
	vs_level_adj_req[1] = (adj_reqs[0] & DP_DPCD_ADJ_REQ_LANE_1_3_VS_MASK) >>
			      DP_DPCD_ADJ_REQ_LANE_1_3_VS_SHIFT;
	pe_level_adj_req[0] = (adj_reqs[0] & DP_DPCD_ADJ_REQ_LANE_0_2_PE_MASK) >>
			      DP_DPCD_ADJ_REQ_LANE_0_2_PE_SHIFT;
	pe_level_adj_req[1] = (adj_reqs[0] & DP_DPCD_ADJ_REQ_LANE_1_3_PE_MASK) >>
			      DP_DPCD_ADJ_REQ_LANE_1_3_PE_SHIFT;

	/*
	 * Change the drive settings to match the adjustment requests. Use the
	 * greatest level requested.
	 */
	dp_sub->link_config.vs_level = 0;
	dp_sub->link_config.pe_level = 0;
	for (index = 0; index < dp_sub->link_config.lane_count; index++) {
		if (vs_level_adj_req[index] > dp_sub->link_config.vs_level)
			dp_sub->link_config.vs_level = vs_level_adj_req[index];

		if (pe_level_adj_req[index] > dp_sub->link_config.pe_level)
			dp_sub->link_config.pe_level = pe_level_adj_req[index];
	}

	if (dp_sub->link_config.pe_level > DP_MAXIMUM_PE_LEVEL)
		dp_sub->link_config.pe_level = DP_MAXIMUM_PE_LEVEL;

	if (dp_sub->link_config.vs_level > DP_MAXIMUM_VS_LEVEL)
		dp_sub->link_config.vs_level = DP_MAXIMUM_VS_LEVEL;

	if (dp_sub->link_config.pe_level >
				(4 - dp_sub->link_config.vs_level)) {
		dp_sub->link_config.pe_level =
				4 - dp_sub->link_config.vs_level;
	}
	/*
	 * Make the adjustments to both the DisplayPort TX core and the RX
	 * device.
	 */
	set_vswing_preemp(dev, aux_data);
	/*
	 * Write the voltage swing and pre-emphasis levels for each lane to the
	 * RX device.
	 */
	return aux_write(dev, DP_DPCD_TRAINING_LANE0_SET, 2, aux_data);
}

/**
 * get_training_delay() - Get training delay
 * @dev:            The DP device
 * @training_state: The training state for which the required training delay
 *                  should be queried
 *
 * Determine what the RX device's required training delay is for
 * link training.
 *
 * Return: The training delay in us
 */
static u32 get_training_delay(struct udevice *dev)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	u8 *dpcd = dp_sub->dpcd_rx_caps;

	if (dpcd[DP_DPCD_TRAIN_AUX_RD_INTERVAL])
		return 400 * dpcd[DP_DPCD_TRAIN_AUX_RD_INTERVAL] * 10;

	return 400;
}

/**
 * training_state_clock_recovery() - Run clock recovery part of link training
 * @dev: The DP device
 *
 * Run the clock recovery sequence as part of link training. The
 * sequence is as follows:
 *
 *      0) Start signaling at the minimum voltage swing, pre-emphasis, and
 *         post- cursor levels.
 *      1) Transmit training pattern 1 over the main link with symbol
 *         scrambling disabled.
 *      2) The clock recovery loop. If clock recovery is unsuccessful after
 *         MaxIterations loop iterations, return.
 *      2a) Wait for at least the period of time specified in the RX device's
 *          DisplayPort Configuration data (DPCD) register,
 *          TRAINING_AUX_RD_INTERVAL.
 *      2b) Check if all lanes have achieved clock recovery lock. If so,
 *          return.
 *      2c) Check if the same voltage swing level has been used 5 consecutive
 *          times or if the maximum level has been reached. If so, return.
 *      2d) Adjust the voltage swing, pre-emphasis, and post-cursor levels as
 *          requested by the RX device.
 *      2e) Loop back to 2a.
 *
 * For a more detailed description of the clock recovery sequence, see section
 * 3.5.1.2.1 of the DisplayPort 1.2a specification document.
 *
 * Return: The next state machine state to advance to
 */
static enum link_training_states training_state_clock_recovery(struct udevice *dev)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	u32 status, delay_us;
	u8 prev_vs_level = 0, same_vs_level_count  = 0;
	struct link_config *link_config = &dp_sub->link_config;

	delay_us = get_training_delay(dev);
	/* Start CRLock. */
	/* Start from minimal voltage swing and pre-emphasis levels. */
	dp_sub->link_config.vs_level = 0;
	dp_sub->link_config.pe_level = 0;
	/* Transmit training pattern 1. */
	status = set_training_pattern(dev, TRAINING_PATTERN_SET_TP1);
	if (status)
		return TS_FAILURE;

	while (1) {
		/* Wait delay specified in TRAINING_AUX_RD_INTERVAL. */
		udelay(delay_us);
		/* Get lane and adjustment requests. */
		status = get_lane_status_adj_reqs(dev);
		if (status)
			/* The AUX read failed. */
			return TS_FAILURE;

		/*
		 * Check if all lanes have realized and maintained the frequency
		 * lock and get adjustment requests.
		 */
		status = check_clock_recovery(dev, dp_sub->link_config.lane_count);
		if (status == 0)
			return TS_CHANNEL_EQUALIZATION;
		/*
		 * Check if the same voltage swing for each lane has been used 5
		 * consecutive times.
		 */
		if (prev_vs_level == link_config->vs_level) {
			same_vs_level_count++;
		} else {
			same_vs_level_count = 0;
			prev_vs_level = link_config->vs_level;
		}
		if (same_vs_level_count >= 5)
			break;

		/* Only try maximum voltage swing once. */
		if (link_config->vs_level == DP_MAXIMUM_VS_LEVEL)
			break;

		/* Adjust the drive settings as requested by the RX device. */
		status = adj_vswing_preemp(dev);
		if (status)
			/* The AUX write failed. */
			return TS_FAILURE;
	}
	return TS_ADJUST_LINK_RATE;
}

/**
 * check_channel_equalization() - Check channel equalization success
 * @dev:        The DP device
 * @lane_count: The number of lanes for which to check channel equalization
 *              success
 *
 * Check if the RX device's DisplayPort Configuration data (DPCD) indicates
 * that the channel equalization sequence during link training was successful -
 * the RX device has achieved channel equalization, symbol lock, and interlane
 * alignment for all lanes currently in use.
 *
 * Return: 0 if channel equalization was successful on all lanes in question,
 *         -ve if not
 */
static int check_channel_equalization(struct udevice *dev, u8 lane_count)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	u8 *lane_status = dp_sub->lane_status_ajd_reqs;

	/* Check that all LANEx_CHANNEL_EQ_DONE bits are set. */
	switch (lane_count) {
	case DP_LANE_COUNT_SET_2:
		if (!(lane_status[0] & DP_DPCD_STATUS_LANE_1_CE_DONE_MASK))
			return -EINVAL;
	case DP_LANE_COUNT_SET_1:
		if (!(lane_status[0] & DP_DPCD_STATUS_LANE_0_CE_DONE_MASK))
			return -EINVAL;
	default:
		/* All (LaneCount) lanes have achieved channel equalization. */
		break;
	}

	/* Check that all LANEx_SYMBOL_LOCKED bits are set. */
	switch (lane_count) {
	case DP_LANE_COUNT_SET_2:
		if (!(lane_status[0] & DP_DPCD_STATUS_LANE_1_SL_DONE_MASK))
			return -EINVAL;
	case DP_LANE_COUNT_SET_1:
		if (!(lane_status[0] & DP_DPCD_STATUS_LANE_0_SL_DONE_MASK))
			return -EINVAL;
	default:
		/* All (LaneCount) lanes have achieved symbol lock. */
		break;
	}

	/* Check that interlane alignment is done. */
	if (!(lane_status[2] & DP_DPCD_LANE_ALIGN_STATUS_UPDATED_IA_DONE_MASK))
		return -EINVAL;
	return 0;
}

/**
 * training_state_channel_equalization() - Run channel equalization part of
 *                                         link training
 * @dev: The DP device
 *
 * Run the channel equalization sequence as part of link
 * training. The sequence is as follows:
 *
 *      0) Start signaling with the same drive settings used at the end of the
 *         clock recovery sequence.
 *      1) Transmit training pattern 2 (or 3) over the main link with symbol
 *         scrambling disabled.
 *      2) The channel equalization loop. If channel equalization is
 *         unsuccessful after 5 loop iterations, return.
 *      2a) Wait for at least the period of time specified in the RX device's
 *          DisplayPort Configuration data (DPCD) register,
 *          TRAINING_AUX_RD_INTERVAL.
 *      2b) Check if all lanes have achieved channel equalization, symbol lock,
 *          and interlane alignment. If so, return.
 *      2c) Check if the same voltage swing level has been used 5 consecutive
 *          times or if the maximum level has been reached. If so, return.
 *      2d) Adjust the voltage swing, pre-emphasis, and post-cursor levels as
 *          requested by the RX device.
 *      2e) Loop back to 2a.
 *
 * For a more detailed description of the channel equalization sequence, see
 * section 3.5.1.2.2 of the DisplayPort 1.2a specification document.
 *
 * Return: The next state machine state to advance to
 */
static enum link_training_states training_state_channel_equalization(struct udevice *dev)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	u32 status, delay_us = 400, iteration_count = 0;

	/* Write the current drive settings. */
	/* Transmit training pattern 2/3. */
	if (dp_sub->dpcd_rx_caps[DP_DPCD_MAX_LANE_COUNT] &
						  DP_DPCD_TPS3_SUPPORT_MASK)
		status = set_training_pattern(dev, TRAINING_PATTERN_SET_TP3);
	else
		status = set_training_pattern(dev, TRAINING_PATTERN_SET_TP2);

	if (status)
		return TS_FAILURE;

	while (iteration_count < 5) {
		/* Wait delay specified in TRAINING_AUX_RD_INTERVAL. */
		udelay(delay_us);

		/* Get lane and adjustment requests. */
		status = get_lane_status_adj_reqs(dev);
		if (status)
			/* The AUX read failed. */
			return TS_FAILURE;

		/* Adjust the drive settings as requested by the RX device. */
		status = adj_vswing_preemp(dev);
		if (status)
			/* The AUX write failed. */
			return TS_FAILURE;

		/* Check that all lanes still have their clocks locked. */
		status = check_clock_recovery(dev, dp_sub->link_config.lane_count);
		if (status)
			break;
		/*
		 * Check that all lanes have accomplished channel
		 * equalization, symbol lock, and interlane alignment.
		 */
		status = check_channel_equalization(dev, dp_sub->link_config.lane_count);
		if (status == 0)
			return TS_SUCCESS;
		iteration_count++;
	}

	/*
	 * Tried 5 times with no success. Try a reduced bitrate first, then
	 * reduce the number of lanes.
	 */
	return TS_ADJUST_LINK_RATE;
}

static int check_lane_align(struct udevice *dev)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	u8 *lane_status = dp_sub->lane_status_ajd_reqs;

	if (!(lane_status[2] & DP_DPCD_LANE_ALIGN_STATUS_UPDATED_IA_DONE_MASK))
		return -EINVAL;
	return 0;
}

/**
 * check_link_status() - Check status of link
 * @dev:        The DP device
 * @lane_count: The lane count to use for the check
 *
 * Check if the receiver's DisplayPort Configuration data (DPCD) indicates the
 * receiver has achieved and maintained clock recovery, channel equalization,
 * symbol lock, and interlane alignment for all lanes currently in use.
 *
 * Return: 0 if the link status is OK, -ve if a error occurred during checking
 */
static int check_link_status(struct udevice *dev, u8 lane_count)
{
	u32 status;

	status = get_lane_status_adj_reqs(dev);
	if (status)
		/* The AUX read failed. */
		return status;

	/* Check if the link needs training. */
	if ((check_clock_recovery(dev, lane_count) == 0) &&
	    (check_channel_equalization(dev, lane_count) == 0) &&
	    (check_lane_align(dev) == 0)) {
		return 0;
	}
	return -EINVAL;
}

/**
 * run_training() - Run link training
 * @dev: The DP device
 *
 * Run the link training process. It is implemented as a state machine, with
 * each state returning the next state. First, the clock recovery sequence will
 * be run; if successful, the channel equalization sequence will run. If either
 * the clock recovery or channel equalization sequence failed, the link rate or
 * the number of lanes used will be reduced and training will be re-attempted.
 * If training fails at the minimal data rate, 1.62 Gbps with a single lane,
 * training will no longer re-attempt and fail.
 *
 * There are undocumented timeout constraints in the link training process. In
 * DP v1.2a spec, Chapter 3.5.1.2.2 a 10ms limit for the complete training
 * process is mentioned. Which individual timeouts are derived and implemented
 * by sink manufacturers is unknown. So each step should be as short as
 * possible and link training should start as soon as possible after HPD.
 *
 * Return: 0 if the training sequence ran successfully, -ve if a error occurred
 *         or the training failed
 */
static int run_training(struct udevice *dev)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	u32 status;
	enum link_training_states training_state = TS_CLOCK_RECOVERY;

	while (1) {
		switch (training_state) {
		case TS_CLOCK_RECOVERY:
				training_state = training_state_clock_recovery(dev);
			break;
		case TS_CHANNEL_EQUALIZATION:
			training_state = training_state_channel_equalization(dev);
			break;
		default:
			break;
		}

		if (training_state == TS_SUCCESS)
			break;
		else if (training_state == TS_FAILURE)
			return -EINVAL;

		if (training_state == TS_ADJUST_LANE_COUNT ||
		    training_state == TS_ADJUST_LINK_RATE) {
			status = set_training_pattern(dev, TRAINING_PATTERN_SET_OFF);
			if (status)
				return -EINVAL;
		}
	}

	/* Final status check. */
	return check_link_status(dev, dp_sub->link_config.lane_count);
}

void reset_dp_phy(struct udevice *dev, u32 reset)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	u32 phyval, regval;

	writel(0, dp_sub->base_addr + DP_ENABLE);
	phyval = readl(dp_sub->base_addr + DP_PHY_CONFIG);
	regval = phyval | reset;
	writel(regval, dp_sub->base_addr + DP_PHY_CONFIG);
	/* Remove the reset. */
	writel(phyval, dp_sub->base_addr + DP_PHY_CONFIG);
	/* Wait for the PHY to be ready. */
	wait_phy_ready(dev);

	writel(1, dp_sub->base_addr + DP_ENABLE);
}

/**
 * establish_link() - Establish a link
 * @dev: The DP device
 *
 * Check if the link needs training and run the training sequence if training
 * is required.
 *
 * Return: 0 if the link was established successfully, -ve on error
 */
static int establish_link(struct udevice *dev)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	u32 status, re_enable_main_link;

	reset_dp_phy(dev, DP_PHY_CONFIG_TX_PHY_8B10BEN_MASK |
			  DP_PHY_CONFIG_PHY_RESET_MASK);

	re_enable_main_link = readl(dp_sub->base_addr + DP_ENABLE_MAIN_STREAM);
	if (re_enable_main_link)
		enable_main_link(dev, 0);

	status = run_training(dev);
	if (status)
		return status;

	status = set_training_pattern(dev, TRAINING_PATTERN_SET_OFF);
	if (status)
		return status;

	if (re_enable_main_link)
		enable_main_link(dev, 1);

	return check_link_status(dev, dp_sub->link_config.lane_count);
}

static int dp_hpd_train(struct udevice *dev)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	struct link_config *link_config = &dp_sub->link_config;
	u32 status;

	status = get_rx_capabilities(dev);
	if (status) {
		debug("! Error getting RX caps.\n");
		return status;
	}

	status = set_enhanced_frame_mode(dev, link_config->support_enhanced_framing_mode ? 1 : 0);
	if (status) {
		debug("! EFM set failed.\n");
		return status;
	}

	status = set_lane_count(dev, (dp_sub->use_max_lane_count) ?
				link_config->max_lane_count : dp_sub->lane_count);
	if (status) {
		debug("! Lane count set failed.\n");
		return status;
	}

	status = set_link_rate(dev, (dp_sub->use_max_link_rate) ?
			       link_config->max_link_rate : dp_sub->link_rate);
	if (status) {
		debug("! Link rate set failed.\n");
		return status;
	}

	status = set_downspread(dev, link_config->support_downspread_control);
	if (status) {
		debug("! Setting downspread failed.\n");
		return status;
	}

	debug("Lane count =%d\n", dp_sub->link_config.lane_count);
	debug("Link rate =%d\n",  dp_sub->link_config.link_rate);

	debug("Starting Training...\n");
	status = establish_link(dev);
	if (status == 0)
		debug("! Training succeeded.\n");
	else
		debug("! Training failed.\n");

	return status;
}

static void display_gfx_frame_buffer(struct udevice *dev)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);

	if (!dp_sub->dp_dma->gfx.channel.cur)
		dp_sub->dp_dma->gfx.trigger_status = DPDMA_TRIGGER_EN;
}

static void set_color_encode(struct udevice *dev)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	struct main_stream_attributes *msa_config = &dp_sub->msa_config;

	msa_config->y_cb_cr_colorimetry = 0;
	msa_config->dynamic_range       = 0;
	msa_config->component_format    = 0;
	msa_config->misc0               = 0;
	msa_config->misc1               = 0;
	msa_config->component_format    = DP_MAIN_STREAM_MISC0_COMPONENT_FORMAT_RGB;
}

static void config_msa_recalculate(struct udevice *dev)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	u32 video_bw, link_bw, words_per_line;
	u8 bits_per_pixel;
	struct main_stream_attributes *msa_config;
	struct link_config *link_config;

	msa_config = &dp_sub->msa_config;
	link_config = &dp_sub->link_config;

	msa_config->user_pixel_width = 1;

	/* Compute the rest of the MSA values. */
	msa_config->n_vid = 27 * 1000 * link_config->link_rate;
	msa_config->h_start = msa_config->vid_timing_mode.video_timing.h_sync_width +
			      msa_config->vid_timing_mode.video_timing.h_back_porch;
	msa_config->v_start = msa_config->vid_timing_mode.video_timing.f0_pv_sync_width +
			      msa_config->vid_timing_mode.video_timing.f0_pv_back_porch;

	/* Miscellaneous attributes. */
	if (msa_config->bits_per_color == 6)
		msa_config->misc0 = DP_MAIN_STREAM_MISC0_BDC_6BPC;
	else if (msa_config->bits_per_color == 8)
		msa_config->misc0 = DP_MAIN_STREAM_MISC0_BDC_8BPC;
	else if (msa_config->bits_per_color == 10)
		msa_config->misc0 = DP_MAIN_STREAM_MISC0_BDC_10BPC;
	else if (msa_config->bits_per_color == 12)
		msa_config->misc0 = DP_MAIN_STREAM_MISC0_BDC_12BPC;
	else if (msa_config->bits_per_color == 16)
		msa_config->misc0 = DP_MAIN_STREAM_MISC0_BDC_16BPC;

	msa_config->misc0 <<= DP_MAIN_STREAM_MISC0_BDC_SHIFT;

	/* Need to set this. */
	msa_config->misc0 |= msa_config->component_format <<
			     DP_MAIN_STREAM_MISC0_COMPONENT_FORMAT_SHIFT;

	msa_config->misc0 |= msa_config->dynamic_range <<
			     DP_MAIN_STREAM_MISC0_DYNAMIC_RANGE_SHIFT;

	msa_config->misc0 |= msa_config->y_cb_cr_colorimetry <<
			     DP_MAIN_STREAM_MISC0_YCBCR_COLORIMETRY_SHIFT;

	msa_config->misc0 |= msa_config->synchronous_clock_mode;
	/*
	 * Determine the number of bits per pixel for the specified color
	 * component format.
	 */
	if (msa_config->misc1 == DP_MAIN_STREAM_MISC1_Y_ONLY_EN_MASK)
		bits_per_pixel = msa_config->bits_per_color;
	else if (msa_config->component_format ==
			DP_MAIN_STREAM_MISC0_COMPONENT_FORMAT_YCBCR422)
		/* YCbCr422 color component format. */
		bits_per_pixel = msa_config->bits_per_color * 2;
	else
		/* RGB or YCbCr 4:4:4 color component format. */
		bits_per_pixel = msa_config->bits_per_color * 3;

	/* Calculate the data per lane. */
	words_per_line = msa_config->vid_timing_mode.video_timing.h_active * bits_per_pixel;
	if (words_per_line % 16)
		words_per_line += 16;

	words_per_line /= 16;
	msa_config->data_per_lane = words_per_line - link_config->lane_count;
	if (words_per_line % link_config->lane_count)
		msa_config->data_per_lane += (words_per_line % link_config->lane_count);

	/* Allocate a fixed size for single-stream transport (SST) operation. */
	msa_config->transfer_unit_size = 64;

	/*
	 * Calculate the average number of bytes per transfer unit.
	 * Note: Both the integer and the fractional part is stored in
	 * AvgBytesPerTU.
	 */
	video_bw = ((msa_config->pixel_clock_hz / 1000) * bits_per_pixel) / 8;
	link_bw = (link_config->lane_count * link_config->link_rate * 27);
	msa_config->avg_bytes_per_tu = ((10 *
					(video_bw * msa_config->transfer_unit_size)
					/ link_bw) + 5) / 10;
	/*
	 * The number of initial wait cycles at the start of a new line by the
	 * framing logic. This allows enough data to be buffered in the input
	 * FIFO before video is sent.
	 */
	if ((msa_config->avg_bytes_per_tu / 1000) <= 4)
		msa_config->init_wait = 64;
	else
		msa_config->init_wait = msa_config->transfer_unit_size -
					(msa_config->avg_bytes_per_tu / 1000);
}

static void set_msa_bpc(struct udevice *dev, u8 bits_per_color)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);

	dp_sub->msa_config.bits_per_color = bits_per_color;
	/* Calculate the rest of the MSA values. */
	config_msa_recalculate(dev);
}

const struct video_timing_mode *get_video_mode_data(enum video_mode vm_id)
{
	if (vm_id < VIDC_VM_NUM_SUPPORTED)
		return &vidc_video_timing_modes[vm_id];

	return NULL;
}

static u64 get_pixelclk_by_vmid(enum video_mode vm_id)
{
	const struct video_timing_mode *vm;
	u64 clk_hz;

	vm = get_video_mode_data(vm_id);
	/* For progressive mode, use only frame 0 vertical total. */
	clk_hz = vm->video_timing.f0_pv_total;
	/* Multiply the number of pixels by the frame rate. */
	clk_hz *= vm->frame_rate;

	/*
	 * Multiply the vertical total by the horizontal total for number of
	 * pixels.
	 */
	clk_hz *= vm->video_timing.h_total;

	return clk_hz;
}

/**
 * config_msa_video_mode() - Enable video output
 * @dev: The DP device
 * @msa: The MSA values to set for the device
 *
 * Return: 0 if the video was enabled successfully, -ve on error
 */
static void config_msa_video_mode(struct udevice *dev, enum video_mode videomode)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	struct main_stream_attributes *msa_config;

	msa_config = &dp_sub->msa_config;

	/* Configure the MSA values from the display monitor DMT table. */
	msa_config->vid_timing_mode.vid_mode = vidc_video_timing_modes[videomode].vid_mode;
	msa_config->vid_timing_mode.frame_rate = vidc_video_timing_modes[videomode].frame_rate;
	msa_config->vid_timing_mode.video_timing.h_active =
				vidc_video_timing_modes[videomode].video_timing.h_active;
	msa_config->vid_timing_mode.video_timing.h_front_porch =
				vidc_video_timing_modes[videomode].video_timing.h_front_porch;
	msa_config->vid_timing_mode.video_timing.h_sync_width =
				vidc_video_timing_modes[videomode].video_timing.h_sync_width;
	msa_config->vid_timing_mode.video_timing.h_back_porch =
				vidc_video_timing_modes[videomode].video_timing.h_back_porch;
	msa_config->vid_timing_mode.video_timing.h_total =
				vidc_video_timing_modes[videomode].video_timing.h_total;
	msa_config->vid_timing_mode.video_timing.h_sync_polarity =
			vidc_video_timing_modes[videomode].video_timing.h_sync_polarity;
	msa_config->vid_timing_mode.video_timing.v_active =
			vidc_video_timing_modes[videomode].video_timing.v_active;
	msa_config->vid_timing_mode.video_timing.f0_pv_front_porch =
			vidc_video_timing_modes[videomode].video_timing.f0_pv_front_porch;
	msa_config->vid_timing_mode.video_timing.f0_pv_sync_width =
			vidc_video_timing_modes[videomode].video_timing.f0_pv_sync_width;
	msa_config->vid_timing_mode.video_timing.f0_pv_back_porch =
			vidc_video_timing_modes[videomode].video_timing.f0_pv_back_porch;
	msa_config->vid_timing_mode.video_timing.f0_pv_total =
			vidc_video_timing_modes[videomode].video_timing.f0_pv_total;
	msa_config->vid_timing_mode.video_timing.f1_v_front_porch =
			vidc_video_timing_modes[videomode].video_timing.f1_v_front_porch;
	msa_config->vid_timing_mode.video_timing.f1_v_sync_width =
			vidc_video_timing_modes[videomode].video_timing.f1_v_sync_width;
	msa_config->vid_timing_mode.video_timing.f1_v_back_porch =
			vidc_video_timing_modes[videomode].video_timing.f1_v_back_porch;
	msa_config->vid_timing_mode.video_timing.f1_v_total =
			vidc_video_timing_modes[videomode].video_timing.f1_v_total;
	msa_config->vid_timing_mode.video_timing.v_sync_polarity =
			vidc_video_timing_modes[videomode].video_timing.v_sync_polarity;
	msa_config->pixel_clock_hz = get_pixelclk_by_vmid(msa_config->vid_timing_mode.vid_mode);

	/* Calculate the rest of the MSA values. */
	config_msa_recalculate(dev);
}

static void set_pixel_clock(u64 freq_hz)
{
	u64 ext_divider, vco, vco_int_frac;
	u32 pll_assigned, frac_int_fb_div, fraction, regpll = 0;
	u8 pll;

	pll_assigned = readl(CLK_FPD_BASEADDR + VIDEO_REF_CTRL) & VIDEO_REF_CTRL_SRCSEL_MASK;
	if (pll_assigned)
		pll = VPLL;

	ext_divider = PLL_OUT_FREQ / freq_hz;
	vco = freq_hz * ext_divider * 2;
	vco_int_frac = (vco * INPUT_FREQ_PRECISION * SHIFT_DECIMAL) /
			AVBUF_INPUT_REF_CLK;
	frac_int_fb_div = vco_int_frac >> PRECISION;
	fraction = vco_int_frac &  AVBUF_DECIMAL;

	regpll |= ENABLE_BIT << PLL_CTRL_BYPASS_SHIFT;
	regpll |= frac_int_fb_div << PLL_CTRL_FBDIV_SHIFT;
	regpll |= (1 << PLL_CTRL_DIV2_SHIFT);
	regpll |= (PSS_REF_CLK << PLL_CTRL_PRE_SRC_SHIFT);
	writel(regpll, CLK_FPD_BASEADDR + VPLL_CTRL);

	regpll = 0;
	regpll |= VPLL_CFG_CP << PLL_CFG_CP_SHIFT;
	regpll |= VPLL_CFG_RES << PLL_CFG_RES_SHIFT;
	regpll |= VPLL_CFG_LFHF << PLL_CFG_LFHF_SHIFT;
	regpll |= VPLL_CFG_LOCK_DLY << PLL_CFG_LOCK_DLY_SHIFT;
	regpll |= VPLL_CFG_LOCK_CNT << PLL_CFG_LOCK_CNT_SHIFT;
	writel(regpll, CLK_FPD_BASEADDR + VPLL_CFG);

	regpll = (1U << PLL_FRAC_CFG_ENABLED_SHIFT) |
		 (fraction << PLL_FRAC_CFG_DATA_SHIFT);
	writel(regpll, CLK_FPD_BASEADDR + VPLL_FRAC_CFG);

	clrsetbits_le32(CLK_FPD_BASEADDR + VPLL_CTRL,
			PLL_CTRL_RESET_MASK,
			(ENABLE_BIT << PLL_CTRL_RESET_SHIFT));

	/* Deassert reset to the PLL. */
	clrsetbits_le32(CLK_FPD_BASEADDR + VPLL_CTRL,
			PLL_CTRL_RESET_MASK,
			(DISABLE_BIT << PLL_CTRL_RESET_SHIFT));

	while (!(readl(CLK_FPD_BASEADDR + PLL_STATUS) &
		(1 << PLL_STATUS_VPLL_LOCK)))
		;

	/* Deassert Bypass. */
	clrsetbits_le32(CLK_FPD_BASEADDR + VPLL_CTRL,
			PLL_CTRL_BYPASS_MASK,
			(DISABLE_BIT << PLL_CTRL_BYPASS_SHIFT));
	udelay(1);

	clrsetbits_le32(CLK_FPD_BASEADDR + VIDEO_REF_CTRL,
			VIDEO_REF_CTRL_CLKACT_MASK,
			(DISABLE_BIT << VIDEO_REF_CTRL_CLKACT_SHIFT));

	clrsetbits_le32(CLK_FPD_BASEADDR + VIDEO_REF_CTRL,
			VIDEO_REF_CTRL_DIVISOR1_MASK,
			(ENABLE_BIT << VIDEO_REF_CTRL_DIVISOR1_SHIFT));

	clrsetbits_le32(CLK_FPD_BASEADDR + VIDEO_REF_CTRL,
			VIDEO_REF_CTRL_DIVISOR0_MASK,
			(ext_divider << VIDEO_REF_CTRL_DIVISOR0_SHIFT));

	clrsetbits_le32(CLK_FPD_BASEADDR + VIDEO_REF_CTRL,
			VIDEO_REF_CTRL_CLKACT_MASK,
			(ENABLE_BIT << VIDEO_REF_CTRL_CLKACT_SHIFT));
}

/**
 * set_msa_values() - Set MSA values
 * @dev: The DP device
 *
 * Set the main stream attributes registers of the DisplayPort TX
 * core with the values specified in the main stream attributes configuration
 * structure.
 */
static void set_msa_values(struct udevice *dev)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	struct main_stream_attributes *msa_config;

	msa_config = &dp_sub->msa_config;

	/*
	 * Set the main stream attributes to the associated DisplayPort TX core
	 * registers.
	 */
	writel(msa_config->vid_timing_mode.video_timing.h_total,
	       dp_sub->base_addr + DP_MAIN_STREAM_HTOTAL);
	writel(msa_config->vid_timing_mode.video_timing.f0_pv_total,
	       dp_sub->base_addr + DP_MAIN_STREAM_VTOTAL);
	writel(msa_config->vid_timing_mode.video_timing.h_sync_polarity |
	       (msa_config->vid_timing_mode.video_timing.v_sync_polarity
		<< DP_MAIN_STREAM_POLARITY_VSYNC_POL_SHIFT),
		dp_sub->base_addr + DP_MAIN_STREAM_POLARITY);
	writel(msa_config->vid_timing_mode.video_timing.h_sync_width,
	       dp_sub->base_addr + DP_MAIN_STREAM_HSWIDTH);
	writel(msa_config->vid_timing_mode.video_timing.f0_pv_sync_width,
	       dp_sub->base_addr + DP_MAIN_STREAM_VSWIDTH);
	writel(msa_config->vid_timing_mode.video_timing.h_active,
	       dp_sub->base_addr + DP_MAIN_STREAM_HRES);
	writel(msa_config->vid_timing_mode.video_timing.v_active,
	       dp_sub->base_addr + DP_MAIN_STREAM_VRES);
	writel(msa_config->h_start, dp_sub->base_addr + DP_MAIN_STREAM_HSTART);
	writel(msa_config->v_start, dp_sub->base_addr + DP_MAIN_STREAM_VSTART);
	writel(msa_config->misc0, dp_sub->base_addr + DP_MAIN_STREAM_MISC0);
	writel(msa_config->misc1, dp_sub->base_addr + DP_MAIN_STREAM_MISC1);
	writel(msa_config->pixel_clock_hz / 1000, dp_sub->base_addr + DP_M_VID);
	writel(msa_config->n_vid, dp_sub->base_addr + DP_N_VID);
	writel(msa_config->user_pixel_width, dp_sub->base_addr + DP_USER_PIXEL_WIDTH);
	writel(msa_config->data_per_lane, dp_sub->base_addr + DP_USER_DATA_COUNT_PER_LANE);
	/*
	 * Set the transfer unit values to the associated DisplayPort TX core
	 * registers.
	 */
	writel(msa_config->transfer_unit_size, dp_sub->base_addr + DP_TU_SIZE);
	writel(msa_config->avg_bytes_per_tu / 1000,
	       dp_sub->base_addr + DP_MIN_BYTES_PER_TU);
	writel((msa_config->avg_bytes_per_tu % 1000) * 1000,
	       dp_sub->base_addr + DP_FRAC_BYTES_PER_TU);
	writel(msa_config->init_wait, dp_sub->base_addr + DP_INIT_WAIT);
}

static void setup_video_stream(struct udevice *dev)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);
	struct main_stream_attributes *msa_config = &dp_sub->msa_config;

	set_color_encode(dev);
	set_msa_bpc(dev, dp_sub->bpc);
	config_msa_video_mode(dev, dp_sub->video_mode);

	/* Set pixel clock. */
	dp_sub->pix_clk = msa_config->pixel_clock_hz;
	set_pixel_clock(dp_sub->pix_clk);

	/* Reset the transmitter. */
	writel(1, dp_sub->base_addr + DP_SOFT_RESET);
	udelay(10);
	writel(0, dp_sub->base_addr + DP_SOFT_RESET);

	set_msa_values(dev);

	/* Issuing a soft-reset (AV_BUF_SRST_REG). */
	writel(3, dp_sub->base_addr + AVBUF_BUF_SRST_REG); // Assert reset.
	udelay(10);
	writel(0, dp_sub->base_addr + AVBUF_BUF_SRST_REG); // De-ssert reset.

	enable_main_link(dev, 1);

	debug("DONE!\n");
}

static int dp_tx_start_link_training(struct udevice *dev)
{
	u32 status;
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);

	enable_main_link(dev, 0);

	if (!is_dp_connected(dev)) {
		debug("! Disconnected.\n");
		return -ENODEV;
	}

	status = dp_tx_wakeup(dev);
	if (status) {
		debug("! Wakeup failed.\n");
		return -EIO;
	}

	do {
		mdelay(100);
		status = dp_hpd_train(dev);
		if (status == -EINVAL) {
			debug("Lost connection\n\r");
			return -EIO;
		} else if (status) {
			continue;
		}
		display_gfx_frame_buffer(dev);
		setup_video_stream(dev);
		status = check_link_status(dev, dp_sub->link_config.lane_count);
		if (status == -EINVAL)
			return -EIO;
	} while (status != 0);

	return 0;
}

static void init_run_config(struct udevice *dev)
{
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);

	dp_sub->dp_dma               = &dp_dma;
	dp_sub->video_mode           = VIDC_VM_1024x768_60_P;
	dp_sub->bpc                  = VIDC_BPC_8;
	dp_sub->color_encode         = DP_CENC_RGB;
	dp_sub->use_max_cfg_caps     = 1;
	dp_sub->lane_count           = LANE_COUNT_1;
	dp_sub->link_rate            = LINK_RATE_540GBPS;
	dp_sub->en_sync_clk_mode     = 0;
	dp_sub->use_max_lane_count   = 1;
	dp_sub->use_max_link_rate    = 1;
}

static int dpdma_setup(struct udevice *dev)
{
	int status;
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);

	writel(DPDMA_ISR_VSYNC_INT_MASK, dp_sub->dp_dma->base_addr + DPDMA_IEN);
	status = wait_for_bit_le32((u32 *)dp_sub->dp_dma->base_addr + DPDMA_ISR,
				   DPDMA_ISR_VSYNC_INT_MASK, false, 1000, false);
	if (status) {
		debug("%s: INTR TIMEDOUT\n", __func__);
		return status;
	}
	debug("INTR dma_vsync_intr_handler called...\n");
	dma_vsync_intr_handler(dev);

	return 0;
}

static int zynqmp_dpsub_init(struct udevice *dev)
{
	int status;
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);

	/* Initialize the dpdma configuration */
	status = init_dpdma_subsys(dev);
	if (status)
		return -EINVAL;

	config_msa_sync_clk_mode(dev, dp_sub->en_sync_clk_mode);
	set_video_clk_source(dev, AVBUF_PS_CLK, AVBUF_PS_CLK);

	return 0;
}

static int dp_tx_run(struct udevice *dev)
{
	u32 interrupt_signal_state, interrupt_status, hpd_state, hpd_event;
	u32 hpd_pulse_detected, hpd_duration, status;
	int attempts = 0;
	struct zynqmp_dpsub_priv *dp_sub = dev_get_priv(dev);

	/* Continuously poll for HPD events. */
	while (attempts < 5) {
		/* Read interrupt registers. */
		interrupt_signal_state = readl(dp_sub->base_addr + DP_INTERRUPT_SIG_STATE);
		interrupt_status = readl(dp_sub->base_addr + DP_INTR_STATUS);
		/* Check for HPD events. */
		hpd_state = interrupt_signal_state & DP_INTERRUPT_SIG_STATE_HPD_STATE_MASK;
		hpd_event = interrupt_status & DP_INTR_HPD_EVENT_MASK;
		hpd_pulse_detected = interrupt_status & DP_INTR_HPD_PULSE_DETECTED_MASK;
		if (hpd_pulse_detected)
			hpd_duration = readl(dp_sub->base_addr + DP_HPD_DURATION);
		else
			attempts++;

		/* HPD event handling. */
		if (hpd_state && hpd_event) {
			debug("+===> HPD connection event detected.\n");
			/* Initiate link training. */
			status = dp_tx_start_link_training(dev);
			if (status) {
				debug("Link training failed\n");
				return status;
			}
			return 0;
		} else if (hpd_state && hpd_pulse_detected && (hpd_duration >= 250)) {
			debug("===> HPD pulse detected.\n");
			/* Re-train if needed. */
			status = dp_tx_start_link_training(dev);
			if (status) {
				debug("HPD pulse detection failed\n");
				return status;
			}
			return 0;
		} else if (!hpd_state && hpd_event) {
			debug("+===> HPD disconnection event detected.\n\n");
			/* Disable main link. */
			enable_main_link(dev, 0);
			break;
		}
	}
	return -EINVAL;
}

static int zynqmp_dpsub_probe(struct udevice *dev)
{
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	struct zynqmp_dpsub_priv *priv = dev_get_priv(dev);
	struct clk clk;
	int ret;
	int mode = RGBA8888;

	ret = clk_get_by_name(dev, "dp_apb_clk", &clk);
	if (ret < 0) {
		dev_err(dev, "failed to get clock\n");
		return ret;
	}

	priv->clock = clk_get_rate(&clk);
	if (IS_ERR_VALUE(priv->clock)) {
		dev_err(dev, "failed to get rate\n");
		return priv->clock;
	}

	ret = clk_enable(&clk);
	if (ret) {
		dev_err(dev, "failed to enable clock\n");
		return ret;
	}

	dev_dbg(dev, "Base addr 0x%x, clock %d\n", (u32)priv->base_addr,
		priv->clock);

	/* Initialize the DisplayPort TX core. */
	ret = init_dp_tx(dev);
	if (ret)
		return -EINVAL;

	/* Initialize the runtime configuration */
	init_run_config(dev);
	/* Set the format graphics frame for Video Pipeline */
	ret = set_nonlive_gfx_format(dev, mode);
	if (ret)
		return ret;

	uc_priv->bpix = ffs(priv->non_live_graphics->bpp) - 1;
	dev_dbg(dev, "BPP in bits %d, bpix %d\n",
		priv->non_live_graphics->bpp, uc_priv->bpix);

	uc_priv->fb = (void *)gd->fb_base;
	uc_priv->xsize = vidc_video_timing_modes[priv->video_mode].video_timing.h_active;
	uc_priv->ysize = vidc_video_timing_modes[priv->video_mode].video_timing.v_active;
	/* Calculated by core but need it for my own setup */
	uc_priv->line_length = uc_priv->xsize * VNBYTES(uc_priv->bpix);
	/* Will be calculated again in video_post_probe() but I need that value now */
	uc_priv->fb_size = uc_priv->line_length * uc_priv->ysize;

	switch (mode) {
	case RGBA8888:
		uc_priv->format = VIDEO_RGBA8888;
		break;
	default:
		debug("Unsupported mode\n");
		return -EINVAL;
	}

	video_set_flush_dcache(dev, true);
	debug("Video: WIDTH[%d]xHEIGHT[%d]xBPP[%d/%d] -- line length %d\n", uc_priv->xsize,
	      uc_priv->ysize, uc_priv->bpix, VNBYTES(uc_priv->bpix), uc_priv->line_length);

	enable_gfx_buffers(dev, 1);
	avbuf_video_select(dev, AVBUF_VIDSTREAM1_NONE, AVBUF_VIDSTREAM2_NONLIVE_GFX);
	config_gfx_pipeline(dev);
	config_output_video(dev);

	ret = zynqmp_dpsub_init(dev);
	if (ret)
		return ret;

	/* Populate the FrameBuffer structure with the frame attributes */
	priv->frame_buffer.stride = uc_priv->line_length;
	priv->frame_buffer.line_size = priv->frame_buffer.stride;
	priv->frame_buffer.size = priv->frame_buffer.line_size * uc_priv->ysize;

	ret = dp_tx_run(dev);
	if (ret)
		return ret;

	return dpdma_setup(dev);
}

static int zynqmp_dpsub_bind(struct udevice *dev)
{
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);

	/* This is maximum size to allocate - it depends on BPP setting */
	plat->size = WIDTH * HEIGHT * 4;
	/* plat->align is not defined that's why 1MB alignment is used */

	/*
	 * plat->base can be used for allocating own location for FB
	 * if not defined then it is allocated by u-boot itself
	 */

	return 0;
}

static int zynqmp_dpsub_of_to_plat(struct udevice *dev)
{
	struct zynqmp_dpsub_priv *priv = dev_get_priv(dev);
	struct resource res;
	int ret;

	ret = dev_read_resource_byname(dev, "dp", &res);
	if (ret)
		return ret;

	priv->base_addr = res.start;

	return 0;
}

static const struct udevice_id zynqmp_dpsub_ids[] = {
	{ .compatible = "xlnx,zynqmp-dpsub-1.7" },
	{ }
};

U_BOOT_DRIVER(zynqmp_dpsub_video) = {
	.name = "zynqmp_dpsub_video",
	.id = UCLASS_VIDEO,
	.of_match = zynqmp_dpsub_ids,
	.plat_auto = sizeof(struct video_uc_plat),
	.bind = zynqmp_dpsub_bind,
	.probe = zynqmp_dpsub_probe,
	.priv_auto = sizeof(struct zynqmp_dpsub_priv),
	.of_to_plat = zynqmp_dpsub_of_to_plat,
};
