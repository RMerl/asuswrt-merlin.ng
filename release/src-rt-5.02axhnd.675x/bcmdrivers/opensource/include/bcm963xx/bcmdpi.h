#ifndef BCMDPI_H
#define BCMDPI_H
/*
 * <:copyright-BRCM:2019:DUAL/GPL:standard
 * 
 *    Copyright (c) 2019 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 */
#include <linux/types.h>

#define DPI_DIR_DS		0
#define DPI_DIR_US		1
#define DPI_XRDP_US_SQ_OFFSET	100

/* put netlink defines here since toolchain does not have updated
 * uapi headers */
#define CTA_DPI			24
#define CTA_DPI_APP_ID		1
#define CTA_DPI_MAC		2
#define CTA_DPI_STATUS		3
#define CTA_DPI_URL		4
#define CTA_DPI_MAX		4

#define DPI_RESET_APPINST_BIT	0
#define DPI_RESET_APP_BIT	1
#define DPI_RESET_DEV_BIT	2
#define DPI_RESET_URL_BIT	3

enum dpi_nlmsg_id {
	DPI_NL_ENABLE,
	DPI_NL_DISABLE,
	DPI_NL_STATUS,
	DPI_NL_MAXPKT,
	DPI_NL_RESET_STATS,
	DPI_NL_MAX
};

struct dpi_nlmsg_hdr {
	__u16 type;
	__u16 length;
};

enum {
	DQNLGRP_NONE,
	DQNLGRP_APPINST,
	DQNLGRP_QUEUE,
	DQNLGRP_CONFIG,
	__DQNLGRP_MAX
};
#define DQNLGRP_MAX		(__DQNLGRP_MAX - 1)

enum {
	DQNL_BASE = 16,

	DQNL_NEWAPPINST = 16,
	DQNL_DELAPPINST,
	DQNL_GETAPPINST,
	DQNL_SETAPPINST,

	DQNL_GETQUEUE = 22,
	DQNL_SETQUEUE,

	DQNL_GETCONFIG = 26,
	DQNL_SETCONFIG,

	__DQNL_MAX,
#define DQNL_MAX		(__DQNL_MAX - 1)
};

/*
 * The following describe the netlink attributes used by DPI QoS when
 * transferring data to/from userspace.
 */
enum {
	DQA_AI_UNSPEC,
	DQA_AI_APP_ID,
	DQA_AI_MAC,
	DQA_AI_DEV_VENDOR,
	DQA_AI_DEV_OS,
	DQA_AI_DEV_OS_CLASS,
	DQA_AI_DEV_ID,
	DQA_AI_DEV_CATEGORY,
	DQA_AI_DEV_FAMILY,
	DQA_AI_DS_QUEUE,
	DQA_AI_US_QUEUE,
	DQA_AI_EG_PRIO,
	__DQA_AI_MAX
};
#define DQA_AI_MAX	(__DQA_AI_MAX - 1)

enum {
	DQA_BW_UNSPEC,
	DQA_BW_MIN,
	DQA_BW_MAX,
	__DQA_BW_MAX
};
#define DQA_BW_MAX	(__DQA_BW_MAX - 1)

enum {
	DQA_QUEUE_UNSPEC,
	DQA_QUEUE_DIR,
	DQA_QUEUE_ID,
	DQA_QUEUE_BW,
	__DQA_QUEUE_MAX
};
#define DQA_QUEUE_MAX	(__DQA_QUEUE_MAX - 1)

enum {
	DQA_CFG_UNSPEC,
	DQA_CFG_DIR,
	DQA_CFG_NUM_QUEUES,
	DQA_CFG_DEFAULT,
	DQA_CFG_BW,
	__DQA_CFG_MAX
};
#define DQA_CFG_MAX	(__DQA_CFG_MAX - 1)

enum {
	EG_PRIO_LOW_LATENCY,
	EG_PRIO_IMPORTANT,
	EG_PRIO_NORMAL,
	EG_PRIO_BACKGROUND,
	__EG_PRIO_MAX
};
#define EG_PRIO_MAX		(__EG_PRIO_MAX - 1)


/* --- common functions --- */
static inline __u32 dpi_make_app_id(__u8 cat_id, __u16 app_id, __u8 beh_id)
{
	return ((cat_id << 24) | (app_id << 8) | beh_id);
}
static inline __u8 dpi_get_cat_id(__u32 app_id)
{
	return (__u8)(app_id >> 24);
}
static inline __u16 dpi_get_app_id(__u32 app_id)
{
	return (__u16)((app_id >> 8) & 0xFFFF);
}
static inline __u8 dpi_get_beh_id(__u32 app_id)
{
	return (__u8)(app_id & 0xFF);
}

#endif /* BCMDPI_H */
