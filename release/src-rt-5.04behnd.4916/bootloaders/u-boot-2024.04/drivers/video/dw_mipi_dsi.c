// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016, Fuzhou Rockchip Electronics Co., Ltd
 * Copyright (C) 2019, STMicroelectronics - All Rights Reserved
 * Author(s): Philippe Cornu <philippe.cornu@st.com> for STMicroelectronics.
 *            Yannick Fertre <yannick.fertre@st.com> for STMicroelectronics.
 *
 * This generic Synopsys DesignWare MIPI DSI host driver is inspired from
 * the Linux Kernel driver drivers/gpu/drm/bridge/synopsys/dw-mipi-dsi.c.
 */

#include <common.h>
#include <clk.h>
#include <dsi_host.h>
#include <dm.h>
#include <errno.h>
#include <panel.h>
#include <video.h>
#include <asm/io.h>
#include <dm/device-internal.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/iopoll.h>
#include <linux/time.h>
#include <video_bridge.h>

#define HWVER_131			0x31333100	/* IP version 1.31 */

#define DSI_VERSION			0x00
#define VERSION				GENMASK(31, 8)

#define DSI_PWR_UP			0x04
#define RESET				0
#define POWERUP				BIT(0)

#define DSI_CLKMGR_CFG			0x08
#define TO_CLK_DIVISION(div)		(((div) & 0xff) << 8)
#define TX_ESC_CLK_DIVISION(div)	((div) & 0xff)

#define DSI_DPI_VCID			0x0c
#define DPI_VCID(vcid)			((vcid) & 0x3)

#define DSI_DPI_COLOR_CODING		0x10
#define LOOSELY18_EN			BIT(8)
#define DPI_COLOR_CODING_16BIT_1	0x0
#define DPI_COLOR_CODING_16BIT_2	0x1
#define DPI_COLOR_CODING_16BIT_3	0x2
#define DPI_COLOR_CODING_18BIT_1	0x3
#define DPI_COLOR_CODING_18BIT_2	0x4
#define DPI_COLOR_CODING_24BIT		0x5

#define DSI_DPI_CFG_POL			0x14
#define COLORM_ACTIVE_LOW		BIT(4)
#define SHUTD_ACTIVE_LOW		BIT(3)
#define HSYNC_ACTIVE_LOW		BIT(2)
#define VSYNC_ACTIVE_LOW		BIT(1)
#define DATAEN_ACTIVE_LOW		BIT(0)

#define DSI_DPI_LP_CMD_TIM		0x18
#define OUTVACT_LPCMD_TIME(p)		(((p) & 0xff) << 16)
#define INVACT_LPCMD_TIME(p)		((p) & 0xff)

#define DSI_DBI_VCID			0x1c
#define DSI_DBI_CFG			0x20
#define DSI_DBI_PARTITIONING_EN		0x24
#define DSI_DBI_CMDSIZE			0x28

#define DSI_PCKHDL_CFG			0x2c
#define CRC_RX_EN			BIT(4)
#define ECC_RX_EN			BIT(3)
#define BTA_EN				BIT(2)
#define EOTP_RX_EN			BIT(1)
#define EOTP_TX_EN			BIT(0)

#define DSI_GEN_VCID			0x30

#define DSI_MODE_CFG			0x34
#define ENABLE_VIDEO_MODE		0
#define ENABLE_CMD_MODE			BIT(0)

#define DSI_VID_MODE_CFG		0x38
#define ENABLE_LOW_POWER		(0x3f << 8)
#define ENABLE_LOW_POWER_MASK		(0x3f << 8)
#define VID_MODE_TYPE_NON_BURST_SYNC_PULSES	0x0
#define VID_MODE_TYPE_NON_BURST_SYNC_EVENTS	0x1
#define VID_MODE_TYPE_BURST			0x2
#define VID_MODE_TYPE_MASK			0x3

#define DSI_VID_PKT_SIZE		0x3c
#define VID_PKT_SIZE(p)			((p) & 0x3fff)

#define DSI_VID_NUM_CHUNKS		0x40
#define VID_NUM_CHUNKS(c)		((c) & 0x1fff)

#define DSI_VID_NULL_SIZE		0x44
#define VID_NULL_SIZE(b)		((b) & 0x1fff)

#define DSI_VID_HSA_TIME		0x48
#define DSI_VID_HBP_TIME		0x4c
#define DSI_VID_HLINE_TIME		0x50
#define DSI_VID_VSA_LINES		0x54
#define DSI_VID_VBP_LINES		0x58
#define DSI_VID_VFP_LINES		0x5c
#define DSI_VID_VACTIVE_LINES		0x60
#define DSI_EDPI_CMD_SIZE		0x64

#define DSI_CMD_MODE_CFG		0x68
#define MAX_RD_PKT_SIZE_LP		BIT(24)
#define DCS_LW_TX_LP			BIT(19)
#define DCS_SR_0P_TX_LP			BIT(18)
#define DCS_SW_1P_TX_LP			BIT(17)
#define DCS_SW_0P_TX_LP			BIT(16)
#define GEN_LW_TX_LP			BIT(14)
#define GEN_SR_2P_TX_LP			BIT(13)
#define GEN_SR_1P_TX_LP			BIT(12)
#define GEN_SR_0P_TX_LP			BIT(11)
#define GEN_SW_2P_TX_LP			BIT(10)
#define GEN_SW_1P_TX_LP			BIT(9)
#define GEN_SW_0P_TX_LP			BIT(8)
#define ACK_RQST_EN			BIT(1)
#define TEAR_FX_EN			BIT(0)

#define CMD_MODE_ALL_LP			(MAX_RD_PKT_SIZE_LP | \
					 DCS_LW_TX_LP | \
					 DCS_SR_0P_TX_LP | \
					 DCS_SW_1P_TX_LP | \
					 DCS_SW_0P_TX_LP | \
					 GEN_LW_TX_LP | \
					 GEN_SR_2P_TX_LP | \
					 GEN_SR_1P_TX_LP | \
					 GEN_SR_0P_TX_LP | \
					 GEN_SW_2P_TX_LP | \
					 GEN_SW_1P_TX_LP | \
					 GEN_SW_0P_TX_LP)

#define DSI_GEN_HDR			0x6c
#define DSI_GEN_PLD_DATA		0x70

#define DSI_CMD_PKT_STATUS		0x74
#define GEN_RD_CMD_BUSY			BIT(6)
#define GEN_PLD_R_FULL			BIT(5)
#define GEN_PLD_R_EMPTY			BIT(4)
#define GEN_PLD_W_FULL			BIT(3)
#define GEN_PLD_W_EMPTY			BIT(2)
#define GEN_CMD_FULL			BIT(1)
#define GEN_CMD_EMPTY			BIT(0)

#define DSI_TO_CNT_CFG			0x78
#define HSTX_TO_CNT(p)			(((p) & 0xffff) << 16)
#define LPRX_TO_CNT(p)			((p) & 0xffff)

#define DSI_HS_RD_TO_CNT		0x7c
#define DSI_LP_RD_TO_CNT		0x80
#define DSI_HS_WR_TO_CNT		0x84
#define DSI_LP_WR_TO_CNT		0x88
#define DSI_BTA_TO_CNT			0x8c

#define DSI_LPCLK_CTRL			0x94
#define AUTO_CLKLANE_CTRL		BIT(1)
#define PHY_TXREQUESTCLKHS		BIT(0)

#define DSI_PHY_TMR_LPCLK_CFG		0x98
#define PHY_CLKHS2LP_TIME(lbcc)		(((lbcc) & 0x3ff) << 16)
#define PHY_CLKLP2HS_TIME(lbcc)		((lbcc) & 0x3ff)

#define DSI_PHY_TMR_CFG			0x9c
#define PHY_HS2LP_TIME(lbcc)		(((lbcc) & 0xff) << 24)
#define PHY_LP2HS_TIME(lbcc)		(((lbcc) & 0xff) << 16)
#define MAX_RD_TIME(lbcc)		((lbcc) & 0x7fff)
#define PHY_HS2LP_TIME_V131(lbcc)	(((lbcc) & 0x3ff) << 16)
#define PHY_LP2HS_TIME_V131(lbcc)	((lbcc) & 0x3ff)

#define DSI_PHY_RSTZ			0xa0
#define PHY_DISFORCEPLL			0
#define PHY_ENFORCEPLL			BIT(3)
#define PHY_DISABLECLK			0
#define PHY_ENABLECLK			BIT(2)
#define PHY_RSTZ			0
#define PHY_UNRSTZ			BIT(1)
#define PHY_SHUTDOWNZ			0
#define PHY_UNSHUTDOWNZ			BIT(0)

#define DSI_PHY_IF_CFG			0xa4
#define PHY_STOP_WAIT_TIME(cycle)	(((cycle) & 0xff) << 8)
#define N_LANES(n)			(((n) - 1) & 0x3)

#define DSI_PHY_ULPS_CTRL		0xa8
#define DSI_PHY_TX_TRIGGERS		0xac

#define DSI_PHY_STATUS			0xb0
#define PHY_STOP_STATE_CLK_LANE		BIT(2)
#define PHY_LOCK			BIT(0)

#define DSI_PHY_TST_CTRL0		0xb4
#define PHY_TESTCLK			BIT(1)
#define PHY_UNTESTCLK			0
#define PHY_TESTCLR			BIT(0)
#define PHY_UNTESTCLR			0

#define DSI_PHY_TST_CTRL1		0xb8
#define PHY_TESTEN			BIT(16)
#define PHY_UNTESTEN			0
#define PHY_TESTDOUT(n)			(((n) & 0xff) << 8)
#define PHY_TESTDIN(n)			((n) & 0xff)

#define DSI_INT_ST0			0xbc
#define DSI_INT_ST1			0xc0
#define DSI_INT_MSK0			0xc4
#define DSI_INT_MSK1			0xc8

#define DSI_PHY_TMR_RD_CFG		0xf4
#define MAX_RD_TIME_V131(lbcc)		((lbcc) & 0x7fff)

#define PHY_STATUS_TIMEOUT_US		10000
#define CMD_PKT_STATUS_TIMEOUT_US	20000

struct dw_mipi_dsi {
	struct mipi_dsi_host dsi_host;
	struct mipi_dsi_device *device;
	void __iomem *base;
	unsigned int lane_mbps; /* per lane */
	u32 channel;
	unsigned int max_data_lanes;
	const struct mipi_dsi_phy_ops *phy_ops;
};

static int dsi_mode_vrefresh(struct display_timing *timings)
{
	int refresh = 0;
	unsigned int calc_val;
	u32 htotal = timings->hactive.typ + timings->hfront_porch.typ +
		     timings->hback_porch.typ + timings->hsync_len.typ;
	u32 vtotal = timings->vactive.typ + timings->vfront_porch.typ +
		     timings->vback_porch.typ + timings->vsync_len.typ;

	if (htotal > 0 && vtotal > 0) {
		calc_val = timings->pixelclock.typ;
		calc_val /= htotal;
		refresh = (calc_val + vtotal / 2) / vtotal;
	}

	return refresh;
}

/*
 * The controller should generate 2 frames before
 * preparing the peripheral.
 */
static void dw_mipi_dsi_wait_for_two_frames(struct display_timing *timings)
{
	int refresh, two_frames;

	refresh = dsi_mode_vrefresh(timings);
	two_frames = DIV_ROUND_UP(MSEC_PER_SEC, refresh) * 2;
	mdelay(two_frames);
}

static inline struct dw_mipi_dsi *host_to_dsi(struct mipi_dsi_host *host)
{
	return container_of(host, struct dw_mipi_dsi, dsi_host);
}

static inline void dsi_write(struct dw_mipi_dsi *dsi, u32 reg, u32 val)
{
	writel(val, dsi->base + reg);
}

static inline u32 dsi_read(struct dw_mipi_dsi *dsi, u32 reg)
{
	return readl(dsi->base + reg);
}

static int dw_mipi_dsi_host_attach(struct mipi_dsi_host *host,
				   struct mipi_dsi_device *device)
{
	struct dw_mipi_dsi *dsi = host_to_dsi(host);

	if (device->lanes > dsi->max_data_lanes) {
		dev_err(device->dev,
			"the number of data lanes(%u) is too many\n",
			device->lanes);
		return -EINVAL;
	}

	dsi->channel = device->channel;

	return 0;
}

static void dw_mipi_message_config(struct dw_mipi_dsi *dsi,
				   const struct mipi_dsi_msg *msg)
{
	bool lpm = msg->flags & MIPI_DSI_MSG_USE_LPM;
	u32 val = 0;

	if (msg->flags & MIPI_DSI_MSG_REQ_ACK)
		val |= ACK_RQST_EN;
	if (lpm)
		val |= CMD_MODE_ALL_LP;

	dsi_write(dsi, DSI_LPCLK_CTRL, lpm ? 0 : PHY_TXREQUESTCLKHS);
	dsi_write(dsi, DSI_CMD_MODE_CFG, val);
}

static int dw_mipi_dsi_gen_pkt_hdr_write(struct dw_mipi_dsi *dsi, u32 hdr_val)
{
	int ret;
	u32 val, mask;

	ret = readl_poll_timeout(dsi->base + DSI_CMD_PKT_STATUS,
				 val, !(val & GEN_CMD_FULL),
				 CMD_PKT_STATUS_TIMEOUT_US);
	if (ret) {
		dev_err(dsi->dsi_host.dev,
			"failed to get available command FIFO\n");
		return ret;
	}

	dsi_write(dsi, DSI_GEN_HDR, hdr_val);

	mask = GEN_CMD_EMPTY | GEN_PLD_W_EMPTY;
	ret = readl_poll_timeout(dsi->base + DSI_CMD_PKT_STATUS,
				 val, (val & mask) == mask,
				 CMD_PKT_STATUS_TIMEOUT_US);
	if (ret) {
		dev_err(dsi->dsi_host.dev, "failed to write command FIFO\n");
		return ret;
	}

	return 0;
}

static int dw_mipi_dsi_write(struct dw_mipi_dsi *dsi,
			     const struct mipi_dsi_packet *packet)
{
	const u8 *tx_buf = packet->payload;
	int len = packet->payload_length, pld_data_bytes = sizeof(u32), ret;
	__le32 word;
	u32 val;

	while (len) {
		if (len < pld_data_bytes) {
			word = 0;
			memcpy(&word, tx_buf, len);
			dsi_write(dsi, DSI_GEN_PLD_DATA, le32_to_cpu(word));
			len = 0;
		} else {
			memcpy(&word, tx_buf, pld_data_bytes);
			dsi_write(dsi, DSI_GEN_PLD_DATA, le32_to_cpu(word));
			tx_buf += pld_data_bytes;
			len -= pld_data_bytes;
		}

		ret = readl_poll_timeout(dsi->base + DSI_CMD_PKT_STATUS,
					 val, !(val & GEN_PLD_W_FULL),
					 CMD_PKT_STATUS_TIMEOUT_US);
		if (ret) {
			dev_err(dsi->dsi_host.dev,
				"failed to get available write payload FIFO\n");
			return ret;
		}
	}

	word = 0;
	memcpy(&word, packet->header, sizeof(packet->header));
	return dw_mipi_dsi_gen_pkt_hdr_write(dsi, le32_to_cpu(word));
}

static int dw_mipi_dsi_read(struct dw_mipi_dsi *dsi,
			    const struct mipi_dsi_msg *msg)
{
	int i, j, ret, len = msg->rx_len;
	u8 *buf = msg->rx_buf;
	u32 val;

	/* Wait end of the read operation */
	ret = readl_poll_timeout(dsi->base + DSI_CMD_PKT_STATUS,
				 val, !(val & GEN_RD_CMD_BUSY),
				 CMD_PKT_STATUS_TIMEOUT_US);
	if (ret) {
		dev_err(dsi->dsi_host.dev, "Timeout during read operation\n");
		return ret;
	}

	for (i = 0; i < len; i += 4) {
		/* Read fifo must not be empty before all bytes are read */
		ret = readl_poll_timeout(dsi->base + DSI_CMD_PKT_STATUS,
					 val, !(val & GEN_PLD_R_EMPTY),
					 CMD_PKT_STATUS_TIMEOUT_US);
		if (ret) {
			dev_err(dsi->dsi_host.dev,
				"Read payload FIFO is empty\n");
			return ret;
		}

		val = dsi_read(dsi, DSI_GEN_PLD_DATA);
		for (j = 0; j < 4 && j + i < len; j++)
			buf[i + j] = val >> (8 * j);
	}

	return ret;
}

static ssize_t dw_mipi_dsi_host_transfer(struct mipi_dsi_host *host,
					 const struct mipi_dsi_msg *msg)
{
	struct dw_mipi_dsi *dsi = host_to_dsi(host);
	struct mipi_dsi_packet packet;
	int ret, nb_bytes;

	ret = mipi_dsi_create_packet(&packet, msg);
	if (ret) {
		dev_err(host->dev, "failed to create packet: %d\n", ret);
		return ret;
	}

	dw_mipi_message_config(dsi, msg);

	ret = dw_mipi_dsi_write(dsi, &packet);
	if (ret)
		return ret;

	if (msg->rx_buf && msg->rx_len) {
		ret = dw_mipi_dsi_read(dsi, msg);
		if (ret)
			return ret;
		nb_bytes = msg->rx_len;
	} else {
		nb_bytes = packet.size;
	}

	return nb_bytes;
}

static const struct mipi_dsi_host_ops dw_mipi_dsi_host_ops = {
	.attach = dw_mipi_dsi_host_attach,
	.transfer = dw_mipi_dsi_host_transfer,
};

static void dw_mipi_dsi_video_mode_config(struct dw_mipi_dsi *dsi)
{
	struct mipi_dsi_device *device = dsi->device;
	u32 val;

	/*
	 * TODO dw drv improvements
	 * enabling low power is panel-dependent, we should use the
	 * panel configuration here...
	 */
	val = ENABLE_LOW_POWER;

	if (device->mode_flags & MIPI_DSI_MODE_VIDEO_BURST)
		val |= VID_MODE_TYPE_BURST;
	else if (device->mode_flags & MIPI_DSI_MODE_VIDEO_SYNC_PULSE)
		val |= VID_MODE_TYPE_NON_BURST_SYNC_PULSES;
	else
		val |= VID_MODE_TYPE_NON_BURST_SYNC_EVENTS;

	dsi_write(dsi, DSI_VID_MODE_CFG, val);
}

static void dw_mipi_dsi_set_mode(struct dw_mipi_dsi *dsi,
				 unsigned long mode_flags)
{
	const struct mipi_dsi_phy_ops *phy_ops = dsi->phy_ops;

	dsi_write(dsi, DSI_PWR_UP, RESET);

	if (mode_flags & MIPI_DSI_MODE_VIDEO) {
		dsi_write(dsi, DSI_MODE_CFG, ENABLE_VIDEO_MODE);
		dw_mipi_dsi_video_mode_config(dsi);
		dsi_write(dsi, DSI_LPCLK_CTRL, PHY_TXREQUESTCLKHS);
	} else {
		dsi_write(dsi, DSI_MODE_CFG, ENABLE_CMD_MODE);
	}

	if (phy_ops->post_set_mode)
		phy_ops->post_set_mode(dsi->device, mode_flags);

	dsi_write(dsi, DSI_PWR_UP, POWERUP);
}

static void dw_mipi_dsi_init_pll(struct dw_mipi_dsi *dsi)
{
	const struct mipi_dsi_phy_ops *phy_ops = dsi->phy_ops;
	unsigned int esc_rate;
	u32 esc_clk_division;

	/*
	 * The maximum permitted escape clock is 20MHz and it is derived from
	 * lanebyteclk, which is running at "lane_mbps / 8".
	 */
	if (phy_ops->get_esc_clk_rate)
		phy_ops->get_esc_clk_rate(dsi->device, &esc_rate);
	else
		esc_rate = 20; /* Default to 20MHz */

	/*
	 * We want:
	 *
	 *     (lane_mbps >> 3) / esc_clk_division < X
	 * which is:
	 *     (lane_mbps >> 3) / X > esc_clk_division
	 */
	esc_clk_division = (dsi->lane_mbps >> 3) / esc_rate + 1;

	dsi_write(dsi, DSI_PWR_UP, RESET);

	/*
	 * TODO dw drv improvements
	 * timeout clock division should be computed with the
	 * high speed transmission counter timeout and byte lane...
	 */
	dsi_write(dsi, DSI_CLKMGR_CFG, TO_CLK_DIVISION(10) |
		  TX_ESC_CLK_DIVISION(esc_clk_division));
}

static void dw_mipi_dsi_dpi_config(struct dw_mipi_dsi *dsi,
				   struct display_timing *timings)
{
	struct mipi_dsi_device *device = dsi->device;
	u32 val = 0, color = 0;

	switch (device->format) {
	case MIPI_DSI_FMT_RGB888:
		color = DPI_COLOR_CODING_24BIT;
		break;
	case MIPI_DSI_FMT_RGB666:
		color = DPI_COLOR_CODING_18BIT_2 | LOOSELY18_EN;
		break;
	case MIPI_DSI_FMT_RGB666_PACKED:
		color = DPI_COLOR_CODING_18BIT_1;
		break;
	case MIPI_DSI_FMT_RGB565:
		color = DPI_COLOR_CODING_16BIT_1;
		break;
	}

	if (timings->flags & DISPLAY_FLAGS_VSYNC_LOW)
		val |= VSYNC_ACTIVE_LOW;
	if (timings->flags & DISPLAY_FLAGS_HSYNC_LOW)
		val |= HSYNC_ACTIVE_LOW;

	dsi_write(dsi, DSI_DPI_VCID, DPI_VCID(dsi->channel));
	dsi_write(dsi, DSI_DPI_COLOR_CODING, color);
	dsi_write(dsi, DSI_DPI_CFG_POL, val);
	/*
	 * TODO dw drv improvements
	 * largest packet sizes during hfp or during vsa/vpb/vfp
	 * should be computed according to byte lane, lane number and only
	 * if sending lp cmds in high speed is enable (PHY_TXREQUESTCLKHS)
	 */
	dsi_write(dsi, DSI_DPI_LP_CMD_TIM, OUTVACT_LPCMD_TIME(4)
		  | INVACT_LPCMD_TIME(4));
}

static void dw_mipi_dsi_packet_handler_config(struct dw_mipi_dsi *dsi)
{
	dsi_write(dsi, DSI_PCKHDL_CFG, CRC_RX_EN | ECC_RX_EN | BTA_EN);
}

static void dw_mipi_dsi_video_packet_config(struct dw_mipi_dsi *dsi,
					    struct display_timing *timings)
{
	/*
	 * TODO dw drv improvements
	 * only burst mode is supported here. For non-burst video modes,
	 * we should compute DSI_VID_PKT_SIZE, DSI_VCCR.NUMC &
	 * DSI_VNPCR.NPSIZE... especially because this driver supports
	 * non-burst video modes, see dw_mipi_dsi_video_mode_config()...
	 */
	dsi_write(dsi, DSI_VID_PKT_SIZE, VID_PKT_SIZE(timings->hactive.typ));
}

static void dw_mipi_dsi_command_mode_config(struct dw_mipi_dsi *dsi)
{
	const struct mipi_dsi_phy_ops *phy_ops = dsi->phy_ops;

	/*
	 * TODO dw drv improvements
	 * compute high speed transmission counter timeout according
	 * to the timeout clock division (TO_CLK_DIVISION) and byte lane...
	 */
	dsi_write(dsi, DSI_TO_CNT_CFG, HSTX_TO_CNT(1000) | LPRX_TO_CNT(1000));
	/*
	 * TODO dw drv improvements
	 * the Bus-Turn-Around Timeout Counter should be computed
	 * according to byte lane...
	 */
	dsi_write(dsi, DSI_BTA_TO_CNT, 0xd00);
	dsi_write(dsi, DSI_MODE_CFG, ENABLE_CMD_MODE);

	if (phy_ops->post_set_mode)
		phy_ops->post_set_mode(dsi->device, 0);
}

/* Get lane byte clock cycles. */
static u32 dw_mipi_dsi_get_hcomponent_lbcc(struct dw_mipi_dsi *dsi,
					   struct display_timing *timings,
					   u32 hcomponent)
{
	u32 frac, lbcc;

	lbcc = hcomponent * dsi->lane_mbps * MSEC_PER_SEC / 8;

	frac = lbcc % (timings->pixelclock.typ / 1000);
	lbcc = lbcc / (timings->pixelclock.typ / 1000);
	if (frac)
		lbcc++;

	return lbcc;
}

static void dw_mipi_dsi_line_timer_config(struct dw_mipi_dsi *dsi,
					  struct display_timing *timings)
{
	u32 htotal, hsa, hbp, lbcc;

	htotal = timings->hactive.typ + timings->hfront_porch.typ +
		 timings->hback_porch.typ + timings->hsync_len.typ;

	hsa = timings->hsync_len.typ;
	hbp = timings->hback_porch.typ;

	/*
	 * TODO dw drv improvements
	 * computations below may be improved...
	 */
	lbcc = dw_mipi_dsi_get_hcomponent_lbcc(dsi, timings, htotal);
	dsi_write(dsi, DSI_VID_HLINE_TIME, lbcc);

	lbcc = dw_mipi_dsi_get_hcomponent_lbcc(dsi, timings, hsa);
	dsi_write(dsi, DSI_VID_HSA_TIME, lbcc);

	lbcc = dw_mipi_dsi_get_hcomponent_lbcc(dsi, timings, hbp);
	dsi_write(dsi, DSI_VID_HBP_TIME, lbcc);
}

static void dw_mipi_dsi_vertical_timing_config(struct dw_mipi_dsi *dsi,
					       struct display_timing *timings)
{
	u32 vactive, vsa, vfp, vbp;

	vactive = timings->vactive.typ;
	vsa =  timings->vsync_len.typ;
	vfp =  timings->vfront_porch.typ;
	vbp = timings->vback_porch.typ;

	dsi_write(dsi, DSI_VID_VACTIVE_LINES, vactive);
	dsi_write(dsi, DSI_VID_VSA_LINES, vsa);
	dsi_write(dsi, DSI_VID_VFP_LINES, vfp);
	dsi_write(dsi, DSI_VID_VBP_LINES, vbp);
}

static void dw_mipi_dsi_dphy_timing_config(struct dw_mipi_dsi *dsi)
{
	const struct mipi_dsi_phy_ops *phy_ops = dsi->phy_ops;
	struct mipi_dsi_phy_timing timing = {0x40, 0x40, 0x40, 0x40};
	u32 hw_version;

	if (phy_ops->get_timing)
		phy_ops->get_timing(dsi->device, dsi->lane_mbps, &timing);

	/*
	 * TODO dw drv improvements
	 * data & clock lane timers should be computed according to panel
	 * blankings and to the automatic clock lane control mode...
	 * note: DSI_PHY_TMR_CFG.MAX_RD_TIME should be in line with
	 * DSI_CMD_MODE_CFG.MAX_RD_PKT_SIZE_LP (see CMD_MODE_ALL_LP)
	 */

	hw_version = dsi_read(dsi, DSI_VERSION) & VERSION;

	if (hw_version >= HWVER_131) {
		dsi_write(dsi, DSI_PHY_TMR_CFG, PHY_HS2LP_TIME_V131(timing.data_hs2lp) |
			  PHY_LP2HS_TIME_V131(timing.data_lp2hs));
		dsi_write(dsi, DSI_PHY_TMR_RD_CFG, MAX_RD_TIME_V131(10000));
	} else {
		dsi_write(dsi, DSI_PHY_TMR_CFG, PHY_HS2LP_TIME(timing.data_hs2lp) |
			  PHY_LP2HS_TIME(timing.data_lp2hs) | MAX_RD_TIME(10000));
	}

	dsi_write(dsi, DSI_PHY_TMR_LPCLK_CFG, PHY_CLKHS2LP_TIME(timing.clk_hs2lp)
		  | PHY_CLKLP2HS_TIME(timing.clk_lp2hs));
}

static void dw_mipi_dsi_dphy_interface_config(struct dw_mipi_dsi *dsi)
{
	struct mipi_dsi_device *device = dsi->device;

	/*
	 * TODO dw drv improvements
	 * stop wait time should be the maximum between host dsi
	 * and panel stop wait times
	 */
	dsi_write(dsi, DSI_PHY_IF_CFG, PHY_STOP_WAIT_TIME(0x20) |
		  N_LANES(device->lanes));
}

static void dw_mipi_dsi_dphy_init(struct dw_mipi_dsi *dsi)
{
	/* Clear PHY state */
	dsi_write(dsi, DSI_PHY_RSTZ, PHY_DISFORCEPLL | PHY_DISABLECLK
		  | PHY_RSTZ | PHY_SHUTDOWNZ);
	dsi_write(dsi, DSI_PHY_TST_CTRL0, PHY_UNTESTCLR);
	dsi_write(dsi, DSI_PHY_TST_CTRL0, PHY_TESTCLR);
	dsi_write(dsi, DSI_PHY_TST_CTRL0, PHY_UNTESTCLR);
}

static void dw_mipi_dsi_dphy_enable(struct dw_mipi_dsi *dsi)
{
	u32 val;
	int ret;

	dsi_write(dsi, DSI_PHY_RSTZ, PHY_ENFORCEPLL | PHY_ENABLECLK |
		  PHY_UNRSTZ | PHY_UNSHUTDOWNZ);

	ret = readl_poll_timeout(dsi->base + DSI_PHY_STATUS, val,
				 val & PHY_LOCK, PHY_STATUS_TIMEOUT_US);
	if (ret)
		dev_dbg(dsi->dsi_host.dev,
			"failed to wait phy lock state\n");

	ret = readl_poll_timeout(dsi->base + DSI_PHY_STATUS,
				 val, val & PHY_STOP_STATE_CLK_LANE,
				 PHY_STATUS_TIMEOUT_US);
	if (ret)
		dev_dbg(dsi->dsi_host.dev,
			"failed to wait phy clk lane stop state\n");
}

static void dw_mipi_dsi_clear_err(struct dw_mipi_dsi *dsi)
{
	dsi_read(dsi, DSI_INT_ST0);
	dsi_read(dsi, DSI_INT_ST1);
	dsi_write(dsi, DSI_INT_MSK0, 0);
	dsi_write(dsi, DSI_INT_MSK1, 0);
}

static void dw_mipi_dsi_bridge_set(struct dw_mipi_dsi *dsi,
				   struct display_timing *timings)
{
	const struct mipi_dsi_phy_ops *phy_ops = dsi->phy_ops;
	struct mipi_dsi_device *device = dsi->device;
	int ret;

	ret = phy_ops->get_lane_mbps(dsi->device, timings, device->lanes,
				     device->format, &dsi->lane_mbps);
	if (ret)
		dev_warn(dsi->dsi_host.dev, "Phy get_lane_mbps() failed\n");

	dw_mipi_dsi_init_pll(dsi);
	dw_mipi_dsi_dpi_config(dsi, timings);
	dw_mipi_dsi_packet_handler_config(dsi);
	dw_mipi_dsi_video_mode_config(dsi);
	dw_mipi_dsi_video_packet_config(dsi, timings);
	dw_mipi_dsi_command_mode_config(dsi);
	dw_mipi_dsi_line_timer_config(dsi, timings);
	dw_mipi_dsi_vertical_timing_config(dsi, timings);

	dw_mipi_dsi_dphy_init(dsi);
	dw_mipi_dsi_dphy_timing_config(dsi);
	dw_mipi_dsi_dphy_interface_config(dsi);

	dw_mipi_dsi_clear_err(dsi);

	ret = phy_ops->init(dsi->device);
	if (ret)
		dev_warn(dsi->dsi_host.dev, "Phy init() failed\n");

	dw_mipi_dsi_dphy_enable(dsi);

	dw_mipi_dsi_wait_for_two_frames(timings);

	/* Switch to cmd mode for panel-bridge pre_enable & panel prepare */
	dw_mipi_dsi_set_mode(dsi, 0);
}

static int dw_mipi_dsi_init(struct udevice *dev,
			    struct mipi_dsi_device *device,
			    struct display_timing *timings,
			    unsigned int max_data_lanes,
			    const struct mipi_dsi_phy_ops *phy_ops)
{
	struct dw_mipi_dsi *dsi = dev_get_priv(dev);
	struct clk clk;
	int ret;

	if (!phy_ops->init || !phy_ops->get_lane_mbps) {
		dev_err(device->dev, "Phy not properly configured\n");
		return -ENODEV;
	}

	dsi->phy_ops = phy_ops;
	dsi->max_data_lanes = max_data_lanes;
	dsi->device = device;
	dsi->dsi_host.dev = (struct device *)dev;
	dsi->dsi_host.ops = &dw_mipi_dsi_host_ops;
	device->host = &dsi->dsi_host;

	dsi->base = dev_read_addr_ptr(device->dev);
	if (!dsi->base) {
		dev_err(device->dev, "dsi dt register address error\n");
		return -EINVAL;
	}

	/*
	 * The Rockchip based devices don't have px_clk, so simply move
	 * on.
	 */
	if (IS_ENABLED(CONFIG_DISPLAY_ROCKCHIP_DW_MIPI)) {
		dw_mipi_dsi_bridge_set(dsi, timings);
		return 0;
	}

	ret = clk_get_by_name(device->dev, "px_clk", &clk);
	if (ret) {
		dev_err(device->dev, "peripheral clock get error %d\n", ret);
		return ret;
	}

	/*  get the pixel clock set by the clock framework */
	timings->pixelclock.typ = clk_get_rate(&clk);

	dw_mipi_dsi_bridge_set(dsi, timings);

	return 0;
}

static int dw_mipi_dsi_enable(struct udevice *dev)
{
	struct dw_mipi_dsi *dsi = dev_get_priv(dev);

	/* Switch to video mode for panel-bridge enable & panel enable */
	dw_mipi_dsi_set_mode(dsi, MIPI_DSI_MODE_VIDEO);

	return 0;
}

struct dsi_host_ops dw_mipi_dsi_ops = {
	.init = dw_mipi_dsi_init,
	.enable = dw_mipi_dsi_enable,
};

static int dw_mipi_dsi_probe(struct udevice *dev)
{
	return 0;
}

U_BOOT_DRIVER(dw_mipi_dsi) = {
	.name			= "dw_mipi_dsi",
	.id			= UCLASS_DSI_HOST,
	.probe			= dw_mipi_dsi_probe,
	.ops			= &dw_mipi_dsi_ops,
	.priv_auto	= sizeof(struct dw_mipi_dsi),
};

MODULE_AUTHOR("Chris Zhong <zyw@rock-chips.com>");
MODULE_AUTHOR("Philippe Cornu <philippe.cornu@st.com>");
MODULE_AUTHOR("Yannick Fertré <yannick.fertre@st.com>");
MODULE_DESCRIPTION("DW MIPI DSI host controller driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:dw-mipi-dsi");
