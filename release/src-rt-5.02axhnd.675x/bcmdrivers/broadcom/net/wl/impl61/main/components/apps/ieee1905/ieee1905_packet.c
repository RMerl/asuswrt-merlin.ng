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

/*
 * IEEE1905 Packet
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <linux/if_ether.h>
#include "ieee1905_timer.h"
#include "ieee1905_message.h"

i5_packet_type *i5PacketNew(void)
{
  i5_packet_type *ppkt;
  unsigned char *pbuf;

  if ((ppkt = (i5_packet_type *)malloc(sizeof(i5_packet_type))) == NULL) {
    printf("Malloc error\n");
    return NULL;
  }

  if ((pbuf = (unsigned char *)malloc(I5_PACKET_BUF_LEN)) == NULL) {
    printf("Malloc error\n");
    free(ppkt);
    return NULL;
  }

  memset(ppkt, 0, sizeof(i5_packet_type));
  ppkt->pbuf = pbuf;

  return ppkt;
}

void i5PacketFree(i5_packet_type *ppkt)
{
  // Only use this function if the packet is not associated with a message
  free(ppkt->pbuf);
  free(ppkt);
}

void i5PacketHeaderInit(i5_packet_type *ppkt, unsigned short message_type, unsigned short message_identifier)
{
  i5_message_header_type *phdr = (i5_message_header_type *)&ppkt->pbuf[sizeof(struct ethhdr)];

  phdr->message_version = I5_MESSAGE_VERSION;
  phdr->reserved_field = 0;
  phdr->message_type = htons(message_type);
  phdr->message_identifier = htons(message_identifier);
  phdr->fragment_identifier = 0;
  phdr->indicators = 0;
  phdr->last_fragment_indicator = 1;

  switch (message_type) {
    case i5MessageTopologyNotificationValue:
    case i5MessageApAutoconfigurationSearchValue:
    case i5MessageApAutoconfigurationRenewValue:
    case i5MessagePushButtonEventNotificationValue:
    case i5MessagePushButtonJoinNotificationValue:
      phdr->relay_indicator = 1;
      break;
  default:
      break;
  }

  ppkt->length += sizeof(i5_message_header_type);
}
