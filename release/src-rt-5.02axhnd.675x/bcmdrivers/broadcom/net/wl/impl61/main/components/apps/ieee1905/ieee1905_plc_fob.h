/***********************************************************************
 *
 *  Copyright (c) 2013  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2013:proprietary:standard
 *
 *  This program is the proprietary software of Broadcom and/or its
 *  licensors, and may only be used, duplicated, modified or distributed pursuant
 *  to the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied), right
 *  to use, or waiver of any kind with respect to the Software, and Broadcom
 *  expressly reserves all rights in and to the Software and all intellectual
 *  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 *  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 *  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1. This program, including its structure, sequence and organization,
 *     constitutes the valuable trade secrets of Broadcom, and you shall use
 *     all reasonable efforts to protect the confidentiality thereof, and to
 *     use this information only in connection with your use of Broadcom
 *     integrated circuit products.
 *
 *  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
 *     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
 *     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
 *     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
 *     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
 *     PERFORMANCE OF THE SOFTWARE.
 *
 *  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 *     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
 *     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
 *     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
 *     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
 *     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
 *     LIMITED REMEDY.
 * :>
 *
 * $Change: 111969 $
 ***********************************************************************/

#ifndef _IEEE1905_PLC_FOB_H_
#define _IEEE1905_PLC_FOB_H_

#define IEEE1905_PLC_FOB_IND_MAX_SIZE 1024

/* FOB IND packet offsets */
#define IEEE1905_PLC_FOB_IND_ETHERTYPE_OFFSET 12
#define IEEE1905_PLC_FOB_IND_MM_VERSION_OFFSET 14
#define IEEE1905_PLC_FOB_IND_MMTYPE_OFFSET 15
#define IEEE1905_PLC_FOB_IND_FMI_OFFSET 17
#define IEEE1905_PLC_FOB_IND_OUI_OFFSET 19
#define IEEE1905_PLC_FOB_IND_MSGID_OFFSET 22
#define IEEE1905_PLC_FOB_IND_PAYLOAD_OFFSET 23

/* FOB IND packet values */
#define IEEE1905_PLC_FOB_IND_ETHERTYPE 0x88E1

/* MM TYPE and Version of FOB indication */
#define IEEE1905_PLC_FOB_IND_MMTYPE      0xA2A0
#define IEEE1905_PLC_FOB_IND_MM_VERSION  0x2

/* FOB IND max nodes */
#define IEEE1905_PLC_FOB_MAX_NODES       15

typedef struct {
  unsigned char  maccAddr[MAC_ADDR_LEN];
  unsigned short interval;
  unsigned short totalBw;
  unsigned short availBw;
} __attribute__((__packed__)) ieee1905_plc_fob_entry_type;

typedef struct {
  unsigned char               macCount;
  ieee1905_plc_fob_entry_type fobEntry[];
} __attribute__((__packed__)) ieee1905_plc_fob_type;

#endif /* _IEEE1905_PLC_FOB_H_ */
