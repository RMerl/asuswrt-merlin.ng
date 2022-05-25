/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
*
*    Copyright (c) 2013 Broadcom
*    All Rights Reserved
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
*
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
*
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
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
    RDPA_FILTER_L2CP,                       /**< Filter: Layer 2 Control Protocol  \XRDP_LIMITED */

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

