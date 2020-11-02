/*
 * VASIP related declarations
 * Broadcom 802.11abg Networking Device Driver
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: wlc_vasip.h 726102 2017-10-11 16:14:30Z $
 */

#ifndef _WLC_VASIP_H_
#define _WLC_VASIP_H_

#ifdef VASIP_HW_SUPPORT

/* defines svmp address */
#define VASIP_COUNTERS_ADDR_OFFSET	          0x20000
#define VASIP_STATUS_ADDR_OFFSET	          0x20100
#define VASIP_STEER_MCS_ADDR_OFFSET		      0x21e10
#define VASIP_RECOMMEND_MCS_ADDR_OFFSET	      0x22ea0
#define VASIP_SPECTRUM_TBL_ADDR_OFFSET	      0x26800
#define VASIP_OVERWRITE_MCS_FLAG_ADDR_OFFSET  0x20060
#define VASIP_OVERWRITE_MCS_BUF_ADDR_OFFSET   0x21d00
#define VASIP_STEERING_MCS_BUF_ADDR_OFFSET    0x21e10
#define VASIP_GROUP_METHOD_ADDR_OFFSET        0x20045
#define VASIP_GROUP_NUMBER_ADDR_OFFSET        0x20046
#define VASIP_GROUP_FORCED_ADDR_OFFSET        0x20047
#define VASIP_GROUP_FORCED_MCS_ADDR_OFFSET    0x20048
#define VASIP_MCS_CAPPING_ENA_OFFSET          0x20044
#define VASIP_MCS_RECMND_MI_ENA_OFFSET        0x20049
#define VASIP_SGI_RECMND_METHOD_OFFSET        0x2004a
#define VASIP_SGI_RECMND_THRES_OFFSET         0x2004b
#define VASIP_DELAY_GROUP_TIME_OFFSET         0x20054
#define VASIP_DELAY_PRECODER_TIME_OFFSET      0x20055
#define VASIP_GROUP_FORCED_OPTION_ADDR_OFFSET 0x21be0
#define VASIP_PPR_TABLE_OFFSET                0x21d20
#define VASIP_MCS_THRESHOLD_OFFSET            0x22EE0
#define VASIP_M2V0_OFFSET                     0x20300
#define VASIP_M2V1_OFFSET                     0x20700
#define VASIP_V2M_GROUP_OFFSET                0x20b00
#define VASIP_V2M_PRECODER_OFFSET             0x20be0
#define VASIP_RATECAP_BLK		              0x22f20

#define VASIP_RTCAP_SGI_NBIT		0x2
#define VASIP_RTCAP_LDPC_NBIT		0x4
#define VASIP_RTCAP_BCMSTA_NBIT		0x5

/* initialize vasip */
void wlc_vasip_init(wlc_hw_info_t *wlc_hw);

/* attach/detach */
extern int wlc_vasip_attach(wlc_info_t *wlc);
extern void wlc_vasip_detach(wlc_info_t *wlc);
int wlc_svmp_mem_blk_set(wlc_hw_info_t *wlc_hw, uint32 offset, uint16 len, uint16 *val);
#else
#define wlc_vasip_init(wlc_hw) do {} while (0)
#define wlc_svmp_mem_blk_set(wlc_hw, offset, len, val) do {} while (0)
#endif /* VASIP_HW_SUPPORT */

#ifdef WL_AIR_IQ
int wlc_svmp_mem_read64(wlc_hw_info_t *wlc_hw, uint64 *ret_svmp_addr, uint32 offset, uint16 len);
int wlc_svmp_mem_set_axi(wlc_hw_info_t *wlc_hw, uint32 offset, uint16 len, uint16 val);
int wlc_svmp_mem_read_axi(wlc_hw_info_t *wlc_hw, uint16 *ret_svmp_addr, uint32 offset, uint16 len);
#endif // endif

#endif /* _WLC_VASIP_H_ */
