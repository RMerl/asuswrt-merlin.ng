/*
* <:copyright-BRCM:2020:DUAL/GPL:standard
* 
*    Copyright (c) 2020 Broadcom 
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

#ifndef _RDPA_SPU_OFFLOAD_H_
#define _RDPA_SPU_OFFLOAD_H_

/** \defgroup SPU Offload Service
 * This object is used to manage the module responsible for SPU offloading \n
 * in Runner \n
 * @{
 */

/** Session (flow) data.\n
 */
typedef struct {
    uint8_t session_id;
    uint8_t digest_size;
    uint8_t key_size;
    uint8_t iv_size;
    uint8_t chksm_offset;
    uint8_t is_esn:1;
    uint8_t esp_o_udp:1;
    uint8_t transport_mode:1;
    uint8_t is_gcm:1;
    uint8_t ipv6:1;
    uint8_t reserved:3;
    uint64_t key_buf_dma_addr;
} rdpa_crypto_session_info_t;

typedef struct {
    uint64_t bd_base;
    uint64_t cmpl_base;
    uint64_t rd_idx_ddr_addr;
    uint32_t doorbell_reg;
    uint32_t cmpl_wr_ptr_reg;

} rdpa_spu_pd_ring_info_t;

typedef struct
{
    bdmf_phys_addr_t wakeup_register;
    uint32_t wakeup_value;

} rdpa_spu_resp_wakeup_info_t;

extern void rdpa_spu_set_pd_ring(rdpa_spu_pd_ring_info_t *info);
extern void rdpa_spu_crypto_session_base_set(uint64_t base_addr);
extern void rdpa_crypto_session_info_set(rdpa_crypto_session_info_t *session);
extern void rdpa_spu_databuf_recycle(uint32_t plen, void *buffer_ptr);
extern int rdpa_spu_resp_wakeup_information_get(rdpa_spu_resp_wakeup_info_t *info);
extern int rdpa_spu_set_cpu_irq(struct bdmf_object *mo, int idx);
extern void rdpa_spu_offload_pkt(uint32_t *req, uint32_t *cmpl);

/** @} end of SPU Offload Service Doxygen group. */

#endif /* _RDPA_SPU_OFFLOAD_H_ */
