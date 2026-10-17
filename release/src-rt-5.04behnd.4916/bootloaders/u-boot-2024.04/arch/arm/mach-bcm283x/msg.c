// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012 Stephen Warren
 */

#include <common.h>
#include <memalign.h>
#include <phys2bus.h>
#include <asm/arch/mbox.h>
#include <linux/delay.h>

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

struct msg_set_sdhost_clock {
	struct bcm2835_mbox_hdr hdr;
	struct bcm2835_mbox_tag_set_sdhost_clock set_sdhost_clock;
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

struct msg_notify_vl805_reset {
	struct bcm2835_mbox_hdr hdr;
	struct bcm2835_mbox_tag_pci_dev_addr dev_addr;
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
	u32 clock_rate = 0;

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

	clock_rate = msg_clk->get_clock_rate.body.resp.rate_hz;

	if (clock_rate == 0) {
		BCM2835_MBOX_INIT_HDR(msg_clk);
		BCM2835_MBOX_INIT_TAG(&msg_clk->get_clock_rate, GET_MAX_CLOCK_RATE);
		msg_clk->get_clock_rate.body.req.clock_id = clock_id;

		ret = bcm2835_mbox_call_prop(BCM2835_MBOX_PROP_CHAN, &msg_clk->hdr);
		if (ret) {
			printf("bcm2835: Could not query max eMMC clock rate\n");
			return -EIO;
		}

		clock_rate = msg_clk->get_clock_rate.body.resp.rate_hz;
	}

	return clock_rate;
}

int bcm2835_set_sdhost_clock(u32 rate_hz, u32 *rate_1, u32 *rate_2)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct msg_set_sdhost_clock, msg_sdhost_clk, 1);
	int ret;

	BCM2835_MBOX_INIT_HDR(msg_sdhost_clk);
	BCM2835_MBOX_INIT_TAG(&msg_sdhost_clk->set_sdhost_clock, SET_SDHOST_CLOCK);

	msg_sdhost_clk->set_sdhost_clock.body.req.rate_hz = rate_hz;

	ret = bcm2835_mbox_call_prop(BCM2835_MBOX_PROP_CHAN, &msg_sdhost_clk->hdr);
	if (ret) {
		printf("bcm2835: Could not query sdhost clock rate\n");
		return -EIO;
	}

	*rate_1 = msg_sdhost_clk->set_sdhost_clock.body.resp.rate_1;
	*rate_2 = msg_sdhost_clk->set_sdhost_clock.body.resp.rate_2;

	return 0;
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

/*
 * On the Raspberry Pi 4, after a PCI reset, VL805's (the xHCI chip) firmware
 * may either be loaded directly from an EEPROM or, if not present, by the
 * SoC's VideoCore. This informs VideoCore that VL805 needs its firmware
 * loaded.
 */
int bcm2711_notify_vl805_reset(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct msg_notify_vl805_reset,
				 msg_notify_vl805_reset, 1);
	int ret;
	static int done = false;

	if (done)
		return 0;

	done = true;

	BCM2835_MBOX_INIT_HDR(msg_notify_vl805_reset);
	BCM2835_MBOX_INIT_TAG(&msg_notify_vl805_reset->dev_addr,
			      NOTIFY_XHCI_RESET);

	/*
	 * The pci device address is expected like this:
	 *
	 *   PCI_BUS << 20 | PCI_SLOT << 15 | PCI_FUNC << 12
	 *
	 * But since RPi4's PCIe setup is hardwired, we know the address in
	 * advance.
	 */
	msg_notify_vl805_reset->dev_addr.body.req.dev_addr = 0x100000;

	ret = bcm2835_mbox_call_prop(BCM2835_MBOX_PROP_CHAN,
				     &msg_notify_vl805_reset->hdr);
	if (ret) {
		printf("bcm2711: Failed to load vl805's firmware, %d\n", ret);
		return -EIO;
	}

	udelay(200);

	return 0;
}
