/***********************************************************************
 * <:copyright-BRCM:2007-2013:proprietary:standard
 * 
 *    Copyright (c) 2007-2013 Broadcom 
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
/** \file l2_driver.h
 *
 * \brief This is a main header - it includes everything else.
 *
 **************************************************/

#ifndef L2_DRIVER_H_
#define L2_DRIVER_H_

/***************************************************
*                 Include section
***************************************************/
#include "l2_driver_settings.h"

#include "base_types.h"

//! Transactions
#include "transactions/apl2c_auth.h"
#include "transactions/apsta_info.h"
#include "transactions/apfw_sta_get_status.h"
#include "transactions/apcm_get_newsta.h"
#include "transactions/apcm_brg_info.h"
#include "transactions/apcm_sta_cap.h"
#include "transactions/apcm_nw_stats.h"
#include "transactions/apcm_nw_info.h"
#include "transactions/apfw_factory_reset.h"
#include "transactions/apcm_sta_restart.h"
#include "transactions/apfw_hdr_cfg_param.h"
#include "transactions/apfw_upgrade.h"
#include "transactions/apfw_set_param.h"
#include "transactions/apfw_get_param.h"
#include "transactions/apcm_authorize.h"
#include "transactions/apcm_set_key.h"
#include "transactions/apcm_uke_cmd.h"
#include "transactions/apfw_get_debug.h"
#include "transactions/apfw_tr069_get_stats.h"
#include "transactions/apfw_tr069_get_tone_info.h"
#include "transactions/apfw_tr069_enable_plc.h"
#include "transactions/apfw_1905_fob_cmd.h"
#include "transactions/apcm_stdby_cmd.h"
#include "transactions/apchannel_monitor.h"
#include "transactions/apfw_loopback_traffic.h"

/***************************************************
 *                 Public Defines Section
 ***************************************************/

/** \brief Versioning information
 *
 * Major: API changes that could cause compatibility problems for older
 * programs such as structure size changes.  No binary compatibility is
 * possible across a change in the major version.
 *
 * Minor: API changes that do not cause binary compatibility problems.
 * Reset to 0 when upgrading Major
 *
 * Patch level: never includes API changes, simply bug fixes.
 * Reset to 0 when upgrading Minor
 *
 * SVN: revision number into the repository
 */
#define L2DRIVER_MAJOR_VERSION       2
#define L2DRIVER_MINOR_VERSION       0
#define L2DRIVER_SVN_VERSION         13400

/***************************************************
 *                 Public Constants Section
 ***************************************************/

/***************************************************
 *                 Public Typedefs Section
 ***************************************************/

/** \brief   Error info related to an upgrade process. */
typedef struct
{
   TS32 error_code;     //!< upgrade process error code
   TChar error_msg[80]; //!< upgrade process error message
} tS_UpgradeErrorInfo;

/** \brief   Upgrade type */
typedef enum
{
   UPG_FIRMWARE,        //!< firmware upgrade
   UPG_PARAM_CONFIG,    //!< configuration parameters upgrade
   UPG_BOOT_HEADER,     //!< boot header upgrade
   UPG_MICROCODE        //!< microcode
} tE_UpgradeType;

/** brief   Discovered PLC device associated data */
typedef struct
{
  unsigned char MACAddress[6];  //!< MAC Address
  char          HFID[65];       //!< HFID 
  unsigned char MMV;            //!< L2 Protocol Version supported  
} tS_DiscoveredPLCDevice;

/***************************************************
 *         Public Function Prototypes Section
 ***************************************************/

/** \brief     Initialize API.
 *
 * \param  ifname   (in) interface name (like "eth0")
 */
void l2driver_ini(TChar* ifname);

/** \brief     Close network device and deallocates resources */
void l2driver_close();

/**
 * \brief                  Set a network device ready to communicate with a STA
 *                         via L2 Config Protocol
 *
 * \param  STA             (in) MAC address for the STA to manage
 * \param  dpw             (in) Device password
 *
 * \return TBool           TRUE if the operation was successful, otherwise is
 *                         FALSE
 *
 * \note                   MAC format with ':' i.e 00:1d:60:88:5b:46
 */
TBool l2driver_open(TChar* STA, TChar* dpw);

/**
 * \brief                  Get the MAC address for a given device
 *
 * \param  p_device_name   (in)  device name (like "eth0")
 * \param  MAC_size        (in)  buffer size
 * \param  p_MAC           (out) MAC address as an array of bytes
 *
 * \return TBool           TRUE if the operation was successful, otherwise is
 *                         FALSE
 *
 * \note                   buffer size must be at minimum 6 bytes!
 */
TBool l2driver_getMAC(TChar* p_device_name, TU8 MAC_size, TU8* p_MAC);

/**
 * \brief                  Get the IP address for a given device
 *
 * \param  p_device_name   (in) device name (like "eth0")
 *
 * \return TChar*          IP addr if the operation was successful, otherwise is
 *						         NULL
 */
TChar* l2driver_getIP(TChar* p_device_name);

/**
 * \brief               Execute an upgrade (for the firmware image, param config
 *                      image, boot header or dsp microcodes)
 *
 * \param  p_path       (in) point to the path where the file is located
 * \param  type         (in) upgrade type
 * \param  p_err_info   (out) point to the struct holding the information
 *                            related to an error if it exists
 *
 * \return TBool        TRUE if the operation was successful, otherwise is FALSE
*/
TBool l2driver_upgrade(
   TChar* p_path,
   tE_UpgradeType type,
   tS_UpgradeErrorInfo* p_err_info);

/**
 * \brief         Generate a network key from a network password
 *
 * \note          The length of a NPW shall be between 8 and 64 characters
 *                inclusive
 *
 * \param  p_NPW  (in) point to the network password
 * \param  p_NMK  (out) point to the generated key
 *
 * \return TBool  FALSE if the generation fails, otherwise is TRUE
 *
 * \note          The size of NMK is 16 bytes
*/
TBool l2driver_GenerateNMK(TChar* p_NPW, TU8* p_NMK);

/**
 * \brief         Generate a dak key from a device password
 *
 * \note          The length of a DPW shall be between 16 and 64 characters
 *                inclusive
 *
 * \param  p_DPW  (in) point to the DPW (device password)
 * \param  p_DAK  (out) poin to the generated key
 *
 * \return TBool  FALSE if the generation fails, otherwise return TRUE
*/
TBool l2driver_GenerateDAK(TChar* p_DPW, TU8* p_DAK);

/**
 * \brief         Generate a NID from a NMK and security level
 *
 * \param p_NMK   (in) Network key
 * \param SL      (in) Security level (0x00 SIMPLE_CONNECT, 0x01 SECURE)
 * \param p_NID   (out) Network id
 *
 * \note          The size of NMK must be 16 bytes
 * \note          The size of NID is 7 bytes
*/
void l2driver_GenerateNID(TU8* p_NMK, TU8 SL, TU8* p_NID);

/** 
 * \brief         Set MMV for the outgoing L2 packets 
*/
void l2driver_set_mmv(TU8 mmv);

/**
 * \brief Discover the PLC device attached to the eth interface passed by arg
 * 
 * \param iface (in)  ethernet interface (like "eth0")
 * \param msecs (in)  number of secs the function will block discovering 
 *                    PLC devices
 * \param discovered_device (out) pointer to the struct that will save data 
 *                                associated to the discovered device
 *
 * \return int   1 if a local PLC device was discovered. Otherwise returns 0.
 * 
*/
int l2driver_discoverLocalPLCDevice(
  const char* iface, 
  int secs, 
  tS_DiscoveredPLCDevice* discovered_device);

/**
 * \brief Discover remote PLC devices attached to the eth interface passed by arg
 * 
 * \param iface (in)  ethernet interface (like "eth0")
 * \param msecs (in)  number of secs the function will block discovering 
 *                    PLC devices
 * \param discovered_device (out) pointer to the struct that will save data 
 *                                associated to the discovered devices
 *
 * \return int   Number of discovered devices.
 * 
*/
int l2driver_discoverRemotePLCDevice(
  const char* iface, 
  int secs, 
  tS_DiscoveredPLCDevice* discovered_devices);

#endif // L2_DRIVER_H_

