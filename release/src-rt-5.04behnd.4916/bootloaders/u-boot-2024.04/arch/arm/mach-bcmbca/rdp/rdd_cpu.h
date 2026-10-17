// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Broadcom
 */

/*
    
*/

#ifndef _BL_LILAC_DRV_RUNNER_CPU_H
#define _BL_LILAC_DRV_RUNNER_CPU_H

#include "rdpa_cpu.h"
#include "bdmf_errno.h"

extern uint8_t  *g_runner_ddr_base_addr;
extern uint32_t  g_ddr_headroom_size;
extern uint32_t  g_cpu_tx_queue_write_ptr[ 4 ];
extern uint32_t  g_cpu_tx_queue_abs_data_ptr_write_ptr[ 4 ];
extern uint32_t  g_cpu_tx_abs_packet_limit;


extern uint16_t  *g_free_skb_indexes_fifo_table;
extern uint8_t **g_cpu_tx_skb_pointers_reference_array;
extern bdmf_fastlock int_lock_irq;

/* Name:                                                                      */
/*                                                                            */
/*   rdd_cpu_reason_to_cpu_rx_queue                                           */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - CPU interface.                                         */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   cpu trap reason to cpu rx queue conversion.                              */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_cpu_reason - the reason for sending the packet to the CPU             */
/*   xi_queue_id - CPU-RX queue index (0-7)                                   */
/*   xi_direction - upstream or downstream                                    */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_CPU_RX_QUEUE_ILLEGAL - CPU-RX queue is illegal.     */
/*     BL_LILAC_RDD_ERROR_CPU_RX_REASON_ILLEGAL - CPU-RX reason is illegal.   */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_cpu_reason_to_cpu_rx_queue ( rdpa_cpu_reason  xi_cpu_reason,
                                                        BL_LILAC_RDD_CPU_RX_QUEUE_DTE   xi_queue_id,
                                                        rdpa_traffic_dir                xi_direction,
                                                        uint32_t                        xi_table_index );

/******************************************************************************/
/*                                                                            */
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_l4_dst_port_read                                                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - CPU interface.                                         */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   read entry from l4 dst port to cpu trap reason mapping table.            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_index - index of entry want to read                                   */
/*   xo_is_static - static or dynamic                                         */
/*   xo_is_tcp - tcp or udp                                                   */  
/*   xo_l4_dst_port - dst port                                                */
/*   xo_reason - the cpu trap reason                                          */
/*   xo_refcnt - reference count                                              */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_l4_dst_port_read ( uint32_t xi_index, bdmf_boolean *xo_is_static, bdmf_boolean *xo_is_tcp, uint16_t *xo_l4_dst_port, rdpa_cpu_reason *xo_reason, uint8_t *xo_refcnt );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_l4_dst_port_delete                                                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - CPU interface.                                         */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   delete entry from l4 dst port to cpu trap reason mapping table.          */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_index - index of entry want to read                                   */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_l4_dst_port_delete ( uint32_t xi_index );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_l4_dst_port_find                                                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - CPU interface.                                         */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   find entry in l4 dst port to cpu trap reason mapping table.              */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_is_tcp - tcp or udp                                                   */  
/*   xi_l4_dst_port - dst port                                                */
/*   xo_index - index of the new add entry                                    */  
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_l4_dst_port_find ( bdmf_boolean xi_is_tcp, uint16_t xi_l4_dst_port, uint32_t *xo_index );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_cpu_tx_write_eth_packet                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - CPU interface.                                         */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This packet is directed to the EMAC TX queue.                            */
/*   check if the CPU TX queue is not full, if not then a DDR buffer is       */
/*   allocated from the BPM, then the packet data is copied to the allocated  */
/*   buffer and a new packet descriptor is written to the CPU TX queue.       */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_packet_ptr - pointer to a DDR buffer that hold the packet data        */
/*   xi_packet_size - packet size in bytes                                    */
/*   xi_emac_id - EMAC port index (ETH0 - ETH4, PCI)                          */
/*   xi_wifi_ssid - service set id for PCI with wifi multiple ssid support    */
/*   xi_queue_id - ETH TX queue index                                         */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_CPU_TX_NOT_ALLOWED - the runner is not enabled.     */
/*     BL_LILAC_RDD_ERROR_CPU_TX_QUEUE_FULL - the CPU TX has no place for new */
/*                                            packets.                        */
/*     BL_LILAC_RDD_ERROR_BPM_ALLOC_FAIL - unable to allocate a DDR buffer to */
/*                                         from the BPM.                      */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_cpu_tx_write_eth_packet ( uint8_t                    *xi_packet_ptr,
                                                     uint32_t                   xi_packet_size,
                                                     BL_LILAC_RDD_EMAC_ID_DTE   xi_emac_id,
                                                     uint8_t                    xi_wifi_ssid,
                                                     BL_LILAC_RDD_QUEUE_ID_DTE  xi_queue_id );

/* local */
BL_LILAC_RDD_ERROR_DTE rdd_cpu_rx_initialize ( void );

BL_LILAC_RDD_ERROR_DTE rdd_cpu_tx_initialize ( void );

BL_LILAC_RDD_ERROR_DTE rdd_spdsvc_initialize( void );

BL_LILAC_RDD_ERROR_DTE rdd_spdsvc_get_tx_result( uint8_t *xo_running_p,
                                                 uint32_t *xo_tx_packets_p,
                                                 uint32_t *xo_tx_discards_p );

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_ring_init                                                            */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - CPU interface.                                         */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   configures CPU-RX ring, set the number packets that are pending to be    */
/*   read by the CPU for that queue, and the interrupt that will be set       */
/*   during packet enterance to the queue, the default interrupt is the queue */
/*   number.                                                                  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_ring_id - CPU-RX ring index (0-7)                                     */
/*   xi_number_of_entries - queue maximum size in packets.                    */
/*   xi_ring_address - address of allocated ring                              */
/*   xi_size_of_entry - size of cpu-rx descriptor                             */
/*   xi_interrupt_id  - interrupt id to set                                   */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_ring_init ( uint32_t  xi_ring_id,
                                       uint8_t unused0,
                                       rdd_phys_addr_t xi_ring_address,
                                       uint32_t  xi_number_of_entries,
                                       uint32_t  xi_size_of_entry,
                                       uint32_t  xi_interrupt_id,
                                       uint32_t unused1,
                                       bdmf_phys_addr_t unused2,
                                       uint8_t unused3
                                       );

#endif /* _BL_LILAC_DRV_RUNNER_CPU_H */

