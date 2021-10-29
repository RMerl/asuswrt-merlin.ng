/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
* :>
*/

#ifndef _RDPA_FILTER_H_
#define _RDPA_FILTER_H_

/** \defgroup filter Ingress Filters
 * @{
 */

/** Filter type */
typedef enum
{
    RDPA_FILTERS_BEGIN = 0,

    RDPA_FILTER_DHCP = RDPA_FILTERS_BEGIN,  /**< Filter: DHCP                           */
    RDPA_FILTER_IGMP,                       /**< Filter: IGMP                           */
    RDPA_FILTER_MLD,                        /**< Filter: MLD                            */
    RDPA_FILTER_ICMPV6,                     /**< Filter: ICMPv6                         */

    RDPA_FILTER_ETYPE_UDEF_0,               /**< Filter: Ether-Type, User-Defined #0    */
    RDPA_FILTER_ETYPE_UDEF_1,               /**< Filter: Ether-Type, User-Defined #1    */
    RDPA_FILTER_ETYPE_UDEF_2,               /**< Filter: Ether-Type, User-Defined #2    */
    RDPA_FILTER_ETYPE_UDEF_3,               /**< Filter: Ether-Type, User-Defined #3    */
    RDPA_FILTER_ETYPE_PPPOE_D,              /**< Filter: Ether-Type, PPPoE, Discovery   */
    RDPA_FILTER_ETYPE_PPPOE_S,              /**< Filter: Ether-Type, PPPoE, Session     */
    RDPA_FILTER_ETYPE_ARP,                  /**< Filter: Ether-Type, ARP                */
    RDPA_FILTER_ETYPE_802_1X,               /**< Filter: Ether-Type, 802.1X             */
    RDPA_FILTER_ETYPE_802_1AG_CFM,          /**< Filter: Ether-Type, 802.1AG, CFM       */
    RDPA_FILTER_ETYPE_PTP_1588,             /**< Filter: Ethernet, PTP-1588             */
    RDPA_FILTER_L4_PTP_1588,                /**< Filter: Ethernet, IP, UDP, PTP-1588    */
    RDPA_FILTER_MCAST_IPV4,                 /**< Filter: Multicast IPv4 (excluding Broadcast) \XRDP_LIMITED */
    RDPA_FILTER_MCAST_IPV6,                 /**< Filter: Multicast IPv6 (excluding Broadcast) \XRDP_LIMITED */
    RDPA_FILTER_MCAST_L2,                   /**< Filter: Multicast L2   (excluding Broadcast) \XRDP_LIMITED */
    RDPA_FILTER_MCAST,                      /**< Filter: Multicast (including Broadcast) */
    RDPA_FILTER_BCAST,                      /**< Filter: Broadcast                      */
    RDPA_FILTER_MAC_ADDR_OUI,               /**< Filter: MAC Address OUI \RDP_LIMITED   */
    RDPA_FILTER_HDR_ERR,                    /**< Filter: Header Error                   */
    RDPA_FILTER_IP_FRAG,                    /**< Filter: IP Fragment                    */
    RDPA_FILTER_TPID,                       /**< Filter: TPID \RDP_LIMITED              */
    RDPA_FILTER_MAC_SPOOFING,               /**< Filter: MAC Spoofing (source and destination MAC addresses equal) */
    RDPA_FILTER_IP_MCAST_CONTROL,           /**< Filter: IP Multicast control traffic. Enabled/Disabled on WAN port
                                              explicitly when Multicast filter is enabled/disabled on WAN port. Filter
                                              catches both IPv4 and IPv6 control packets. */

    RDPA_FILTERS_QUANT
} rdpa_filter;

#define RDPA_FILTER_ETYPE_UDEF_INDX_MIN 0
#define RDPA_FILTER_ETYPE_UDEF_INDX_MAX 3

#define RDPA_FILTER_OUI_VAL_INDX_MIN 0
#define RDPA_FILTER_OUI_VAL_INDX_MAX 3

/** Global configuration */
typedef struct
{
    bdmf_boolean ls_enabled; /**< Local switching enabled */
    bdmf_boolean cpu_bypass; /**< Bypass filter for packets injected from CPU using "bridge" method \XRDP_LIMITED */
} rdpa_filter_global_cfg_t;

/** MAC Address OUI filter, Value: Key */
typedef struct
{
    rdpa_ports ports;  /**< Ports (mask)   */
    uint8_t val_id; /**< Value ID       */
} rdpa_filter_oui_val_key_t;

/** TPID filter, Values */
typedef struct
{
    uint16_t val_ds; /**< Value, Downstream */
    uint16_t val_us; /**< Value, Upstream   */
} rdpa_filter_tpid_vals_t;

/** Filter: Key */
typedef struct
{
    rdpa_filter filter; /**< Filter         */
    rdpa_ports ports;  /**< Ports (mask)   */
} rdpa_filter_key_t;

/** Filter control */
typedef struct
{
    bdmf_boolean enabled;    /**< Enabled    */
    rdpa_forward_action action;     /**< Action     */
} rdpa_filter_ctrl_t;


/** Filter statistics: Key */
typedef struct
{
    rdpa_filter         filter; /**< Filter     */
    rdpa_traffic_dir    dir;    /**< Direction  */
} rdpa_filter_stats_key_t;

/** @} end of filter Doxygen group */

#endif /* _RDPA_FILTER_H_ */

