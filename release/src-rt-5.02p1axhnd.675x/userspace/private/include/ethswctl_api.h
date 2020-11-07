/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:proprietary:standard

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
 *
 ************************************************************************/

#ifndef _ETHSWCTL_API_H_
#define _ETHSWCTL_API_H_

#include <bcmnet.h>
#include <bcm/bcmswapitypes.h>
#include <bcm/bcmswapistat.h>

#ifdef ETHSWCTL_DEBUG
#define DBG( body ) do { body } while(0);
#else
//#define DBG( body ) do { body } while(0);
#define DBG( body )
#endif

/* define used to identify communication with external chip configured as SPI slave device */
#define BCM_EXT_6829 0x80

#if defined(CHIP_63138) || defined(CHIP_63148) || defined(CHIP_4908) || defined(CHIP_63158) || defined(CHIP_63178)
 #define HAS_SF2     1
 #if !defined(CHIP_63178)
  #define HAS_SF2_CFP 1
 #endif
#endif

#if defined(CHIP_47622)
 #define HAS_SF2     1
#endif

#if defined(CHIP_63178) || defined(CHIP_47622)
 #define HAS_ARCHER     1
#else
 #define HAS_RUNNER     1
#endif

/****************************************************************************/
/* Enable Hardware Switching                                                */
/* Returns: 0 on success; a negative value on failure                       */
/****************************************************************************/
int ethswctl_enable_switching(void);

/****************************************************************************/
/* Disable Hardware Switching                                               */
/* Returns: 0 on success; a negative value on failure                       */
/****************************************************************************/
int ethswctl_disable_switching(void);

/****************************************************************************/
/* Get Hardware Switching Enable/Disable Status                             */
/* Returns status(0=Disable;1=Enable) on success; negative value on failure */
/****************************************************************************/
int ethswctl_get_switching_status(int *status);

/****************************************************************************/
/* dumps the registers of a given page                                      */
/* Returns: 0 on success; a negative value on failure                       */
/****************************************************************************/
int ethswctl_pagedump(int unit, int page);

/****************************************************************************/
/* dumps the mib counters of a given port                                   */
/* type = 0 to dump a subset of MIB counters. type = 1 to dump all counters */
/* unit = 0 Internal Switch. unit = 1 External switch */
/* Returns: 0 on success; a negative value on failure                       */
/****************************************************************************/
#define ethswctl_mibdump(port, unit, type) ethswctl_mibdump_x(unit, port, -1, type)
int ethswctl_mibdump_x(int unit, int port, int priority, int type);
int ethswctl_mibdump_us(int unit, int port, char *data); // add by Andrew

/****************************************************************************/
/* dump/set the enet rx & tx iuDMA info for channels controlled by Host MIPS*/
/* channel: Logical Channel Number; -1 means all channels                   */
/* mode: Channel type; 1: Read; 2: Write; 3: R/W channels                   */
/* Enable: 1: Enable; 0: Disable; -1: Just Read the channel                 */
/* Returns: 0 on success; a negative value on failure                       */
/****************************************************************************/
int bcm_iudma_op(int channel, int mode, int all, int enable, int descCnt, int byteCnt);

/****************************************************************************/
/* gets or sets the iudma channel of a given ethernet port                  */
/* iudma = -1 is the get operation                                          */
/* Returns: 0 on success; a negative value on failure                       */
/****************************************************************************/
int bcm_iudma_split(int port, int iudma);

/****************************************************************************/
/*  Switch Control API for Buffer Management                                                                   */
/****************************************************************************/

/*
 * Function:
 *  bcm_switch_control_set
 * Description:
 *  Set Flow Control Drop/Pause Control mechanisms of the switch.
 *  unit - RoboSwitch PCI device unit number (driver internal).
 *    type - The desired configuration parameter to modify.
 *    arg - The value with which to set the parameter.
 * Returns:
 *    BCM_E_xxxx
 */
int bcm_switch_control_set(int unit, bcm_switch_control_t type, int arg);
int bcm_switch_control_setX(int unit, bcm_switch_control_t type,
                                  bcm_switch_fc_t sub_type, int arg);

/***************************************************************************
 * Function:
 * int ethswctl_quemap_call(int unit, int port, int que, int type);
 * Description:
 *  Set/Get WAN/LAN Queue bit map
 *  port - Port number to be monitored.
 * Returns:
 *    BCM_E_xxxx
***************************************************************************/
int ethswctl_quemap_call(int unit, int *val, int *queRemap, int set);

/****************************************************************************/
/*  Switch Control API for Flow Control Monitoring                          */
/****************************************************************************/
/*
 * Function:
 * int ethswctl_quemon_get(int unit, int port, int que, int type, int *val);
 * Description:
 *  Set Flow Control Drop/Pause Control mechanisms of the switch.
 *  Unit - Unit number to be monitored.
 *  port - Port number to be monitored.
 *  type - Flow control type to be monitored.
 *  data - Flow data 
 * Returns:
 *    BCM_E_xxxx
 */
int ethswctl_quemon_get(int unit, int port, int que, int type, int *val);

/*
 * Function:
 *  bcm_switch_control_get
 * Description:
 *  Get Flow Control Drop/Pause Control mechanisms of the switch.
 * Parameters:
 *  unit - RoboSwitch PCI device unit number (driver internal).
 *    type - The desired configuration parameter to retrieve.
 *    arg - Pointer to where the retrieved value will be written.
 * Returns:
 *    BCM_E_xxxx
 */
int bcm_switch_control_get(int unit, bcm_switch_control_t type, int *arg);


/*
 * Function:
 *  bcm_switch_control_priority_set
 * Description:
 *  Set switch parameters on a per-priority (cos) basis.
 * Parameters:
 *  unit - Device unit number
 *  priority - The priority to affect
 *  type - The desired configuration parameter to modify
 *  arg - The value with which to set the parameter
 * Returns:
 *  BCM_E_xxx
 */
int bcm_switch_control_priority_set(int unit, bcm_cos_t priority,
                bcm_switch_control_t type, int arg);

int bcm_switch_control_priority_setX(int unit, int port, bcm_cos_t priority,
                bcm_switch_control_t type, int arg);

/*
 * Function:
 *  bcm_switch_control_priority_get
 * Description:
 *  Get switch parameters on a per-priority (cos) basis.
 * Parameters:
 *  unit - Device unit number
 *  priority - The priority to affect
 *  type - The desired configuration parameter to retrieve
 *  arg - Pointer to where the retrieved value will be written
 * Returns:
 *  BCM_E_xxx
 */
int bcm_switch_control_priority_get(int unit, bcm_cos_t priority,
                bcm_switch_control_t type, int *arg);
int bcm_switch_control_priority_getX(int unit, int port, bcm_cos_t priority,
                bcm_switch_control_t type, int *arg);


/*
 * Function:
 *  bcm_acb_cfg_set
 * Description:
 *  Set switch acb parameters  - gegenric or per queue
 * Parameters:
 *  unit - Device unit number
 *  queue - The target queue to affect
 *  type - The desired acb configuration parameter to modify
 *  arg - The value with which to set the parameter
 * Returns:
 *  BCM_E_xxx
 */
int bcm_acb_cfg_set(int unit, int queue, bcm_switch_acb_control_t type, int arg);

/*
 * Function:
 *  bcm_acb_cfg_get
 * Description:
 *  Get switch acb parameters  - gegenric or per queue
 * Parameters:
 *  unit - Device unit number
 *  queue - The target queue to affect
 *  type - The desired acb configuration parameter to retrieve
 *  arg - Pointer to where the retrieved value will be written
 * Returns:
 *  BCM_E_xxx
 */
int bcm_acb_cfg_get(int unit, int queue, bcm_switch_acb_control_t type, void *arg);

/****************************************************************************/
/* Port Configuration API: For Configuring Egress Tag Replacement Registers */
/****************************************************************************/

/* Set the replacement VLAN tag */
int bcm_port_replace_tag_set(int unit, bcm_port_t port, unsigned int vlan_tag);

/* Retrieve the replacement VLAN tag */
int bcm_port_replace_tag_get(int unit, bcm_port_t port, unsigned int *vlan_tag);

/* Set the replacement TPID */
int bcm_port_replace_tpid_set(int unit, bcm_port_t port, unsigned short tpid);

/* Retrieve the replacement TPID */
int bcm_port_replace_tpid_get(int unit, bcm_port_t port, unsigned short *tpid);

/* Set the replacement TCI (802.1p+CFI+VID) */
int bcm_port_replace_tci_set(int unit, bcm_port_t port, bcm_vlan_t vid);

/* Retrieve the replacement TCI (802.1p+CFI+VID) */
int bcm_port_replace_tci_get(int unit, bcm_port_t port, bcm_vlan_t *vid);

/* Set the replacement VLAN ID */
int bcm_port_replace_vid_set(int unit, bcm_port_t port, bcm_vlan_t vid);

/* Retrieve the replacement VLAN ID */
int bcm_port_replace_vid_get(int unit,  bcm_port_t port, bcm_vlan_t *vid);

/* Set the replacement 802.1p bits */
int bcm_port_replace_8021p_set(int unit, bcm_port_t port, char priority);

/* Retrieve the replacement 802.1p bits */
int bcm_port_replace_8021p_get(int unit, bcm_port_t port, char *priority);

/* Set the replacement CFI bit */
int bcm_port_replace_cfi_set(int unit, bcm_port_t port,	char cfi);

/* Retrieve the replacement CFI bit */
int bcm_port_replace_cfi_get(int unit, bcm_port_t port, char *cfi);

/* Set the egress tag mangling operations for a port */
int bcm_port_tag_mangle_set(int unit,	bcm_port_t port, unsigned short op_map);


/* Get the egress tag mangling operations configured for a port */
int bcm_port_tag_mangle_get(int unit, bcm_port_t port, unsigned short *op_map);

/* Set the match_vid used for egress tag mangling */
int bcm_port_tag_mangle_match_vid_set(int unit, bcm_port_t port,
                                               bcm_vlan_t vid);

/* Retrieve the match_vid used for egress tag mangling */
int bcm_port_tag_mangle_match_vid_get(int unit, bcm_port_t port,
                                               bcm_vlan_t *vid);

/* Set the tag stripping of a port for packets with match_vid */
int bcm_port_tag_strip_set(int unit, bcm_port_t port, char val);

/* Get the tag stripping status of a port for packets with match_vid */
int bcm_port_tag_strip_get(int unit, bcm_port_t port, char *val);

int bcm_port_maclmt_set(int unit,bcm_port_t port,int type, int *val);
int bcm_port_maclmt_get(int unit,bcm_port_t port,int type, int *val);

/*
 * Function:
 *  bcm_port_pause_capability_set
 * Description:
 *  Enable/Disable the Tx and Rx Pause
 * Parameters:
 *  val - NONE=0, AUTO=1, BOTH=2, TX=3, RX=4
 * Returns:
 *  BCM_E_xxx
 */
int bcm_port_pause_capability_set(int unit, bcm_port_t port, char val);

/*
 * Function:
 *  bcm_port_pause_capability_get
 * Description:
 * Get Pause Capability
 * Parameters:
 *  val - NONE=0, AUTO=1, BOTH=2, TX=3, RX=4, PFC_BOTH=5, PFC_TX=6, PFC_RX=7
 * Returns:
 *  BCM_E_xxx
 */
int bcm_port_pause_capability_get2(int unit, bcm_port_t port, int *val, int *val2);

/* For backword compatible with non PFC versoin */
static inline int bcm_port_pause_capability_get(int unit, bcm_port_t port, char *val)
{
    int _val, rc;
    rc = bcm_port_pause_capability_get2(unit, port, &_val, 0);
    *val = _val;
    return rc;
}

/*
 * Function:
 *  bcm_port_rate_ingress_set
 * Purpose:
 *  Set ingress rate limiting parameters
 * Parameters:
 *  unit - Device number
 *  port - Port number
 *  kbits_sec - Rate in kilobits (1000 bits) per second.
 *            Rate of 0 disables rate limiting.
 *  kbits_burst - Maximum burst size in kilobits (1000 bits).
 * Returns:
 *  BCM_E_XXX
 * Note :
 *  1. Robo Switch support 2 ingress buckets for different packet type.
 *     And the bucket1 contains higher priority if PKT_MSK confilict
 *       with bucket0's PKT_MSK.
 *  2. Robo Switch allowed system basis rate/packet type assignment for
 *     Rate Control. The RATE_TYPE and PKT_MSK will be set once in the
 *       initial routine.
 */
int bcm_port_rate_ingress_set(int unit,
                  bcm_port_t port,
                  unsigned int kbits_sec,
                  unsigned int kbits_burst);

/*
 * Function:
 *  bcm_port_rate_ingress_get
 * Purpose:
 *  Get ingress rate limiting parameters
 * Parameters:
 *  unit - Device number
 *  port - Port number
 *  kbits_sec - (OUT) Rate in kilobits (1000 bits) per second, or
 *                  zero if rate limiting is disabled.
 *  kbits_burst - (OUT) Maximum burst size in kilobits (1000 bits).
 * Returns:
 *  BCM_E_XXX
 */
int bcm_port_rate_ingress_get(int unit,
                  bcm_port_t port,
                  unsigned int *kbits_sec,
                  unsigned int *kbits_burst);

/*
 * Function:
 *  bcm_port_rate_egress_set
 * Purpose:
 *  Set egress rate limiting parameters
 * Parameters:
 *  unit - Device number
 *  port - Port number
 *  kbits_sec - Rate in kilobits (1000 bits) per second.
 *          Rate of 0 disables rate limiting.
 *  kbits_burst - Maximum burst size in kilobits (1000 bits).
 * Returns:
 *  BCM_E_XXX
 * Note :
 *  1. Robo Switch support 2 ingress buckets for different packet type.
 *     And the bucket1 contains higher priority if PKT_MSK confilict
 *       with bucket0's PKT_MSK.
 *  2. Robo Switch allowed system basis rate/packet type assignment for
 *     Rate Control. The RATE_TYPE and PKT_MSK will be set once in the
 *       initial routine.
 */
int bcm_port_rate_egress_set(int unit,
                 bcm_port_t port,
                 unsigned int kbits_sec,
                 unsigned int kbits_burst);
int bcm_port_rate_egress_set_X(int unit,
                 bcm_port_t port,
                 unsigned int erc_limit,
                 unsigned int erc_burst,
                 int queue, int is_pkt_mode);


int bcm_port_shaper_cfg(int unit,
                 bcm_port_t port,
                 int queue,
                 int shaper_cfg_flags,
                 int val);

int bcm_enet_map_ifname_to_unit_port(const char *ifName,
                 int *unit,
                 bcm_port_t *port);

int bcm_enet_map_ifname_to_unit_portmap(const char *ifName,
                 int *unit,
                 unsigned int *portmap);

int bcm_enet_map_oam_idx_to_unit_port(int oamIdx,
                 int *unit,
                 bcm_port_t *port);

int bcm_enet_map_oam_idx_to_rdpaif(int oamIdx);
int bcm_enet_map_oam_idx_to_phys_port(int oamIdx);
int bcm_enet_map_unit_port_to_oam_idx(int unit, bcm_port_t port);

/*
 * Function:
 *  bcm_port_rate_egress_get
 * Purpose:
 *  Get egress rate limiting parameters
 * Parameters:
 *  unit - Device number
 *  port - Port number
 *  kbits_sec - (OUT) Rate in kilobits (1000 bits) per second, or
 *              zero if rate limiting is disabled.
 *  kbits_burst - (OUT) Maximum burst size in kilobits (1000 bits).
 * Returns:
 *  BCM_E_XXX
 */
int bcm_port_rate_egress_get(int unit,
                 bcm_port_t port,
                 unsigned int *kbits_sec,
                 unsigned int *kbits_burst);
int bcm_port_rate_egress_get_X(int unit,
                 bcm_port_t port,
                 unsigned int *kbits_sec,
                 unsigned int *kbits_burst,
                 int queue,
                 void *vptr);

/* Set the 802.1p bits of port default tag */
int bcm_port_untagged_priority_set(int unit, bcm_port_t port, int priority);

/* Retrieve the 802.1p bits from port default tag */
int bcm_port_untagged_priority_get(int unit, bcm_port_t port, int *priority);

/*set sa / da lookup per port*/
int bcm_port_learning_ind_set(int unit, bcm_port_t port, unsigned char learningInd);
/*set max frame size*/
int bcm_mtu_set(unsigned int mtu);

/*set transparent per port*/
int bcm_port_transparent_set(int unit, bcm_port_t port, unsigned char enable);

/*set vlan isolation per port*/
int bcm_port_vlan_isolation_set(int unit, bcm_port_t port, unsigned char us_enable, unsigned char ds_enable);

/*set broadcast rate limit per port*/
int bcm_port_bc_rate_limit_set(int unit, bcm_port_t port, unsigned int rate);

/****************************************************************************/
/*Enet Driver Config/Control API: For Configuring Enet Driver Rx Scheduling */
/****************************************************************************/

/*
 * Function:
 *  bcm_robo_enet_driver_rx_scheduling_set
 * Description:
 *  Select the enet driver rx scheduling mechanism
 * Parameters:
 *  unit - Device unit number
 *  scheduling - scheduling mechanism selection
 * Returns:
 *  BCM_E_xxx
 */
int bcm_enet_driver_rx_scheduling_set(int unit, int scheduling);

/*
 * Function:
 *  bcm_robo_enet_driver_rx_scheduling_get
 * Description:
 *  Get the enet driver rx scheduling mechanism
 * Parameters:
 *  unit - Device unit number
 *  scheduling - scheduling mechanism
 * Returns:
 *  BCM_E_xxx
 */
int bcm_enet_driver_rx_scheduling_get(int unit, int *scheduling);


/*
 * Function:
 *  bcm_robo_enet_driver_wrr_config_set
 * Description:
 *  Configure the WRR parameters
 * Parameters:
 *  unit - Device unit number
 *  max_pkts_per_iter - Max number of packets when the weights will be reloaded.
 *  weights - Pointer to integer array of WRR weights of all 4 channels.
 * Returns:
 *  BCM_E_xxx
 */
int bcm_enet_driver_wrr_weights_set(int unit, int max_pkts_per_iter,
                                                 int *weights);

/*
 * Function:
 *  bcm_robo_enet_driver_wrr_config_get
 * Description:
 *  Retrieve the WRR parameters
 * Parameters:
 *  unit - Device unit number
 *  max_pkts_per_iter - Max number of packets when the weights will be reloaded.
 *  weights - Pointer to integer array of WRR weights of all 4 channels.
 * Returns:
 *  BCM_E_xxx
 */
int bcm_enet_driver_wrr_weights_get2(int unit, int *max_pkts_per_iter, 
    int *weights, int *weight_pkts, int *rx_queues);
#define bcm_enet_driver_wrr_weights_get(unit, max_pkts_per_iter, weights) \
    bcm_enet_driver_wrr_weights_get2(unit, max_pkts_per_iter, weights, NULL, NULL);


/* Get enable/disable status of using default queue as egress queue */
int bcm_enet_driver_use_default_txq_status_get(int unit, char *ifname,
                                               int *operation);

/* Set enable/disable status of using default queue as egress queue */
int bcm_enet_driver_use_default_txq_status_set(int unit, char *ifname,
                                               int operation);

/* Get default the egress queue of given interface */
int bcm_enet_driver_default_txq_get(int unit, char *ifname,
                                    bcm_cos_queue_t *cosq);

/* Set default the egress queue of given interface */
int bcm_enet_driver_default_txq_set(int unit, char *ifname,
                                    bcm_cos_queue_t cosq);


/*
 * Function:
 *  bcm_enet_driver_rx_rate_limit_cfg_get
 * Description:
 *  Get enable/disable status of rx rate limiting of given channel
 * Parameters:
 *  unit - Device unit number
 *  channel - iuDMA channel # (rx iuDMA channel from MIPS perspective)
 *  status - rate limiting enable/disable status
 * Returns:
 *  BCM_E_xxx
 */
int bcm_enet_driver_rx_rate_limit_cfg_get(int unit, int channel, int *status);

/*
 * Function:
 *  bcm_enet_driver_rx_rate_limit_cfg_set
 * Description:
 *  Set enable/disable status of rx rate limiting of given channel
 * Parameters:
 *  unit - Device unit number
 *  channel - iuDMA channel # (rx iuDMA channel from MIPS perspective)
 *  status - rate limiting enable/disable status
 * Returns:
 *  BCM_E_xxx
 */
int bcm_enet_driver_rx_rate_limit_cfg_set(int unit, int channel, int status);

/*
 * Function:
 *  bcm_enet_driver_rx_rate_get
 * Description:
 *  Get rx byte rate of a given channel
 * Parameters:
 *  unit - Device unit number
 *  channel - iuDMA channel # (rx iuDMA channel from MIPS perspective)
 *  rate - rx byte rate of the channel in bytes per second
 * Returns:
 *  BCM_E_xxx
 */
int bcm_enet_driver_rx_rate_get(int unit, int channel, int *rate);

/*
 * Function:
 *  bcm_enet_driver_rx_rate_set
 * Description:
 *  Set rx byte rate of a given channel
 * Parameters:
 *  unit - Device unit number
 *  channel - iuDMA channel # (rx iuDMA channel from MIPS perspective)
 *  rate - rx byte rate of the channel in bytes per second
 * Returns:
 *  BCM_E_xxx
 */
int bcm_enet_driver_rx_rate_set(int unit, int channel, int rate);

/*
 * Function:
 *  bcm_enet_driver_rx_pkt_rate_cfg_get
 * Description:
 *  Get enable/disable status of rx pkt rate limiting of given channel
 * Parameters:
 *  unit - Device unit number
 *  channel - iuDMA channel # (rx iuDMA channel from MIPS perspective)
 *  status - rate limiting enable/disable status
 * Returns:
 *  BCM_E_xxx
 */
int bcm_enet_driver_rx_pkt_rate_cfg_get(int unit, int channel, int *status);

/*
 * Function:
 *  bcm_enet_driver_rx_pkt_rate_cfg_set
 * Description:
 *  Set enable/disable status of rx pkt rate limiting of given channel
 * Parameters:
 *  unit - Device unit number
 *  channel - iuDMA channel # (rx iuDMA channel from MIPS perspective)
 *  status - rate limiting enable/disable status
 * Returns:
 *  BCM_E_xxx
 */
int bcm_enet_driver_rx_pkt_rate_cfg_set(int unit, int channel, int status);


/*
 * Function:
 *  bcm_enet_driver_rx_pkt_rate_get
 * Description:
 *  Get rx pkt rate of a given channel
 * Parameters:
 *  unit - Device unit number
 *  channel - iuDMA channel # (rx iuDMA channel from MIPS perspective)
 *  rate - rx packet rate of the channel in packets per second
 * Returns:
 *  BCM_E_xxx
 */
int bcm_enet_driver_rx_pkt_rate_get(int unit, int channel, int *rate);

/*
 * Function:
 *  bcm_enet_driver_rx_pkt_rate_set
 * Description:
 *  Set rx pkt rate of a given channel
 * Parameters:
 *  unit - Device unit number
 *  channel - iuDMA channel # (rx iuDMA channel from MIPS perspective)
 *  rate - rx packet rate of the channel in packets per second
 * Returns:
 *  BCM_E_xxx
 */
int bcm_enet_driver_rx_pkt_rate_set(int unit, int channel, int rate);

/*
 * Function:
 *  bcm_enet_driver_enable_soft_switching_port
 * Description:
 *  Enable or disable soft switching on given port
 * Parameters:
 *  bEnable - 0 - disable, 1 - enable
 *  ifName - device name
 * Returns:
 *  BCM_E_xxx
 */
int bcm_enet_driver_enable_soft_switching_port(int bEnable, char *ifName);

/*
 * Function:
 *  bcm_enet_driver_get_soft_switching_status
 * Description:
 *  Retrieve current soft switching setting
 * Parameters:
 * portmap - bit map of ports
 * Returns:
 *  BCM_E_xxx
 */
int bcm_enet_driver_get_soft_switching_status(unsigned int *portmap);

/*
 * Function:
 *  bcm_enet_driver_hw_stp_set
 * Description:
 *  Enable or disable hardware STP support
 * Parameters:
 *  bEnable - 0 - disable, 1 - enable
 *  ifname - device name
 * Returns:
 *  BCM_E_xxx
 */
int bcm_enet_driver_hw_stp_set(int bEnable, char * ifName);

/*
 * Function:
 *  bcm_enet_driver_hw_stp_status
 * Description:
 *  Retrieve current setting for hardware STP
 * Parameters:
 * portmap - bit map of ports
 * Returns:
 *  BCM_E_xxx
 */
int bcm_enet_driver_get_hw_stp_status(unsigned int *portmap);

/*
 * Function:
 *  bcm_enet_driver_if_stp_set
 * Description:
 *  Set interface STP state - interface can be physical or virtual (VLAN)
 * Parameters:
 *  0-disabled 1-listening 2-learning 3-forwarding 4-blocking
 *  ifname - device name
 * Returns:
 *  BCM_E_xxx
 */
int bcm_enet_driver_if_stp_set(char *ifName, int state);

/*
 * Function:
 *  bcm_enet_driver_if_stp_get
 * Description:
 *  Retrieve current STP state for interface
 * Parameters:
 *  ifname - device name
 *  state - 0-disabled 1-listening 2-learning 3-forwarding 4-blocking
 * Returns:
 *  BCM_E_xxx
 */
int bcm_enet_driver_if_stp_get(char * ifName, unsigned int *state);

/* For testing and debugging */
int bcm_enet_driver_test_config_get(int unit, int type, int param, int *val);
/* For testing and debugging */
int bcm_enet_driver_test_config_set(int unit, int type, int param, int val);

/****************************************************************************/
/*  VLAN API:  For set/get of VLAN Table Entry                              */
/****************************************************************************/

/*
 * Function:
 *  bcm_vlan_port_set
 * Description:
 *  Write the given VLAN table entry to hardware
 * Parameters:
 *  unit - Device unit number
 *  vid -  VLAN ID
 *  fwd_map - Members of the VLAN
 *  untag_map - Untagged members of the VLAN
 * Returns:
 *  BCM_E_xxx
 */
int bcm_vlan_port_set(int unit, int vid, int fwd_map, int untag_map);

/*
 * Function:
 *  bcm_vlan_port_get
 * Description:
 *  Retrieve the forward_map and untag_map of the given VLAN
 * Parameters:
 *  unit - Device unit number
 *  vid -  VLAN ID
 *  fwd_map - Members of the VLAN
 *  untag_map - Untagged members of the VLAN
 * Returns:
 *  BCM_E_xxx
 */
int bcm_vlan_port_get(int unit, int vid, int * fwd_map, int *untag_map);



int bcm_port_pbvlanmap_set(int unit, int port, int fwd_map);
int bcm_port_pbvlanmap_get(int unit, int port, int *fwd_map);


/****************************************************************************/
/*  CoS API:  For CoS configuration                                         */
/****************************************************************************/

/*
 * Function:
 *  bcm_cosq_config_get
 * Description:
 *  Retrieve the Number of CoS queues
 * Parameters:
 *  unit - Device unit number
 *  numq - Number of CoS queues
 * Returns:
 *  BCM_E_xxx
 */
int bcm_cosq_config_get(int unit, int *numq);

/*
 * Function:
 *  bcm_cosq_config_set
 * Description:
 *  Set the number of CoS queues
 * Parameters:
 *  unit - Device unit number
 *  numq - Number of CoS queues. For BCM6816, valid values are 1 and 8.
 * Returns:
 *  BCM_E_xxx
 */
int bcm_cosq_config_set(int unit, int numq);

/*
 * Function:
 *  bcm_cosq_sched_get
 * Description:
 *  Retrieve the scheduling policy and its parameters
 * Parameters:
 *  unit - Device unit number
 *  mode -  Scheduling policy
 *  sp_endq - The queue where SP ends. Valid only when mode is COMBO (SP + WRR)
 *     All queues lower than sp_endq are serviced in WRR and remaining queues
 *     are serviced in SP. Note that SP starts with highest queue.
 *  weights - queue weights for WRR scheduling. Valid only when mode is WRR
 * Returns:
 *  BCM_E_xxx
 */
typedef struct port_qos_sched_s port_qos_sched_t;

int bcm_cosq_sched_get(int unit, int *mode, int *sp_endq,
                        int weights[BCM_COS_COUNT]);
// new interface to be used by both SF2 and legacy switches
/*
 *    We used a new struct port_qos_sched_s for passing parameters that 
 *    integrates [first] four config params in GET/SET ops and
 *    next three port qos capabilities in GET.
 *    struct port_qos_sched_s
 *      {
 *          unsigned short sched_mode;        // configured val  -- SP/WRR
 *          unsigned short num_spq;           // configured SP q value in Combo mode
 *          unsigned short wrr_type;          // WRR or WDR?
 *          unsigned short weights_upper;     // when setting, upper/lower 4 weights -- useful for CLI
 *          unsigned short max_egress_spq;    // CAP -  max SP q in Combo mode
 *          unsigned short max_egress_q;      // CAP - per port max queues supported
 *          unsigned int port_qos_caps;       // CAP - scheduling/shaping capabilities
 */
int bcm_cosq_sched_get_X(int unit, int port, int weights[BCM_COS_COUNT],
                        port_qos_sched_t *qs); 

/*
 * Function:
 *  bcm_cosq_sched_set
 * Description:
 *  Set the scheduling policy and its parameters
 * Parameters:
 *  unit - Device unit number
 *  mode -  Scheduling policy
 *  sp_endq - The queue where SP ends. Valid only when mode is COMBO (SP + WRR)
 *     All queues lower than sp_endq are serviced in WRR and remaining queues
 *     are serviced in SP. Note that SP starts with highest queue.
 *  weights - queue weights for WRR scheduling. Valid only when mode is WRR
 * Returns:
 *  BCM_E_xxx
 */
int bcm_cosq_sched_set(int unit, int mode, int sp_endq,
                        int weights[BCM_COS_COUNT]);
/*
 *    We used a new struct port_qos_sched_s for passing parameters that 
 *    integrates [first] four config params in GET/SET ops and
 *    next three port qos capabilities in GET.
 *    struct port_qos_sched_s
 *      {
 *          unsigned short sched_mode;        // configured val  -- SP/WRR
 *          unsigned short num_spq;           // configured SP q value in Combo mode
 *          unsigned short wrr_type;          // WRR or WDR?
 *          unsigned short weights_upper;     // when setting, upper/lower 4 weights -- useful for CLI
 *          unsigned short max_egress_spq;    // CAP -  max SP q in Combo mode
 *          unsigned short max_egress_q;      // CAP - per port max queues supported
 *          unsigned int port_qos_caps;       // CAP - scheduling/shaping capabilities
 */
int bcm_cosq_sched_set_X(int unit, int port, 
                        int weights[BCM_COS_COUNT], port_qos_sched_t *qs);

/*
 * Function:
 *  bcm_cosq_port_mapping_get
 * Description:
 *  Retrieve the internal priority to CoS queue mapping
 * Parameters:
 *  unit - Device unit number
 *  priority - Internal Priority
 *    port - Port number
 *  cosq - CoS queue
 * Returns:
 *  BCM_E_xxx
 */
int bcm_cosq_port_mapping_get(int unit, bcm_port_t port, bcm_cos_t priority, bcm_cos_queue_t *cosq);

/*
 * Function:
 *  bcm_cosq_port_mapping_set
 * Description:
 *  Set the internal priority to CoS queue mapping
 * Parameters:
 *  unit - Device unit number
 *  priority - Internal Priority
 *    port - Port number
 *  cosq - CoS queue
 * Returns:
 *  BCM_E_xxx
 */
int bcm_cosq_port_mapping_set(int unit, bcm_port_t port, bcm_cos_t priority,
                              bcm_cos_queue_t cosq);


/*
 * Function:
 *  bcm_cosq_rxchannel_mapping_get
 * Description:
 *  Retrieve the CoS queue to Rx iuDMA channel mapping
 * Parameters:
 *  unit - Device unit number
 *  cosq - Egress queue #
 *  channel - iuDMA channel (rx iuDMA channel from MIPS perspective)
 * Returns:
 *  BCM_E_xxx
 */
int bcm_cosq_rxchannel_mapping_get(int unit, bcm_cos_queue_t cosq,
                                   int *channel);

/*
 * Function:
 *  bcm_cosq_rxchannel_mapping_set
 * Description:
 *  Set the CoS queue to Rx iuDMA channel mapping
 * Parameters:
 *  unit - Device unit number
 *  cosq - Egress queue #
 *  channel - iuDMA channel # (rx iuDMA channel from MIPS perspective)
 * Returns:
 *  BCM_E_xxx
 */
int bcm_cosq_rxchannel_mapping_set(int unit, bcm_cos_queue_t cosq, int channel);

/*
 * Function:
 *  bcm_cosq_txchannel_mapping_get
 * Description:
 *  Retrieve the Tx iuDMA channel to egress queue mapping
 * Parameters:
 *  unit - Device unit number
 *  channel - iuDMA channel # (tx iuDMA channel from MIPS perspective)
 *  cosq - Egress queue #
 * Returns:
 *  BCM_E_xxx
 */
int bcm_cosq_txchannel_mapping_get(int unit, int channel, bcm_cos_queue_t *cosq);

/*
 * Function:
 *  bcm_cosq_txchannel_mapping_set
 * Description:
 *  Configure the Tx iuDMA channel to egress queue mapping
 * Parameters:
 *  unit - Device unit number
 *  channel - iuDMA channel # (tx iuDMA channel from MIPS perspective)
 *  cosq - Egress queue #
 * Returns:
 *  BCM_E_xxx
 */
int bcm_cosq_txchannel_mapping_set(int unit, int channel, bcm_cos_queue_t cosq);

/*
 * Function:
 *  bcm_cosq_txchannel_mapping_set
 * Description:
 *  Configure the Tx iuDMA channel to egress queue mapping
 * Parameters:
 *  unit - Device unit number
 *  channel - iuDMA channel # (tx iuDMA channel from MIPS perspective)
 *  cosq - Egress queue #
 * Returns:
 *  BCM_E_xxx
 */
int bcm_cosq_txq_selection_get(int unit, int *method);

/*
 * Function:
 *  bcm_cosq_txchannel_mapping_set
 * Description:
 *  Configure the Tx iuDMA channel to egress queue mapping
 * Parameters:
 *  unit - Device unit number
 *  channel - iuDMA channel # (tx iuDMA channel from MIPS perspective)
 *  cosq - Egress queue #
 * Returns:
 *  BCM_E_xxx
 */
int bcm_cosq_txq_selection_set(int unit, int method);

/*
 * Function:
 *  bcm_cosq_priority_method_get
 * Description:
 * Retrieve the method for deciding on frame priority
 * Parameters:
 *  unit - RoboSwitch PCI device unit number (driver internal).
 *  method -
 *  PORT_QOS: Frame priority is based on the priority of port default tag
 *  MAC_QOS: Frame priority is based on the destination MAC address
 *  IEEE8021P_QOS: Frame priority is based on 802.1p field of the frame
 *  DIFFSERV_QOS: Frame priority is based on the diffserv field of the frame
 * Returns:
 *  BCM_E_NONE - Success.
 */
// new interface to be used by both SF2 and legacy switches
int bcm_cosq_priority_method_get_X(int unit, int port, int *method, int pkt_type_mask);
int bcm_cosq_priority_method_get(int unit, int port, int *method);

/*
 * Function:
 *  bcm_cosq_priority_method_set
 * Description:
 *  Set the method for deciding on frame priority
 * Parameters:
 *  unit - RoboSwitch PCI device unit number (driver internal).
 *  method -
 *  PORT_QOS: Frame priority is based on the priority of port default tag
 *  MAC_QOS: Frame priority is based on the destination MAC address
 *  IEEE8021P_QOS: Frame priority is based on 802.1p field of the frame
 *  DIFFSERV_QOS: Frame priority is based on the diffserv field of the frame
 * Returns:
 *  BCM_E_NONE - Success.
 */
// new interface to be used by both SF2 and legacy switches
int bcm_cosq_priority_method_set_X(int unit, int port, int method, int pkt_type_mask);
int bcm_cosq_priority_method_set(int unit, int port, int method);

/*
 * Function:
 *  bcm_cosq_dscp_priority_mapping_set
 * Description:
 *  Configure DSCP to priority mapping
 * Parameters:
 *  unit - RoboSwitch PCI device unit number (driver internal).
 *  dscp:  6-bit dscp value
 *  priority:  switch priority
 * Returns:
 *  BCM_E_NONE - Success.
 */
int bcm_cosq_dscp_priority_mapping_set(int unit, int dscp, bcm_cos_t priority);

/*
 * Function:
 *  bcm_cosq_dscp_priority_mapping_get
 * Description:
 *  Get DSCP to priority mapping
 * Parameters:
 *  unit - RoboSwitch PCI device unit number (driver internal).
 *  dscp:  6-bit dscp value
 *  priority:  switch priority
 * Returns:
 *  BCM_E_NONE - Success.
 */
int bcm_cosq_dscp_priority_mapping_get(int unit, int dscp, bcm_cos_t *priority);


/*
 * Function:
 *  bcm_cosq_pcp_priority_mapping_set
 * Description:
 *  Configure PCP to priority mapping
 * Parameters:
 *  unit - RoboSwitch PCI device unit number (driver internal).
 *  port - Port number
 *  pcp:   3-bit pcp value
 *  priority:  switch priority
 * Returns:
 *  BCM_E_NONE - Success.
 */
int bcm_cosq_pcp_priority_mapping_set(int unit, int port, int dscp, bcm_cos_t priority);

/*
 * Function:
 *  bcm_cosq_pcp_priority_mapping_get
 * Description:
 *  Get PCP to priority mapping
 * Parameters:
 *  unit - RoboSwitch PCI device unit number (driver internal).
 *  port - Port number
 *  pcp:   3-bit pcp value
 *  priority:  switch priority
 * Returns:
 *  BCM_E_NONE - Success.
 */
int bcm_cosq_pcp_priority_mapping_get(int unit, int port, int dscp, bcm_cos_t *priority);

/*
 * Function:
 *  bcm_cosq_pcp_priority_mapping_get
 * Description:
 *  Get PID to priority mapping
 * Parameters:
 *  unit - RoboSwitch PCI device unit number (driver internal).
 *  priority:  switch priority
 * Returns:
 *  BCM_E_NONE - Success.
 */
int bcm_cosq_pid_priority_mapping_get(int unit, int port, bcm_cos_t *priority);
/*
 * Function:
 *  bcm_cosq_pid_priority_mapping_set
 * Description:
 *  Configure DSCP to priority mapping
 * Parameters:
 *  unit - RoboSwitch PCI device unit number (driver internal).
 *  priority:  switch priority
 * Returns:
 *  BCM_E_NONE - Success.
 */
int bcm_cosq_pid_priority_mapping_set(int unit, int port, bcm_cos_t priority);
/*
 * Function:
 *  bcm_port_traffic_control_set
 * Description:
 *  Enable/Disable tx/rx of a switch port
 * Parameters:
 *  unit - RoboSwitch PCI device unit number (driver internal).
 *  port - Port number
 *  ctrl_map: bit0 = rx_disable (1 = disable rx; 0 = enable rx)
 *            bit1 = tx_disable (1 = disable tx; 0 = enable tx)
 * Returns:
 *  BCM_E_NONE - Success.
 */
int bcm_port_traffic_control_set(int unit, bcm_port_t port, int ctrl_map);

/*
 * Function:
 *  bcm_port_traffic_control_get
 * Description:
 *  Get Enable/Disable status of tx/rx of a switch port
 * Parameters:
 *  unit - RoboSwitch PCI device unit number (driver internal).
 *  port - Port number
 *  ctrl_map: bit0 = rx_disable (1 = disable rx; 0 = enable rx)
 *            bit1 = tx_disable (1 = disable tx; 0 = enable tx)
 * Returns:
 *  BCM_E_NONE - Success.
 */
int bcm_port_traffic_control_get(int unit, bcm_port_t port, int *ctrl_map);

/*
 * Function:
 *  bcm_port_loopback_set
 * Description:
 *  Enable/Disable of loopback of USB port or LAN port Phy
 * Parameters:
 *  unit - RoboSwitch PCI device unit number (driver internal).
 *  status:  1 = Enable loopback; 0 = Disable loopback
 * Returns:
 *  BCM_E_NONE - Success.
 */
int bcm_port_subport_loopback_set(int unit, bcm_port_t port, int sub_port, int cfg_speed, int status);
#define bcm_port_loopback_set(unit, port, status) bcm_port_subport_loopback_set(unit, port, -1, 0, status)

/*
 * Function:
 *  bcm_port_loopback_get
 * Description:
 *  Get loopback status of USB port or LAN port Phy
 * Parameters:
 *  unit - RoboSwitch PCI device unit number (driver internal).
 *  status:  1 = Enable loopback; 0 = Disable loopback
 * Returns:
 *  BCM_E_NONE - Success.
 */
int bcm_port_subport_loopback_get(int unit, bcm_port_t port, int sub_port, int *cfg_speed, int *status);
#define bcm_port_loopback_get(unit, port, status) bcm_port_subport_loopback_get(unit, port, -1, 0, status)

/*
 * Function:
 *	bcm_phy_mode_set
 * Description:
 *  Set phy mode
 * Parameters:
 *    unit - RoboSwitch PCI device unit number (driver internal).
 *	port - port.
 *	speed - 0 is auto or 10, 100, 100
 *    duplex - 0: full, 1:half
 * Returns:
 *	BCM_E_NONE - Success.
 */
int bcm_phy_mode_set(int unit, int port, int speed, int duplex);

/*
 * Function:
 *	bcm_phy_mode_get
 * Description:
 *  Set phy mode
 * Parameters:
 *	port - port.
 * Returns:
 *	speed - 0 is auto or 10, 100, 100
 *    duplex - 0: full, 1:half
 *	BCM_E_NONE - Success.
 */
int bcm_phy_mode_get(int unit, int port, int *speed, int *duplex);
int bcm_phy_mode_setV(char *ifname, int  speed, int  duplex);
int bcm_phy_mode_getV(char *ifname, int *speed, int *duplex);

/*
 * Function:
 *  bcm_port_jumbo_control_set
 * Description:
 *  Set jumbo accept/reject control of selected port(s)
 * Parameters:
 * unit  - 0 internal switch, 1 external switch
 *  port - Port number 9(ALL), 8(MIPS), 7(GPON), 6(USB), 5(MOCA), 4(GPON_SERDES), 3(GMII_2), 2(GMII_1), 1(GPHY_1), 0(GPHY_0)
 *  ctrlValPtr - pointer to result
 * Returns:
 *  BCM_E_NONE - Success.
 */
int bcm_port_jumbo_control_set(int unit, bcm_port_t port, int* ctrlValPtr); // bill

/*
 * Function:
 *  bcm_port_jumbo_control_get
 * Description:
 *  Get jumbo accept/reject status of selected port(s)
 * Parameters:
 * unit  - 0 internal switch, 1 external switch
 *  port - Port number 9(ALL), 8(MIPS), 7(GPON), 6(USB), 5(MOCA), 4(GPON_SERDES), 3(GMII_2), 2(GMII_1), 1(GPHY_1), 0(GPHY_0)
 *  ctrlValPtr - pointer to result
 * Returns:
 *  BCM_E_NONE - Success.
 */
int bcm_port_jumbo_control_get(int unit, bcm_port_t port, int *ctrlValPtr); // bill

/*
    int bcm_arl_read(int unit, char *mac, bcm_vlan_t vid, unsigned short *value);
    ******* This is Deprecated API ***********
        unit: Switch Unit Number
        mac: Mac address in the reversed network byte order; mac[5][4][3][2][1][0]
        *value: Variable to Receive Read Result. Format is raw hardware register.
*/
int bcm_arl_read(int unit, char *mac, bcm_vlan_t vid, unsigned short *value);

/*
    int bcm_arl_write(int unit, char *mac, bcm_vlan_t vid, unsigned short *value);
    ******* This is Deprecated API ***********
        unit: Switch Unit Number
        mac: Mac address in the reversed network byte order; mac[5][4][3][2][1][0]
        vid: Vlan ID
        value: Variable to write. Format is raw hardware register.
*/
int bcm_arl_write(int unit, char *mac, bcm_vlan_t vid, unsigned short value); /* Deprecated API */

/*
    int bcm_arl_read2(int *unit, char *mac, unsigned int *vid, unsigned short *value);
    ******* This is Recommended New API ***********
        *unit: Input&Output; Switch Unit Number; 
               If (-1) is passed in, the unit number found entry will be returned.
        mac: Mac address in the network byte order; mac[0][1][2][3][4][5]
        *vid: Input&Ouput; Vlan ID;
              If (-1) is passed in, VID will not be used as a search criteria and
                found entry's VID will be returned. 
        *value: Ouput; Variable to hold value read back. 
                Format:
                15:Valid,14:Static,13:Age,Prio(12-10),8-0:Port No for Unicat or Port map for Multicast;
                Does not applied to external switch which request 16bit value.
                Use new bcm_arl_write() for external switch instead.
*/
int bcm_arl_read2(int *unit, char *mac, bcm_vlan_t *vid, unsigned short *value);

/*
    int bcm_arl_write2(int unit, char *mac, bcm_vlan_t vid, int value);
    ******* This is Recommended New API ***********
        *unit: Input&Output; Switch Unit Number; 
               If (-1) is passed in, the unit number found entry will be returned.
        mac: Mac address in the network byte order; mac[0][1][2][3][4][5]
        vid: Ouput; Vlan ID of ARL entry.
        value: Ouput; Variable to write to. 
               Format:
               15:Valid,14:Static,13:Age,Prio(12-10),8-0:Port No for Unicat or Port map for Multicast;
*/
int bcm_arl_write2(int unit, char *mac, bcm_vlan_t vid, int value);

int bcm_arl_dump(int unit);
int bcm_arl_dump_us(int unit, char *data); // add by Andrew
int bcm_arl_flush(int unit);

/*
 * Function:
 *	bcm_reg_read
 * Description:
 *  Read from a switch register 
 * Parameters:
 *	addr = offset 
 *	len = length of register 
 *	val = value read from register 
 * Returns:
 *	BCM_E_NONE - Success.
 */
int bcm_reg_read(unsigned int addr, char* data, int len);
int bcm_reg_read_X(int unit, unsigned int addr, char* data, int len);

/*
 * Function:
 *	bcm_reg_write 
 * Description:
 *  Write to a switch register 
 * Parameters:
 *	addr = offset 
 *	len = length of register 
 *	val = value to be written to register 
 * Returns:
 *	BCM_E_NONE - Success.
 */
int bcm_reg_write(unsigned int addr, char *data, int len);
int bcm_reg_write_X(int unit, unsigned int addr, char *data, int len);

int bcm_spi_read(unsigned int addr, char *data, int len);
int bcm_spi_write(unsigned int addr, char *data, int len); 

int bcm_pseudo_mdio_read(unsigned int addr, char *data, int len);
int bcm_pseudo_mdio_write(unsigned int addr, char *data, int len);

int bcm_get_switch_info(int switch_id, unsigned int *vendor_id,
  unsigned int *dev_id, unsigned int *rev_id, int *bus_type, 
  unsigned int *spi_id, unsigned int *chip_id, unsigned int *pbmp, 
  unsigned int *phypbmp, int *epon_port);

int bcm_set_linkstatus(int unit, int port, int linkstatus, int speed, int duplex);
int bcm_get_linkstatus(int unit, int port, int *linkstatus);
int bcm_set_extphylinkstatus(int unit, int port, int phyType, 
                             int linkstatus, int speed, int duplex);
int bcm_ethsw_kernel_poll(struct mdk_kernel_poll *mdk_kernel_poll);
int bcm_ethsw_poll_link_change(void);
int bcm_ethsw_ack_link_changed(void);

/*
 * Function:
 *  bcm_packet_padding_set
 * Description:
 *  Configure enable/disable of packet padding and min length with padding
 * Parameters:
 *  unit - RoboSwitch PCI device unit number (driver internal).
 *  enable - 1 = Enable padding; 0 = Disable padding
 *  length -  min length of the packet after padding. valid only when enable=1
 * Returns:
 *  BCM_E_NONE - Success.
 */
int bcm_packet_padding_set(int unit, int enable, int length);

/*
 * Function:
 *  bcm_packet_padding_get
 * Description:
 *  Get enable/disable of packet padding and min length with padding
 * Parameters:
 *  unit - RoboSwitch PCI device unit number (driver internal).
 *  enable - 1 = padding enabled; 0 = padding disabled
 *  length -  min length of the packet after padding. valid only when enable=1
 * Returns:
 *  BCM_E_NONE - Success.
 */
int bcm_packet_padding_get(int unit, int *enable, int *length);

/*
 * Function:
 *  bcm_multiport_set
 * Description:
 *  Set Multicast address in Switch Multiport register
 * Parameters:
 *  unit - RoboSwitch PCI device unit number (set to 0)
 *  mac  - MAC address to be added to second multiport register
 * Returns:
 *  BCM_E_NONE - Success.
 */
int bcm_multiport_set(int unit, unsigned char *mac);

/*
 * Function:
 *  bcm_dos_ctrl_set
 * Description:
 *  Configure the switch for DOS prevention/control
 * Parameters:
 *  unit - Ethernet switch unit number (Robo-switch unit=0 does not support)
 *         Always set unit = 1 for external switch
 *  pDosCtrlParams  - See structure definition below.
 * Returns:
 *  BCM_E_NONE - Success.
 */
struct bcm_dos_ctrl_params
{
    unsigned char da_eq_sa_drop_en; /* SrcMac = DstMac. Drop = 1*/
    unsigned char ip_lan_drop_en;   /* IPDA = IPSA in an IPv4/v6 datagram. Drop = 1*/
    unsigned char tcp_blat_drop_en; /* DPort = SPort in a TCP header carried in 
                                       an unfragmented IP datagram or in the first 
                                       fragment of a fragmented IP datagram. Drop = 1*/
    unsigned char udp_blat_drop_en; /* DPort = SPort in a UDP header carried in 
                                       an unfragmented IP datagram or in the first 
                                       fragment of a fragmented IP datagram. Drop = 1*/
    unsigned char tcp_null_scan_drop_en; /* Seq_Num = 0 and all TCP_FLAGs = 0 in a 
                                            TCP header carried in an unfragmented IP 
                                            datagram or in the first fragment of a 
                                            fragmented IP datagram. Drop = 1*/
    unsigned char tcp_xmas_scan_drop_en; /* Seq_Num = 0, FIN = 1, URG = 1, and PSH = 1 
                                            in a TCP header carried in an unfragmented 
                                            IP datagram or in the first fragment of a 
                                            fragmented IP datagram. Drop = 1*/
    unsigned char tcp_synfin_scan_drop_en; /* SYN = 1 and FIN = 1 in a TCP header 
                                              carried in an unfragmented IP datagram 
                                              or in the first fragment of a fragmented 
                                              IP datagram. Drop = 1*/
    unsigned char tcp_synerr_drop_en; /* SYN = 1, ACK = 0, and SRC_Port<1024 in a TCP 
                                         header carried in an unfragmented IP datagram 
                                         or in the first fragment of a fragmented IP 
                                         datagram. Drop = 1*/
    unsigned char tcp_shorthdr_drop_en; /* The length of a TCP header carried in an 
                                           unfragmented IP datagram or the first fragment 
                                           of a fragmented IP datagram is less than 
                                           MIN_TCP_Header_Size. Drop = 1*/
    unsigned char tcp_fragerr_drop_en; /* The Fragment_Offset = 1 in any fragment of a 
                                          fragmented IP datagram carrying part of TCP data.
                                          Drop = 1*/
    unsigned char icmpv4_frag_drop_en; /* The ICMPv4 protocol data unit carried in a 
                                          fragmented IPv4 datagram. Drop = 1 */
    unsigned char icmpv6_frag_drop_en; /* The ICMPv6 protocol data unit carried in a 
                                          fragmented IPv6 datagram. Drop = 1 */
    unsigned char icmpv4_longping_drop_en; /* The ICMPv4 ping (echo request) protocol data 
                                              unit carried in an unfragmented IPv4 datagram 
                                              with its Total Length indicating a value greater 
                                              than the MAX_ICMPv4_Size + size of IPv4 header
                                              Drop = 1. */
    unsigned char icmpv6_longping_drop_en; /* The ICMPv6 ping (echo request) protocol data 
                                              unit carried in an unfragmented IPv6 datagram 
                                              with its payload length indicating a value 
                                              greater than the MAX_ICMPv6_Size. Drop = 1*/
    unsigned char dos_disable_lrn; /* When this param set to 1, all frames dropped by DOS ctrl (as above)
                                      will not be learned. Recommended to set if any of above is enabled */
};
int bcm_dos_ctrl_set(int unit, struct bcm_dos_ctrl_params *pDosCtrlParams);
int bcm_dos_ctrl_get(int unit, struct bcm_dos_ctrl_params *pDosCtrlParams);
int bcm_enet_driver_wan_interface_set(char *ifname, unsigned int val);

int bcm_enet_driver_get_port_list_name(char *ifname, unsigned int sz, int ioctlVal, char *ioctlName);
#define bcm_enet_driver_get_port_list(ifname, sz, ioctl) \
    bcm_enet_driver_get_port_list_name(ifname, sz, ioctl, #ioctl)
    
#define bcm_enet_driver_wan_interface_get(ifname, sz) \
    bcm_enet_driver_get_port_list(ifname, sz, SIOCGWANPORT)
#define bcm_enet_driver_get_port_wan_only(ifnames, sz) \
    bcm_enet_driver_get_port_list(ifname, sz, SIOCGPORTWANONLY)
#define bcm_enet_driver_get_port_wan_preferred(ifnames, sz) \
    bcm_enet_driver_get_port_list(ifname, sz, SIOCGPORTWANPREFERRED)
#define bcm_enet_driver_get_port_lan_only(ifnames, sz) \
    bcm_enet_driver_get_port_list(ifname, sz, SIOCGPORTLANONLY)
/****************************************************************************/
/*  Debug API                                                               */
/****************************************************************************/

int bcm_switch_getrxcntrs(void);
int bcm_switch_resetrxcntrs(void);

typedef struct spi_dev_ids {
    int spi_id;
  int chip_id;
} spi_device;

int linux_user_spi_read(void *dvc, unsigned int addr, unsigned char *data, unsigned int len);
int linux_user_spi_write(void *dvc, unsigned int addr, const unsigned char *data, unsigned int len);
int linux_user_mdio_read(void *dvc, unsigned int addr, unsigned char *data, unsigned int len);
int linux_user_mdio_write(void *dvc, unsigned int addr, const unsigned char *data, unsigned int len);
int linux_user_ubus_read(void *dvc, unsigned int addr, unsigned char *data, unsigned int len);
int linux_user_ubus_write(void *dvc, unsigned int addr, const unsigned char *data, unsigned int len);
int linux_user_mmap_read(void *dvc, unsigned int addr, unsigned char *data, unsigned int len);
int linux_user_mmap_write(void *dvc, unsigned int addr, const unsigned char *data, unsigned int len);

/* Get PHY ID from unit, port and sub_port */
int bcm_get_phyid(int unit, bcm_port_t port, int *sub_port);

/* Get the Phy Config from Board Params */
int bcm_phy_config_get(int unit, bcm_port_t port, int *phy_config);

/* Get the name of interface for a given switch port */
int bcm_ifname_get(int unit, int port, char *name);
int bcm_enet_debug_set(int unit, int enable);
int bcm_enet_debug_get(int unit, int *enable);
int bcm_stat_get_emac(int unit, bcm_port_t port, struct emac_stats* value);
int bcm_stat_clear_emac(int unit, bcm_port_t port);
int bcm_phy_autoneg_info_get(int unit, int port, unsigned char *autoneg, unsigned short *local_cap, unsigned short* ad_cap);    
int bcm_phy_autoneg_info_set(int unit, int port, unsigned char autoneg);
int bcm_phy_autoneg_cap_adv_set(int unit, int port, unsigned char autoneg, unsigned short* ad_cap);
int bcm_phy_force_auto_mdix_set(int unit, int port, int enable);
int bcm_phy_force_auto_mdix_get(int unit, int port, int *enable);
int bcm_port_mirror_set(int unit,int enbl,int port,unsigned int ing_pmap,
                        unsigned int eg_pmap, unsigned int blk_no_mrr,
                        int tx_port, int rx_port);
int bcm_port_mirror_get(int unit,int *enbl,int *port,unsigned int *ing_pmap,
                        unsigned int *eg_pmap, unsigned int *blk_no_mrr,
                        int *tx_port, int *rx_port);
int bcm_port_trunk_set(int unit,unsigned int hash_sel);
int bcm_port_trunk_get(int unit,int *enbl,unsigned int *hash_sel,unsigned int *grp0_pmap,unsigned int *grp1_pmap);
int bcm_enet_map_phys_port_to_rdpa_if(int unit, bcm_port_t port);
int bcm_enet_get_rdpa_if_from_if_name(const char* ifname, int* rdpaIf_p);
int bcm_cfp_op(cfpArg_t *cfpArg);


int bcm_phy_apd_get(unsigned int* apd_en);
int bcm_phy_apd_set(unsigned int apd_en);
int bcm_phy_eee_get(unsigned int* eee_en);
int bcm_phy_eee_set(unsigned int eee_en);

#if defined(SUPPORT_ETH_DEEP_GREEN_MODE)
int bcm_DeepGreenMode_get(unsigned int* dgm_en);
int bcm_DeepGreenMode_set(unsigned int dgm_en);
#endif

#if defined(HAS_SF2)
int bcm_port_storm_ctrl_set(int unit,bcm_port_t port,int pkt_msk, unsigned int rate, unsigned int bucket_size);
int bcm_port_storm_ctrl_get(int unit,bcm_port_t port,int *pkt_msk, unsigned int *rate,unsigned int *bucket_size);
#endif
#if defined(HAS_RUNNER)
int bcm_KeepAlive(unsigned int timeout);
#endif

int bcm_port_create(struct net_port_t *net_port);
int bcm_port_delete(int port);

int ethswctl_gpon_mcast_gem_set(int mcast_idx);

struct IntfStats_s {
    unsigned long long byteRx, packetRx, packetErrRx, packetDropRx, packetFifoRx, packetFrameRx, packetCompRx,
        packetMultiRx, byteTx, packetTx, packetErrTx, packetDropTx, packetFifoTx, packetCollTx, packetCarrTx,
        packetCompTx, packetMultiTx, byteMultiRx, byteMultiTx, packetUniRx, packetUniTx, packetBcastRx,
        packetBcastTx, packetUnknownerrRx;
};

extern void getIntfStats(const char *devName, struct IntfStats_s *s);

extern void port_to_Intf(int port, char *intf);

#if defined (SUPPORT_RDPA)
#define SF2_ETHSWCTL_UNIT   1
#else
#define SF2_ETHSWCTL_UNIT   0
#endif

#endif /* _ETHSWCTL_API_H_ */

