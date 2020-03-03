/*
 * Copyright (c) 2012-2014 Qualcomm Atheros, Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/moduleparam.h>
#include <linux/etherdevice.h>
#include <linux/if_arp.h>

#include "wil6210.h"
#include "txrx.h"
#include "wmi.h"
#include "trace.h"

static uint max_assoc_sta = WIL6210_MAX_CID;
module_param(max_assoc_sta, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(max_assoc_sta, " Max number of stations associated to the AP");

int agg_wsize; /* = 0; */
module_param(agg_wsize, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(agg_wsize, " Window size for Tx Block Ack after connect;"
		 " 0 - use default; < 0 - don't auto-establish");

/**
 * WMI event receiving - theory of operations
 *
 * When firmware about to report WMI event, it fills memory area
 * in the mailbox and raises misc. IRQ. Thread interrupt handler invoked for
 * the misc IRQ, function @wmi_recv_cmd called by thread IRQ handler.
 *
 * @wmi_recv_cmd reads event, allocates memory chunk  and attaches it to the
 * event list @wil->pending_wmi_ev. Then, work queue @wil->wmi_wq wakes up
 * and handles events within the @wmi_event_worker. Every event get detached
 * from list, processed and deleted.
 *
 * Purpose for this mechanism is to release IRQ thread; otherwise,
 * if WMI event handling involves another WMI command flow, this 2-nd flow
 * won't be completed because of blocked IRQ thread.
 */

/**
 * Addressing - theory of operations
 *
 * There are several buses present on the WIL6210 card.
 * Same memory areas are visible at different address on
 * the different busses. There are 3 main bus masters:
 *  - MAC CPU (ucode)
 *  - User CPU (firmware)
 *  - AHB (host)
 *
 * On the PCI bus, there is one BAR (BAR0) of 2Mb size, exposing
 * AHB addresses starting from 0x880000
 *
 * Internally, firmware uses addresses that allows faster access but
 * are invisible from the host. To read from these addresses, alternative
 * AHB address must be used.
 *
 * Memory mapping
 * Linker address         PCI/Host address
 *                        0x880000 .. 0xa80000  2Mb BAR0
 * 0x800000 .. 0x807000   0x900000 .. 0x907000  28k DCCM
 * 0x840000 .. 0x857000   0x908000 .. 0x91f000  92k PERIPH
 */

/**
 * @fw_mapping provides memory remapping table
 *
 * array size should be in sync with the declaration in the wil6210.h
 */
const struct fw_map fw_mapping[] = {
	{0x000000, 0x040000, 0x8c0000, "fw_code"}, /* FW code RAM      256k */
	{0x800000, 0x808000, 0x900000, "fw_data"}, /* FW data RAM       32k */
	{0x840000, 0x860000, 0x908000, "fw_peri"}, /* periph. data RAM 128k */
	{0x880000, 0x88a000, 0x880000, "rgf"},     /* various RGF       40k */
	{0x88a000, 0x88b000, 0x88a000, "AGC_tbl"}, /* AGC table          4k */
	{0x88b000, 0x88c000, 0x88b000, "rgf_ext"}, /* Pcie_ext_rgf       4k */
	{0x8c0000, 0x949000, 0x8c0000, "upper"},   /* upper area       548k */
	/*
	 * 920000..930000 ucode code RAM
	 * 930000..932000 ucode data RAM
	 * 932000..949000 back-door debug data
	 */
};

/**
 * return AHB address for given firmware/ucode internal (linker) address
 * @x - internal address
 * If address have no valid AHB mapping, return 0
 */
static u32 wmi_addr_remap(u32 x)
{
	uint i;

	for (i = 0; i < ARRAY_SIZE(fw_mapping); i++) {
		if ((x >= fw_mapping[i].from) && (x < fw_mapping[i].to))
			return x + fw_mapping[i].host - fw_mapping[i].from;
	}

	return 0;
}

/**
 * Check address validity for WMI buffer; remap if needed
 * @ptr - internal (linker) fw/ucode address
 *
 * Valid buffer should be DWORD aligned
 *
 * return address for accessing buffer from the host;
 * if buffer is not valid, return NULL.
 */
void __iomem *wmi_buffer(struct wil6210_priv *wil, __le32 ptr_)
{
	u32 off;
	u32 ptr = le32_to_cpu(ptr_);

	if (ptr % 4)
		return NULL;

	ptr = wmi_addr_remap(ptr);
	if (ptr < WIL6210_FW_HOST_OFF)
		return NULL;

	off = HOSTADDR(ptr);
	if (off > WIL6210_MEM_SIZE - 4)
		return NULL;

	return wil->csr + off;
}

/**
 * Check address validity
 */
void __iomem *wmi_addr(struct wil6210_priv *wil, u32 ptr)
{
	u32 off;

	if (ptr % 4)
		return NULL;

	if (ptr < WIL6210_FW_HOST_OFF)
		return NULL;

	off = HOSTADDR(ptr);
	if (off > WIL6210_MEM_SIZE - 4)
		return NULL;

	return wil->csr + off;
}

int wmi_read_hdr(struct wil6210_priv *wil, __le32 ptr,
		 struct wil6210_mbox_hdr *hdr)
{
	void __iomem *src = wmi_buffer(wil, ptr);

	if (!src)
		return -EINVAL;

	wil_memcpy_fromio_32(hdr, src, sizeof(*hdr));

	return 0;
}

static int __wmi_send(struct wil6210_priv *wil, u16 cmdid, void *buf, u16 len)
{
	struct {
		struct wil6210_mbox_hdr hdr;
		struct wil6210_mbox_hdr_wmi wmi;
	} __packed cmd = {
		.hdr = {
			.type = WIL_MBOX_HDR_TYPE_WMI,
			.flags = 0,
			.len = cpu_to_le16(sizeof(cmd.wmi) + len),
		},
		.wmi = {
			.mid = 0,
			.id = cpu_to_le16(cmdid),
		},
	};
	struct wil6210_mbox_ring *r = &wil->mbox_ctl.tx;
	struct wil6210_mbox_ring_desc d_head;
	u32 next_head;
	void __iomem *dst;
	void __iomem *head = wmi_addr(wil, r->head);
	uint retry;

	if (sizeof(cmd) + len > r->entry_size) {
		wil_err(wil, "WMI size too large: %d bytes, max is %d\n",
			(int)(sizeof(cmd) + len), r->entry_size);
		return -ERANGE;
	}

	might_sleep();

	if (!test_bit(wil_status_fwready, wil->status)) {
		wil_err(wil, "WMI: cannot send command while FW not ready\n");
		return -EAGAIN;
	}

	if (!head) {
		wil_err(wil, "WMI head is garbage: 0x%08x\n", r->head);
		return -EINVAL;
	}
	/* read Tx head till it is not busy */
	for (retry = 5; retry > 0; retry--) {
		wil_memcpy_fromio_32(&d_head, head, sizeof(d_head));
		if (d_head.sync == 0)
			break;
		msleep(20);
	}
	if (d_head.sync != 0) {
		wil_err(wil, "WMI head busy\n");
		return -EBUSY;
	}
	/* next head */
	next_head = r->base + ((r->head - r->base + sizeof(d_head)) % r->size);
	wil_dbg_wmi(wil, "Head 0x%08x -> 0x%08x\n", r->head, next_head);
	/* wait till FW finish with previous command */
	for (retry = 5; retry > 0; retry--) {
		r->tail = ioread32(wil->csr + HOST_MBOX +
				   offsetof(struct wil6210_mbox_ctl, tx.tail));
		if (next_head != r->tail)
			break;
		msleep(20);
	}
	if (next_head == r->tail) {
		wil_err(wil, "WMI ring full\n");
		return -EBUSY;
	}
	dst = wmi_buffer(wil, d_head.addr);
	if (!dst) {
		wil_err(wil, "invalid WMI buffer: 0x%08x\n",
			le32_to_cpu(d_head.addr));
		return -EINVAL;
	}
	cmd.hdr.seq = cpu_to_le16(++wil->wmi_seq);
	/* set command */
	wil_dbg_wmi(wil, "WMI command 0x%04x [%d]\n", cmdid, len);
	wil_hex_dump_wmi("Cmd ", DUMP_PREFIX_OFFSET, 16, 1, &cmd,
			 sizeof(cmd), true);
	wil_hex_dump_wmi("cmd ", DUMP_PREFIX_OFFSET, 16, 1, buf,
			 len, true);
	wil_memcpy_toio_32(dst, &cmd, sizeof(cmd));
	wil_memcpy_toio_32(dst + sizeof(cmd), buf, len);
	/* mark entry as full */
	iowrite32(1, wil->csr + HOSTADDR(r->head) +
		  offsetof(struct wil6210_mbox_ring_desc, sync));
	/* advance next ptr */
	iowrite32(r->head = next_head, wil->csr + HOST_MBOX +
		  offsetof(struct wil6210_mbox_ctl, tx.head));

	trace_wil6210_wmi_cmd(&cmd.wmi, buf, len);

	/* interrupt to FW */
	iowrite32(SW_INT_MBOX, wil->csr + HOST_SW_INT);

	return 0;
}

int wmi_send(struct wil6210_priv *wil, u16 cmdid, void *buf, u16 len)
{
	int rc;

	mutex_lock(&wil->wmi_mutex);
	rc = __wmi_send(wil, cmdid, buf, len);
	mutex_unlock(&wil->wmi_mutex);

	return rc;
}

/*=== Event handlers ===*/
static void wmi_evt_ready(struct wil6210_priv *wil, int id, void *d, int len)
{
	struct wireless_dev *wdev = wil->wdev;
	struct wmi_ready_event *evt = d;

	wil->fw_version = le32_to_cpu(evt->sw_version);
	wil->n_mids = evt->numof_additional_mids;

	wil_info(wil, "FW ver. %d; MAC %pM; %d MID's\n", wil->fw_version,
		 evt->mac, wil->n_mids);
	/* ignore MAC address, we already have it from the boot loader */
	snprintf(wdev->wiphy->fw_version, sizeof(wdev->wiphy->fw_version),
		 "%d", wil->fw_version);
}

static void wmi_evt_fw_ready(struct wil6210_priv *wil, int id, void *d,
			     int len)
{
	wil_dbg_wmi(wil, "WMI: got FW ready event\n");

	wil_set_recovery_state(wil, fw_recovery_idle);
	set_bit(wil_status_fwready, wil->status);
	/* let the reset sequence continue */
	complete(&wil->wmi_ready);
}

static void wmi_evt_rx_mgmt(struct wil6210_priv *wil, int id, void *d, int len)
{
	struct wmi_rx_mgmt_packet_event *data = d;
	struct wiphy *wiphy = wil_to_wiphy(wil);
	struct ieee80211_mgmt *rx_mgmt_frame =
			(struct ieee80211_mgmt *)data->payload;
	int ch_no = data->info.channel+1;
	u32 freq = ieee80211_channel_to_frequency(ch_no,
			IEEE80211_BAND_60GHZ);
	struct ieee80211_channel *channel = ieee80211_get_channel(wiphy, freq);
	s32 signal = data->info.sqi;
	__le16 fc = rx_mgmt_frame->frame_control;
	u32 d_len = le32_to_cpu(data->info.len);
	u16 d_status = le16_to_cpu(data->info.status);

	wil_dbg_wmi(wil, "MGMT: channel %d MCS %d SNR %d SQI %d%%\n",
		    data->info.channel, data->info.mcs, data->info.snr,
		    data->info.sqi);
	wil_dbg_wmi(wil, "status 0x%04x len %d fc 0x%04x\n", d_status, d_len,
		    le16_to_cpu(fc));
	wil_dbg_wmi(wil, "qid %d mid %d cid %d\n",
		    data->info.qid, data->info.mid, data->info.cid);

	if (!channel) {
		wil_err(wil, "Frame on unsupported channel\n");
		return;
	}

	if (ieee80211_is_beacon(fc) || ieee80211_is_probe_resp(fc)) {
		struct cfg80211_bss *bss;
		u64 tsf = le64_to_cpu(rx_mgmt_frame->u.beacon.timestamp);
		u16 cap = le16_to_cpu(rx_mgmt_frame->u.beacon.capab_info);
		u16 bi = le16_to_cpu(rx_mgmt_frame->u.beacon.beacon_int);
		const u8 *ie_buf = rx_mgmt_frame->u.beacon.variable;
		size_t ie_len = d_len - offsetof(struct ieee80211_mgmt,
						 u.beacon.variable);
		wil_dbg_wmi(wil, "Capability info : 0x%04x\n", cap);
		wil_dbg_wmi(wil, "TSF : 0x%016llx\n", tsf);
		wil_dbg_wmi(wil, "Beacon interval : %d\n", bi);
		wil_hex_dump_wmi("IE ", DUMP_PREFIX_OFFSET, 16, 1, ie_buf,
				 ie_len, true);

		bss = cfg80211_inform_bss_frame(wiphy, channel, rx_mgmt_frame,
						d_len, signal, GFP_KERNEL);
		if (bss) {
			wil_dbg_wmi(wil, "Added BSS %pM\n",
				    rx_mgmt_frame->bssid);
			cfg80211_put_bss(wiphy, bss);
		} else {
			wil_err(wil, "cfg80211_inform_bss_frame() failed\n");
		}
	} else {
		cfg80211_rx_mgmt(wil->wdev, freq, signal,
				 (void *)rx_mgmt_frame, d_len, 0);
	}
}

static void wmi_evt_scan_complete(struct wil6210_priv *wil, int id,
				  void *d, int len)
{
	if (wil->scan_request) {
		struct wmi_scan_complete_event *data = d;
		bool aborted = (data->status != WMI_SCAN_SUCCESS);

		wil_dbg_wmi(wil, "SCAN_COMPLETE(0x%08x)\n", data->status);
		wil_dbg_misc(wil, "Complete scan_request 0x%p aborted %d\n",
			     wil->scan_request, aborted);

		del_timer_sync(&wil->scan_timer);
		cfg80211_scan_done(wil->scan_request, aborted);
		wil->scan_request = NULL;
	} else {
		wil_err(wil, "SCAN_COMPLETE while not scanning\n");
	}
}

static void wmi_evt_connect(struct wil6210_priv *wil, int id, void *d, int len)
{
	struct net_device *ndev = wil_to_ndev(wil);
	struct wireless_dev *wdev = wil->wdev;
	struct wmi_connect_event *evt = d;
	int ch; /* channel number */
	struct station_info sinfo;
	u8 *assoc_req_ie, *assoc_resp_ie;
	size_t assoc_req_ielen, assoc_resp_ielen;
	/* capinfo(u16) + listen_interval(u16) + IEs */
	const size_t assoc_req_ie_offset = sizeof(u16) * 2;
	/* capinfo(u16) + status_code(u16) + associd(u16) + IEs */
	const size_t assoc_resp_ie_offset = sizeof(u16) * 3;

	if (len < sizeof(*evt)) {
		wil_err(wil, "Connect event too short : %d bytes\n", len);
		return;
	}
	if (len != sizeof(*evt) + evt->beacon_ie_len + evt->assoc_req_len +
		   evt->assoc_resp_len) {
		wil_err(wil,
			"Connect event corrupted : %d != %d + %d + %d + %d\n",
			len, (int)sizeof(*evt), evt->beacon_ie_len,
			evt->assoc_req_len, evt->assoc_resp_len);
		return;
	}
	if (evt->cid >= WIL6210_MAX_CID) {
		wil_err(wil, "Connect CID invalid : %d\n", evt->cid);
		return;
	}

	ch = evt->channel + 1;
	wil_dbg_wmi(wil, "Connect %pM channel [%d] cid %d\n",
		    evt->bssid, ch, evt->cid);
	wil_hex_dump_wmi("connect AI : ", DUMP_PREFIX_OFFSET, 16, 1,
			 evt->assoc_info, len - sizeof(*evt), true);

	/* figure out IE's */
	assoc_req_ie = &evt->assoc_info[evt->beacon_ie_len +
					assoc_req_ie_offset];
	assoc_req_ielen = evt->assoc_req_len - assoc_req_ie_offset;
	if (evt->assoc_req_len <= assoc_req_ie_offset) {
		assoc_req_ie = NULL;
		assoc_req_ielen = 0;
	}

	assoc_resp_ie = &evt->assoc_info[evt->beacon_ie_len +
					 evt->assoc_req_len +
					 assoc_resp_ie_offset];
	assoc_resp_ielen = evt->assoc_resp_len - assoc_resp_ie_offset;
	if (evt->assoc_resp_len <= assoc_resp_ie_offset) {
		assoc_resp_ie = NULL;
		assoc_resp_ielen = 0;
	}

	if ((wdev->iftype == NL80211_IFTYPE_STATION) ||
	    (wdev->iftype == NL80211_IFTYPE_P2P_CLIENT)) {
		if (!test_bit(wil_status_fwconnecting, wil->status)) {
			wil_err(wil, "Not in connecting state\n");
			return;
		}
		del_timer_sync(&wil->connect_timer);
		cfg80211_connect_result(ndev, evt->bssid,
					assoc_req_ie, assoc_req_ielen,
					assoc_resp_ie, assoc_resp_ielen,
					WLAN_STATUS_SUCCESS, GFP_KERNEL);

	} else if ((wdev->iftype == NL80211_IFTYPE_AP) ||
		   (wdev->iftype == NL80211_IFTYPE_P2P_GO)) {
		memset(&sinfo, 0, sizeof(sinfo));

		sinfo.generation = wil->sinfo_gen++;

		if (assoc_req_ie) {
			sinfo.assoc_req_ies = assoc_req_ie;
			sinfo.assoc_req_ies_len = assoc_req_ielen;
		}

		cfg80211_new_sta(ndev, evt->bssid, &sinfo, GFP_KERNEL);
	}
	clear_bit(wil_status_fwconnecting, wil->status);
	set_bit(wil_status_fwconnected, wil->status);

	/* FIXME FW can transmit only ucast frames to peer */
	/* FIXME real ring_id instead of hard coded 0 */
	ether_addr_copy(wil->sta[evt->cid].addr, evt->bssid);
	wil->sta[evt->cid].status = wil_sta_conn_pending;

	wil->pending_connect_cid = evt->cid;
	queue_work(wil->wq_service, &wil->connect_worker);
}

static void wmi_evt_disconnect(struct wil6210_priv *wil, int id,
			       void *d, int len)
{
	struct wmi_disconnect_event *evt = d;
	u16 reason_code = le16_to_cpu(evt->protocol_reason_status);

	wil_dbg_wmi(wil, "Disconnect %pM reason [proto %d wmi %d]\n",
		    evt->bssid, reason_code, evt->disconnect_reason);

	wil->sinfo_gen++;

	mutex_lock(&wil->mutex);
	wil6210_disconnect(wil, evt->bssid, reason_code, true);
	mutex_unlock(&wil->mutex);
}

/*
 * Firmware reports EAPOL frame using WME event.
 * Reconstruct Ethernet frame and deliver it via normal Rx
 */
static void wmi_evt_eapol_rx(struct wil6210_priv *wil, int id,
			     void *d, int len)
{
	struct net_device *ndev = wil_to_ndev(wil);
	struct wmi_eapol_rx_event *evt = d;
	u16 eapol_len = le16_to_cpu(evt->eapol_len);
	int sz = eapol_len + ETH_HLEN;
	struct sk_buff *skb;
	struct ethhdr *eth;
	int cid;
	struct wil_net_stats *stats = NULL;

	wil_dbg_wmi(wil, "EAPOL len %d from %pM\n", eapol_len,
		    evt->src_mac);

	cid = wil_find_cid(wil, evt->src_mac);
	if (cid >= 0)
		stats = &wil->sta[cid].stats;

	if (eapol_len > 196) { /* TODO: revisit size limit */
		wil_err(wil, "EAPOL too large\n");
		return;
	}

	skb = alloc_skb(sz, GFP_KERNEL);
	if (!skb) {
		wil_err(wil, "Failed to allocate skb\n");
		return;
	}

	eth = (struct ethhdr *)skb_put(skb, ETH_HLEN);
	ether_addr_copy(eth->h_dest, ndev->dev_addr);
	ether_addr_copy(eth->h_source, evt->src_mac);
	eth->h_proto = cpu_to_be16(ETH_P_PAE);
	memcpy(skb_put(skb, eapol_len), evt->eapol, eapol_len);
	skb->protocol = eth_type_trans(skb, ndev);
	if (likely(netif_rx_ni(skb) == NET_RX_SUCCESS)) {
		ndev->stats.rx_packets++;
		ndev->stats.rx_bytes += sz;
		if (stats) {
			stats->rx_packets++;
			stats->rx_bytes += sz;
		}
	} else {
		ndev->stats.rx_dropped++;
		if (stats)
			stats->rx_dropped++;
	}
}

static void wil_addba_tx_cid(struct wil6210_priv *wil, u8 cid, u16 wsize)
{
	struct vring_tx_data *t;
	int i;

	for (i = 0; i < WIL6210_MAX_TX_RINGS; i++) {
		if (cid != wil->vring2cid_tid[i][0])
			continue;
		t = &wil->vring_tx_data[i];
		if (!t->enabled)
			continue;

		wil_addba_tx_request(wil, i, wsize);
	}
}

static void wmi_evt_linkup(struct wil6210_priv *wil, int id, void *d, int len)
{
	struct wmi_data_port_open_event *evt = d;
	u8 cid = evt->cid;

	wil_dbg_wmi(wil, "Link UP for CID %d\n", cid);

	if (cid >= ARRAY_SIZE(wil->sta)) {
		wil_err(wil, "Link UP for invalid CID %d\n", cid);
		return;
	}

	wil->sta[cid].data_port_open = true;
	if (agg_wsize >= 0)
		wil_addba_tx_cid(wil, cid, agg_wsize);
}

static void wmi_evt_linkdown(struct wil6210_priv *wil, int id, void *d, int len)
{
	struct net_device *ndev = wil_to_ndev(wil);
	struct wmi_wbe_link_down_event *evt = d;
	u8 cid = evt->cid;

	wil_dbg_wmi(wil, "Link DOWN for CID %d, reason %d\n",
		    cid, le32_to_cpu(evt->reason));

	if (cid >= ARRAY_SIZE(wil->sta)) {
		wil_err(wil, "Link DOWN for invalid CID %d\n", cid);
		return;
	}

	wil->sta[cid].data_port_open = false;
	netif_carrier_off(ndev);
}

static void wmi_evt_ba_status(struct wil6210_priv *wil, int id, void *d,
			      int len)
{
	struct wmi_vring_ba_status_event *evt = d;
	struct vring_tx_data *txdata;

	wil_dbg_wmi(wil, "BACK[%d] %s {%d} timeout %d AMSDU%s\n",
		    evt->ringid,
		    evt->status == WMI_BA_AGREED ? "OK" : "N/A",
		    evt->agg_wsize, __le16_to_cpu(evt->ba_timeout),
		    evt->amsdu ? "+" : "-");

	if (evt->ringid >= WIL6210_MAX_TX_RINGS) {
		wil_err(wil, "invalid ring id %d\n", evt->ringid);
		return;
	}

	if (evt->status != WMI_BA_AGREED) {
		evt->ba_timeout = 0;
		evt->agg_wsize = 0;
		evt->amsdu = 0;
	}

	txdata = &wil->vring_tx_data[evt->ringid];

	txdata->agg_timeout = le16_to_cpu(evt->ba_timeout);
	txdata->agg_wsize = evt->agg_wsize;
	txdata->agg_amsdu = evt->amsdu;
	txdata->addba_in_progress = false;
}

static void wmi_evt_addba_rx_req(struct wil6210_priv *wil, int id, void *d,
				 int len)
{
	struct wmi_rcp_addba_req_event *evt = d;

	wil_addba_rx_request(wil, evt->cidxtid, evt->dialog_token,
			     evt->ba_param_set, evt->ba_timeout,
			     evt->ba_seq_ctrl);
}

static void wmi_evt_delba(struct wil6210_priv *wil, int id, void *d, int len)
__acquires(&sta->tid_rx_lock) __releases(&sta->tid_rx_lock)
{
	struct wmi_delba_event *evt = d;
	u8 cid, tid;
	u16 reason = __le16_to_cpu(evt->reason);
	struct wil_sta_info *sta;
	struct wil_tid_ampdu_rx *r;

	might_sleep();
	parse_cidxtid(evt->cidxtid, &cid, &tid);
	wil_dbg_wmi(wil, "DELBA CID %d TID %d from %s reason %d\n",
		    cid, tid,
		    evt->from_initiator ? "originator" : "recipient",
		    reason);
	if (!evt->from_initiator) {
		int i;
		/* find Tx vring it belongs to */
		for (i = 0; i < ARRAY_SIZE(wil->vring2cid_tid); i++) {
			if ((wil->vring2cid_tid[i][0] == cid) &&
			    (wil->vring2cid_tid[i][1] == tid)) {
				struct vring_tx_data *txdata =
					&wil->vring_tx_data[i];

				wil_dbg_wmi(wil, "DELBA Tx vring %d\n", i);
				txdata->agg_timeout = 0;
				txdata->agg_wsize = 0;
				txdata->addba_in_progress = false;

				break; /* max. 1 matching ring */
			}
		}
		if (i >= ARRAY_SIZE(wil->vring2cid_tid))
			wil_err(wil, "DELBA: unable to find Tx vring\n");
		return;
	}

	sta = &wil->sta[cid];

	spin_lock_bh(&sta->tid_rx_lock);

	r = sta->tid_rx[tid];
	sta->tid_rx[tid] = NULL;
	wil_tid_ampdu_rx_free(wil, r);

	spin_unlock_bh(&sta->tid_rx_lock);
}

static const struct {
	int eventid;
	void (*handler)(struct wil6210_priv *wil, int eventid,
			void *data, int data_len);
} wmi_evt_handlers[] = {
	{WMI_READY_EVENTID,		wmi_evt_ready},
	{WMI_FW_READY_EVENTID,		wmi_evt_fw_ready},
	{WMI_RX_MGMT_PACKET_EVENTID,	wmi_evt_rx_mgmt},
	{WMI_SCAN_COMPLETE_EVENTID,	wmi_evt_scan_complete},
	{WMI_CONNECT_EVENTID,		wmi_evt_connect},
	{WMI_DISCONNECT_EVENTID,	wmi_evt_disconnect},
	{WMI_EAPOL_RX_EVENTID,		wmi_evt_eapol_rx},
	{WMI_DATA_PORT_OPEN_EVENTID,	wmi_evt_linkup},
	{WMI_WBE_LINKDOWN_EVENTID,	wmi_evt_linkdown},
	{WMI_BA_STATUS_EVENTID,		wmi_evt_ba_status},
	{WMI_RCP_ADDBA_REQ_EVENTID,	wmi_evt_addba_rx_req},
	{WMI_DELBA_EVENTID,		wmi_evt_delba},
};

/*
 * Run in IRQ context
 * Extract WMI command from mailbox. Queue it to the @wil->pending_wmi_ev
 * that will be eventually handled by the @wmi_event_worker in the thread
 * context of thread "wil6210_wmi"
 */
void wmi_recv_cmd(struct wil6210_priv *wil)
{
	struct wil6210_mbox_ring_desc d_tail;
	struct wil6210_mbox_hdr hdr;
	struct wil6210_mbox_ring *r = &wil->mbox_ctl.rx;
	struct pending_wmi_event *evt;
	u8 *cmd;
	void __iomem *src;
	ulong flags;
	unsigned n;

	if (!test_bit(wil_status_reset_done, wil->status)) {
		wil_err(wil, "Reset in progress. Cannot handle WMI event\n");
		return;
	}

	for (n = 0;; n++) {
		u16 len;
		bool q;

		r->head = ioread32(wil->csr + HOST_MBOX +
				   offsetof(struct wil6210_mbox_ctl, rx.head));
		if (r->tail == r->head)
			break;

		wil_dbg_wmi(wil, "Mbox head %08x tail %08x\n",
			    r->head, r->tail);
		/* read cmd descriptor from tail */
		wil_memcpy_fromio_32(&d_tail, wil->csr + HOSTADDR(r->tail),
				     sizeof(struct wil6210_mbox_ring_desc));
		if (d_tail.sync == 0) {
			wil_err(wil, "Mbox evt not owned by FW?\n");
			break;
		}

		/* read cmd header from descriptor */
		if (0 != wmi_read_hdr(wil, d_tail.addr, &hdr)) {
			wil_err(wil, "Mbox evt at 0x%08x?\n",
				le32_to_cpu(d_tail.addr));
			break;
		}
		len = le16_to_cpu(hdr.len);
		wil_dbg_wmi(wil, "Mbox evt %04x %04x %04x %02x\n",
			    le16_to_cpu(hdr.seq), len, le16_to_cpu(hdr.type),
			    hdr.flags);

		/* read cmd buffer from descriptor */
		src = wmi_buffer(wil, d_tail.addr) +
		      sizeof(struct wil6210_mbox_hdr);
		evt = kmalloc(ALIGN(offsetof(struct pending_wmi_event,
					     event.wmi) + len, 4),
			      GFP_KERNEL);
		if (!evt)
			break;

		evt->event.hdr = hdr;
		cmd = (void *)&evt->event.wmi;
		wil_memcpy_fromio_32(cmd, src, len);
		/* mark entry as empty */
		iowrite32(0, wil->csr + HOSTADDR(r->tail) +
			  offsetof(struct wil6210_mbox_ring_desc, sync));
		/* indicate */
		if ((hdr.type == WIL_MBOX_HDR_TYPE_WMI) &&
		    (len >= sizeof(struct wil6210_mbox_hdr_wmi))) {
			struct wil6210_mbox_hdr_wmi *wmi = &evt->event.wmi;
			u16 id = le16_to_cpu(wmi->id);
			u32 tstamp = le32_to_cpu(wmi->timestamp);

			wil_dbg_wmi(wil, "WMI event 0x%04x MID %d @%d msec\n",
				    id, wmi->mid, tstamp);
			trace_wil6210_wmi_event(wmi, &wmi[1],
						len - sizeof(*wmi));
		}
		wil_hex_dump_wmi("evt ", DUMP_PREFIX_OFFSET, 16, 1,
				 &evt->event.hdr, sizeof(hdr) + len, true);

		/* advance tail */
		r->tail = r->base + ((r->tail - r->base +
			  sizeof(struct wil6210_mbox_ring_desc)) % r->size);
		iowrite32(r->tail, wil->csr + HOST_MBOX +
			  offsetof(struct wil6210_mbox_ctl, rx.tail));

		/* add to the pending list */
		spin_lock_irqsave(&wil->wmi_ev_lock, flags);
		list_add_tail(&evt->list, &wil->pending_wmi_ev);
		spin_unlock_irqrestore(&wil->wmi_ev_lock, flags);
		q = queue_work(wil->wmi_wq, &wil->wmi_event_worker);
		wil_dbg_wmi(wil, "queue_work -> %d\n", q);
	}
	/* normally, 1 event per IRQ should be processed */
	wil_dbg_wmi(wil, "%s -> %d events queued\n", __func__, n);
}

int wmi_call(struct wil6210_priv *wil, u16 cmdid, void *buf, u16 len,
	     u16 reply_id, void *reply, u8 reply_size, int to_msec)
{
	int rc;
	int remain;

	mutex_lock(&wil->wmi_mutex);

	rc = __wmi_send(wil, cmdid, buf, len);
	if (rc)
		goto out;

	wil->reply_id = reply_id;
	wil->reply_buf = reply;
	wil->reply_size = reply_size;
	remain = wait_for_completion_timeout(&wil->wmi_call,
					     msecs_to_jiffies(to_msec));
	if (0 == remain) {
		wil_err(wil, "wmi_call(0x%04x->0x%04x) timeout %d msec\n",
			cmdid, reply_id, to_msec);
		rc = -ETIME;
	} else {
		wil_dbg_wmi(wil,
			    "wmi_call(0x%04x->0x%04x) completed in %d msec\n",
			    cmdid, reply_id,
			    to_msec - jiffies_to_msecs(remain));
	}
	wil->reply_id = 0;
	wil->reply_buf = NULL;
	wil->reply_size = 0;
 out:
	mutex_unlock(&wil->wmi_mutex);

	return rc;
}

int wmi_echo(struct wil6210_priv *wil)
{
	struct wmi_echo_cmd cmd = {
		.value = cpu_to_le32(0x12345678),
	};

	return wmi_call(wil, WMI_ECHO_CMDID, &cmd, sizeof(cmd),
			 WMI_ECHO_RSP_EVENTID, NULL, 0, 20);
}

int wmi_set_mac_address(struct wil6210_priv *wil, void *addr)
{
	struct wmi_set_mac_address_cmd cmd;

	ether_addr_copy(cmd.mac, addr);

	wil_dbg_wmi(wil, "Set MAC %pM\n", addr);

	return wmi_send(wil, WMI_SET_MAC_ADDRESS_CMDID, &cmd, sizeof(cmd));
}

int wmi_pcp_start(struct wil6210_priv *wil, int bi, u8 wmi_nettype, u8 chan)
{
	int rc;

	struct wmi_pcp_start_cmd cmd = {
		.bcon_interval = cpu_to_le16(bi),
		.network_type = wmi_nettype,
		.disable_sec_offload = 1,
		.channel = chan - 1,
		.pcp_max_assoc_sta = max_assoc_sta,
	};
	struct {
		struct wil6210_mbox_hdr_wmi wmi;
		struct wmi_pcp_started_event evt;
	} __packed reply;

	if (!wil->privacy)
		cmd.disable_sec = 1;

	if ((cmd.pcp_max_assoc_sta > WIL6210_MAX_CID) ||
	    (cmd.pcp_max_assoc_sta <= 0)) {
		wil_info(wil,
			 "Requested connection limit %u, valid values are 1 - %d. Setting to %d\n",
			 max_assoc_sta, WIL6210_MAX_CID, WIL6210_MAX_CID);
		cmd.pcp_max_assoc_sta = WIL6210_MAX_CID;
	}

	/*
	 * Processing time may be huge, in case of secure AP it takes about
	 * 3500ms for FW to start AP
	 */
	rc = wmi_call(wil, WMI_PCP_START_CMDID, &cmd, sizeof(cmd),
		      WMI_PCP_STARTED_EVENTID, &reply, sizeof(reply), 5000);
	if (rc)
		return rc;

	if (reply.evt.status != WMI_FW_STATUS_SUCCESS)
		rc = -EINVAL;

	return rc;
}

int wmi_pcp_stop(struct wil6210_priv *wil)
{
	return wmi_call(wil, WMI_PCP_STOP_CMDID, NULL, 0,
			WMI_PCP_STOPPED_EVENTID, NULL, 0, 20);
}

int wmi_set_ssid(struct wil6210_priv *wil, u8 ssid_len, const void *ssid)
{
	struct wmi_set_ssid_cmd cmd = {
		.ssid_len = cpu_to_le32(ssid_len),
	};

	if (ssid_len > sizeof(cmd.ssid))
		return -EINVAL;

	memcpy(cmd.ssid, ssid, ssid_len);

	return wmi_send(wil, WMI_SET_SSID_CMDID, &cmd, sizeof(cmd));
}

int wmi_get_ssid(struct wil6210_priv *wil, u8 *ssid_len, void *ssid)
{
	int rc;
	struct {
		struct wil6210_mbox_hdr_wmi wmi;
		struct wmi_set_ssid_cmd cmd;
	} __packed reply;
	int len; /* reply.cmd.ssid_len in CPU order */

	rc = wmi_call(wil, WMI_GET_SSID_CMDID, NULL, 0, WMI_GET_SSID_EVENTID,
		      &reply, sizeof(reply), 20);
	if (rc)
		return rc;

	len = le32_to_cpu(reply.cmd.ssid_len);
	if (len > sizeof(reply.cmd.ssid))
		return -EINVAL;

	*ssid_len = len;
	memcpy(ssid, reply.cmd.ssid, len);

	return 0;
}

int wmi_set_channel(struct wil6210_priv *wil, int channel)
{
	struct wmi_set_pcp_channel_cmd cmd = {
		.channel = channel - 1,
	};

	return wmi_send(wil, WMI_SET_PCP_CHANNEL_CMDID, &cmd, sizeof(cmd));
}

int wmi_get_channel(struct wil6210_priv *wil, int *channel)
{
	int rc;
	struct {
		struct wil6210_mbox_hdr_wmi wmi;
		struct wmi_set_pcp_channel_cmd cmd;
	} __packed reply;

	rc = wmi_call(wil, WMI_GET_PCP_CHANNEL_CMDID, NULL, 0,
		      WMI_GET_PCP_CHANNEL_EVENTID, &reply, sizeof(reply), 20);
	if (rc)
		return rc;

	if (reply.cmd.channel > 3)
		return -EINVAL;

	*channel = reply.cmd.channel + 1;

	return 0;
}

int wmi_p2p_cfg(struct wil6210_priv *wil, int channel)
{
	struct wmi_p2p_cfg_cmd cmd = {
		.discovery_mode = WMI_DISCOVERY_MODE_NON_OFFLOAD,
		.channel = channel - 1,
	};

	return wmi_send(wil, WMI_P2P_CFG_CMDID, &cmd, sizeof(cmd));
}

int wmi_del_cipher_key(struct wil6210_priv *wil, u8 key_index,
		       const void *mac_addr)
{
	struct wmi_delete_cipher_key_cmd cmd = {
		.key_index = key_index,
	};

	if (mac_addr)
		memcpy(cmd.mac, mac_addr, WMI_MAC_LEN);

	return wmi_send(wil, WMI_DELETE_CIPHER_KEY_CMDID, &cmd, sizeof(cmd));
}

int wmi_add_cipher_key(struct wil6210_priv *wil, u8 key_index,
		       const void *mac_addr, int key_len, const void *key)
{
	struct wmi_add_cipher_key_cmd cmd = {
		.key_index = key_index,
		.key_usage = WMI_KEY_USE_PAIRWISE,
		.key_len = key_len,
	};

	if (!key || (key_len > sizeof(cmd.key)))
		return -EINVAL;

	memcpy(cmd.key, key, key_len);
	if (mac_addr)
		memcpy(cmd.mac, mac_addr, WMI_MAC_LEN);

	return wmi_send(wil, WMI_ADD_CIPHER_KEY_CMDID, &cmd, sizeof(cmd));
}

int wmi_set_ie(struct wil6210_priv *wil, u8 type, u16 ie_len, const void *ie)
{
	int rc;
	u16 len = sizeof(struct wmi_set_appie_cmd) + ie_len;
	struct wmi_set_appie_cmd *cmd = kzalloc(len, GFP_KERNEL);

	if (!cmd)
		return -ENOMEM;
	if (!ie)
		ie_len = 0;

	cmd->mgmt_frm_type = type;
	/* BUG: FW API define ieLen as u8. Will fix FW */
	cmd->ie_len = cpu_to_le16(ie_len);
	memcpy(cmd->ie_info, ie, ie_len);
	rc = wmi_send(wil, WMI_SET_APPIE_CMDID, cmd, len);
	kfree(cmd);

	return rc;
}

/**
 * wmi_rxon - turn radio on/off
 * @on:		turn on if true, off otherwise
 *
 * Only switch radio. Channel should be set separately.
 * No timeout for rxon - radio turned on forever unless some other call
 * turns it off
 */
int wmi_rxon(struct wil6210_priv *wil, bool on)
{
	int rc;
	struct {
		struct wil6210_mbox_hdr_wmi wmi;
		struct wmi_listen_started_event evt;
	} __packed reply;

	wil_info(wil, "%s(%s)\n", __func__, on ? "on" : "off");

	if (on) {
		rc = wmi_call(wil, WMI_START_LISTEN_CMDID, NULL, 0,
			      WMI_LISTEN_STARTED_EVENTID,
			      &reply, sizeof(reply), 100);
		if ((rc == 0) && (reply.evt.status != WMI_FW_STATUS_SUCCESS))
			rc = -EINVAL;
	} else {
		rc = wmi_call(wil, WMI_DISCOVERY_STOP_CMDID, NULL, 0,
			      WMI_DISCOVERY_STOPPED_EVENTID, NULL, 0, 20);
	}

	return rc;
}

int wmi_rx_chain_add(struct wil6210_priv *wil, struct vring *vring)
{
	struct wireless_dev *wdev = wil->wdev;
	struct net_device *ndev = wil_to_ndev(wil);
	struct wmi_cfg_rx_chain_cmd cmd = {
		.action = WMI_RX_CHAIN_ADD,
		.rx_sw_ring = {
			.max_mpdu_size = cpu_to_le16(wil_mtu2macbuf(mtu_max)),
			.ring_mem_base = cpu_to_le64(vring->pa),
			.ring_size = cpu_to_le16(vring->size),
		},
		.mid = 0, /* TODO - what is it? */
		.decap_trans_type = WMI_DECAP_TYPE_802_3,
		.reorder_type = WMI_RX_SW_REORDER,
		.host_thrsh = cpu_to_le16(rx_ring_overflow_thrsh),
	};
	struct {
		struct wil6210_mbox_hdr_wmi wmi;
		struct wmi_cfg_rx_chain_done_event evt;
	} __packed evt;
	int rc;

	if (wdev->iftype == NL80211_IFTYPE_MONITOR) {
		struct ieee80211_channel *ch = wdev->preset_chandef.chan;

		cmd.sniffer_cfg.mode = cpu_to_le32(WMI_SNIFFER_ON);
		if (ch)
			cmd.sniffer_cfg.channel = ch->hw_value - 1;
		cmd.sniffer_cfg.phy_info_mode =
			cpu_to_le32(ndev->type == ARPHRD_IEEE80211_RADIOTAP);
		cmd.sniffer_cfg.phy_support =
			cpu_to_le32((wil->monitor_flags & MONITOR_FLAG_CONTROL)
				    ? WMI_SNIFFER_CP : WMI_SNIFFER_DP);
	} else {
		/* Initialize offload (in non-sniffer mode).
		 * Linux IP stack always calculates IP checksum
		 * HW always calculate TCP/UDP checksum
		 */
		cmd.l3_l4_ctrl |= (1 << L3_L4_CTRL_TCPIP_CHECKSUM_EN_POS);
	}

	if (rx_align_2)
		cmd.l2_802_3_offload_ctrl |=
				L2_802_3_OFFLOAD_CTRL_SNAP_KEEP_MSK;

	/* typical time for secure PCP is 840ms */
	rc = wmi_call(wil, WMI_CFG_RX_CHAIN_CMDID, &cmd, sizeof(cmd),
		      WMI_CFG_RX_CHAIN_DONE_EVENTID, &evt, sizeof(evt), 2000);
	if (rc)
		return rc;

	vring->hwtail = le32_to_cpu(evt.evt.rx_ring_tail_ptr);

	wil_dbg_misc(wil, "Rx init: status %d tail 0x%08x\n",
		     le32_to_cpu(evt.evt.status), vring->hwtail);

	if (le32_to_cpu(evt.evt.status) != WMI_CFG_RX_CHAIN_SUCCESS)
		rc = -EINVAL;

	return rc;
}

int wmi_get_temperature(struct wil6210_priv *wil, u32 *t_bb, u32 *t_rf)
{
	int rc;
	struct wmi_temp_sense_cmd cmd = {
		.measure_baseband_en = cpu_to_le32(!!t_bb),
		.measure_rf_en = cpu_to_le32(!!t_rf),
		.measure_mode = cpu_to_le32(TEMPERATURE_MEASURE_NOW),
	};
	struct {
		struct wil6210_mbox_hdr_wmi wmi;
		struct wmi_temp_sense_done_event evt;
	} __packed reply;

	rc = wmi_call(wil, WMI_TEMP_SENSE_CMDID, &cmd, sizeof(cmd),
		      WMI_TEMP_SENSE_DONE_EVENTID, &reply, sizeof(reply), 100);
	if (rc)
		return rc;

	if (t_bb)
		*t_bb = le32_to_cpu(reply.evt.baseband_t1000);
	if (t_rf)
		*t_rf = le32_to_cpu(reply.evt.rf_t1000);

	return 0;
}

int wmi_disconnect_sta(struct wil6210_priv *wil, const u8 *mac, u16 reason)
{
	struct wmi_disconnect_sta_cmd cmd = {
		.disconnect_reason = cpu_to_le16(reason),
	};

	ether_addr_copy(cmd.dst_mac, mac);

	wil_dbg_wmi(wil, "%s(%pM, reason %d)\n", __func__, mac, reason);

	return wmi_send(wil, WMI_DISCONNECT_STA_CMDID, &cmd, sizeof(cmd));
}

int wmi_addba(struct wil6210_priv *wil, u8 ringid, u8 size, u16 timeout)
{
	struct wmi_vring_ba_en_cmd cmd = {
		.ringid = ringid,
		.agg_max_wsize = size,
		.ba_timeout = cpu_to_le16(timeout),
		.amsdu = 0,
	};

	wil_dbg_wmi(wil, "%s(ring %d size %d timeout %d)\n", __func__,
		    ringid, size, timeout);

	return wmi_send(wil, WMI_VRING_BA_EN_CMDID, &cmd, sizeof(cmd));
}

int wmi_delba_tx(struct wil6210_priv *wil, u8 ringid, u16 reason)
{
	struct wmi_vring_ba_dis_cmd cmd = {
		.ringid = ringid,
		.reason = cpu_to_le16(reason),
	};

	wil_dbg_wmi(wil, "%s(ring %d reason %d)\n", __func__,
		    ringid, reason);

	return wmi_send(wil, WMI_VRING_BA_DIS_CMDID, &cmd, sizeof(cmd));
}

int wmi_delba_rx(struct wil6210_priv *wil, u8 cidxtid, u16 reason)
{
	struct wmi_rcp_delba_cmd cmd = {
		.cidxtid = cidxtid,
		.reason = cpu_to_le16(reason),
	};

	wil_dbg_wmi(wil, "%s(CID %d TID %d reason %d)\n", __func__,
		    cidxtid & 0xf, (cidxtid >> 4) & 0xf, reason);

	return wmi_send(wil, WMI_RCP_DELBA_CMDID, &cmd, sizeof(cmd));
}

int wmi_addba_rx_resp(struct wil6210_priv *wil, u8 cid, u8 tid, u8 token,
		      u16 status, bool amsdu, u16 agg_wsize, u16 timeout)
{
	int rc;
	struct wmi_rcp_addba_resp_cmd cmd = {
		.cidxtid = mk_cidxtid(cid, tid),
		.dialog_token = token,
		.status_code = cpu_to_le16(status),
		/* bit 0: A-MSDU supported
		 * bit 1: policy (should be 0 for us)
		 * bits 2..5: TID
		 * bits 6..15: buffer size
		 */
		.ba_param_set = cpu_to_le16((amsdu ? 1 : 0) | (tid << 2) |
					    (agg_wsize << 6)),
		.ba_timeout = cpu_to_le16(timeout),
	};
	struct {
		struct wil6210_mbox_hdr_wmi wmi;
		struct wmi_rcp_addba_resp_sent_event evt;
	} __packed reply;

	wil_dbg_wmi(wil,
		    "ADDBA response for CID %d TID %d size %d timeout %d status %d AMSDU%s\n",
		    cid, tid, agg_wsize, timeout, status, amsdu ? "+" : "-");

	rc = wmi_call(wil, WMI_RCP_ADDBA_RESP_CMDID, &cmd, sizeof(cmd),
		      WMI_ADDBA_RESP_SENT_EVENTID, &reply, sizeof(reply), 100);
	if (rc)
		return rc;

	if (reply.evt.status) {
		wil_err(wil, "ADDBA response failed with status %d\n",
			le16_to_cpu(reply.evt.status));
		rc = -EINVAL;
	}

	return rc;
}

void wmi_event_flush(struct wil6210_priv *wil)
{
	struct pending_wmi_event *evt, *t;

	wil_dbg_wmi(wil, "%s()\n", __func__);

	list_for_each_entry_safe(evt, t, &wil->pending_wmi_ev, list) {
		list_del(&evt->list);
		kfree(evt);
	}
}

static bool wmi_evt_call_handler(struct wil6210_priv *wil, int id,
				 void *d, int len)
{
	uint i;

	for (i = 0; i < ARRAY_SIZE(wmi_evt_handlers); i++) {
		if (wmi_evt_handlers[i].eventid == id) {
			wmi_evt_handlers[i].handler(wil, id, d, len);
			return true;
		}
	}

	return false;
}

static void wmi_event_handle(struct wil6210_priv *wil,
			     struct wil6210_mbox_hdr *hdr)
{
	u16 len = le16_to_cpu(hdr->len);

	if ((hdr->type == WIL_MBOX_HDR_TYPE_WMI) &&
	    (len >= sizeof(struct wil6210_mbox_hdr_wmi))) {
		struct wil6210_mbox_hdr_wmi *wmi = (void *)(&hdr[1]);
		void *evt_data = (void *)(&wmi[1]);
		u16 id = le16_to_cpu(wmi->id);

		wil_dbg_wmi(wil, "Handle WMI 0x%04x (reply_id 0x%04x)\n",
			    id, wil->reply_id);
		/* check if someone waits for this event */
		if (wil->reply_id && wil->reply_id == id) {
			if (wil->reply_buf) {
				memcpy(wil->reply_buf, wmi,
				       min(len, wil->reply_size));
			} else {
				wmi_evt_call_handler(wil, id, evt_data,
						     len - sizeof(*wmi));
			}
			wil_dbg_wmi(wil, "Complete WMI 0x%04x\n", id);
			complete(&wil->wmi_call);
			return;
		}
		/* unsolicited event */
		/* search for handler */
		if (!wmi_evt_call_handler(wil, id, evt_data,
					  len - sizeof(*wmi))) {
			wil_err(wil, "Unhandled event 0x%04x\n", id);
		}
	} else {
		wil_err(wil, "Unknown event type\n");
		print_hex_dump(KERN_ERR, "evt?? ", DUMP_PREFIX_OFFSET, 16, 1,
			       hdr, sizeof(*hdr) + len, true);
	}
}

/*
 * Retrieve next WMI event from the pending list
 */
static struct list_head *next_wmi_ev(struct wil6210_priv *wil)
{
	ulong flags;
	struct list_head *ret = NULL;

	spin_lock_irqsave(&wil->wmi_ev_lock, flags);

	if (!list_empty(&wil->pending_wmi_ev)) {
		ret = wil->pending_wmi_ev.next;
		list_del(ret);
	}

	spin_unlock_irqrestore(&wil->wmi_ev_lock, flags);

	return ret;
}

/*
 * Handler for the WMI events
 */
void wmi_event_worker(struct work_struct *work)
{
	struct wil6210_priv *wil = container_of(work, struct wil6210_priv,
						 wmi_event_worker);
	struct pending_wmi_event *evt;
	struct list_head *lh;

	wil_dbg_wmi(wil, "Start %s\n", __func__);
	while ((lh = next_wmi_ev(wil)) != NULL) {
		evt = list_entry(lh, struct pending_wmi_event, list);
		wmi_event_handle(wil, &evt->event.hdr);
		kfree(evt);
	}
	wil_dbg_wmi(wil, "Finished %s\n", __func__);
}
