/*
* <:copyright-BRCM:2020:DUAL/GPL:standard
*
*    Copyright (c) 2020 Broadcom
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
    uint8_t is_esn:1;
    uint8_t esp_o_udp:1;
    uint8_t reserved:6;
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

/** @} end of SPU Offload Service Doxygen group. */

#endif /* _RDPA_SPU_OFFLOAD_H_ */
