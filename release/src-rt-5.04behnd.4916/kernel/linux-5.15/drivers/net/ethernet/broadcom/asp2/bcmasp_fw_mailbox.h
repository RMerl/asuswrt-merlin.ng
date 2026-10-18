// SPDX-License-Identifier: GPL-2.0
#ifndef __BCMASP_FW_MAILBOX_H
#define __BCMASP_FW_MAILBOX_H

#include <linux/types.h>
#include <asm/unaligned.h>

enum bcmasp_fw_msg_types
{
	/* Host to FW message*/
	ASP_MSG_TYPE_H2FW_RES = 0,
	ASP_MSG_TYPE_H2FW_S2_ENTER,
	ASP_MSG_TYPE_H2FW_S2_WAKEUP,
	ASP_MSG_TYPE_H2FW_MAX,
	/* FW To Host messages */
	ASP_MSG_TYPE_FW2H_TRANSFER_DATA = 10,
	ASP_MSG_TYPE_FW2H_DONE,
	ASP_MSG_TYPE_FW2H_MAX,
};

struct bcmasp_fw_header
{
	__u8 message_id;
	__u8 message_cnt;
	__u16 message_ver;
	__u32 reserved;
} __packed;

struct bcmasp_fw_msg_mode_switch
{
	__u8 mode; /* 0: Normal FW, 1: S2 MDNS FW */
} __packed;

struct bcmasp_fw_msg_enter_s2
{
	__u32 netfilt_id_mask;
	__u32 netfilt_id_mask_direct;
	__u32 pkt_dump_dma_hi;
	__u32 pkt_dump_dma_lo;
	__u32 pkt_dump_dma_size;
} __packed;

struct bcmasp_fw_msg_wake_s2
{
	__u32 reserved;
} __packed;

struct bcmasp_fw_h2fw_msg
{
	struct bcmasp_fw_header msg_header;
	union
	{
		struct bcmasp_fw_msg_mode_switch mode_switch;
		struct bcmasp_fw_msg_enter_s2 enter_s2;
		struct bcmasp_fw_msg_wake_s2 wake_s2;
	};
} __packed;

struct bcmasp_fw_msg_transfer_s2
{
	__u32 valid_wakeup_pkt;
	__u32 transfer_size;
} __packed;

struct bcmasp_fw_msg_done
{
	__u32 status;
} __packed;

struct bcmasp_fw_fw2h_msg
{
	struct bcmasp_fw_header msg_header;
	union
	{
		struct bcmasp_fw_msg_transfer_s2 transfer_s2;
		struct bcmasp_fw_msg_done done;
	};
} __packed;

#endif
