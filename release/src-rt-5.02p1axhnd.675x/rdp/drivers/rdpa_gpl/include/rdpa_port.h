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


#ifndef _RDPA_PORT_H_
#define _RDPA_PORT_H_

/** \defgroup port Port Management
 * \ingroup rdpa
 * Logical port properties.\n
 * - Traffic management properties.
 *      - Ports can form an hierarchy for port aggregation and LAG.
 *        For example, SSID2 can be owned by PCI1
 *      - Port can own a scheduler.
 * - Bridge properties
 *      - SA/DA lookup enable and miss action
 *      - Default VID
 *      - Port can be linked to multiple bridge objects
 *      - Port can be linked to multiple VLAN objects
 * @{
 */

/** External physical port ID - identifies the external switch physical port. */
typedef enum
{
    rdpa_physical_port0, /**< Physical port 0 */
    rdpa_physical_port1, /**< Physical port 1 */
    rdpa_physical_port2, /**< Physical port 2 */
    rdpa_physical_port3, /**< Physical port 3 */
    rdpa_physical_port4, /**< Physical port 4 */
    rdpa_physical_port5, /**< Physical port 5 */
    rdpa_physical_port6, /**< Physical port 6 */
    rdpa_physical_port7, /**< Physical port 7 */
    rdpa_physical_none
} rdpa_physical_port;

/** Data path port configuration.
 * Underlying type for port_dp aggregate
 */
typedef struct
{
    rdpa_emac emac; /**< EMAC */
    bdmf_boolean enable; /**< Set=false to remove port from bridge */
    bdmf_boolean sal_enable; /**< Do SA lookup */
    bdmf_boolean dal_enable; /**< Do DA lookup */
    rdpa_forward_action sal_miss_action; /**< SA miss action */
    rdpa_forward_action dal_miss_action; /**< DA miss action */
    rdpa_physical_port physical_port; /**< Physical port for external switch. */
    bdmf_boolean ls_fc_enable; /**< Local switching via flow cache enable */
    rdpa_if control_sid; /**< Control SID on LAG iface (relevent for G9991 only) */
    bdmf_boolean ae_enable; /**< Indicates active ethernet port */
    uint8_t min_packet_size; /**< Minimum packet size \XRDP_LIMITED */
} rdpa_port_dp_cfg_t;


/** sa_limit
 * Underlying structure for port_sa_limit aggregate
 */
typedef struct
{
    uint16_t max_sa;                        /**< Max number of FDB entries allowed to be learned on the port, RDPA_VALUE_UNASSIGNED for unlimited. */
    uint16_t min_sa;                        /**< Number of FDB entries reserved to this port, 0 for unreserved.*/
    uint16_t num_sa;                        /**< RO: Number of FDB entries learned on the port. Ignored when setting configuration */
} rdpa_port_sa_limit_t;


/** Port TM configuration.
 * Underlying type for port_tm aggregate
 */
typedef struct
{
    bdmf_object_handle sched;               /**< Traffic scheduler */
    rdpa_discard_prty discard_prty;         /**< Port Ingress QoS priority */
} rdpa_port_tm_cfg_t;


/** Port QoS mapping configuration.
 * Underlying type for port_qos aggregate
 */
typedef struct
{
    bdmf_object_handle dscp_to_pbit;        /**< DSCP --> PBIT mapping table ID */
    bdmf_object_handle pbit_to_pbit;        /**< PBIT --> PBIT mapping table ID */
    bdmf_object_handle pbit_to_prty;        /**< PBIT --> PRTY mapping table ID */
} rdpa_port_qos_cfg_t;

/** Port statistics */
typedef struct {
    uint32_t rx_valid_pkt;               /**< Valid Received packets */
    uint32_t rx_crc_error_pkt;           /**< Received packets with CRC error */
    uint32_t rx_discard_1;               /**< RX discard 1 */
    uint32_t rx_discard_2;               /**< RX discard 2  */    
    uint32_t bbh_drop_1;                 /**< BBH drop 1  */    
    uint32_t bbh_drop_2;                 /**< BBH drop 2 */    
    uint32_t bbh_drop_3;                 /**< BBH drop 3 */
    uint32_t rx_discard_max_length;      /**< Packets discarded due to size bigger than MTU  */    
    uint32_t rx_discard_min_length;      /**< Packets discarded due to size smaller than 64  */    
    uint32_t tx_valid_pkt;               /**< Valid transmitted packets */
    uint32_t tx_discard;                 /**< TX discarded packets (TX FIFO full) */
    uint32_t discard_pkt;                /**< Dropped filtered Packets */
    uint32_t rx_valid_bytes;             /**< FTTdp only: Received valid bytes */
    uint32_t rx_multicast_pkt;           /**< FTTdp only: Received multicast Packets */    
    uint32_t rx_broadcast_pkt;           /**< FTTdp only: Received broadcast Packets */    
    uint32_t tx_valid_bytes;             /**< FTTdp only: Sent valid bytes */
    uint32_t tx_multicast_pkt;           /**< FTTdp only: Sent multicast Packets */    
    uint32_t tx_broadcast_pkt;           /**< FTTdp only: Sent broadcast Packets */
    uint32_t rx_frag_discard;            /**< FTTdp only: G.9991 RX fragments reassembly drop counter */
} rdpa_port_stat_t;

typedef struct {
    uint32_t bbh_rx_crc_err_ploam_drop;  /**< PLOAMs drops due to CRC error */
    uint32_t bbh_rx_third_flow_drop;     /**< Drop due to third flow error */
    uint32_t bbh_rx_sop_after_sop_drop;  /**< Drop due to Start of Packet received after SOP issue */
    uint32_t bbh_rx_no_sbpm_bn_ploam_drop;/**< Drop due to no more buffers in SRAM pool*/
    uint32_t bbh_rx_no_sdma_cd_drop;     /**<  Drop due to no Carrier Sense Multiple Access/Collision Delivery (SDMA/CD) */
    uint32_t bbh_rx_ploam_no_sdma_cd_drop; /**< Drop due to no SDMA/CD for PLOAM messages */
    uint32_t bbh_rx_ploam_disp_cong_drop; /**< Drop due to dispatcher congestion error for PLOAM */
} rdpa_port_debug_stat_t;

/** Port statistics per packet size*/
typedef struct {
    uint32_t rx_pkts_64_octets;           /**< Valid Recieved packets up to 64 octets. \XRDP_LIMITED */
    uint32_t rx_pkts_65to127_octets;      /**< Valid Recieved packets from 65 to 127 octets. \XRDP_LIMITED */
    uint32_t rx_pkts_128to255_octets;     /**< Valid Recieved packets from 128 to 255 octets. \XRDP_LIMITED */
    uint32_t rx_pkts_256to511_octets;     /**< Valid Recieved packets from 256 to 511 octets. \XRDP_LIMITED */
    uint32_t rx_pkts_512to1023_octets;    /**< Valid Recieved packets from 512 to 1023 octets. \XRDP_LIMITED */
    uint32_t rx_pkts_1024to1522_octets;   /**< Valid Recieved packets from 1024 to 1522 octets. \XRDP_LIMITED */
    uint32_t rx_pkts_1523tomtu_octets;    /**< Valid Recieved packets from 1523 to MTU octets. \XRDP_LIMITED */
    uint32_t tx_pkts_64_octets;           /**< Valid Transmitted packets up to 64 octets. \XRDP_LIMITED */
    uint32_t tx_pkts_65to127_octets;      /**< Valid Transmitted packets from 65 to 127 octets. \XRDP_LIMITED */
    uint32_t tx_pkts_128to255_octets;     /**< Valid Transmitted packets from 128 to 255 octets. \XRDP_LIMITED */ 
    uint32_t tx_pkts_256to511_octets;     /**< Valid Transmitted packets from 256 to 511 octets. \XRDP_LIMITED */
    uint32_t tx_pkts_512to1023_octets;    /**< Valid Transmitted packets from 512 to 1023 octets. \XRDP_LIMITED */
    uint32_t tx_pkts_1024to1518_octets;   /**< Valid Transmitted packets from 1024 to 1518 octets. \XRDP_LIMITED */
    uint32_t tx_pkts_1519tomtu_octets;    /**< Valid Transmitted packets from 1519 to MTU octets. \XRDP_LIMITED */
} rdpa_port_pkt_size_stat_t;

typedef uint32_t rdpa_ingress_rate_limit_traffic; /**< Mask of \ref rdpa_rl_traffic_fields ingress rate limit traffic types */

/** Flow control configuration parameters */
typedef struct {
    uint32_t rate; /**< Ingress limit rate [bits/sec] */
    uint32_t mbs; /**< Maximal burst size in bytes */
    uint32_t threshold; /**< Flow control threshold in bytes */
    bdmf_mac_t src_address; /**< Flow control mac source address . \RDP_LIMITED */
    bdmf_boolean ingress_congestion; /**< Flow control enable/disable in case of ingress congestion . \XRDP_LIMITED */
} rdpa_port_flow_ctrl_t;

/** Ingress rate limit configuration parameters \XRDP_LIMITED */
typedef struct {
    rdpa_ingress_rate_limit_traffic traffic_types; /**< Ingress rate limit traffic types bitmask . */
    bdmf_object_handle policer; /**< Referenced policer for flow control values . */
} rdpa_port_ingress_rate_limit_t;

/* Actions of the optional flow control traffic types */
typedef enum
{
    RDPA_RATE_LIMIT_BROADCAST,
    RDPA_RATE_LIMIT_MULTICAST,
    RDPA_RATE_LIMIT_UNKNOWN_DA,
	RDPA_RATE_LIMIT_ALL_TRAFFIC
} rdpa_rl_traffic_value;

/** Type of traffic to be rate limited */
typedef enum
{
    RDPA_RATE_LIMIT_MASK_BROADCAST = (1 << RDPA_RATE_LIMIT_BROADCAST), /**< In this mode, broadcast packets are rate limited */
    RDPA_RATE_LIMIT_MASK_MULTICAST = (1 << RDPA_RATE_LIMIT_MULTICAST), /**< In this mode, multicast packets are rate limited */
    RDPA_RATE_LIMIT_MASK_UNKNOWN_DA = (1 << RDPA_RATE_LIMIT_UNKNOWN_DA), /**< In this mode, unknown_da packets are rate limited */
	RDPA_RATE_LIMIT_MASK_ALL_TRAFFIC = (1 << RDPA_RATE_LIMIT_ALL_TRAFFIC), /**< In this mode, all traffic are rate limited */
} rdpa_rl_traffic_fields;

/** Port mirroring configuration parameters */
typedef struct {
    bdmf_object_handle rx_dst_port; /**< Destination port for rx traffic */
    bdmf_object_handle tx_dst_port; /**< Destination port for tx traffic */
} rdpa_port_mirror_cfg_t;

/** VLAN Isolation configuration */
typedef struct {
    bdmf_boolean us; /**< Enables VLAN isolation at ingress */
    bdmf_boolean ds; /**< Enables VLAN isolation at egress */
} rdpa_port_vlan_isolation_t;

/** loopback type - identifies the point of loopback in the system */
typedef enum
{
    rdpa_loopback_type_none,
    rdpa_loopback_type_fw,  /**< loopback in firmware */
    rdpa_loopback_type_mac, /**< loopback in mac */
    rdpa_loopback_type_phy,  /**< loopback in phy */
} rdpa_loopback_type;

/** loopback operation - identifies the loopback operation in respect of onu */
typedef enum
{
    rdpa_loopback_op_none,   /**< no loopback operation */
    rdpa_loopback_op_local,  /**< loopback is local */
    rdpa_loopback_op_remote, /**< loopback in remote */
} rdpa_loopback_op;

/** port loopback configuration */
typedef struct {
    rdpa_loopback_type  type; /**< loopback type */
    rdpa_loopback_op    op;  /**< loopback direction bitmask */
    int32_t             wan_flow; /**< gem id or llid  on which wan-wan loopback will return */
    int32_t             queue; /**< queue id on which wan-wan loopback will return */
    uint32_t            ic_idx; /** < IC context for wan-wan loopback */
} rdpa_port_loopback_t;

#define FFS(n, port) do { \
    port = ffs(n) - 1; \
    n &= ~(1L << port); \
} while (0)

/** Get index (rdpa_if) of next port from ports.
 * \param [in, out] ports Ports iterator, represented as a combination of rdpa_if_id mask. When next port is found, it's
 *     cleared from the mask. Hence, the function can be called in a loop until mask is empty.
 * \return  rdpa_if if next port found, rdpa_if_none otherwise.
 */
static inline rdpa_if rdpa_port_get_next(rdpa_ports *ports)
{
    rdpa_if port;
    unsigned long low, high;

    if (!*ports)
        return rdpa_if_none;

    high = *ports >> 32;
    low = *ports & 0xffffffff;
    if (low)
        FFS(low, port);
    else
    {
        FFS(high, port);
        port += 32;
    }
    *ports = ((unsigned long long)high << 32) | low;
    return port;
}

#ifdef BDMF_DRIVER
int rdpa_port_transparent(rdpa_if index, bdmf_boolean *enable);
#endif

rdpa_if rdpa_physical_port_to_rdpa_if(rdpa_physical_port port);
rdpa_if rdpa_port_map_from_hw_port(int hw_port, bdmf_boolean emac_only);
bdmf_boolean rdpa_if_port_rx_mirrored(rdpa_if index);
int rdpa_reconfigure_port_rx_mirroring(rdpa_if index);

/** @} end of port Doxygen group */
#endif /* _RDPA_PORT_H_ */
