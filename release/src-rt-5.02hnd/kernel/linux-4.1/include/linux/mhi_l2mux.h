#ifdef CONFIG_BCM_KF_MHI
/*
<:copyright-BRCM:2012:DUAL/GPL:standard

   Copyright (c) 2012 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/
/*
 * File: mhi_l2mux.h
 *
 * MHI L2MUX kernel definitions
 */

#ifndef LINUX_L2MUX_H
#define LINUX_L2MUX_H

#include <linux/types.h>
#include <linux/socket.h>

#ifdef __KERNEL__
#include <net/sock.h>
#define ACTIVATE_L2MUX_STAT

#ifdef ACTIVATE_L2MUX_STAT
#include <linux/list.h>
#include <linux/time.h>
#endif /* ACTIVATE_L2MUX_STAT */
#endif /*__KERNEL__*/

/* Official L3 protocol IDs */
#define MHI_L3_PHONET		0x00
#define MHI_L3_FILE		0x01
#define MHI_L3_AUDIO		0x02
#define MHI_L3_SECURITY		0x03
#define MHI_L3_TEST		0x04
#define MHI_L3_TEST_PRIO	0x05
#define MHI_L3_XFILE		0x06
#define MHI_L3_MHDP_DL		0x07
#define MHI_L3_MHDP_UL		0x08
#define MHI_L3_AUX_HOST		0x09
#define MHI_L3_LOG		0x0A
#define MHI_L3_CELLULAR_AUDIO	0x0B
#define MHI_L3_IMS              0x0D
#define MHI_L3_OEM_CP		0x0E
#define MHI_L3_MHDP_DL_PS2	0x17
#define MHI_L3_MHDP_UL_PS2	0x18
#define MHI_L3_CTRL_TMODEM	0xC0
#define MHI_L3_THERMAL		0xC1
#define MHI_L3_MHDP_UDP_FILTER	0xFC
#define MHI_L3_HIGH_PRIO_TEST	0xFD
#define MHI_L3_MED_PRIO_TEST	0xFE
#define MHI_L3_LOW_PRIO_TEST	0xFF

/* 256 possible protocols */
#define MHI_L3_NPROTO		256

/* Special value for ANY */
#define MHI_L3_ANY		0xFFFF

#ifdef __KERNEL__
typedef int (l2mux_skb_fn)(struct sk_buff *skb, struct net_device *dev);
typedef int (l2mux_audio_fn)(unsigned char *buffer, size_t size, uint8_t phonet_dev_id);

struct l2muxhdr {
	__u8	l3_len[3];
	__u8	l3_prot;
} __packed;

#ifdef ACTIVATE_L2MUX_STAT

enum l2mux_direction {
	UPLINK_DIR = 0,
	DOWNLINK_DIR,
};

enum l2mux_trace_state {
	ON = 0,
	OFF,
	KERNEL,
};


struct l2muxstat {
	unsigned l3pid;
	unsigned l3len;
	enum l2mux_direction dir;
	struct timeval time_val;
	struct list_head list;
	unsigned int stat_counter;
};

struct l2mux_stat_info {
	struct proc_dir_entry *proc_entry;
	struct l2muxstat l2muxstat_tab;
	int l2mux_stat_id;
	int previous_stat_counter;
	unsigned int l2mux_total_stat_counter;
	enum l2mux_trace_state l2mux_traces_state;
	int l2mux_traces_activation_done;
	struct net_device *dev;
	struct work_struct l2mux_stat_work;
};

#endif /* ACTIVATE_L2MUX_STAT */


#define L2MUX_HDR_SIZE  (sizeof(struct l2muxhdr))


static inline struct l2muxhdr *l2mux_hdr(struct sk_buff *skb)
{
	return (struct l2muxhdr *)skb_mac_header(skb);
}

static inline void l2mux_set_proto(struct l2muxhdr *hdr, int proto)
{
	hdr->l3_prot = proto;
}

static inline int l2mux_get_proto(struct l2muxhdr *hdr)
{
	return hdr->l3_prot;
}

static inline void l2mux_set_length(struct l2muxhdr *hdr, unsigned len)
{
	hdr->l3_len[0] = (len) & 0xFF;
	hdr->l3_len[1] = (len >>  8) & 0xFF;
	hdr->l3_len[2] = (len >> 16) & 0xFF;
}

static inline unsigned l2mux_get_length(struct l2muxhdr *hdr)
{
	return (((unsigned)hdr->l3_len[2]) << 16) |
		(((unsigned)hdr->l3_len[1]) << 8) |
		((unsigned)hdr->l3_len[0]);
}

extern int l2mux_netif_rx_register(int l3, l2mux_skb_fn *rx_fn);
extern int l2mux_netif_rx_unregister(int l3);

extern int l2mux_netif_tx_register(int pt, l2mux_skb_fn *rx_fn);
extern int l2mux_netif_tx_unregister(int pt);

extern int l2mux_skb_rx(struct sk_buff *skb, struct net_device *dev);
extern int l2mux_skb_tx(struct sk_buff *skb, struct net_device *dev);

enum l2mux_audio_dev_id {
	L2MUX_AUDIO_DEV_ID0,
	L2MUX_AUDIO_DEV_ID1,
	L2MUX_AUDIO_DEV_ID2,
	L2MUX_AUDIO_DEV_ID3,
	L2MUX_AUDIO_DEV_MAX
};
#define L2MUX_AUDIO_DEV_TYPE_RX		(0x1 << 24)
#define L2MUX_AUDIO_DEV_TYPE_TX		(0x2 << 24)

/* both register functions will return a positive integer value when
 * registration complete successfully, please use the handle for
 * unregistration.
 * We assume that there should be only 1 voice code on the host side, but
 * there might be more than 1 modem */
extern int l2mux_audio_rx_register(l2mux_audio_fn *fn);
extern int l2mux_audio_rx_unregister(int handle);

extern int l2mux_audio_tx_register(uint8_t phonet_dev_id, l2mux_audio_fn *fn);
extern int l2mux_audio_tx_unregister(int handle);

/* 
 * Input: buffer: pointer to L2muxhdr + audio payload, since payload length
 *		  info is in l2muxhdr, so we don't need another argument
 *	  pn_dev_id: the phonet ID this device is binded to.
 * Note: the buffer should never be freed by anyone, since this is a static
 * 	 buffer allocated by the modem driver. */
extern int l2mux_audio_rx(unsigned char *buffer, uint8_t pn_dev_id);

/*
 * Input: buffer: pointer to audio payload WITHOUT l2muxhdr.  The buffer should
 * 		  reserve at least 4 bytes in the headroom, such that L2mux
 * 		  does not need to allocate a new memory for inserting l2muxhdr.
 * 	  size: the size of the payload
 * 	  pn_dev_id: The phonet ID that indicates the device which this audio
 * 		     data is destiend to
 * Note: The buffer should never be freed by anyone.  It should be a static
 * 	 buffer allocated by the voice code.
 */
extern int l2mux_audio_tx(unsigned char *buffer, size_t size,
		uint8_t pn_dev_id);

#endif /*__KERNEL__*/

#endif /* LINUX_L2MUX_H */
#endif /* CONFIG_BCM_KF_MHI */
