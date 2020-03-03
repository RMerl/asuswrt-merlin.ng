/*******************************************************************************
  This contains the functions to handle the enhanced descriptors.

  Copyright (C) 2007-2009  STMicroelectronics Ltd

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Author: Giuseppe Cavallaro <peppe.cavallaro@st.com>
*******************************************************************************/

#include <linux/stmmac.h>
#include "common.h"
#include "descs_com.h"

static int enh_desc_get_tx_status(void *data, struct stmmac_extra_stats *x,
				  struct dma_desc *p, void __iomem *ioaddr)
{
	int ret = 0;
	struct net_device_stats *stats = (struct net_device_stats *)data;

	if (unlikely(p->des01.etx.error_summary)) {
		if (unlikely(p->des01.etx.jabber_timeout))
			x->tx_jabber++;

		if (unlikely(p->des01.etx.frame_flushed)) {
			x->tx_frame_flushed++;
			dwmac_dma_flush_tx_fifo(ioaddr);
		}

		if (unlikely(p->des01.etx.loss_carrier)) {
			x->tx_losscarrier++;
			stats->tx_carrier_errors++;
		}
		if (unlikely(p->des01.etx.no_carrier)) {
			x->tx_carrier++;
			stats->tx_carrier_errors++;
		}
		if (unlikely(p->des01.etx.late_collision))
			stats->collisions += p->des01.etx.collision_count;

		if (unlikely(p->des01.etx.excessive_collisions))
			stats->collisions += p->des01.etx.collision_count;

		if (unlikely(p->des01.etx.excessive_deferral))
			x->tx_deferred++;

		if (unlikely(p->des01.etx.underflow_error)) {
			dwmac_dma_flush_tx_fifo(ioaddr);
			x->tx_underflow++;
		}

		if (unlikely(p->des01.etx.ip_header_error))
			x->tx_ip_header_error++;

		if (unlikely(p->des01.etx.payload_error)) {
			x->tx_payload_error++;
			dwmac_dma_flush_tx_fifo(ioaddr);
		}

		ret = -1;
	}

	if (unlikely(p->des01.etx.deferred))
		x->tx_deferred++;

#ifdef STMMAC_VLAN_TAG_USED
	if (p->des01.etx.vlan_frame)
		x->tx_vlan++;
#endif

	return ret;
}

static int enh_desc_get_tx_len(struct dma_desc *p)
{
	return p->des01.etx.buffer1_size;
}

static int enh_desc_coe_rdes0(int ipc_err, int type, int payload_err)
{
	int ret = good_frame;
	u32 status = (type << 2 | ipc_err << 1 | payload_err) & 0x7;

	/* bits 5 7 0 | Frame status
	 * ----------------------------------------------------------
	 *      0 0 0 | IEEE 802.3 Type frame (length < 1536 octects)
	 *      1 0 0 | IPv4/6 No CSUM errorS.
	 *      1 0 1 | IPv4/6 CSUM PAYLOAD error
	 *      1 1 0 | IPv4/6 CSUM IP HR error
	 *      1 1 1 | IPv4/6 IP PAYLOAD AND HEADER errorS
	 *      0 0 1 | IPv4/6 unsupported IP PAYLOAD
	 *      0 1 1 | COE bypassed.. no IPv4/6 frame
	 *      0 1 0 | Reserved.
	 */
	if (status == 0x0)
		ret = llc_snap;
	else if (status == 0x4)
		ret = good_frame;
	else if (status == 0x5)
		ret = csum_none;
	else if (status == 0x6)
		ret = csum_none;
	else if (status == 0x7)
		ret = csum_none;
	else if (status == 0x1)
		ret = discard_frame;
	else if (status == 0x3)
		ret = discard_frame;
	return ret;
}

static void enh_desc_get_ext_status(void *data, struct stmmac_extra_stats *x,
				    struct dma_extended_desc *p)
{
	if (unlikely(p->basic.des01.erx.rx_mac_addr)) {
		if (p->des4.erx.ip_hdr_err)
			x->ip_hdr_err++;
		if (p->des4.erx.ip_payload_err)
			x->ip_payload_err++;
		if (p->des4.erx.ip_csum_bypassed)
			x->ip_csum_bypassed++;
		if (p->des4.erx.ipv4_pkt_rcvd)
			x->ipv4_pkt_rcvd++;
		if (p->des4.erx.ipv6_pkt_rcvd)
			x->ipv6_pkt_rcvd++;
		if (p->des4.erx.msg_type == RDES_EXT_SYNC)
			x->rx_msg_type_sync++;
		else if (p->des4.erx.msg_type == RDES_EXT_FOLLOW_UP)
			x->rx_msg_type_follow_up++;
		else if (p->des4.erx.msg_type == RDES_EXT_DELAY_REQ)
			x->rx_msg_type_delay_req++;
		else if (p->des4.erx.msg_type == RDES_EXT_DELAY_RESP)
			x->rx_msg_type_delay_resp++;
		else if (p->des4.erx.msg_type == RDES_EXT_PDELAY_REQ)
			x->rx_msg_type_pdelay_req++;
		else if (p->des4.erx.msg_type == RDES_EXT_PDELAY_RESP)
			x->rx_msg_type_pdelay_resp++;
		else if (p->des4.erx.msg_type == RDES_EXT_PDELAY_FOLLOW_UP)
			x->rx_msg_type_pdelay_follow_up++;
		else
			x->rx_msg_type_ext_no_ptp++;
		if (p->des4.erx.ptp_frame_type)
			x->ptp_frame_type++;
		if (p->des4.erx.ptp_ver)
			x->ptp_ver++;
		if (p->des4.erx.timestamp_dropped)
			x->timestamp_dropped++;
		if (p->des4.erx.av_pkt_rcvd)
			x->av_pkt_rcvd++;
		if (p->des4.erx.av_tagged_pkt_rcvd)
			x->av_tagged_pkt_rcvd++;
		if (p->des4.erx.vlan_tag_priority_val)
			x->vlan_tag_priority_val++;
		if (p->des4.erx.l3_filter_match)
			x->l3_filter_match++;
		if (p->des4.erx.l4_filter_match)
			x->l4_filter_match++;
		if (p->des4.erx.l3_l4_filter_no_match)
			x->l3_l4_filter_no_match++;
	}
}

static int enh_desc_get_rx_status(void *data, struct stmmac_extra_stats *x,
				  struct dma_desc *p)
{
	int ret = good_frame;
	struct net_device_stats *stats = (struct net_device_stats *)data;

	if (unlikely(p->des01.erx.error_summary)) {
		if (unlikely(p->des01.erx.descriptor_error)) {
			x->rx_desc++;
			stats->rx_length_errors++;
		}
		if (unlikely(p->des01.erx.overflow_error))
			x->rx_gmac_overflow++;

		if (unlikely(p->des01.erx.ipc_csum_error))
			pr_err("\tIPC Csum Error/Giant frame\n");

		if (unlikely(p->des01.erx.late_collision)) {
			stats->collisions++;
		}
		if (unlikely(p->des01.erx.receive_watchdog))
			x->rx_watchdog++;

		if (unlikely(p->des01.erx.error_gmii))
			x->rx_mii++;

		if (unlikely(p->des01.erx.crc_error)) {
			x->rx_crc++;
			stats->rx_crc_errors++;
		}
		ret = discard_frame;
	}

	/* After a payload csum error, the ES bit is set.
	 * It doesn't match with the information reported into the databook.
	 * At any rate, we need to understand if the CSUM hw computation is ok
	 * and report this info to the upper layers. */
	ret = enh_desc_coe_rdes0(p->des01.erx.ipc_csum_error,
		p->des01.erx.frame_type, p->des01.erx.rx_mac_addr);

	if (unlikely(p->des01.erx.dribbling))
		x->dribbling_bit++;

	if (unlikely(p->des01.erx.sa_filter_fail)) {
		x->sa_rx_filter_fail++;
		ret = discard_frame;
	}
	if (unlikely(p->des01.erx.da_filter_fail)) {
		x->da_rx_filter_fail++;
		ret = discard_frame;
	}
	if (unlikely(p->des01.erx.length_error)) {
		x->rx_length++;
		ret = discard_frame;
	}
#ifdef STMMAC_VLAN_TAG_USED
	if (p->des01.erx.vlan_tag)
		x->rx_vlan++;
#endif

	return ret;
}

static void enh_desc_init_rx_desc(struct dma_desc *p, int disable_rx_ic,
				  int mode, int end)
{
	p->des01.all_flags = 0;
	p->des01.erx.own = 1;
	p->des01.erx.buffer1_size = BUF_SIZE_8KiB - 1;

	if (mode == STMMAC_CHAIN_MODE)
		ehn_desc_rx_set_on_chain(p, end);
	else
		ehn_desc_rx_set_on_ring(p, end);

	if (disable_rx_ic)
		p->des01.erx.disable_ic = 1;
}

static void enh_desc_init_tx_desc(struct dma_desc *p, int mode, int end)
{
	p->des01.all_flags = 0;
	if (mode == STMMAC_CHAIN_MODE)
		ehn_desc_tx_set_on_chain(p, end);
	else
		ehn_desc_tx_set_on_ring(p, end);
}

static int enh_desc_get_tx_owner(struct dma_desc *p)
{
	return p->des01.etx.own;
}

static int enh_desc_get_rx_owner(struct dma_desc *p)
{
	return p->des01.erx.own;
}

static void enh_desc_set_tx_owner(struct dma_desc *p)
{
	p->des01.etx.own = 1;
}

static void enh_desc_set_rx_owner(struct dma_desc *p)
{
	p->des01.erx.own = 1;
}

static int enh_desc_get_tx_ls(struct dma_desc *p)
{
	return p->des01.etx.last_segment;
}

static void enh_desc_release_tx_desc(struct dma_desc *p, int mode)
{
	int ter = p->des01.etx.end_ring;

	memset(p, 0, offsetof(struct dma_desc, des2));
	if (mode == STMMAC_CHAIN_MODE)
		enh_desc_end_tx_desc_on_chain(p, ter);
	else
		enh_desc_end_tx_desc_on_ring(p, ter);
}

static void enh_desc_prepare_tx_desc(struct dma_desc *p, int is_fs, int len,
				     int csum_flag, int mode)
{
	p->des01.etx.first_segment = is_fs;

	if (mode == STMMAC_CHAIN_MODE)
		enh_set_tx_desc_len_on_chain(p, len);
	else
		enh_set_tx_desc_len_on_ring(p, len);

	if (likely(csum_flag))
		p->des01.etx.checksum_insertion = cic_full;
}

static void enh_desc_clear_tx_ic(struct dma_desc *p)
{
	p->des01.etx.interrupt = 0;
}

static void enh_desc_close_tx_desc(struct dma_desc *p)
{
	p->des01.etx.last_segment = 1;
	p->des01.etx.interrupt = 1;
}

static int enh_desc_get_rx_frame_len(struct dma_desc *p, int rx_coe_type)
{
	/* The type-1 checksum offload engines append the checksum at
	 * the end of frame and the two bytes of checksum are added in
	 * the length.
	 * Adjust for that in the framelen for type-1 checksum offload
	 * engines. */
	if (rx_coe_type == STMMAC_RX_COE_TYPE1)
		return p->des01.erx.frame_length - 2;
	else
		return p->des01.erx.frame_length;
}

static void enh_desc_enable_tx_timestamp(struct dma_desc *p)
{
	p->des01.etx.time_stamp_enable = 1;
}

static int enh_desc_get_tx_timestamp_status(struct dma_desc *p)
{
	return p->des01.etx.time_stamp_status;
}

static u64 enh_desc_get_timestamp(void *desc, u32 ats)
{
	u64 ns;

	if (ats) {
		struct dma_extended_desc *p = (struct dma_extended_desc *)desc;
		ns = p->des6;
		/* convert high/sec time stamp value to nanosecond */
		ns += p->des7 * 1000000000ULL;
	} else {
		struct dma_desc *p = (struct dma_desc *)desc;
		ns = p->des2;
		ns += p->des3 * 1000000000ULL;
	}

	return ns;
}

static int enh_desc_get_rx_timestamp_status(void *desc, u32 ats)
{
	if (ats) {
		struct dma_extended_desc *p = (struct dma_extended_desc *)desc;
		return p->basic.des01.erx.ipc_csum_error;
	} else {
		struct dma_desc *p = (struct dma_desc *)desc;
		if ((p->des2 == 0xffffffff) && (p->des3 == 0xffffffff))
			/* timestamp is corrupted, hence don't store it */
			return 0;
		else
			return 1;
	}
}

const struct stmmac_desc_ops enh_desc_ops = {
	.tx_status = enh_desc_get_tx_status,
	.rx_status = enh_desc_get_rx_status,
	.get_tx_len = enh_desc_get_tx_len,
	.init_rx_desc = enh_desc_init_rx_desc,
	.init_tx_desc = enh_desc_init_tx_desc,
	.get_tx_owner = enh_desc_get_tx_owner,
	.get_rx_owner = enh_desc_get_rx_owner,
	.release_tx_desc = enh_desc_release_tx_desc,
	.prepare_tx_desc = enh_desc_prepare_tx_desc,
	.clear_tx_ic = enh_desc_clear_tx_ic,
	.close_tx_desc = enh_desc_close_tx_desc,
	.get_tx_ls = enh_desc_get_tx_ls,
	.set_tx_owner = enh_desc_set_tx_owner,
	.set_rx_owner = enh_desc_set_rx_owner,
	.get_rx_frame_len = enh_desc_get_rx_frame_len,
	.rx_extended_status = enh_desc_get_ext_status,
	.enable_tx_timestamp = enh_desc_enable_tx_timestamp,
	.get_tx_timestamp_status = enh_desc_get_tx_timestamp_status,
	.get_timestamp = enh_desc_get_timestamp,
	.get_rx_timestamp_status = enh_desc_get_rx_timestamp_status,
};
