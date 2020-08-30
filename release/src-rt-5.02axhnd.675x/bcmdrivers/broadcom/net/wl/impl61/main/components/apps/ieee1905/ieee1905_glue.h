/***********************************************************************
 * <:copyright-BRCM:2013:proprietary:standard
 *
 *    Copyright (c) 2013 Broadcom
 *    All Rights Reserved
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

#ifndef _IEEE1905_GLUE_H_
#define _IEEE1905_GLUE_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <security_ipc.h>
#define I5_GLUE_DAEMON_PID_FILENAME "/var/run/ieee1905.pid"

#define I5_GLUE_CONTROL_SOCK_PORT		EAPD_WKSP_WBD_AGENTCLI_PORT
#define I5_GLUE_CONTROL_SOCK_CONTROLLER_PORT	EAPD_WKSP_WBD_CTRLCLI_PORT

#define I5_GLUE_WLCFG_WL_NAME_STRING  "wl"
#define I5_GLUE_WLCFG_WDS_NAME_STRING "wds"

int             i5GlueInterfaceIsSocketRequired(char const *ifname, unsigned short media_type,
                  char *real_ifname);
int             i5GlueInterfaceFilter(char const*ifname);
unsigned short  i5GlueInterfaceGetMediaInfoFromName(char const *ifname, unsigned char *pMediaInfo,
                  int *pMediaLen, unsigned char *pNetTechOui,  unsigned char *pNetTechVariant,
                  unsigned char *pNetTechName, unsigned char *url, int sizeUrl, char *real_ifname);
int             i5GlueMainInit(unsigned int multiapMode);
int             i5GlueAssignFriendlyName(unsigned char *deviceId, char *pFriendlyName, int maxLen);
void            i5GlueSaveConfig();
void            i5GlueLoadConfig(unsigned int supServiceFlag, int isRegistrar);
void		i5GlueMainDeinit();
/* Check if the operational status UP or Down */
int i5GlueIsOperStateUP(const char *ifname);
#if defined(MULTIAP) && defined(CONFIG_HOSTAPD)
int i5GlueIsHapdEnabled();
int i5GlueWpsPbc(char *fhIfname, char *bhIfname);
#endif	/* MULTIAP && CONFIG_HOSTAPD */
/* Checks whether VLANs can be created or not */
int i5GlueIsCreateVLANS();
/* Delete VLAN interface from the list */
void i5GlueDeleteVlanIfr(char *ifname);
/* For an interface create VLAN interfaces */
void i5GlueCreateVLAN(char *ifname, int isWireless);
/* Checks whether the ifname is Virtual VLAN interface or not. If yes, then it stores the
 * real_ifname argument with the primary ifname from which the VLAN interface is created
 */
int i5GlueIsIfnameVLAN(const char *ifname, char *real_ifname, int *isSecondary);
/* Remove the socket created for primary interface if the virtual VLAN interface is created. */
void i5GlueRemovePrimarySocketOnVirtualSocketCreate(const char *ifname);
#endif // endif
