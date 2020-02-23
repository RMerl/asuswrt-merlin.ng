/*
   Copyright (c) 2014 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2014:DUAL/GPL:standard

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.

    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.

:>
*/

#ifdef CONFIG_DHD_RUNNER
#include "rdd.h"
#include "rdd_runner_defs_auto.h"
#include "rdd_data_structures.h"

#include "rdd_dhd_helper.h"
#include "rdpa_dhd_helper_basic.h"
#if !defined(FIRMWARE_INIT)
#include <linux/nbuff.h>
#include "bcm_mm.h"
#include "rdp_mm.h"
#else
#include "rdp_drv_bpm.h"
#include "access_macros.h"
#endif
#ifdef LEGACY_RDP
#include "rdd_legacy_conv.h"
#endif
#include "rdd_ih_defs.h"

#ifndef G9991
bdmf_boolean is_dhd_enabled[RDPA_MAX_RADIOS] = {0};
extern int flow_ring_format[];

rdd_dhd_complete_ring_descriptor_t g_dhd_complete_ring_desc[RDPA_MAX_RADIOS] = {{0, 0, 0, 0},};

extern uint8_t *g_runner_ddr_base_addr;
extern uint32_t g_runner_ddr_base_addr_phys;

extern bdmf_fastlock int_lock_irq;
extern rdpa_bpm_buffer_size_t g_bpm_buffer_size;

#if !defined(FIRMWARE_INIT)
#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
void *g_dhd_tx_post_mgmt_fr_base_ptr[RDPA_MAX_RADIOS] = {NULL};
#endif
rdd_dhd_rx_post_ring_t g_dhd_rx_post_ring_priv[RDPA_MAX_RADIOS] = {{0, 0, 0, 0},};
#endif

#if defined(WL4908)
// FPM Related External References
extern uint32_t fpm_alloc_max_size_token_pool(int pool);
extern void fpm_free_token(uint32_t);
extern uint8_t *fpm_token_to_buffer(uint32_t);
extern uint32_t fpm_convert_fpm_token_to_rdp_token(uint32_t token);
extern uint32_t fpm_convert_rdp_token_to_fpm_token(uint32_t token);
#define FPM_TOKEN_VALID_MASK_4RDH          0x80000000
#define BPM_BUFFER_NUMBER_DDR_OFFSET       17
#define BPM_BUFFER_NUMBER_INDEX_WIDTH      16
#define BPM_BUFFER_NUMBER_INDEX_OFFSET     0
#endif

int flow_ring_format[RDPA_MAX_RADIOS];

#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
#define HEADROOM_SIZE_4RDH ((g_ddr_headroom_size + DRV_RDD_IH_PACKET_HEADER_OFFSET + 7) & (~7))  // Must pad up to 4 byte boundary
#else
#define HEADROOM_SIZE_4RDH ((g_ddr_headroom_size + DHD_DATA_OFFSET + 7) & (~7))
#endif

static void rdd_rx_post_descr_init(uint32_t radio_idx, uint8_t *descr_ptr, uint32_t bpm_buffer_number,
    bdmf_boolean valid_bpm)
{
    uint32_t req_id;
    uint16_t len;
    uint8_t* data_buf_ptr_low_virt;
    bdmf_phys_addr_t data_buf_ptr_low_phys;

#if defined(WL4908)
    req_id = bpm_buffer_number | (DRV_BPM_SP_SPARE_1 << 18);
    len = __swap2bytes(DHD_DATA_LEN);
#elif defined(DSL_63138)
    req_id = bpm_buffer_number | (DRV_BPM_SP_SPARE_1 << 15);
    len = __swap2bytes(DHD_DATA_LEN);
#elif defined(DSL_63148)
    req_id = bpm_buffer_number | (DRV_BPM_SP_SPARE_1 << 14);
    len = __swap2bytes(DHD_DATA_LEN);
#else
    req_id = bpm_buffer_number | (DRV_BPM_SP_SPARE_1 << 14);
    len = cpu_to_le16(DHD_DATA_LEN);
#endif

#if defined(WL4908) && !defined(FIRMWARE_INIT)
    data_buf_ptr_low_virt = fpm_token_to_buffer(fpm_convert_rdp_token_to_fpm_token(bpm_buffer_number)) + HEADROOM_SIZE_4RDH;
#else
    data_buf_ptr_low_virt = valid_bpm ? g_runner_ddr_base_addr + bpm_buffer_number * g_bpm_buffer_size +
        HEADROOM_SIZE_4RDH : NULL;
#endif

    if (data_buf_ptr_low_virt == NULL)
        data_buf_ptr_low_phys = 0;
    else
        data_buf_ptr_low_phys = RDD_RSV_VIRT_TO_PHYS(g_runner_ddr_base_addr, g_runner_ddr_base_addr_phys,
            data_buf_ptr_low_virt);

#if !defined(FIRMWARE_INIT) && (defined(DSL_63138) || defined(DSL_63148) || defined(WL4908))
    data_buf_ptr_low_phys = __swap4bytes(data_buf_ptr_low_phys);
#else
    data_buf_ptr_low_phys = cpu_to_le32(data_buf_ptr_low_phys);
#endif

    switch (flow_ring_format[radio_idx]) {
        case FR_FORMAT_WI_WI64: /* Legacy Work Item */
        {
            RDD_DHD_RX_POST_DESCRIPTOR_MSG_TYPE_WRITE(DHD_MSG_TYPE_RX_POST, descr_ptr);
            RDD_DHD_RX_POST_DESCRIPTOR_IF_ID_WRITE(radio_idx, descr_ptr);
            RDD_DHD_RX_POST_DESCRIPTOR_COMMON_HDR_FLAGS_WRITE(0, descr_ptr);
            RDD_DHD_RX_POST_DESCRIPTOR_EPOCH_WRITE(0, descr_ptr);
            RDD_DHD_RX_POST_DESCRIPTOR_REQUEST_ID_WRITE(req_id, descr_ptr);
            RDD_DHD_RX_POST_DESCRIPTOR_META_BUF_LEN_WRITE(0, descr_ptr);
            RDD_DHD_RX_POST_DESCRIPTOR_DATA_LEN_WRITE(len, descr_ptr);
            RDD_DHD_RX_POST_DESCRIPTOR_METADATA_BUF_ADDR_HI_WRITE(0, descr_ptr);
            RDD_DHD_RX_POST_DESCRIPTOR_METADATA_BUF_ADDR_LOW_WRITE(0, descr_ptr);
            RDD_DHD_RX_POST_DESCRIPTOR_DATA_BUF_ADDR_HI_WRITE(0, descr_ptr);
            RDD_DHD_RX_POST_DESCRIPTOR_DATA_BUF_ADDR_LOW_WRITE(data_buf_ptr_low_phys, descr_ptr);
            break;
        }

        case FR_FORMAT_WI_CWI32: /* Compact Work Item with 32b haddr */
        {
            RDD_DHD_RX_POST_DESCRIPTOR_CWI32_REQUEST_ID_WRITE(req_id, descr_ptr);
            RDD_DHD_RX_POST_DESCRIPTOR_CWI32_DATA_BUF_ADDR_LOW_WRITE(data_buf_ptr_low_phys, descr_ptr);
            break;
        }

        default:
        {
            bdmf_print("ERROR: flow ring format: %d not supported\n", flow_ring_format[radio_idx]);
            BUG();
        }
    } /* switch item_type */

#if !defined(FIRMWARE_INIT)
    if (valid_bpm)
        cache_flush_len((void *)descr_ptr, 32);
#endif
}

#if 0
static void rdd_rx_post_descr_dump(uint32_t desc)
{
    uint32_t req_id, buf_addr;
    uint16_t data_len;

    RDD_DHD_RX_POST_DESCRIPTOR_REQUEST_ID_READ(req_id, desc);
    RDD_DHD_RX_POST_DESCRIPTOR_DATA_LEN_READ(data_len, desc);
    RDD_DHD_RX_POST_DESCRIPTOR_DATA_BUF_ADDR_LOW_READ(buf_addr, desc);
}
#endif

int rdd_dhd_rx_post_init(uint32_t radio_idx, rdpa_dhd_init_cfg_t *init_cfg, uint32_t num_items)
{
    int rc = DRV_BPM_ERROR_NO_ERROR;
#if !defined(FIRMWARE_INIT)
    rdd_dhd_rx_post_ring_t *ring_info = &g_dhd_rx_post_ring_priv[radio_idx];
    uint32_t bpm_buffer_number;
    uint8_t *write_ptr;
    uint32_t i, rx_post_last_wr_idx;
    RDD_DHD_RADIO_INSTANCE_COMMON_B_DATA_DTS *radio_instance_data_ptr;
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_DTS *radio_instance_entry_ptr;


    for (i = 0, write_ptr = init_cfg->rx_post_flow_ring_base_addr; i < num_items; i++)
    {
#if defined(WL4908)
        uint32_t token;
        token = fpm_alloc_max_size_token_pool(1);
        if (token & FPM_TOKEN_VALID_MASK_4RDH)
            bpm_buffer_number = fpm_convert_fpm_token_to_rdp_token(token);
        else
            return DRV_BPM_ERROR_NO_FREE_BUFFER;
#else
        rc = fi_bl_drv_bpm_req_buffer(DRV_BPM_SP_SPARE_1, (uint32_t *)&bpm_buffer_number);
        if (rc != DRV_BPM_ERROR_NO_ERROR)
            return rc;
#endif
        rdd_rx_post_descr_init(radio_idx, write_ptr, bpm_buffer_number, 1);

        /* Suport for various RxPost Work Item Formats */
        switch (flow_ring_format[radio_idx])
        {
            case FR_FORMAT_WI_WI64: /* Legacy Work Item */
            {
                write_ptr += sizeof(RDD_DHD_RX_POST_DESCRIPTOR_DTS);
                break;
            }

            case FR_FORMAT_WI_CWI32: /* Compact Work Item with 32b haddr */
            {
                write_ptr += sizeof(RDD_DHD_RX_POST_DESCRIPTOR_CWI32_DTS);
                break;
            }

            default :
            {
                bdmf_print("ERROR: flow ring format: %d not supported\n", flow_ring_format[radio_idx]);
                BUG();
            }
        } /* switch item_type */

    }

    /* Update RX_post WR/RD index both in SRAM and DDR, doorbell will be send by DHD if needed */
    *ring_info->rd_idx_addr = 0;
    *ring_info->wr_idx_addr = cpu_to_le16(num_items);

#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
    /* for all the ARM based, little endian platform, we need to swap from little endian
     * to big endian, then the following "RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_RX_POST_R2D_INDEX_WRITE"
     * call will do another swapping but to little endian and write it... kind of redundant */
    rx_post_last_wr_idx = __swap2bytes(num_items);
#else
    rx_post_last_wr_idx = cpu_to_le16(num_items);
#endif

    radio_instance_data_ptr = (RDD_DHD_RADIO_INSTANCE_COMMON_B_DATA_DTS *)(DEVICE_ADDRESS(RUNNER_COMMON_1_OFFSET) +
        DHD_RADIO_INSTANCE_COMMON_B_DATA_ADDRESS - sizeof(RUNNER_COMMON));
    radio_instance_entry_ptr = &radio_instance_data_ptr->entry[radio_idx];

    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_RX_POST_R2D_INDEX_WRITE(rx_post_last_wr_idx, radio_instance_entry_ptr);

#endif // !defined(FIRMWARE_INIT)
    return rc;
}

int rdd_dhd_rx_post_uninit(uint32_t radio_idx, rdpa_dhd_init_cfg_t *init_cfg, uint32_t *num_items)
{
#if !defined(FIRMWARE_INIT)
    uint32_t bpm_buffer_number;
    uint8_t *descr_ptr;
    int rc = 0;
    uint16_t start, end;
    rdd_dhd_rx_post_ring_t *ring_info = &g_dhd_rx_post_ring_priv[radio_idx];

    /* the logic behind this is, there should always be
     * (DHD_RX_POST_FLOW_RING_SIZE - 1) buffers in RxPost ring, because Runner
     * allocates 1 back when it receives 1 in RxComplete.  (Wr_idx - 1) should
     * represent the last refilled buffer, and WRAP(wr_idx + 1) should be
     * the oldest refilled buffer in RxPost.  Therefore, we will free by
     * going from wr_idx + 1, toward wr_idx + 2, and on until it wraps
     * around and gets to (wr_idx - 1) */
    end = le16_to_cpu(*ring_info->wr_idx_addr);
    *num_items = 0;
    rmb();
    start = (end + 1) & (DHD_RX_POST_FLOW_RING_SIZE - 1);

    do {
        descr_ptr = init_cfg->rx_post_flow_ring_base_addr;

        /* Suport for various RxPost Work Item Formats */
        switch (flow_ring_format[radio_idx])
        {
            case FR_FORMAT_WI_WI64: /* Legacy Work Item */
            {
                descr_ptr += (sizeof(RDD_DHD_RX_POST_DESCRIPTOR_DTS) * start);
                RDD_DHD_RX_POST_DESCRIPTOR_REQUEST_ID_READ(bpm_buffer_number, descr_ptr);
                break;
            }

            case FR_FORMAT_WI_CWI32: /* Compact Work Item with 32b haddr */
            {
                descr_ptr += (sizeof(RDD_DHD_RX_POST_DESCRIPTOR_CWI32_DTS) * start);
                RDD_DHD_RX_POST_DESCRIPTOR_CWI32_REQUEST_ID_READ(bpm_buffer_number, descr_ptr);
                break;
            }

            default :
            {
                bdmf_print("ERROR: flow ring format: %d not supported\n", flow_ring_format[radio_idx]);
                BUG();
            }
        } /* switch item_type */

#if defined(WL4908)
        bpm_buffer_number &= ~(DRV_BPM_SP_SPARE_1 << 18);
        bpm_buffer_number = fpm_convert_rdp_token_to_fpm_token(bpm_buffer_number);
        fpm_free_token(bpm_buffer_number);
#elif defined(DSL_63138)
        bpm_buffer_number &= ~(DRV_BPM_SP_SPARE_1 << 15);
        rc = fi_bl_drv_bpm_free_buffer(DRV_BPM_SP_SPARE_1, bpm_buffer_number);
#elif defined(DSL_63148)
        bpm_buffer_number &= ~(DRV_BPM_SP_SPARE_1 << 14);
        rc = fi_bl_drv_bpm_free_buffer(DRV_BPM_SP_SPARE_1, bpm_buffer_number);
#else
        bpm_buffer_number &= ~(DRV_BPM_SP_SPARE_1 << 14);
        rc = fi_bl_drv_bpm_free_buffer(DRV_BPM_SP_SPARE_1, bpm_buffer_number);
#endif // defined(DSL_63138)

        if (rc)
            bdmf_trace("Error releasing BPM num %d, rc = %d\n", bpm_buffer_number, rc);

        (*num_items)++;
        start++;
        if (unlikely(start == DHD_RX_POST_FLOW_RING_SIZE))
            start = 0;

    } while (start != end);
#endif //  !defined(FIRMWARE_INIT)

    return 0;
}

int rdd_dhd_rx_post_reinit(uint32_t radio_idx, rdpa_dhd_init_cfg_t *init_cfg)
{
  uint32_t num_items;
  int rc = 0;

  /* First phase - just empty and refill the ring */
  rc = rdd_dhd_rx_post_uninit(radio_idx, init_cfg, &num_items);
  rc = rc ? rc: rdd_dhd_rx_post_init(radio_idx, init_cfg, num_items);

  return 0;
}

void rdd_dhd_mode_enable(uint32_t radio_idx, bdmf_boolean enable)
{
    is_dhd_enabled[radio_idx] = enable;
}

/* FIXME! will need to sync the design of low/high/excl threshold similar to XRDP.
 * Right now, we will just use low_th */
/* The tx post buffers thresholds are global for all radios. */
int rdd_dhd_helper_tx_thresholds_set(uint32_t low_th, uint32_t high_th, uint32_t excl_th)
{
#ifndef U_LINUX
    uint8_t *threshold_ptr = (uint8_t *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) + DHD_TX_POST_BUFFERS_THRESHOLD_ADDRESS);

#if defined(WL4908)
    MWRITE_32(threshold_ptr, low_th);
#else
    MWRITE_16(threshold_ptr, (uint16_t)low_th);
#endif
#endif
    return 0;
}

int rdd_dhd_helper_tx_thresholds_get(uint32_t *low_th, uint32_t *high_th, uint32_t *excl_th)
{
#ifndef U_LINUX
    uint8_t *threshold_ptr = (uint8_t *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) + DHD_TX_POST_BUFFERS_THRESHOLD_ADDRESS);
#if defined(WL4908)
    uint32_t threshold;

    MREAD_32(threshold_ptr, threshold);
#else
    uint16_t threshold;

    MREAD_16(threshold_ptr, threshold);
#endif

    *low_th = *high_th = *excl_th = (uint32_t)threshold;
#endif
    return 0;
}

int rdd_dhd_helper_tx_used_get(uint32_t *used)
{
#ifndef U_LINUX
    uint8_t *used_ptr = (uint8_t *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) + DHD_TX_POST_BUFFERS_IN_USE_COUNTER_ADDRESS);
#if defined(WL4908)
    uint32_t count;

    MREAD_32(used_ptr, count);
#else
    uint16_t count;

    MREAD_16(used_ptr, count);
#endif
    *used = (uint32_t)count;
#endif
    return 0;
}

int rdd_dhd_helper_cpu_tx_threshold_set(uint32_t threshold)
{
#if !defined(FIRMWARE_INIT) && !defined(RDD_BASIC)
    unsigned long flags;

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);

    if (g_cpu_tx_dhd_free_counter != g_cpu_tx_dhd_threshold)
    {
        bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
        return -1;
    }

    g_cpu_tx_dhd_free_counter = g_cpu_tx_dhd_threshold = threshold;
    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
#endif
    return 0;
}

int rdd_dhd_helper_cpu_tx_threshold_get(uint32_t *threshold)
{
    *threshold = g_cpu_tx_dhd_threshold;

    return 0;
}

int rdd_dhd_helper_cpu_tx_used_get(uint32_t *used)
{
    *used = g_cpu_tx_dhd_threshold - g_cpu_tx_dhd_free_counter;

    return 0;
}

static void rdd_dhd_helper_tx_post_buffers_threshold_init(void)
{
    uint32_t threshold;
    BPM_MODULE_REGS_BPM_GL_TRSH global_configuration;

    BPM_MODULE_REGS_BPM_GL_TRSH_READ(global_configuration);

    /* Get total number of BPM buffers. */
    if (global_configuration.gl_bat <= DRV_BPM_GLOBAL_THRESHOLD_30K)
        threshold = (global_configuration.gl_bat + 1) * 2560;
    else
        threshold = 2560;

    /* Set tx post buffers threshold to half of the total number of BPM buffers. */
    threshold /= 2;
    rdd_dhd_helper_tx_thresholds_set(threshold, threshold, threshold);
}

static void rdd_dhd_helper_tx_post_cpu_buffers_threshold_init(void)
{
    /* The tx post cpu buffers threshold is global for all radios, and it's set to
     * 3/4 of the total available of absolute packet limit */
    rdd_dhd_helper_cpu_tx_threshold_set((LILAC_RDD_CPU_TX_SKB_LIMIT_DEFAULT >> 2) * 3);
}

void rdd_dhd_mode_enable_init(void)
{
    uint32_t radio_idx;

    for (radio_idx = 0; radio_idx < RDPA_MAX_RADIOS; radio_idx++)
        is_dhd_enabled[radio_idx] = (bdmf_boolean) 0;

    rdd_timer_task_config(rdpa_dir_ds, TIMER_SCHEDULER_TASK_PERIOD, DOWNSTREAM_DHD_TX_POST_CLOSE_AGGREGATION_CODE_ID);

    rdd_dhd_helper_tx_post_buffers_threshold_init();
    rdd_dhd_helper_tx_post_cpu_buffers_threshold_init();
}

static void rdd_dhd_tx_post_descr_init(uint32_t *descr_addr, uint8_t flow_ring_format)
{
    switch (flow_ring_format)
    {
        case FR_FORMAT_WI_WI64: /* Legacy Work Item */
        {
            RDD_DHD_TX_POST_DESCRIPTOR_DTS *descr = (RDD_DHD_TX_POST_DESCRIPTOR_DTS *)descr_addr;
            RDD_DHD_TX_POST_DESCRIPTOR_MSG_TYPE_WRITE(DHD_MSG_TYPE_TX_POST, descr);
            RDD_DHD_TX_POST_DESCRIPTOR_COMMON_HDR_FLAGS_WRITE(0, descr);
            RDD_DHD_TX_POST_DESCRIPTOR_EPOCH_WRITE(0, descr);
            RDD_DHD_TX_POST_DESCRIPTOR_SEG_CNT_WRITE(1, descr);
            RDD_DHD_TX_POST_DESCRIPTOR_METADATA_BUF_ADDR_HI_WRITE(0, descr);
            RDD_DHD_TX_POST_DESCRIPTOR_METADATA_BUF_ADDR_LOW_WRITE(0, descr);
            RDD_DHD_TX_POST_DESCRIPTOR_META_BUF_LEN_WRITE(0, descr);
            RDD_DHD_TX_POST_DESCRIPTOR_DATA_BUF_ADDR_HI_WRITE(0, descr);
            break;
        }

        case FR_FORMAT_WI_CWI32: /* Compact Work Item with 32b haddr */
        {
            RDD_DHD_TX_POST_DESCRIPTOR_CWI32_DTS *descr = (RDD_DHD_TX_POST_DESCRIPTOR_CWI32_DTS *)descr_addr;
            RDD_DHD_TX_POST_DESCRIPTOR_CWI32_REQUEST_ID_WRITE(0, descr);
            RDD_DHD_TX_POST_DESCRIPTOR_CWI32_DATA_BUF_ADDR_LOW_WRITE(0, descr);
            RDD_DHD_TX_POST_DESCRIPTOR_CWI32_PRIO_WRITE(0, descr);
            RDD_DHD_TX_POST_DESCRIPTOR_CWI32_IF_ID_WRITE(0, descr);
            RDD_DHD_TX_POST_DESCRIPTOR_CWI32_FLAGS_WRITE(0, descr);
            RDD_DHD_TX_POST_DESCRIPTOR_CWI32_COPY_WRITE(0, descr);
            RDD_DHD_TX_POST_DESCRIPTOR_CWI32_DATA_LEN_WRITE(0, descr);
            RDD_DHD_TX_POST_DESCRIPTOR_CWI32_FLOWID_OVERRIDE_WRITE(0, descr);
            RDD_DHD_TX_POST_DESCRIPTOR_CWI32_INFO_WRITE(0, descr);
            break;
        }

        default:
        {
            bdmf_print("ERROR: flow ring format: %d not supported\n", flow_ring_format);

            BUG();
        }
    } /* switch item_type */
}

#if !defined(FIRMWARE_INIT)
#define DHD_VTOP(val) (uint32_t)VIRT_TO_PHYS(val)
#else
#define DHD_VTOP(val) (uint32_t)val
#endif

int rdd_dhd_hlp_cfg(uint32_t radio_idx, rdpa_dhd_init_cfg_t *init_cfg, int enable)
{
    RDD_DHD_RADIO_INSTANCE_COMMON_A_DATA_DTS *radio_instance_data_a_ptr;
    RDD_DHD_RADIO_INSTANCE_COMMON_A_ENTRY_DTS *radio_instance_entry_a_ptr;
    RDD_DHD_RADIO_INSTANCE_COMMON_B_DATA_DTS *radio_instance_data_ptr;
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_DTS *radio_instance_entry_ptr;
    RDD_DHD_RX_POST_FLOW_RING_BUFFER_DTS *rxp_flring_buffer_ptr;
    RDD_DHD_RX_POST_DESCRIPTOR_DTS *rxp_flring_entry_ptr;
    RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY_DTS *ds_lkp_table_ptr;
    RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY_DTS *us_lkp_table_ptr;
    uint32_t i, rx_post_last_wr_idx;

#if 0 /* not used */
    uintptr_t rx_post_ptr;
#endif

    radio_instance_data_ptr = (RDD_DHD_RADIO_INSTANCE_COMMON_B_DATA_DTS *)(DEVICE_ADDRESS(RUNNER_COMMON_1_OFFSET) +
        DHD_RADIO_INSTANCE_COMMON_B_DATA_ADDRESS - sizeof(RUNNER_COMMON));
    radio_instance_entry_ptr = &radio_instance_data_ptr->entry[radio_idx];

    /* Invalidate lkp entries*/
    for (i = 0; i < RDD_DS_DHD_FLOW_RING_CACHE_LKP_TABLE_SIZE; i++)
    {
        ds_lkp_table_ptr = (RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY_DTS *)RDD_DS_DHD_FLOW_RING_CACHE_LKP_TABLE_PTR() + i;
        us_lkp_table_ptr = (RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY_DTS *)RDD_US_DHD_FLOW_RING_CACHE_LKP_TABLE_PTR() + i;
        RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY_INVALID_WRITE(1 , ds_lkp_table_ptr);
        RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY_INVALID_WRITE(1 , us_lkp_table_ptr);
    }

    /* setting up default value for packet aggregation related configurations */
    for (i = 0; i < RDPA_MAX_AC; i++) {
        RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_PER_AC_AGGREGATION_THRESHOLDS_WRITE(1, radio_instance_entry_ptr, i);
        rdd_dhd_helper_aggregation_timeout_set(radio_idx, i, (1 << i));  /* values get overwritten by DHD driver */
    }
    rdd_dhd_helper_aggregation_bypass_cpu_tx_set(radio_idx, 1);
    rdd_dhd_helper_aggregation_bypass_non_udp_tcp_set(radio_idx, 1);
    rdd_dhd_helper_aggregation_bypass_tcp_pktlen_set(radio_idx, 64);

    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_RX_POST_FR_BASE_PTR_WRITE(DHD_VTOP(init_cfg->rx_post_flow_ring_base_addr), radio_instance_entry_ptr);
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_RX_COMPLETE_FR_BASE_PTR_WRITE(DHD_VTOP(init_cfg->rx_complete_flow_ring_base_addr), radio_instance_entry_ptr);
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_TX_COMPLETE_FR_BASE_PTR_WRITE(DHD_VTOP(init_cfg->tx_complete_flow_ring_base_addr), radio_instance_entry_ptr);

#if !defined FIRMWARE_INIT && (defined(DSL_63138) || defined(DSL_63148) || defined(WL4908))
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_R2D_WR_FR_DESC_BASE_PTR_WRITE(init_cfg->r2d_wr_arr_base_phys_addr, radio_instance_entry_ptr);
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_D2R_RD_FR_DESC_BASE_PTR_WRITE(init_cfg->d2r_rd_arr_base_phys_addr, radio_instance_entry_ptr);
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_R2D_RD_FR_DESC_BASE_PTR_WRITE(init_cfg->r2d_rd_arr_base_phys_addr, radio_instance_entry_ptr);
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_D2R_WR_FR_DESC_BASE_PTR_WRITE(init_cfg->d2r_wr_arr_base_phys_addr, radio_instance_entry_ptr);
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_TX_POST_MGMT_FR_BASE_PTR_WRITE(init_cfg->tx_post_mgmt_arr_base_phys_addr, radio_instance_entry_ptr);
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_FLOW_RING_FORMAT_WRITE(init_cfg->flow_ring_format, radio_instance_entry_ptr);

    /* Used by rdd_dhd_helper_shell.c. In order to support both 64 bit and 32 bit host, we have to do this type of casting */
    g_dhd_tx_post_mgmt_fr_base_ptr[radio_idx] = (void *)init_cfg->tx_post_mgmt_arr_base_addr;
#else
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_R2D_WR_FR_DESC_BASE_PTR_WRITE(DHD_VTOP(init_cfg->r2d_wr_arr_base_addr), radio_instance_entry_ptr);
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_D2R_RD_FR_DESC_BASE_PTR_WRITE(DHD_VTOP(init_cfg->d2r_rd_arr_base_addr), radio_instance_entry_ptr);
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_R2D_RD_FR_DESC_BASE_PTR_WRITE(DHD_VTOP(init_cfg->r2d_rd_arr_base_addr), radio_instance_entry_ptr);
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_D2R_WR_FR_DESC_BASE_PTR_WRITE(DHD_VTOP(init_cfg->d2r_wr_arr_base_addr), radio_instance_entry_ptr);
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_TX_POST_MGMT_FR_BASE_PTR_WRITE(DHD_VTOP(init_cfg->tx_post_mgmt_arr_base_addr), radio_instance_entry_ptr);
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_FLOW_RING_FORMAT_WRITE(DHD_VTOP(init_cfg->flow_ring_format), radio_instance_entry_ptr);
#endif

#if !defined(FIRMWARE_INIT)
    g_dhd_rx_post_ring_priv[radio_idx].wr_idx_addr = (uint16_t *)init_cfg->r2d_wr_arr_base_addr + 1;
    g_dhd_rx_post_ring_priv[radio_idx].rd_idx_addr = (uint16_t *)init_cfg->r2d_rd_arr_base_addr + 1;
#endif

    rdd_dhd_tx_post_descr_init((uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_DHD_TX_POST_FLOW_RING_BUFFER_ADDRESS), init_cfg->flow_ring_format);
    rdd_dhd_tx_post_descr_init((uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_DHD_TX_POST_FLOW_RING_BUFFER_ADDRESS), init_cfg->flow_ring_format);

    rxp_flring_buffer_ptr = (RDD_DHD_RX_POST_FLOW_RING_BUFFER_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + DHD_RX_POST_FLOW_RING_BUFFER_ADDRESS);
    rxp_flring_entry_ptr =  &rxp_flring_buffer_ptr->entry[radio_idx].dhd_rx_post_descriptor;
    rdd_rx_post_descr_init(radio_idx, (uint8_t*)rxp_flring_entry_ptr, 0, 0);

    rdd_dhd_mode_enable(radio_idx, enable);

    /* Update RX_post WR index both in SRAM and DDR. No need doorbell DHD that there are buffers available in RX Post.*/
#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
    /* for all the ARM based, little endian platform, we need to swap from little endian
     * to big endian, then the following "RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_RX_POST_R2D_INDEX_WRITE"
     * call will do another swapping but to little endian and write it... kind of redundant */
    rx_post_last_wr_idx = __swap2bytes(DHD_RX_POST_FLOW_RING_SIZE - 1);
#else
    rx_post_last_wr_idx = cpu_to_le16(DHD_RX_POST_FLOW_RING_SIZE - 1);
#endif
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_RX_POST_R2D_INDEX_WRITE(rx_post_last_wr_idx, radio_instance_entry_ptr);

    radio_instance_data_a_ptr = (RDD_DHD_RADIO_INSTANCE_COMMON_A_DATA_DTS *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) + DHD_RADIO_INSTANCE_COMMON_A_DATA_ADDRESS);
    radio_instance_entry_a_ptr = &radio_instance_data_a_ptr->entry[radio_idx];

    RDD_DHD_RADIO_INSTANCE_COMMON_A_ENTRY_DS_DHD_DOORBELL_POST_WRITE(init_cfg->dongle_wakeup_register, radio_instance_entry_a_ptr);
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_US_DHD_DOORBELL_POST_WRITE(init_cfg->dongle_wakeup_register, radio_instance_entry_ptr);

#ifdef RDPA_DHD_HELPER_FEATURE_HWA_WAKEUP_SUPPORT
    RDD_DHD_RADIO_INSTANCE_COMMON_A_ENTRY_IDMA_ACTIVE_WRITE(init_cfg->dongle_wakeup_hwa, radio_instance_entry_a_ptr);
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_IDMA_ACTIVE_WRITE(init_cfg->dongle_wakeup_hwa, radio_instance_entry_ptr);

    RDD_DHD_RADIO_INSTANCE_COMMON_A_ENTRY_DS_DHD_DOORBELL_COMPLETE_WRITE(init_cfg->dongle_wakeup_register_2, radio_instance_entry_a_ptr);
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_US_DHD_DOORBELL_COMPLETE_WRITE(init_cfg->dongle_wakeup_register_2, radio_instance_entry_ptr);
#else
    RDD_DHD_RADIO_INSTANCE_COMMON_A_ENTRY_IDMA_ACTIVE_WRITE(0, radio_instance_entry_a_ptr);
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_IDMA_ACTIVE_WRITE(0, radio_instance_entry_ptr);

    RDD_DHD_RADIO_INSTANCE_COMMON_A_ENTRY_DS_DHD_DOORBELL_COMPLETE_WRITE(init_cfg->dongle_wakeup_register, radio_instance_entry_a_ptr);
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_US_DHD_DOORBELL_COMPLETE_WRITE(init_cfg->dongle_wakeup_register, radio_instance_entry_ptr);
#endif

    RDD_DHD_RADIO_INSTANCE_COMMON_A_ENTRY_FLOW_RING_FORMAT_WRITE(init_cfg->flow_ring_format, radio_instance_entry_a_ptr);
    if (!enable)
    {
        /* Reset the rest of the fields */
        RDD_DHD_RADIO_INSTANCE_COMMON_A_ENTRY_DS_RD_FR_R2D_INDEXES_WRITE(0, radio_instance_entry_a_ptr);
        RDD_DHD_RADIO_INSTANCE_COMMON_A_ENTRY_DS_WR_FR_R2D_INDEXES_WRITE(0, radio_instance_entry_a_ptr);
        RDD_DHD_RADIO_INSTANCE_COMMON_A_ENTRY_TX_RD_FR_D2R_INDEXES_WRITE(0, radio_instance_entry_a_ptr);
        RDD_DHD_RADIO_INSTANCE_COMMON_A_ENTRY_TX_WR_FR_D2R_INDEXES_WRITE(0, radio_instance_entry_a_ptr);
        RDD_DHD_RADIO_INSTANCE_COMMON_A_ENTRY_TX_COMPLETE_PACKET_COUNTER_WRITE(0, radio_instance_entry_a_ptr);
        RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_RX_RD_FR_D2R_INDEXES_WRITE(0, radio_instance_entry_ptr);
        RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_RX_WR_FR_D2R_INDEXES_WRITE(0, radio_instance_entry_ptr);
    }

    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_ADD_LLCSNAP_HEADER_WRITE(init_cfg->add_llcsnap_header, radio_instance_entry_ptr);

#if 0
    for (i = 0, rx_post_ptr = (uintptr_t)init_cfg->rx_post_flow_ring_base_addr; enable && i < DHD_RX_POST_FLOW_RING_SIZE; i++)
    {
        rdd_rx_post_descr_dump(rx_post_ptr);
        rx_post_ptr += sizeof(RDD_DHD_RX_POST_DESCRIPTOR_DTS);
    }
#endif

    return 0;
}

void rdd_dhd_helper_flow_ring_flush(uint32_t radio_idx, uint32_t read_idx_flow_ring_idx)
{
    unsigned long flags;
    rdpa_dhd_ffd_data_t  params = (rdpa_dhd_ffd_data_t)read_idx_flow_ring_idx;

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);
    rdd_cpu_tx_send_message(RDD_CPU_TX_MESSAGE_DHD_MESSAGE, PICO_RUNNER_A, RUNNER_PRIVATE_0_OFFSET,
        DHD_MSG_TYPE_FLOW_RING_FLUSH, params.flowring_idx | (radio_idx << 14) | (params.read_idx << 16) | (params.read_idx_valid << 31), 0, 1);
    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
}

void rdd_dhd_helper_flow_ring_disable(uint32_t radio_idx, uint32_t flow_ring_idx)
{
    unsigned long flags;

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);
    rdd_cpu_tx_send_message(RDD_CPU_TX_MESSAGE_DHD_MESSAGE, PICO_RUNNER_A, RUNNER_PRIVATE_0_OFFSET,
        DHD_MSG_TYPE_FLOW_RING_SET_DISABLED, flow_ring_idx | (radio_idx << 14), 0, BL_LILAC_RDD_WAIT);
    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
}

void rdd_dhd_helper_wakeup_information_get(rdpa_dhd_wakeup_info_t *wakeup_info)
{
    RUNNER_REGS_CFG_CPU_WAKEUP runner_cpu_wakeup_register;

#if defined(USE_SOC_BASE_ADDR)
    wakeup_info->tx_complete_wakeup_register = RUNNER_REGS_0_CFG_CPU_WAKEUP_ADDRESS + RDP_PHYS_BASE;
#else
#if (defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)) && !defined(FIRMWARE_INIT)
    wakeup_info->tx_complete_wakeup_register = (uint32_t)(RUNNER_REGS_0_CFG_CPU_WAKEUP_ADDRESS - RDP_BASE + RDP_PHYS_BASE);
#else
    wakeup_info->tx_complete_wakeup_register = RUNNER_REGS_0_CFG_CPU_WAKEUP_ADDRESS;
#endif
#endif

    runner_cpu_wakeup_register.req_trgt = (DHD_TX_COMPLETE_FAST_A_THREAD_NUMBER + wakeup_info->radio_idx) >> 5;
    runner_cpu_wakeup_register.thread_num = (DHD_TX_COMPLETE_FAST_A_THREAD_NUMBER + wakeup_info->radio_idx) & 0x1f;
    runner_cpu_wakeup_register.urgent_req = 0;
    runner_cpu_wakeup_register.reserved0 = 0;
    MWRITE_32(&wakeup_info->tx_complete_wakeup_value, *(uint32_t *)&runner_cpu_wakeup_register);

#if defined(USE_SOC_BASE_ADDR)
    wakeup_info->rx_complete_wakeup_register = RUNNER_REGS_1_CFG_CPU_WAKEUP_ADDRESS + RDP_PHYS_BASE;
#else
#if (defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)) && !defined(FIRMWARE_INIT)
    wakeup_info->rx_complete_wakeup_register = (uint32_t)(RUNNER_REGS_1_CFG_CPU_WAKEUP_ADDRESS - RDP_BASE + RDP_PHYS_BASE);
#else
    wakeup_info->rx_complete_wakeup_register = RUNNER_REGS_1_CFG_CPU_WAKEUP_ADDRESS;
#endif
#endif

    runner_cpu_wakeup_register.req_trgt = (DHD_RX_THREAD_NUMBER + wakeup_info->radio_idx) >> 5;
    runner_cpu_wakeup_register.thread_num = (DHD_RX_THREAD_NUMBER + wakeup_info->radio_idx) & 0x1f;
    runner_cpu_wakeup_register.urgent_req = 0;
    runner_cpu_wakeup_register.reserved0 = 0;
    MWRITE_32(&wakeup_info->rx_complete_wakeup_value, *(uint32_t *)&runner_cpu_wakeup_register);
}

int rdd_dhd_helper_dhd_complete_ring_create(uint32_t radio_idx, uint32_t ring_size)
{
    int rc = 0;
#if !defined(FIRMWARE_INIT)
    RDD_DHD_COMPLETE_RING_DESCRIPTOR_BUFFER_DTS *dhd_complete_desc_buffer_ptr;
    RDD_DHD_COMPLETE_RING_DESCRIPTOR_DTS *dhd_complete_desc_ptr;
    rdd_dhd_complete_ring_descriptor_t *pdesc = &g_dhd_complete_ring_desc[radio_idx];
    uint32_t i;
    bdmf_phys_addr_t phy_addr;
    uint8_t *ring_ptr;

    if (ring_size)
    {
        /* create an array of ring elements */
        if (pdesc->ring_base == 0)
        {
            pdesc->ring_base = rdp_mm_aligned_alloc(sizeof(RDD_DHD_COMPLETE_RING_ENTRY_DTS) * ring_size, &phy_addr);
            if (pdesc->ring_base)
            {
                /* Initialize RDD descriptor */
                pdesc->ring_size = ring_size;
                pdesc->ring_end = pdesc->ring_base + ((ring_size - 1) * sizeof(RDD_DHD_COMPLETE_RING_ENTRY_DTS));
                pdesc->ring_ptr = pdesc->ring_base;

                /* Initialize firmware descriptor */
                dhd_complete_desc_buffer_ptr = RDD_DHD_COMPLETE_RING_DESCRIPTOR_BUFFER_PTR();
                dhd_complete_desc_ptr = &dhd_complete_desc_buffer_ptr->entry[radio_idx];

                RDD_DHD_COMPLETE_RING_DESCRIPTOR_RING_SIZE_WRITE(ring_size, dhd_complete_desc_ptr);
                RDD_DHD_COMPLETE_RING_DESCRIPTOR_RING_BASE_WRITE(phy_addr, dhd_complete_desc_ptr);
                RDD_DHD_COMPLETE_RING_DESCRIPTOR_RING_END_WRITE(phy_addr + ((ring_size - 1) * sizeof(RDD_DHD_COMPLETE_RING_ENTRY_DTS)), dhd_complete_desc_ptr);
                RDD_DHD_COMPLETE_RING_DESCRIPTOR_RING_PTR_WRITE(phy_addr, dhd_complete_desc_ptr);

                /* Initialize the ring elements to be owned by Runner */
                for (i = 0, ring_ptr = pdesc->ring_ptr; i < ring_size; i++, ring_ptr += sizeof(RDD_DHD_COMPLETE_RING_ENTRY_DTS))
                {
                    RDD_DHD_COMPLETE_RING_ENTRY_RING_VALUE_WRITE(0, ring_ptr);
                    RDD_DHD_COMPLETE_RING_ENTRY_OWNERSHIP_WRITE(DHD_COMPLETE_OWNERSHIP_RUNNER, ring_ptr);
                }
            }
            else
                rc = BDMF_ERR_NOMEM;
        }
        else
            rc = BDMF_ERR_ALREADY;
    }
#endif

    return rc;
}

int rdd_dhd_helper_dhd_complete_ring_destroy(uint32_t radio_idx, uint32_t ring_size)
{
    int rc = 0;
#if !defined(FIRMWARE_INIT)
    RDD_DHD_COMPLETE_RING_DESCRIPTOR_BUFFER_DTS *dhd_complete_desc_buffer_ptr;
    RDD_DHD_COMPLETE_RING_DESCRIPTOR_DTS *dhd_complete_desc_ptr;
    rdd_dhd_complete_ring_descriptor_t *pdesc = &g_dhd_complete_ring_desc[radio_idx];

    if (ring_size)
    {
        /* create an array of ring elements */
        if (pdesc->ring_base)
        {
            rdp_mm_aligned_free((void *)pdesc->ring_base, sizeof(RDD_DHD_COMPLETE_RING_ENTRY_DTS) * ring_size);
            pdesc->ring_size = 0;
            pdesc->ring_base = 0;
            pdesc->ring_end = 0;
            pdesc->ring_ptr = 0;

            dhd_complete_desc_buffer_ptr = RDD_DHD_COMPLETE_RING_DESCRIPTOR_BUFFER_PTR();
            dhd_complete_desc_ptr = &dhd_complete_desc_buffer_ptr->entry[radio_idx];

            RDD_DHD_COMPLETE_RING_DESCRIPTOR_RING_SIZE_WRITE(0, dhd_complete_desc_ptr);
            RDD_DHD_COMPLETE_RING_DESCRIPTOR_RING_BASE_WRITE(0, dhd_complete_desc_ptr);
            RDD_DHD_COMPLETE_RING_DESCRIPTOR_RING_END_WRITE(0, dhd_complete_desc_ptr);
            RDD_DHD_COMPLETE_RING_DESCRIPTOR_RING_PTR_WRITE(0, dhd_complete_desc_ptr);
        }
        else
            rc = BDMF_ERR_ALREADY;
    }
#endif

    return rc;
}

uint16_t rdd_dhd_helper_ssid_tx_dropped_packets_get(uint32_t radio_idx, uint32_t ssid)
{
    uint16_t counter;

#if defined DSL_63138 || defined DSL_63148 || defined(WL4908)
    rdd_2_bytes_counter_get(DHD_SSID_DROP_PACKET_GROUP + radio_idx, ssid, &counter);
#else
    rdd_2_bytes_counter_get(DHD_SSID_DROP_PACKET_GROUP + radio_idx, ssid, 0, &counter);
#endif

    return counter;
}
#endif

static RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_DTS* get_radio_instance_data_ptr(uint32_t radio_idx)
{
    RDD_DHD_RADIO_INSTANCE_COMMON_B_DATA_DTS *radio_instance_data_ptr;

    radio_instance_data_ptr = (RDD_DHD_RADIO_INSTANCE_COMMON_B_DATA_DTS *)(DEVICE_ADDRESS(RUNNER_COMMON_1_OFFSET) +
        DHD_RADIO_INSTANCE_COMMON_B_DATA_ADDRESS - sizeof(RUNNER_COMMON));

    return (&radio_instance_data_ptr->entry[radio_idx]);
}

int rdd_dhd_helper_aggregation_timeout_set(uint32_t radio_idx, int access_category, uint8_t aggregation_timeout)
{
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_PER_AC_AGGREGATION_TIMEOUTS_WRITE(
        aggregation_timeout, get_radio_instance_data_ptr(radio_idx), access_category);
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_PER_AC_AGGREGATION_TIMEOUT_CNTRS_WRITE(
        0, get_radio_instance_data_ptr(radio_idx), access_category);

    return 0;
}

int rdd_dhd_helper_aggregation_timeout_get(uint32_t radio_idx, int access_category, uint8_t *aggregation_timeout)
{
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_PER_AC_AGGREGATION_TIMEOUTS_READ(
        *aggregation_timeout, get_radio_instance_data_ptr(radio_idx), access_category);

    return 0;
}

int rdd_dhd_helper_aggregation_size_set(uint32_t radio_idx, int access_category, uint8_t aggregation_size)
{
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_PER_AC_AGGREGATION_THRESHOLDS_WRITE(
        aggregation_size, get_radio_instance_data_ptr(radio_idx), access_category);

    return 0;
}

int rdd_dhd_helper_aggregation_size_get(uint32_t radio_idx, int access_category, uint8_t *aggregation_size)
{
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_PER_AC_AGGREGATION_THRESHOLDS_READ(
        *aggregation_size, get_radio_instance_data_ptr(radio_idx), access_category);

    return 0;
}

int rdd_dhd_helper_aggregation_bypass_cpu_tx_set(uint32_t radio_idx, bdmf_boolean enable)
{
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_AGGREGATION_BYPASS_CPU_TX_WRITE(
        enable, get_radio_instance_data_ptr(radio_idx));

    return 0;
}

int rdd_dhd_helper_aggregation_bypass_cpu_tx_get(uint32_t radio_idx, bdmf_boolean *enable)
{
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_AGGREGATION_BYPASS_CPU_TX_READ(
        *enable, get_radio_instance_data_ptr(radio_idx));

    return 0;
}

int rdd_dhd_helper_aggregation_bypass_non_udp_tcp_set(uint32_t radio_idx, bdmf_boolean enable)
{
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_AGGREGATION_BYPASS_NON_UDP_TCP_WRITE(
        enable, get_radio_instance_data_ptr(radio_idx));

    return 0;
}

int rdd_dhd_helper_aggregation_bypass_non_udp_tcp_get(uint32_t radio_idx, bdmf_boolean *enable)
{
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_AGGREGATION_BYPASS_NON_UDP_TCP_READ(
        *enable, get_radio_instance_data_ptr(radio_idx));

    return 0;
}

int rdd_dhd_helper_aggregation_bypass_tcp_pktlen_set(uint32_t radio_idx, uint8_t pkt_len)
{
    /* dhd TX packet doesn't have eth header, so we are dealing with that 14 byte subtraction here */
    if (pkt_len < (DHD_TX_POST_PKT_AGGR_TCP_LEN_MIN + 14))
        pkt_len = 0;
    else
        pkt_len -= 14;

    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_AGGREGATION_BYPASS_TCP_PKTLEN_WRITE(
        pkt_len, get_radio_instance_data_ptr(radio_idx));

    return 0;
}

int rdd_dhd_helper_aggregation_bypass_tcp_pktlen_get(uint32_t radio_idx, uint8_t *pkt_len)
{
    uint8_t pkt_local;
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_AGGREGATION_BYPASS_TCP_PKTLEN_READ(
        pkt_local, get_radio_instance_data_ptr(radio_idx));

    if (pkt_local == 0)
        *pkt_len = 0;
    else
        *pkt_len = pkt_local + 14;
    return 0;
}

#endif
