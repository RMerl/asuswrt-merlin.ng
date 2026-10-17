/*
   <:copyright-BRCM:2024:DUAL/GPL:standard
   
      Copyright (c) 2024 Broadcom 
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

#ifndef __BCMNET_WOL_H__
#define __BCMNET_WOL_H__

/*
*******************************************************************************
*
* File Name  : bcmnet_wol.h
*
* Description: This file defins WOL data elements
*
*******************************************************************************
*/
#define FIELD_MASK(bits, shift)		(((1ULL<<(bits)) - 1) << shift)

typedef struct {
  uint32_t config;
  uint32_t control;
  uint32_t status;
  uint32_t _na_;
  uint32_t timer_cfg;
} ET_XPHY_WOL;

#define ET_XPHY_WOL_CONTROL_WOL_EN_M       FIELD_MASK(1,0)
#define ET_XPHY_WOL_CONTROL_WOL_RX_EN_M    FIELD_MASK(1,1)
#define ET_XPHY_WOL_CONTROL_WOL_RX_EN_S    1
#define ET_XPHY_WOL_CONTROL_WOL_TX_EN_M    FIELD_MASK(1,2)
#define ET_XPHY_WOL_CONTROL_WOL_TX_EN_S    1

typedef struct {
  uint32_t config;
  uint32_t control;
  uint32_t status;
  uint32_t _na_;
  uint32_t mac_low;
  uint32_t mac_hi;
  uint32_t psw_low;
  uint32_t psw_hi;
} ET_WOL_DET_MPD;

#define ET_WOL_DET_MPD_CTRL_EN_M	FIELD_MASK(1,0)
#define ET_WOL_DET_MPD_STAT_DETECTED	FIELD_MASK(1,0)

typedef struct {
  uint32_t config;
  uint32_t control;
  uint32_t status;
  uint32_t _na_;
  uint32_t tag_cfg;
} ET_WOL_DET_ARD;

#define ET_WOL_DET_ARD_CTRL_EN_M	FIELD_MASK(1,0)
#define ET_WOL_DET_ARD_STAT_DETECTED	FIELD_MASK(1,0)

typedef struct {
  ET_XPHY_WOL     wol_ctl;
  uint8_t         dummy_1[0x6C];
  ET_WOL_DET_MPD  wol_mpd;
  ET_WOL_DET_ARD  wol_ard;
} ET_WOL_DET;

#define WOL_PORT_MISSING		0xFFFFFFFF
#define WOL_PORT_COUNT			7

#if defined(CONFIG_BCM96813)
#define WOL_PORT_0_OFFSET		(0x00003500 - 0x80) /* Adjust for missing ET_XPHY_WOL registers in XPORT_WOL*/
#define WOL_PORT_1_OFFSET		(0x00007500 - 0x80) /* Adjust for missing ET_XPHY_WOL registers in XPORT_WOL*/
#define WOL_PORT_2_OFFSET		WOL_PORT_MISSING
#define WOL_PORT_3_OFFSET		WOL_PORT_MISSING
#define WOL_PORT_4_OFFSET		WOL_PORT_MISSING
#define WOL_PORT_5_OFFSET		WOL_PORT_MISSING
#define WOL_PORT_6_OFFSET		WOL_PORT_MISSING
#else
#define WOL_PORT_0_OFFSET		0x00000000
#define WOL_PORT_1_OFFSET		WOL_PORT_MISSING
#define WOL_PORT_2_OFFSET		WOL_PORT_MISSING
#define WOL_PORT_3_OFFSET		WOL_PORT_MISSING
#define WOL_PORT_4_OFFSET		WOL_PORT_MISSING
#define WOL_PORT_5_OFFSET		0x00001000
#if defined(CONFIG_BCM96766) || defined(CONFIG_BCM96764)
#define WOL_PORT_6_OFFSET		0x00001800
#else
#define WOL_PORT_6_OFFSET		WOL_PORT_MISSING
#endif
#endif

#endif /* __BCMNET_WOL_H__ */
