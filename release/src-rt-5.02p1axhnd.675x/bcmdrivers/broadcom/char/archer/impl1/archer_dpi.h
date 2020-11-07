/*
  <:copyright-BRCM:2019:proprietary:standard

  Copyright (c) 2019 Broadcom 
  All Rights Reserved

  This program is the proprietary software of Broadcom and/or its
  licensors, and may only be used, duplicated, modified or distributed pursuant
  to the terms and conditions of a separate, written license agreement executed
  between you and Broadcom (an "Authorized License").  Except as set forth in
  an Authorized License, Broadcom grants no license (express or implied), right
  to use, or waiver of any kind with respect to the Software, and Broadcom
  expressly reserves all rights in and to the Software and all intellectual
  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

  Except as expressly set forth in the Authorized License,

  1. This program, including its structure, sequence and organization,
  constitutes the valuable trade secrets of Broadcom, and you shall use
  all reasonable efforts to protect the confidentiality thereof, and to
  use this information only in connection with your use of Broadcom
  integrated circuit products.

  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
  ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
  FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
  COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
  TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
  PERFORMANCE OF THE SOFTWARE.

  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
  ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
  WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
  IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
  OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
  SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
  SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
  LIMITED REMEDY.
  :> 
*/

#ifndef __ARCHER_DPI_H_INCLUDED__
#define __ARCHER_DPI_H_INCLUDED__

extern int archer_dpi_enabled_g;

int archer_dpi_ds_enqueue_bin(uint8_t *packet_p, int packet_len,
                              sysport_classifier_flow_t *flow_p);

static inline int archer_dpi_ds_enqueue(uint8_t *packet_p, int packet_len,
                                        sysport_classifier_flow_t *flow_p)
{
    if(archer_dpi_enabled_g)
    {
        return archer_dpi_ds_enqueue_bin(packet_p, packet_len, flow_p);
    }

    return 0;
}

int archer_dpi_us_enqueue_bin(uint8_t *packet_p, int packet_len,
                              sysport_classifier_flow_t *flow_p,
                              int recycle_type, int egress_port);

static inline int archer_dpi_us_enqueue(uint8_t *packet_p, int packet_len,
                                        sysport_classifier_flow_t *flow_p,
                                        int recycle_type, int egress_port)
{
    if(archer_dpi_enabled_g)
    {
        return archer_dpi_us_enqueue_bin(packet_p, packet_len, flow_p,
                                         recycle_type, egress_port);
    }

    return 0;
}

int archer_dpi_command(archer_dpi_cmd_t cmd);

int archer_dpi_construct(void);

#endif  /* defined(__ARCHER_DPI_H_INCLUDED__) */
