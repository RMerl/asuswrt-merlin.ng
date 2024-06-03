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

#ifndef RDPA_CPU_H_
#define RDPA_CPU_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "rdpa_types.h"
#include "rdpa_cpu_basic.h"
#if defined(CONFIG_RUNNER_IPSEC)
#include "rdpa_ipsec.h"
#endif

/** \defgroup cpu CPU Interface
 * Functions in this module:
 * - Send packets from the host to any egress port or to virtual "bridge" port
 * - Receive packets via one of host termination (CPU_RX) or Wi-Fi acceleration (WLAN_TX) queues
 * - Connect per-port and/or per-receive queue interrupt handlers
 * - Configure CPU trap reason --> queue mapping
 *
 * Runner to host traffic management parameters can be configured similarly to any other egress queue.
 * Initial port level (CPU, WLAN0, WLAN1) configuration is done using the appropriate port configuration
 * that fixes parameters such as scheduling type, number of queues, etc.\n
 * @{
 */

/** \defgroup isr Interrupt Control
 * \ingroup cpu
 * Functions in this group allow to
 * - Register / unregister per-queue interrupt handler
 * - Enable / disable / clear per-queue interrupt
 * @{
 */

/** Enable CPU queue interrupt
 * \param[in]   port        port. rdpa_cpu, rdpa_wlan0, rdpa_wlan1
 * \param[in]   queue       Queue index < num_queues in port tm configuration
 */
void rdpa_cpu_int_enable(rdpa_cpu_port port, int queue);

/** Disable CPU queue interrupt
 * \param[in]   port        port. rdpa_cpu, rdpa_wlan0, rdpa_wlan1
 * \param[in]   queue       Queue index < num_queues in port tm configuration
 */
void rdpa_cpu_int_disable(rdpa_cpu_port port, int queue);

/** Clear CPU queue interrupt
 * \param[in]   port        port. rdpa_cpu, rdpa_wlan0, rdpa_wlan1
 * \param[in]   queue       Queue index < num_queues in port tm configuration
 */
void rdpa_cpu_int_clear(rdpa_cpu_port port, int queue);

/** Enable Runner interrupt
 * \param[in]   intr_idx    Interrupt index < 32
 */
void rdpa_rnr_int_enable(uint8_t intr_idx);

/** Disable Runner interrupt
 * \param[in]   intr_idx    Interrupt index < 32
 */
void rdpa_rnr_int_disable(uint8_t intr_idx);

/** Clear Runner interrupt
 * \param[in]   intr_idx    Interrupt index < 32
 */
void rdpa_rnr_int_clear(uint8_t intr_idx);

/** @} end of isr Doxygen group */

/** \defgroup cpu_rx Receive
 * \ingroup cpu
 * Functions in this group allow to
 * - Register/unregister per-port and per-queue interrupt handlers
 * - Register/unregister packet handler
 * - Pull received packets from port (scheduling) or queue
 * @{
 */

/** CPU meter SR validation constants */


#ifdef XRDP
#define RDPA_CPU_METER_MAX_SR       500000     /**< Max CPU meter SR value in pps */
#define RDPA_CPU_METER_MIN_SR       10         /**< Min CPU meter SR value in pps */
#define RDPA_CPU_METER_SR_QUANTA    10         /**< CPU meter SR must be a multiple of quanta */
#else
#define RDPA_CPU_METER_MAX_SR       40000       /**< Max CPU meter SR value in pps */
#define RDPA_CPU_METER_MIN_SR       100         /**< Min CPU meter SR value in pps */
#define RDPA_CPU_METER_SR_QUANTA    100         /**< CPU meter SR must be a multiple of quanta */
#endif



/** Reason index. Underlying type of cpu_reason_index aggregate */
typedef struct
{
#if defined(BCM_DSL_RDP)
    rdpa_traffic_dir dir;       /**< Traffic direction */
#endif
    rdpa_cpu_reason reason;     /**< CPU reason */
#if defined(BCM_DSL_RDP)
    int table_index;            /**< reason table index */
#endif
    int entry_index;            /* Reserved for internal use. */
} rdpa_cpu_reason_index_t;

/** Reason configuration structure.
 * Underlying structure for cpu_reason_cfg aggregate
 */
typedef struct
{
    bdmf_index queue;           /**< reason --> queue mapping */
    bdmf_index meter;           /**< CPU interface meter index < RDPA_CPU_MAX_METERS or BDMF_INDEX_UNDEFINED */
} rdpa_cpu_reason_cfg_t;

/** CPU meter configuration
 * Underlying structure for cpu_meter_cfg aggregate
 */
typedef struct
{
    uint32_t sir;               /**< SIR ( packets per sec ) */
    uint32_t burst_size;        /**< Burst size ( packets ) */
} rdpa_cpu_meter_cfg_t;

/** L4 dst port to reason configuration
 * Underlying structure for l4_dst_port_to_reason aggregate
 */
typedef struct
{
    bdmf_boolean is_tcp;        /**< TCP or UDP */
    uint16_t l4_dst_port;       /**< l4 dst port */
    rdpa_cpu_reason reason;     /**< rdpa cpu reason */
    bdmf_boolean is_static;     /**< static or dynamic */
    uint8_t refcnt;             /**< reference count */
} rdpa_l4_dst_port_to_reason_cfg_t;

/** Receive packet info */
typedef struct
{
    rdpa_cpu_reason reason;     /**< trap reason */
    uint16_t vport;             /**< vport - used for dump */
    uint32_t handle;            /**< private context for network driver */ 
    uint32_t reason_data;       /**< Reason-specific data.
                                     For CPU port it usually contains src_flow
                                     For Wi-Fi port it contains source SSID index in 16 MSB and destination SSID vector in 16 LSB
                                     It can contain other reason-specific info (such as OAM info, etc.).
                                */
    void *data;        /**<data pointer or FPM/BPM hardware token*/
    uint32_t size;        /**<data size */
    uint32_t data_offset;    /**<data offset inside pointer */
    struct {
        uint32_t rx_csum_verified:1;
        uint32_t reserved:31;
    };
    uint32_t dest_ssid;   /** <when traffic is to be forward to WFD */
    uint32_t wl_metadata; /** WLAN metadata */
    uint16_t ptp_index; /**< index of ptp RX entry */
    /* (CONFIG_RUNNER_GDX_SUPPORT) RX */
    uint16_t gdx_pd_data;
    uint8_t mcast_tx_prio; /**< TX priority (0-7) for multicast traffic \XRDP_LIMITED */
    uint8_t color;
    uint8_t is_exception;
    uint8_t is_rx_offload;
    uint8_t is_ucast;
    uint8_t omci_encrypted_key_index; /**< encryption key-index in NGPON modes */
    /* (CONFIG_CPU_REDIRECT_MODE_SUPPORT) */
    uint8_t  cpu_redirect_egress_queue;
    uint8_t  cpu_redirect_wan_flow;
    /* (CONFIG_RUNNER_SPU_OFFLOAD) */
   uint8_t  spu_session_id;
    /* (not XRDP) */
    uint16_t free_index;
} rdpa_cpu_rx_info_t;

/** Extended CPU info (debugging) */
typedef struct
{
    uint8_t valid;                      /**< 1 means that the following fields contain valid data */
    struct bdmf_object *egress_object;  /**< Egress object (port, tcont, llid) */
    uint32_t egress_queue_id;           /**< Egress queue_id */
    uint16_t wan_flow;                  /**< Egress/ingress WAN flow */
} rdpa_cpu_rx_ext_info_t;

typedef void (*rdpa_cpu_rxq_rx_isr_cb_t)(long isr_priv);
typedef void (*rdpa_cpu_rxq_rx_cb_t)(long priv, bdmf_sysb sysb, rdpa_cpu_rx_info_t *info);
typedef void (*rdpa_cpu_rxq_dump_data_cb_t)(bdmf_index queue, bdmf_boolean enabled);

/** Host receive configuration.
 * Underlying structure of cpu_rx_cfg aggregate.
 */
typedef struct {
    bdmf_boolean  ic_enable; /**< True - Enable. False - Disable */
#define RDPA_CPU_IC_MAX_PKT_CNT       63
    uint8_t  ic_max_pktcnt;  /**< Interrupt Coalescing max packet count.
                                  MAX value 63
                             */
#define RDPA_CPU_IC_MAX_TIMEOUT_IN_US 1023
    uint16_t ic_timeout_us;  /**< The hardware timer period is configured to
                                  100 us. The timeout period
                                  will round up to the next 100 us.
                                  For example, if the timeout is 201 us,
                                  a timeout will not be identified until 300us.
                                  This means for practical purposes, the
                                  timeout value is 100 us, 200 us, 300 us,
                                  400 us, 500 us, 600 us, 700 us, 800 us,
                                  900 us, 1000 us and 1100 us.
                             */
} rdpa_cpu_rxq_ic_cfg_t;

typedef struct {
    uint32_t received;
    uint32_t queued;
    uint32_t dropped;
} extern_rxq_stat_t;

typedef void (*extern_rxq_stat_callback)(int qid, extern_rxq_stat_t *stat, bdmf_boolean clear);

typedef void (*reason_stat_extern_callback_t)(uint32_t *stat, rdpa_cpu_reason_index_t *rindex);

#define RDPA_CPU_QUEUE_MAX_SIZE  1024       /**< Maximal Host queue size */
#define RDPA_CPU_QUEUE_MIN_SIZE  64         /**< Minimal Host queue size */
#define RDPA_CPU_WLAN_QUEUE_MAX_SIZE  1024  /**< Maximal WFD queue size */
#define RDPA_CPU_GDX_QUEUE_MAX_SIZE   (16 * 1024)  /**< Maximal GDX queue size */
#if !defined(CONFIG_CPU_RX_FROM_XPM)
#define RDPA_FEED_QUEUE_MAX_SIZE  (1024 * 32)  /**< Maximal FEED queue size */
#endif
#define RDPA_RECYCLE_QUEUE_MAX_SIZE  (1024 * 32)  /**< Maximal FEED queue size */
#if !defined(RDP_SIM) 
#define RDPA_CPU_TX_RING_HIGH_PRIO_SIZE (1024 * 2)  /**< Maximal CPU_TX queue for high priority size */
#define RDPA_CPU_TX_RING_LOW_PRIO_SIZE (1024 * 2)  /**< Maximal CPU_TX queue for low priority size */
#define RDPA_CPU_TX_RING_MCORE_SIZE (1024 * 2)  /**< Maximal CPU_TX queue for mcore priority size */
#define RDPA_CPU_TX_RING_SIZE (1024 * 2) /**< Maximal CPU_TX queue size for validation */
#define RDPA_CPU_TX_RING_EXLUSIVE_RESERVED_SIZE 128 /**< Minimal CPU_TX queue size for high priority packets reservation */
#else
#define RDPA_CPU_TX_RING_HIGH_PRIO_SIZE (64)  /**< Maximal CPU_TX queue for high priority size */
#define RDPA_CPU_TX_RING_LOW_PRIO_SIZE (64)  /**< Maximal CPU_TX queue for low priority size */
#define RDPA_CPU_TX_RING_MCORE_SIZE (64)  /**< Maximal CPU_TX queue for mcore priority size */
#define RDPA_CPU_TX_RING_SIZE (64) /**< Maximal CPU_TX queue size for validation */
#define RDPA_CPU_TX_RING_EXLUSIVE_RESERVED_SIZE 2 /**< Minimal CPU_TX queue size for high priority packets reservation */
#endif
#define RING_NORMAL_PRIO  0
#define RING_HIGH_PRIO    1   

/** Receive packet info
*   Underlying structure for cpu_rxq_cfg aggregate
*/
typedef struct {
    /** ISR callback
     * If set ISR is connected to the appropriate port+queue
     * \param[in]   isr_priv
     */
    rdpa_cpu_rxq_rx_isr_cb_t rx_isr;
    void *ring_head;            /**< Allocated by client ring */
    uint32_t size;              /**< Queue size */
    rdpa_ring_type_t type;               /**< ring type \XRDP_LIMITED */
    bdmf_boolean dump;          /**< Dump rx data */
    long isr_priv;              /**< Parameter to be passed to isr callback */
    rdpa_cpu_rxq_ic_cfg_t ic_cfg; /**< Interrupt Coalescing config */
    rdpa_cpu_rxq_dump_data_cb_t rx_dump_data_cb; /**< Dump RX data callback */
    extern_rxq_stat_callback rxq_stat; /**< Optional internal statistics callback */
    long unsigned int irq_affinity_mask; /**< Queue ISR affinity mask - every bit represents CPU */
    uint32_t  ring_prio;         /**< Queue Priority - used for buffer allocation request \RDP_LIMITED  */
} rdpa_cpu_rxq_cfg_t;

/** Receive statistics.
 * Underlying structure for cpu_rx_stat aggregate
 */
typedef struct
{
    uint32_t received;      /**< Packets passed to rx callback */
    uint32_t queued;        /**< Packets in the queue */
    uint32_t dropped;       /**< Packets dropped by RDD / FW */
    uint32_t interrupts;    /**< interrupts */
    uint32_t bufmng_low_prio_drop;        /**< Drop due to FPM budget for low priority exceed\RDP_UFC limited relevant for CPU and WLANs ports */
    uint32_t bufmng_high_prio_drop;       /**< Drop due to FPM budget for high priority exceed\RDP_UFC limited relevant for CPU and WLANs ports */
} rdpa_cpu_rx_stat_t;

/** Cpu loopback type
 */
typedef enum
{
    rdpa_cpu_loopback_ipsec,      /**< IPsec offload packet loopback */

    rdpa_cpu_loopback__num_of
} rdpa_cpu_loopback_type;

/** Pull a single received packet from host queue.
 * This function pulls a single packet from the host queue.
 * \param[in]   port            CPU port (host, wlan0, wlan1, wlan2)
 * \param[in]   queue           Queue index < num_queues in rdpa_port_tm_cfg_t or UNASSIGNED
 * \param[out]  info            Packet info
 * \return =0:packet pulled ok,  <0:int error code\n
 */
int rdpa_cpu_packet_get(rdpa_cpu_port port, bdmf_index queue, rdpa_cpu_rx_info_t *info);

/** Pull a single received packet from host queue with extended RX info.
 * Extended info is only available for packets redirected to CPU
 * using system.rx_cpu_redirect attribute
 *
 * This function pulls a single packet from the host queue.
 * \param[in]   port            CPU port (host0, host1, ..)
 * \param[in]   queue           Queue index < num_queues in rdpa_port_tm_cfg_t or UNASSIGNED
 * \param[out]  info            Packet info
 * \param[out]  ext_info        Packet extended info
 * \return =0:packet pulled ok,  <0:int error code\n
 */
int rdpa_cpu_packet_get_redirected(rdpa_cpu_port port, bdmf_index queue,
    rdpa_cpu_rx_info_t *info, rdpa_cpu_rx_ext_info_t *ext_info);

#define CPU_RX_PACKETS_BULK_SIZE 32

/** Pull a single offloaded packet from host queue.
 * This function receives a single packet that was offloaded from CPU.
 * \param[in]   loopback_type   Loopback type
 * \param[in]   queue           Queue index < num_queues in rdpa_port_tm_cfg_t or UNASSIGNED
 * \param[out]  sysb            Packet data (buffer)
 * \param[out]  info            Packet info
 * \return =1:packet pulled ok, ==0:no packet pulled, <0:int error code\n
 */
int rdpa_cpu_loopback_packet_get(rdpa_cpu_loopback_type loopback_type, bdmf_index queue,
                                 bdmf_sysb *sysb, rdpa_cpu_rx_info_t *info);

/** Retrieves Opaque CPU handle
 * \param[in]   rdpa_cpu_type      CPU port identifier ::rdpa_cpu_port
 * \return Opaque CPU handle.
*/
void *rdpa_cpu_data_get(int rdpa_cpu_type);


/** @} end of cpu_rx Doxygen group */

/** \defgroup cpu_tx Transmit
 * \ingroup cpu
 * Packets can be transmitted
 * - via specific port/queue
 * - using FW bridging & classification logic
 *
 * All rdpa_cpu_send_to_xx functions return int error code
 * - If rc==0, buffer ownership is passed to RDPA. The buffer will be released when safe.
 * - If rc!=0, buffer ownership stays with the caller
 * @{
 */

#if defined(BCM_DSL_RDP) || defined(BCM_DSL_XRDP)
/** Extra data that can be passed along with the packet to be transmitted */
typedef struct
{
    union
    {
        uint32_t u32;
        struct {
            uint32_t is_spdsvc_setup_packet:1;  /**<when set, indicates that a Speed Service Setup packet is being transmitted */
            uint32_t lag_port:2;                /**<Runner to external switch lag port (EMAC) */
            uint32_t tc_id:6;                   /**<traffic class value assigned with this packet */
            uint32_t unused:23;
        };
    };
    uint32_t so_mark;
} rdpa_cpu_tx_extra_info_t;
#endif

/** Transmit statistics */
typedef struct
{
    uint32_t tx_ok;                     /**< Successfully passed to RDD for transmission */
    uint32_t tx_invalid_queue;          /**< Discarded because transmit queue is not configured */
    uint32_t tx_no_buf;                 /**< Discarded because couldn't allocate BPM buffer */
    uint32_t tx_too_long;               /**< Discarded packets with length > max MTU size */
    uint32_t tx_rdd_error;              /**< Discarded because RDD returned error */
} rdpa_cpu_tx_stat_t;


/** Send system buffer
 *
 * \param[in]   sysb        System buffer. Released regardless on the function outcome.
 * \param[in]   info        Tx info
 * \return 0=OK or int error code\n
 */
int rdpa_cpu_send_sysb(bdmf_sysb sysb, const rdpa_cpu_tx_info_t *info);

/** Send system buffer allocated from FPM
 *
 * \param[in] sysb System buffer. Released regardless on the function outcome.
 * \param[in] info Tx info
 * \return 0=OK or int error code\n
 */
int rdpa_cpu_send_sysb_fpm(bdmf_sysb sysb, const rdpa_cpu_tx_info_t *info);

#if defined(BCM_DSL_RDP) || defined(BCM_DSL_XRDP)
/** Send system buffer to Ethernet/DSL WAN Interface
 *
 * \param[in]   sysb          System buffer. Released regardless on the function outcome.
 * \param[in]   egress_queue  Ethernet Egress Queue
 * \return 0=OK or int error code\n
 */
int rdpa_cpu_tx_port_enet_or_dsl_wan(bdmf_sysb sysb, uint32_t egress_queue, rdpa_flow wan_flow, bdmf_object_handle port_obj,
                                     rdpa_cpu_tx_extra_info_t extra_info);

/** Send system buffer to Ethernet LAN Interface
 *
 * \param[in]   sysb          System buffer. Released regardless on the function outcome.
 * \param[in]   egress_queue  Ethernet Egress Queue
 * \param[in]   phys_port     Ethernet LAN physical port
 * \return 0=OK or int error code\n
 */
int rdpa_cpu_tx_port_enet_lan(bdmf_sysb sysb, uint32_t egress_queue,
                              bdmf_object_handle port_obj, rdpa_cpu_tx_extra_info_t extra_info);

/** Send system buffer to Flow Cache Offload
 *
 * \param[in] sysb: System buffer. Released regardless on the function outcome.
 * \param[in] cpu_rx_queue: CPU Rx Queue index, in case of Runner Flow miss.
 * \param[in] dirty: Indicates whether a packet flush from D$ is required.
 * \return 0=OK or int error code\n
 */
int rdpa_cpu_tx_flow_cache_offload(bdmf_sysb sysb, uint32_t cpu_rx_queue, int dirty);

/** Frees the given free index and returns a pointer to the associated
 *  System Buffer. It is up to the caller to process the System Buffer.
 *
 * \param[in] free_index: Runner free index
 * \return: Pointer to the associated system buffer\n
 */
bdmf_sysb rdpa_cpu_return_free_index(uint16_t free_index);

/** Free all pending Tx Buffers
 */
void rdpa_cpu_tx_reclaim(void);

/** return port handle for a vport
 */
uint32_t rdpa_cpu_get_port_handle(uint16_t vport, void **data);

/** Send system buffer to IPsec Offload
 *
 * \param[in] sysb: System buffer.
 * \param[in] dir: Indicates whether the packet is upstream or downstream.
 * \param[in] esphdr_offset: ESP header byte offset into the packet.
 * \param[in] sa_index: Entry index of the ddr SA descriptor table.
 * \param[in] sa_update: 0- sa_index entry of the ddr sa descriptor table is new.
 *                       1- sa_index entry of the ddr sa descriptor table has been updated.
 * \param[in] cpu_qid:  Runner - HostCPU queue id
 * \return 0=OK or int error code\n
 */
int rdpa_cpu_tx_ipsec_offload(bdmf_sysb sysb, rdpa_traffic_dir dir, uint8_t esphdr_offset,
                              uint8_t sa_index, uint8_t sa_update, uint8_t cpu_qid);
#endif

/** Send raw packet
 *
 * \param[in]   data        Packet data
 * \param[in]   length      Packet length
 * \param[in]   info        Info
 * \return 0=OK or int error code\n
 */
int rdpa_cpu_send_raw(void *data, uint32_t length, const rdpa_cpu_tx_info_t *info);

/** Get cpu queue emptiness status
 *
 * \param[in]   port        CPU port (CPU, PCI1, PCI2)
 * \param[in]   queue       Queue index < num_queues in rdpa_port_tm_cfg_t or UNASSIGNED
 * \return 0=no packets are waiting or at least one packet in queue\n
 */
int rdpa_cpu_queue_not_empty(rdpa_cpu_port port, bdmf_index queue);

/** Get cpu queue fullness status
 *
 * \param[in]   port        CPU port (CPU, PCI1, PCI2)
 * \param[in]   queue       Queue index < num_queues in rdpa_port_tm_cfg_t or UNASSIGNED
 * \return 1 = if queue is full or 0 if there is still place to put packets\n
 */
int rdpa_cpu_queue_is_full(rdpa_cpu_port port, bdmf_index queue);

/** @} end of cpu_tx Doxygen group */
/** @} end of cpu Doxygen group */

void rdpa_cpu_rx_dump_packet(char *name, rdpa_cpu_port port,
    bdmf_index queue, rdpa_cpu_rx_info_t *info, uint32_t dst_ssid);

#ifdef XRDP
/** Platform buffer */
typedef void *bdmf_pbuf_t;

static inline void bdmf_pbuf_init(uint32_t size, uint32_t offset)
{}
static inline void bdmf_pbuf_free(bdmf_pbuf_t *pbuf)
{}
static inline int bdmf_pbuf_alloc(void *data, uint32_t length, uint16_t source, bdmf_pbuf_t *pbuf)
{
    return 0;
}
#endif

#if defined(CONFIG_RNR_SHARED_HW_SW_BUF_POOL)
/** Send raw packet from pre-allocated FPM.
 *
 * \param[in]   data        FPM buffer that includes the packet data
 * \param[in]   length      Packet length
 * \param[in]   info        Info
 * \return 0=OK or int error code\n
 */
int rdpa_cpu_send_raw_from_fpm(void *data, uint32_t length, const rdpa_cpu_tx_info_t *info);
#endif


#if defined(CONFIG_RUNNER_GDX_SUPPORT) /* RX */
typedef struct rdpa_gdx_params
{
    uint64_t base_addr;
    uint8_t hit_value;
    uint8_t drop_value;
} rdpa_gdx_params_t;

void rdpa_gdx_params_set(rdpa_gdx_params_t *gdx_params);
#endif

/** Check whether buffer related to FPM (HW managed)
 *
 * \param[in]   buffer_ptr          Pointer to the buffer
 * \return true=YES\n
*/
uint32_t rdpa_buffer_is_hw_buffer(void *buffer_ptr);

#ifdef __cplusplus
}
#endif


#endif /* RDPA_CPU_H_ */

