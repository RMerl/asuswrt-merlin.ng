/*
   <:copyright-BRCM:2022:DUAL/GPL:standard

      Copyright (c) 2022 Broadcom
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
*******************************************************************************
*
* File Name  : tc.h
*
* Description: bcm common defines for tc rules
*
*******************************************************************************
*/

#ifndef __TC_H_INCLUDED__
#define __TC_H_INCLUDED__

struct net_device;

int tc_ingress_setup_block(struct net_device *dev, void *type_data);

/*0 is default chain id of tc rules*/
#define BCM_TC_CHAIN_ID_DEFAULT                0

/*
 chain id 1~16 are reserved for tc rules of IC.
 keep in sync with following files:
 (a)userspace/public/apps/scripts/init/tc-config
 (b)targets/fs.src/etc/mdev_wan.sh
 (c)targets/fs.src/etc/mdev_eth.sh
*/
#define BCM_TC_CHAIN_ID_IC_INGRESS_FILTER      1
#define BCM_TC_CHAIN_ID_IC_RX                  2
#define BCM_TC_CHAIN_ID_IC_PRIMARY             3
#define BCM_TC_CHAIN_ID_IC_FLOW_MISS           4
#define BCM_TC_CHAIN_ID_IC_MAX                 16

/*tc chain id for user space start from (BCM_TC_CHAIN_ID_IC_MAX + 1)*/
#define BCM_TC_CHAIN_ID_USR_BASE               (BCM_TC_CHAIN_ID_IC_MAX + 1)

/*special definitions for tc chain id*/
/*To EPON*/
#define BCM_TC_CHAIN_ID_EPON_BASE              (BCM_TC_CHAIN_ID_USR_BASE + 3) /*it is 20*/
/*The max chain id currently used by EPON*/
#define BCM_TC_CHAIN_ID_EPON_MAX               10606

/*
 * OMCI, EPON-OAM and other userspace management protocols should never
 * dual-manage/configure the same interface, therefore, overlapping
 * chain IDs are OK.
 */
#define BCM_TC_CHAIN_ID_OMCI_BASE              (BCM_TC_CHAIN_ID_USR_BASE + 3) /* 20 */
#define BCM_TC_CHAIN_ID_OMCI_MAX               65535 /* 0xFFFF */

#define BCM_TC_IF_ETYPE_ARP                    11     /* dedicated priority number for ARP */
#define BCM_TC_IF_ETYPE_802_1X                 12     /* dedicated priority number for 802.1X PAE */

#endif  /* __TC_H_INCLUDED__ */
