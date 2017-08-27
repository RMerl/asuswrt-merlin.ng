/***********************************************************************
 * <:copyright-BRCM:2008-2013:proprietary:standard
 * 
 *    Copyright (c) 2008-2013 Broadcom 
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
 * :> *
 * $Change: 121810 $
 ***********************************************************************/

/* \file l2_driver_settings.h
 *
 * \brief File for L2 driver compilation settings
 *
 **************************************************/


/* ///////////////////////////////////////////////////////////////
   //      Select below L2 library options and features         //
   /////////////////////////////////////////////////////////////// */


/**
 *  Default MMV version
 *
 *  Management message version used by default to connect to PLC
 *  devices.
 *  Note: MMV1 requires ENCRYPTION support: L2_DRIVER_ENCRYPTION.
 **/
#define L2_DRIVER_DEFAULT_MMV 2


/**
 *  L2 driver encryption support
 *
 *  Enables support for L2 primitive encryption.
 *  Note: requires DAK generation:  L2_DRIVER_DAK_GENERATION.
 **/
//#define L2_DRIVER_ENCRYPTION


/**
 *  Modem Authentication
 *  
 *  Authenticates PC vs modem connection.
 **/
//#define L2_DRIVER_AUTHENTICATE


/**
 *  FW upgrade support 
 *  
 *  Allows to upgrade the FW through Wireconf.
 *  Meaningless if PLC SoC has no dedicated flash.
 **/
//#define L2_DRIVER_UPGRADE


/**
 *  DAK generation
 *  
 *  DAK generation feature.
 **/
#define L2_DRIVER_DAK_GENERATION


/**
 *  NMK generation
 *  
 *  NMK generation feature.
 **/
#define L2_DRIVER_NMK_NID_GENERATION


/**
 *  APSTA_INFO
 *  
 *  Get STA information about firmware/hardware/reference design.
 **/
#define L2_DRIVER_APSTA_INFO


/**
 *  APCM_GET_NEWSTA
 *  
 *  Get associated PLC stations and the used band feature.
 **/
//#define L2_DRIVER_APCM_GET_NEWSTA


/**
 *  APCM_NW_STATS
 *  
 *  Get PLC STA network status feature.
 *  Read associated devices and their TX/RX PHY rates.
 **/
#define L2_DRIVER_APCM_NW_STATS


/**
 *  APCM_NW_INFO
 *  
 *  Get network information feature.
 **/
#define L2_DRIVER_APCM_NW_INFO


/**
 *  APCM_BRG_INFO
 *  
 *  Get bridge table information (bridge connected devices).
 **/
#define L2_DRIVER_APCM_BRG_INFO


/**
 *  APCM_UKE_CMD
 *  
 *  Universal key exchange support
 *  Triggers UKE protocol to associate with other PLC STAs.
 **/
#define L2_DRIVER_APCM_UKE_CMD


/**
 *  APCM_STDB_CMD
 *
 *  Enable Standby command support.
 *  Triggers the standby status, wakeable by button press
 *  or gphy link detection.
 **/
#define L2_DRIVER_APCM_STDBY_CMD

/**
 *  APCM_SET_KEY
 *  
 *  Enable support for changing the network key.
 **/
#define  L2_DRIVER_APCM_SET_KEY


/**
 *  APCM_AUTHORIZE
 *  
 *  Enable support for PLC authorize STAs.
 **/
#define  L2_DRIVER_APCM_AUTHORIZE


/**
 *  APCM_STA_CAP
 *  
 *  Get STA capabilities information.
 **/
//#define L2_DRIVER_APCM_STA_CAP


/**
 *  APCM_STA_RESTART
 *  
 *  Soft-restarting PLC STA feature.
 **/
//#define L2_DRIVER_APCM_STA_RESTART


/**
 *  APFW_GET_PARAM/APFW_SET_PARAM
 *  
 *  Enable support for reading and writing paramconfig parameters.
 **/
#define L2_DRIVER_APFW_GET_SET_PARAM


/**
 *  APFW_TR069_GET_STATS/APFW_TR069_GET_STATS
 *  
 *  Enable support for reading TR069 statistics
 **/
#define L2_DRIVER_APFW_TR069_GET_STATS


/**
 *  APFW_HDR_CFG_PARAM
 *  
 *  Enable support for editing PLC flash header.
 *  Note: Disable this for a systems without PLC flash.
 **/
//#define  L2_DRIVER_APFW_HDR_CFG


/**
 *  APFW_FACTORY_RESET
 *  
 *  PLC factory reset primitive.
 *  Note: In systems where the PLC filesystem is located
 *        in the host's flash filesystem (NVMFS), 
 *        it is not really needed since the host could directly
 *        wipe out the paramconfig file/partition 
 *        to restore factory defaults.
 **/
//#define L2_DRIVER_APFW_FACTORY_RESET


/**
 *  APFW_GET_DEBUG
 *  
 *  Get PLC debug information primitive.
 **/
#define L2_DRIVER_APFW_GET_DEBUG



/**
 *  APFW_1905_FOB
 *  
 *  Enable support 1905 Failover Bonding commands
 **/
#define L2_DRIVER_APFW_1905_FOB


/**
 *  APFW_LOOPBACK_TRAFFIC
 *  
 *  Enable support for loopback traffic
 **/
#define L2_DRIVER_APFW_LOOPBACK_TRAFFIC

/* ///////////////////////////////////////////////////////////////
   //        End of L2 feature and options selection            //
   /////////////////////////////////////////////////////////////// */







/* Options dependency check (Do not modify below to select options) */


#if (L2_DRIVER_DEFAULT_MMV==1) && !defined(L2_DRIVER_ENCRYPTION)
  #error MMV=1 requires encryption enabled. Please, enable L2_DRIVER_ENCRYPTION
#endif


#if defined(L2_DRIVER_ENCRYPTION) && !defined(L2_DRIVER_DAK_GENERATION)
  #error L2_DRIVER_ENCRYPTION requires DAK generation enabled. Please enable L2_DRIVER_DAK_GENERATION
#endif







