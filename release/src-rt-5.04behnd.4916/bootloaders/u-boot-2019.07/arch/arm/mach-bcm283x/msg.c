// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012 Stephen Warren
 */

#include <common.h>
#include <memalign.h>
#include <phys2bus.h>
#include <asm/arch/mbox.h>

struct msg_set_power_state {
	struct bcm2835_mbox_hdr hdr;
	struct bcm2835_mbox_tag_set_power_state set_power_state;
	u32 end_tag;
};

struct msg_get_clock_rate {
	struct bcm2835_mbox_hdr hdr;
	struct bcm2835_mbox_tag_get_clock_rate get_clock_rate;
	u32 end_tag;
};

struct msg_query {
	struct bcm2835_mbox_hdr hdr;
	struct bcm2835_mbox_tag_physical_w_h physical_w_h;
	u32 end_tag;
};

struct msg_setup {
	struct bcm2835_mbox_hdr hdr;
	struct bcm2835_mbox_tag_physical_w_h physical_w_h;
	struct bcm2835_mbox_tag_virtual_w_h virtual_w_h;
	struct bcm2835_mbox_tag_depth depth;
	struct bcm2835_mbox_tag_pixel_order pixel_order;
	struct bcm2835_mbox_tag_alpha_mode alpha_mode;
	struct bcm2835_mbox_tag_virtual_offset virtual_offset;
	struct bcm2835_mbox_tag_overscan overscan;
	struct bcm2835_mbox_tag_allocate_buffer allocate_buffer;
	struct bcm2835_mbox_tag_pitch pitch;
	u32 end_tag;
};

int bcm2835_power_on_module(u32 module)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct msg_set_power_state, msg_pwr, 1);
	int ret;

	BCM2835_MBOX_INIT_HDR(msg_pwr);
	BCM2835_MBOX_INIT_TAG(&msg_pwr->set_power_state,
			      SET_POWER_STATE);
	msg_pwr->set_power_state.body.req.device_id = module;
	msg_pwr->set_power_state.body.req.state =
		BCM2835_MBOX_SET_POWER_STATE_REQ_ON |
		BCM2835_MBOX_SET_POWER_STATE_REQ_WAIT;

	ret = bcm2835_mbox_call_prop(BCM2835_MBOX_PROP_CHAN,
				     &msg_pwr->hdr);
	if (ret) {
		printf("bcm2835: Could not set module %u power state\n",
		       module);
		return -EIO;
	}

	return 0;
}

int bcm2835_get_mmc_clock(u32 clock_id)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct msg_get_clock_rate, msg_clk, 1);
	int ret;

	ret = bcm2835_power_on_module(BCM2835_MBOX_POWER_DEVID_SDHCI);
	if (ret)
		return ret;

	BCM2835_MBOX_INIT_HDR(msg_clk);
	BCM2835_MBOX_INIT_TAG(&msg_clk->get_clock_rate, GET_CLOCK_RATE);
	msg_clk->get_clock_rate.body.req.clock_id = clock_id;

	ret = bcm2835_mbox_call_prop(BCM2835_MBOX_PROP_CHAN, &msg_clk->hdr);
	if (ret) {
		printf("bcm2835: Could not query eMMC clock rate\n");
		return -EIO;
	}

	return msg_clk->get_clock_rate.body.resp.rate_hz;
}

int bcm2835_get_video_size(int *widthp, int *heightp)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct msg_query, msg_query, 1);
	int ret;

	BCM2835_MBOX_INIT_HDR(msg_query);
	BCM2835_MBOX_INIT_TAG_NO_REQ(&msg_query->physical_w_h,
				     GET_PHYSICAL_W_H);
	ret = bcm2835_mbox_call_prop(BCM2835_MBOX_PROP_CHAN, &msg_query->hdr);
	if (ret) {
		printf("bcm2835: Could not query display resolution\n");
		return ret;
	}
	*widthp = msg_query->physical_w_h.body.resp.width;
	*heightp = msg_query->physical_w_h.body.resp.height;

	return 0;
}

int bcm2835_set_video_params(int *widthp, int *heightp, int depth_bpp,
			     int pixel_order, int alpha_mode, ulong *fb_basep,
			     ulong *fb_sizep, int *pitchp)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct msg_setup, msg_setup, 1);
	int ret;

	BCM2835_MBOX_INIT_HDR(msg_setup);
	BCM2835_MBOX_INIT_TAG(&msg_setup->physical_w_h, SET_PHYSICAL_W_H);
	msg_setup->physical_w_h.body.req.width = *widthp;
	msg_setup->physical_w_h.body.req.height = *heightp;
	BCM2835_MBOX_INIT_TAG(&msg_setup->virtual_w_h, SET_VIRTUAL_W_H);
	msg_setup->virtual_w_h.body.req.width = *widthp;
	msg_setup->virtual_w_h.body.req.height = *heightp;
	BCM2835_MBOX_INIT_TAG(&msg_setup->depth, SET_DEPTH);
	msg_setup->depth.body.req.bpp = 32;
	BCM2835_MBOX_INIT_TAG(&msg_setup->pixel_order, SET_PIXEL_ORDER);
	msg_setup->pixel_order.body.req.order = pixel_order;
	BCM2835_MBOX_INIT_TAG(&msg_setup->alpha_mode, SET_ALPHA_MODE);
	msg_setup->alpha_mode.body.req.alpha = alpha_mode;
	BCM2835_MBOX_INIT_TAG(&msg_setup->virtual_offset, SET_VIRTUAL_OFFSET);
	msg_setup->virtual_offset.body.req.x = 0;
	msg_setup->virtual_offset.body.req.y = 0;
	BCM2835_MBOX_INIT_TAG(&msg_setup->overscan, SET_OVERSCAN);
	msg_setup->overscan.body.req.top = 0;
	msg_setup->overscan.body.req.bottom = 0;
	msg_setup->overscan.body.req.left = 0;
	msg_setup->overscan.body.req.right = 0;
	BCM2835_MBOX_INIT_TAG(&msg_setup->allocate_buffer, ALLOCATE_BUFFER);
	msg_setup->allocate_buffer.body.req.alignment = 0x100;
	BCM2835_MBOX_INIT_TAG_NO_REQ(&msg_setup->pitch, GET_PITCH);

	ret = bcm2835_mbox_call_prop(BCM2835_MBOX_PROP_CHAN, &msg_setup->hdr);
	if (ret) {
		printf("bcm2835: Could not configure display\n");
		return ret;
	}
	*widthp = msg_setup->physical_w_h.body.resp.width;
	*heightp = msg_setup->physical_w_h.body.resp.height;
	*pitchp = msg_setup->pitch.body.resp.pitch;
	*fb_basep = bus_to_phys(
			msg_setup->allocate_buffer.body.resp.fb_address);
	*fb_sizep = msg_setup->allocate_buffer.body.resp.fb_size;

	return 0;
}
