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
 * $Change: 116460 $
 ***********************************************************************/

#ifndef _IEEE1905_INTERFACE_H_
#define _IEEE1905_INTERFACE_H_

/*
 * IEEE1905 Interface
 */
#include "ieee1905_socket.h"
#include "ieee1905_message.h"
#include "ieee1905_datamodel_priv.h"

/* cannot include both net/if.h and linux/if.h due to conflicts
   extern functions from net/if.h here instead */
extern unsigned int if_nametoindex (__const char *__ifname) __THROW;
extern char *if_indextoname(unsigned int ifindex, char *ifname);

/* Interface options */
#define USE_IF_MAC_AS_SRC (1<<0)
#define USE_AL_MAC_AS_SRC (1<<1)

int i5InterfacePacketSend(i5_socket_type *psock, i5_packet_type *ppkt);
i5_socket_type *i5InterfaceSocketSet(char *ifname, unsigned short media_type, unsigned char const *pNetTechOui);
void i5InterfaceNew(char *ifname, unsigned short media_type, unsigned char const *media_specific_info,
                    unsigned int media_specific_info_size, unsigned char const *pNetTechOui,
                    unsigned char const *pNetTechVariant,  unsigned char const *pNetTechName,
                    unsigned char const *url, int sizeUrl, i5MacAddressDeliveryFunc deliverFunc,
                    char *real_ifname);
int i5InterfaceInfoGet(char *ifname, unsigned char *mac_address);
void i5InterfaceInit();
void i5InterfaceAdd(char *ifname, unsigned short matchMediaType);
void i5InterfaceSearchAdd(unsigned short matchMediaType);
int i5InterfaceSearchFileForString(char const *file, char const *searchString);
int i5InterfaceSearchFileForIndex(char const *file, unsigned int wdsIndex, char* macString, unsigned int size);
void i5InterfaceBridgeNotifyReceiveOperStatus( i5_socket_type *psock, unsigned char oper_status );
void i5GetInterfaceIDFromIfname(char *ifname, unsigned char *mac);
void i5GetIfnameFromMacAdress(unsigned char *mac, char *ifname);

#endif // endif
