/*
* <:copyright-BRCM:2014:proprietary:standard
* 
*    Copyright (c) 2014 Broadcom 
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
* :>
*/


#include "bdmf_errno.h"
#include "rdd.h"
#include "XRDP_AG.h"
#include "rdd_spdsvc.h"
#include "rdp_drv_rnr.h"
#include "rdp_drv_fpm.h"
#include "rdd_cpu_tx.h"

#if !defined(G9991)
#include "rdp_drv_dhd.h"
#endif

#ifndef _CFE_

#ifdef CONFIG_BCM_SPDSVC_SUPPORT

/*
 * rdd_spdsvc.c
 */

#define SPDSVC_TIMER_PERIOD                                             100 /* usec */
#define SPDSVC_TIMER_HZ                                                 ( 1000000 / SPDSVC_TIMER_PERIOD ) /* sec */
#define SPDSVC_ETH_IFG                                                  20 /* bytes */
#define SPDSVC_ETH_CRC_LEN                                              4  /* bytes */
#define SPDSVC_ETH_OVERHEAD                                             (SPDSVC_ETH_CRC_LEN + SPDSVC_ETH_IFG) /* bytes */
                                                                        /* Ethernet packet + 2 VLAN Tags + PPPoE + Overhead */
#define SPDSVC_BUCKET_SIZE_MIN                                          (1514 + 8 + 8 + SPDSVC_ETH_OVERHEAD) /* bytes */

#define SPDSVC_TIMEOUT_MS_BEFORE_RELEASE                                 2000
#define SPDSVC_TIMEOUT_MS_AFTER_TX                                       20 /* number of timer iterations left after TX task queued PD to BBH_TX */

#if defined(BCM63158)
#define US_TM_CORE                                                      1
#define DS_TM_CORE                                                      0
#define SPDSVC_CLEANUP_TIMER_DURATION_MS                                10
#define SPDSVC_ITERS_AFTER_TX                                           (SPDSVC_TIMEOUT_MS_AFTER_TX/SPDSVC_CLEANUP_TIMER_DURATION_MS)
#else
#define SPDSVC_CLEANUP_TIMER_DURATION_MS                                1000
#endif

#define SPDSVC_ITERS_BEFORE_RELEASE                                     (SPDSVC_TIMEOUT_MS_BEFORE_RELEASE/SPDSVC_CLEANUP_TIMER_DURATION_MS)


#if !defined(G9991)
#define SPDSVC_DHD_GEN_ENABLE                                           1
#else
#define SPDSVC_DHD_GEN_ENABLE                                           0
#endif

/* Uncomment the following line to enable debug prints */
/* #define SPDSVC_DEBUG */
#ifdef SPDSVC_DEBUG
#define SPDSVC_DBG(fmt, args...)   bdmf_print("%s#%d " fmt, __FUNCTION__, __LINE__, ## args)
#else
#define SPDSVC_DBG(fmt, args...)
#endif

#define CMD_STREAM                   "STREAM........."
#define CMD_STREAM_LENGTH            15

/* Speed service control block */
static struct spdsvc_control_block
{
    uint32_t kbps;
    uint32_t mbs;
    uint32_t copies;
    uint32_t total_length;
    bdmf_timer_t timer;         /* Timer used to release sysb when generation is finished */
    pbuf_t pbuf;
    int iters_before_release;   /* Number of timer iterations left before sysb release after copies becomes 0 (queued to QM) */
    int gen_initialized;
    int radio_idx;
    RDD_DHD_CPU_QM_DESCRIPTOR_DTS dhd_pd;

#if defined(BCM63158)
    // abs tx counters of each interface
    uint8_t  ae2p5_tx_abs;
    uint8_t  ae10_tx_abs;
    uint32_t dsl_tx_abs;
    uint32_t pon_tx_abs;
    uint32_t ds_tx_abs;
#endif

} gen_control_block;

static inline uint32_t _rdd_spdsvc_kbps_to_tokens(uint32_t xi_kbps)
{
    return ( uint32_t )( (1000 * ((xi_kbps + 7) / 8)) / SPDSVC_TIMER_HZ );
}

static inline uint32_t _rdd_spdsvc_mbs_to_bucket_size(uint32_t xi_mbs)
{
    uint32_t bucket_size = xi_mbs;

    if(bucket_size < SPDSVC_BUCKET_SIZE_MIN)
        bucket_size = SPDSVC_BUCKET_SIZE_MIN;

    return bucket_size;
}

static inline int _rdd_spdsvc_is_running(void)
{
    return (gen_control_block.gen_initialized && ((gen_control_block.pbuf.length != 0) || gen_control_block.radio_idx >= 0));
}


/* Timer that checks whether generation has finished and
 * releases sysb when yes.
 */
static void _rdd_spdsvc_gen_timer_cb(bdmf_timer_t *timer, unsigned long priv)
{
    uint32_t copies, contd = 1;
#if SPDSVC_DHD_GEN_ENABLE
    uint32_t tx_complete, post_drop;
#endif

    if (gen_control_block.radio_idx < 0 && !gen_control_block.pbuf.length)
        return;

    RDD_SPDSVC_GEN_PARAMS_TOTAL_COPIES_READ_G(copies, RDD_SPDSVC_GEN_PARAMS_TABLE_ADDRESS_ARR, 0);

    /* Generation still running ? */
    if (copies)
    {
        bdmf_timer_start(timer, bdmf_ms_to_ticks(SPDSVC_CLEANUP_TIMER_DURATION_MS));
        return;
    }

    /* all packets generation has been queued to TM, now track when the transmission is done */
    /* transmission tracking is only for non WIFI DHD interfaces and is tracked on BCM63158 */
#if defined(BCM63158)
    if (gen_control_block.pbuf.length)
    {
        uint32_t tx_packets, us_tx, ds_tx;
        uint32_t tx_dropped;
      
        uint32_t i, tx_abs, bbh_id;

        RDD_US_TM_DSL_BBH_TX_ABS_COUNTER_TABLE_DTS *dsl_tx_abs = RDD_US_TM_DSL_BBH_TX_ABS_COUNTER_TABLE_PTR(US_TM_CORE);
        RDD_US_TM_PON_BBH_TX_ABS_COUNTER_TABLE_DTS *pon_tx_abs = RDD_US_TM_PON_BBH_TX_ABS_COUNTER_TABLE_PTR(US_TM_CORE);
        RDD_US_TM_AE10_BBH_TX_ABS_COUNTER_TABLE_DTS *ae10_tx_abs = RDD_US_TM_AE10_BBH_TX_ABS_COUNTER_TABLE_PTR(US_TM_CORE);
        RDD_US_TM_AE2P5_BBH_TX_ABS_COUNTER_TABLE_DTS *ae2p5_tx_abs = RDD_US_TM_AE2P5_BBH_TX_ABS_COUNTER_TABLE_PTR(US_TM_CORE);
        RDD_DS_TM_BBH_TX_ABS_COUNTER_TABLE_DTS *ds_tx_abs = RDD_DS_TM_BBH_TX_ABS_COUNTER_TABLE_PTR(DS_TM_CORE);
        RDD_SPDSVC_BBH_TX_PARAMS_DTS *us_params;
        RDD_SPDSVC_BBH_TX_PARAMS_DTS *ds_params;


        RDD_SPDSVC_GEN_PARAMS_TX_PACKETS_READ_G(tx_packets, RDD_SPDSVC_GEN_PARAMS_TABLE_ADDRESS_ARR, 0);
        RDD_SPDSVC_GEN_PARAMS_TX_DROPPED_READ_G(tx_dropped, RDD_SPDSVC_GEN_PARAMS_TABLE_ADDRESS_ARR, 0);

        /* BBH ABS TX counters is only 6 bit */
        tx_packets = tx_packets - tx_dropped;
        us_params = &(RDD_SPDSVC_BBH_TX_PARAMS_TABLE_PTR(US_TM_CORE))->entry;
        RDD_SPDSVC_BBH_TX_PARAMS_SPDSVC_SENT_READ(us_tx, us_params);

        ds_params = &(RDD_SPDSVC_BBH_TX_PARAMS_TABLE_PTR(DS_TM_CORE))->entry;
        RDD_SPDSVC_BBH_TX_PARAMS_SPDSVC_SENT_READ(ds_tx, ds_params);

        if ((us_tx == tx_packets) || (ds_tx == tx_packets))
        {
            RDD_SPDSVC_BBH_TX_PARAMS_PROCESSING_TX_PD_3_WRITE_G(0, RDD_SPDSVC_BBH_TX_PARAMS_TABLE_ADDRESS_ARR, 0);

            if ((SPDSVC_ITERS_BEFORE_RELEASE - gen_control_block.iters_before_release) > SPDSVC_ITERS_AFTER_TX)
            {
                /* in case of the wrong tracking (abs host buffers being used other than spdsvc, make sure we release soon */
                gen_control_block.iters_before_release = (SPDSVC_ITERS_BEFORE_RELEASE - SPDSVC_ITERS_AFTER_TX);
            }
        }
        
        // the following counters will only work IF speed service is the only sender of absolute host buffers
        if (us_tx == tx_packets)
        {
            // check which interface speed service was sent out
            RDD_SPDSVC_BBH_TX_PARAMS_SPDSVC_BBH_ID_READ(bbh_id, us_params);
            if (bbh_id == BB_ID_TX_2P5)
            {
                bdmf_trace ("AE2P5 counter 0x%x\n", ae2p5_tx_abs->entry.bits);
                tx_abs = gen_control_block.ae2p5_tx_abs + us_tx;
                if ((tx_abs & 0x3F) == (ae2p5_tx_abs->entry.bits & 0x3F))
                {
                    contd = 0;
                }
            }
            else if (bbh_id == BB_ID_TX_10G)
            {
                bdmf_trace ("AE10 counter 0x%x\n", ae10_tx_abs->entry.bits);
                tx_abs = gen_control_block.ae10_tx_abs + us_tx;
                if ((tx_abs & 0x3F) == (ae10_tx_abs->entry.bits & 0x3F))
                {
                    contd = 0;
                }
            }
            else if (bbh_id == BB_ID_TX_DSL)
            {
                tx_abs = gen_control_block.dsl_tx_abs + us_tx;
                us_tx = 0;
                for (i=0; i < RDD_US_TM_DSL_BBH_TX_ABS_COUNTER_TABLE_SIZE; i++)
                {
                    us_tx += dsl_tx_abs->entry[i].bits;
                }

                bdmf_trace ("DSL counter 0x%x\n", dsl_tx_abs->entry[0].bits);
                if ((tx_abs & 0x3F) == (us_tx & 0x3F))
                {
                    contd = 0;
                }
            }
            else if (bbh_id == BB_ID_TX_PON)
            {
                tx_abs = gen_control_block.pon_tx_abs + us_tx;
                us_tx = 0;
                for (i=0; i < RDD_US_TM_PON_BBH_TX_ABS_COUNTER_TABLE_SIZE; i++)
                {
                    us_tx += pon_tx_abs->entry[i].bits;
                }
                if ((tx_abs & 0x3F) == (us_tx & 0x3F))
                {
                    contd = 0;
                }
            }
            else
            {
                BDMF_TRACE_ERR("unknown BB_ID 0x%x\n", bbh_id);
            }
        }
        else if (ds_tx == tx_packets)
        {
            bdmf_trace("ds 0x%x + 0x%x\n", ds_tx_abs->entry[0].bits, ds_tx_abs->entry[1].bits);

            tx_abs = gen_control_block.ds_tx_abs + ds_tx;
            ds_tx = 0;
            for (i=0; i < RDD_DS_TM_BBH_TX_ABS_COUNTER_TABLE_SIZE; i++)
            {
                ds_tx += ds_tx_abs->entry[i].bits;
            }
            if( (tx_abs & 0x3F) == (ds_tx & 0x3F) )
            {
                contd = 0;
            }
        }
    }
#endif

    /* Finished transmitting. Wait a bit to let queued packets exit */
    if ((contd == 1) && (gen_control_block.iters_before_release++ < SPDSVC_ITERS_BEFORE_RELEASE))
    {
        bdmf_timer_start(timer, bdmf_ms_to_ticks(SPDSVC_CLEANUP_TIMER_DURATION_MS));
        return;
    }

#if SPDSVC_DHD_GEN_ENABLE
    if (gen_control_block.radio_idx >= 0)
    {
        uint32_t cntr_arr[MAX_NUM_OF_COUNTERS_PER_READ] = {};

        drv_cntr_counter_read(CNTR_GROUP_GENERAL, GENERAL_COUNTER_DHD_SPDSVC_TX_DROPS_0 + gen_control_block.radio_idx, cntr_arr);
        post_drop = cntr_arr[0];

        gen_control_block.radio_idx = -1;

        RDD_SPDSVC_WLAN_GEN_PARAMS_COMPLETE_TRACKED_READ_G(tx_complete, RDD_SPDSVC_WLAN_GEN_PARAMS_TABLE_ADDRESS_ARR, 0);
        bdmf_trace("copies %d tx_complete %d post_drop %d\n", gen_control_block.copies, tx_complete, post_drop);

        if (gen_control_block.copies != (tx_complete + post_drop))
        {
            BDMF_TRACE_ERR("WLAN spdsvc generation packet lost? copies %d complete %d dropped %d\n", gen_control_block.copies, tx_complete, post_drop);
        }
    }
#endif
    // RDD_SPDSVC_GEN_PARAMS_PROCESSING_TX_PD_3_WRITE_G(0, RDD_SPDSVC_GEN_PARAMS_TABLE_ADDRESS_ARR, 0);

    if (gen_control_block.pbuf.sysb)
    {
        /* Clear buffer address word in PD. This is the address TX_ABS_RECYCLE task
         * compares with to identify speed service PDs dropped by QM
         */
        bdmf_sysb_free(gen_control_block.pbuf.sysb);
        gen_control_block.pbuf.sysb = NULL;
    }
    gen_control_block.pbuf.length = 0;
    gen_control_block.pbuf.fpm_bn = 0;
}


bdmf_error_t rdd_spdsvc_gen_config ( uint32_t xi_kbps,
                                 uint32_t xi_mbs,
                                 uint32_t xi_copies,
                                 uint32_t xi_total_length,
                                 uint32_t xi_test_time_ms )
{
    gen_control_block.radio_idx = -1;
    if (_rdd_spdsvc_is_running())
    {
        BDMF_TRACE_RET(BDMF_ERR_ALREADY, "Can't reconfigure speed service while running.\n");
    }
    gen_control_block.copies = xi_copies;
    gen_control_block.kbps = xi_kbps;
    gen_control_block.mbs = xi_mbs;
    gen_control_block.total_length = xi_total_length;

    return BDMF_ERR_OK;
}

bdmf_error_t rdd_spdsvc_get_tx_result ( uint8_t *xo_running_p,
                                        uint32_t *xo_tx_packets_p,
                                        uint32_t *xo_tx_discards_p )
{
    uint32_t tx_packets;
    uint32_t tx_dropped;

    RDD_SPDSVC_GEN_PARAMS_TX_PACKETS_READ_G(tx_packets, RDD_SPDSVC_GEN_PARAMS_TABLE_ADDRESS_ARR, 0);
    RDD_SPDSVC_GEN_PARAMS_TX_DROPPED_READ_G(tx_dropped, RDD_SPDSVC_GEN_PARAMS_TABLE_ADDRESS_ARR, 0);
    *xo_running_p = _rdd_spdsvc_is_running();
    *xo_tx_discards_p = tx_dropped;
    *xo_tx_packets_p = tx_packets - tx_dropped;

    return BDMF_ERR_OK;
}

void rdd_spdsvc_gen_params_init(uint32_t kbps, uint32_t mbs, uint32_t total_length,
    uint32_t copies)
{
    uint32_t tokens;
    uint32_t bucket_size;

    tokens = _rdd_spdsvc_kbps_to_tokens(kbps);
    bucket_size = _rdd_spdsvc_mbs_to_bucket_size(mbs + tokens);

    RDD_SPDSVC_GEN_PARAMS_BUCKET_SIZE_WRITE_G(bucket_size, RDD_SPDSVC_GEN_PARAMS_TABLE_ADDRESS_ARR, 0);
    RDD_SPDSVC_GEN_PARAMS_BUCKET_WRITE_G(0, RDD_SPDSVC_GEN_PARAMS_TABLE_ADDRESS_ARR, 0);
    RDD_SPDSVC_GEN_PARAMS_TOKENS_WRITE_G(tokens, RDD_SPDSVC_GEN_PARAMS_TABLE_ADDRESS_ARR, 0);
    RDD_SPDSVC_GEN_PARAMS_TOTAL_LENGTH_WRITE_G(total_length, RDD_SPDSVC_GEN_PARAMS_TABLE_ADDRESS_ARR, 0);
    RDD_SPDSVC_GEN_PARAMS_TIMER_PERIOD_WRITE_G((SPDSVC_TIMER_PERIOD*TIMER_TICKS_PER_USEC), RDD_SPDSVC_GEN_PARAMS_TABLE_ADDRESS_ARR, 0);
    RDD_SPDSVC_GEN_PARAMS_TX_PACKETS_WRITE_G(0, RDD_SPDSVC_GEN_PARAMS_TABLE_ADDRESS_ARR, 0);
    RDD_SPDSVC_GEN_PARAMS_TX_DROPPED_WRITE_G(0, RDD_SPDSVC_GEN_PARAMS_TABLE_ADDRESS_ARR, 0);
    RDD_SPDSVC_GEN_PARAMS_TOTAL_COPIES_WRITE_G(copies, RDD_SPDSVC_GEN_PARAMS_TABLE_ADDRESS_ARR, 0);

    SPDSVC_DBG("Speed Service Generator init: tokens=%u, bucket_size=%u, copies=%u --> %p\n",
        tokens, bucket_size, copies,
        DEVICE_ADDRESS(rdp_runner_core_addr[spdsvc_gen_runner_image] + RDD_SPDSVC_GEN_PARAMS_TABLE_ADDRESS_ARR[spdsvc_gen_runner_image]));
}

bdmf_error_t rdd_spdsvc_gen_start(pbuf_t *pbuf,
                                  const rdpa_cpu_tx_info_t *info,
                                  RDD_CPU_TX_DESCRIPTOR_DTS *cpu_tx)
{
    uint32_t length = pbuf->length;
    RDD_PROCESSING_TX_DESCRIPTOR_DTS processing_tx_pd = {};
    uint32_t *p_processing_tx_pd=(uint32_t *)&processing_tx_pd;

#if defined(BCM63158)
    volatile RDD_US_TM_DSL_BBH_TX_ABS_COUNTER_TABLE_DTS *dsl_tx_abs = RDD_US_TM_DSL_BBH_TX_ABS_COUNTER_TABLE_PTR(US_TM_CORE);
    volatile RDD_US_TM_PON_BBH_TX_ABS_COUNTER_TABLE_DTS *pon_tx_abs = RDD_US_TM_PON_BBH_TX_ABS_COUNTER_TABLE_PTR(US_TM_CORE);
    volatile RDD_US_TM_AE10_BBH_TX_ABS_COUNTER_TABLE_DTS *ae10_tx_abs = RDD_US_TM_AE10_BBH_TX_ABS_COUNTER_TABLE_PTR(US_TM_CORE);
    volatile RDD_US_TM_AE2P5_BBH_TX_ABS_COUNTER_TABLE_DTS *ae2p5_tx_abs = RDD_US_TM_AE2P5_BBH_TX_ABS_COUNTER_TABLE_PTR(US_TM_CORE);
    volatile RDD_DS_TM_BBH_TX_ABS_COUNTER_TABLE_DTS *ds_tx_abs = RDD_DS_TM_BBH_TX_ABS_COUNTER_TABLE_PTR(DS_TM_CORE);
    uint32_t i, tx_abs;
#endif

    if (!gen_control_block.gen_initialized)
    {
        bdmf_timer_init(&gen_control_block.timer, _rdd_spdsvc_gen_timer_cb, 0);
        gen_control_block.gen_initialized = 1;
    }
    gen_control_block.radio_idx = -1;

    /* Make sure that it isn't running */
    if (_rdd_spdsvc_is_running())
    {
        if (pbuf->abs_flag)
            bdmf_sysb_free(pbuf->sysb);
        BDMF_TRACE_RET(BDMF_ERR_ALREADY, "Can't start speed service.\n");
    }

    if (!gen_control_block.total_length || !gen_control_block.copies)
    {
        if (pbuf->abs_flag)
            bdmf_sysb_free(pbuf->sysb);
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Can't start speed service. It is not configured.\n");
    }

    /* Fill in SPDSVC configuration record */
    rdd_spdsvc_gen_params_init(gen_control_block.kbps, gen_control_block.mbs, gen_control_block.total_length, gen_control_block.copies);

    if (pbuf->abs_flag)
    {
        uintptr_t data_phys_addr = RDD_VIRT_TO_PHYS(pbuf->data);
        processing_tx_pd.abs = 1;
        processing_tx_pd.buffer_number_0_or_abs_0 = data_phys_addr & 0x3ffff;
        processing_tx_pd.payload_offset_or_abs_1 = (data_phys_addr >> 18) & 0x7ff;
        processing_tx_pd.union3 = (data_phys_addr >> 29) & 0x7;
    }
    else
    {
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Speed Service requires transmit from ABS address.\n");
        //processing_tx_pd.buffer_number_0_or_abs_0 = pbuf->fpm_bn;
        //processing_tx_pd.payload_offset_or_abs_1 = pbuf->offset;
    }
    processing_tx_pd.valid = 1;
    processing_tx_pd.first_level_q = cpu_tx->first_level_q;
    processing_tx_pd.packet_length = length;
    processing_tx_pd.lan = cpu_tx->lan;
    processing_tx_pd.ingress_port = cpu_tx->wan_flow_source_port;

    RDD_SPDSVC_GEN_PARAMS_PROCESSING_TX_PD_0_WRITE_G(
        p_processing_tx_pd[0], RDD_SPDSVC_GEN_PARAMS_TABLE_ADDRESS_ARR, 0);
    RDD_SPDSVC_GEN_PARAMS_PROCESSING_TX_PD_1_WRITE_G(
        p_processing_tx_pd[1], RDD_SPDSVC_GEN_PARAMS_TABLE_ADDRESS_ARR, 0);
    RDD_SPDSVC_GEN_PARAMS_PROCESSING_TX_PD_2_WRITE_G(
        p_processing_tx_pd[2], RDD_SPDSVC_GEN_PARAMS_TABLE_ADDRESS_ARR, 0);
    RDD_SPDSVC_GEN_PARAMS_PROCESSING_TX_PD_3_WRITE_G(
        p_processing_tx_pd[3], RDD_SPDSVC_GEN_PARAMS_TABLE_ADDRESS_ARR, 0);

    SPDSVC_DBG("   bn=%x  va=%p  pa=%llx length=%u  offset=%u pd=%08x %08x %08x %08x\n",
        pbuf->fpm_bn, pbuf->data, RDD_VIRT_TO_PHYS(pbuf->data),
        pbuf->length, info->data_offset, p_processing_tx_pd[0], p_processing_tx_pd[1], p_processing_tx_pd[2], p_processing_tx_pd[3]);

    RDD_SPDTEST_GEN_CFG_IS_ENDLESS_TX_WRITE_G(0, RDD_SPDTEST_GEN_PARAM_ADDRESS_ARR, 0);
    RDD_SPDTEST_GEN_CFG_TEST_TYPE_WRITE_G(SPDTEST_BRCM_SPDSVC, RDD_SPDTEST_GEN_PARAM_ADDRESS_ARR, 0);

    RDD_SPDTEST_GEN_CFG_IS_ON_WRITE_G(1, RDD_SPDTEST_GEN_PARAM_ADDRESS_ARR, 0);

#if defined(BCM63158)
    // save the TX ABS counter before the test start
    gen_control_block.ae2p5_tx_abs = ae2p5_tx_abs->entry.bits;
    gen_control_block.ae10_tx_abs = ae10_tx_abs->entry.bits;

    tx_abs = 0;
    for (i=0; i < RDD_US_TM_DSL_BBH_TX_ABS_COUNTER_TABLE_SIZE; i++)
    {
        tx_abs += dsl_tx_abs->entry[i].bits;
    }
    gen_control_block.dsl_tx_abs = tx_abs;

    tx_abs = 0;
    for (i=0; i < RDD_US_TM_PON_BBH_TX_ABS_COUNTER_TABLE_SIZE; i++)
    {
        tx_abs += pon_tx_abs->entry[i].bits;
    }
    gen_control_block.pon_tx_abs = tx_abs;

    tx_abs = 0;
    for (i=0; i < RDD_DS_TM_BBH_TX_ABS_COUNTER_TABLE_SIZE; i++)
    {
        tx_abs += ds_tx_abs->entry[i].bits;
    }
    gen_control_block.ds_tx_abs = tx_abs;

    RDD_SPDSVC_BBH_TX_PARAMS_SPDSVC_BBH_ID_WRITE_G(0, RDD_SPDSVC_BBH_TX_PARAMS_TABLE_ADDRESS_ARR, 0);
    RDD_SPDSVC_BBH_TX_PARAMS_SPDSVC_SENT_WRITE_G(0, RDD_SPDSVC_BBH_TX_PARAMS_TABLE_ADDRESS_ARR, 0);
    RDD_SPDSVC_BBH_TX_PARAMS_PROCESSING_TX_PD_3_WRITE_G(
        p_processing_tx_pd[3], RDD_SPDSVC_BBH_TX_PARAMS_TABLE_ADDRESS_ARR, 0);
#endif

    /* Kick speed service task */
    WMB();
    ag_drv_rnr_regs_cfg_cpu_wakeup_set(get_runner_idx(spdsvc_gen_runner_image), SPDSVC_GEN_THREAD_NUMBER);

    /* Start timer to release the buffer when test is done */
    gen_control_block.iters_before_release = 0;
    gen_control_block.pbuf = *pbuf;

    return bdmf_timer_start(&gen_control_block.timer, bdmf_ms_to_ticks(SPDSVC_CLEANUP_TIMER_DURATION_MS));
}

#if SPDSVC_DHD_GEN_ENABLE
bdmf_error_t rdd_wlan_spdsvc_gen_start(pbuf_t *pbuf,
                                       const rdpa_dhd_tx_post_info_t *info,
                                       RDD_DHD_CPU_QM_DESCRIPTOR_DTS *cpu_tx)
{
    uint32_t *p_processing_tx_pd=(uint32_t *)cpu_tx;

    RDD_SPDSVC_WLAN_GEN_PARAMS_SPDSVC_FREE_IDX_WRITE_G(cpu_tx->pkt_id_or_read_idx, RDD_SPDSVC_WLAN_GEN_PARAMS_TABLE_ADDRESS_ARR, 0);

    RDD_SPDSVC_WLAN_TXPOST_PARAMS_SPDSVC_FREE_IDX_WRITE_G(cpu_tx->pkt_id_or_read_idx, RDD_SPDSVC_WLAN_TXPOST_PARAMS_TABLE_ADDRESS_ARR, 0);
    RDD_SPDSVC_WLAN_GEN_PARAMS_COMPLETE_TRACKED_WRITE_G(0, RDD_SPDSVC_WLAN_GEN_PARAMS_TABLE_ADDRESS_ARR, 0);

    gen_control_block.dhd_pd = *cpu_tx;
    gen_control_block.radio_idx = info->radio_idx;

    // clear ABS and target_mem_0 field to make sure QM will not copy the packet to DDR
    // Note: force_copy field is overlapped with flow_ring_id so it might be set
    cpu_tx->abs = 0;
    cpu_tx->target_mem_0 = 0;

    if (!gen_control_block.gen_initialized)
    {
        bdmf_timer_init(&gen_control_block.timer, _rdd_spdsvc_gen_timer_cb, 0);
        gen_control_block.gen_initialized = 1;
    }
    /* Fill in SPDSVC configuration record */
    SPDSVC_DBG("Speed Service (WLAN) Generator started\n");
    rdd_spdsvc_gen_params_init(gen_control_block.kbps, gen_control_block.mbs, gen_control_block.total_length, gen_control_block.copies);

    drv_cntr_counter_clr(CNTR_GROUP_GENERAL, GENERAL_COUNTER_DHD_SPDSVC_TX_DROPS_0 + gen_control_block.radio_idx);

    RDD_SPDSVC_GEN_PARAMS_PROCESSING_TX_PD_0_WRITE_G(
        p_processing_tx_pd[0], RDD_SPDSVC_GEN_PARAMS_TABLE_ADDRESS_ARR, 0);
    RDD_SPDSVC_GEN_PARAMS_PROCESSING_TX_PD_1_WRITE_G(
        p_processing_tx_pd[1], RDD_SPDSVC_GEN_PARAMS_TABLE_ADDRESS_ARR, 0);
    RDD_SPDSVC_GEN_PARAMS_PROCESSING_TX_PD_2_WRITE_G(
        p_processing_tx_pd[2], RDD_SPDSVC_GEN_PARAMS_TABLE_ADDRESS_ARR, 0);
    RDD_SPDSVC_GEN_PARAMS_PROCESSING_TX_PD_3_WRITE_G(
        p_processing_tx_pd[3], RDD_SPDSVC_GEN_PARAMS_TABLE_ADDRESS_ARR, 0);

    RDD_SPDTEST_GEN_CFG_TEST_TYPE_WRITE_G(SPDTEST_BRCM_SPDSVC, RDD_SPDTEST_GEN_PARAM_ADDRESS_ARR, 0);
    RDD_SPDSVC_WLAN_GEN_PARAMS_TX_COPIES_WRITE_G(gen_control_block.copies, RDD_SPDSVC_WLAN_GEN_PARAMS_TABLE_ADDRESS_ARR, 0);

    RDD_SPDTEST_GEN_CFG_IS_ON_WRITE_G(1, RDD_SPDTEST_GEN_PARAM_ADDRESS_ARR, 0);

    /* Kick speed service task */
    WMB();
    ag_drv_rnr_regs_cfg_cpu_wakeup_set(get_runner_idx(spdsvc_gen_runner_image), SPDSVC_GEN_THREAD_NUMBER);

    gen_control_block.iters_before_release = 0;

    return bdmf_timer_start(&gen_control_block.timer, bdmf_ms_to_ticks(SPDSVC_CLEANUP_TIMER_DURATION_MS));
}

bdmf_error_t rdd_wlan_spdsvc_gen_complete(void)
{
    if ( gen_control_block.radio_idx == -1)
    {
        BDMF_TRACE_RET(BDMF_ERR_PARM, "There is no WLAN spdsvc generator running.\n");
    }
    gen_control_block.iters_before_release = SPDSVC_ITERS_BEFORE_RELEASE;
    return BDMF_ERR_OK;
}
#endif

bdmf_error_t rdd_spdsvc_analyzer_config(void)
{
    const char stream_prefix[]=CMD_STREAM;
    int i;

    /* Set up stream prefix */
    for ( i = 0 ; i < sizeof(stream_prefix); i++ )
    {
        GROUP_MWRITE_I_8(RDD_SPDSVC_ANALYZER_PARAMS_TABLE_ADDRESS_ARR, 0, i, stream_prefix[i]);
    }
    rdd_ag_cpu_rx_bcm_spdsvc_stream_rx_ts_table_set(0, 0, 0);
    SPDSVC_DBG("   prefix at %p\n", RDD_SPDSVC_ANALYZER_PARAMS_TABLE_PTR(processing_runner_image));

    RDD_SPDTEST_GEN_CFG_TEST_TYPE_WRITE_G(SPDTEST_BRCM_SPDSVC, RDD_SPDTEST_GEN_PARAM_ADDRESS_ARR, 0);
    RDD_SPDTEST_GEN_CFG_IS_ON_WRITE_G(1, RDD_SPDTEST_GEN_PARAM_ADDRESS_ARR, 0);
    
    return BDMF_ERR_OK;
}

bdmf_error_t rdd_spdsvc_analyzer_delete(void)
{
    RDD_SPDTEST_GEN_CFG_IS_ON_WRITE_G(0, RDD_SPDTEST_GEN_PARAM_ADDRESS_ARR, 0);
  
    return BDMF_ERR_OK;
}

bdmf_error_t rdd_spdsvc_analyzer_get_rx_time(uint32_t *rx_time_us)
{
    uint32_t ts_first;
    uint32_t ts_last;
    bdmf_boolean ts_first_set;

    rdd_ag_cpu_rx_bcm_spdsvc_stream_rx_ts_table_get(&ts_first, &ts_last, &ts_first_set);
    *rx_time_us = (ts_last - ts_first) / TIMER_TICKS_PER_USEC;

    return BDMF_ERR_OK;
}

#endif /* #ifndef _CFE_ */

#endif /* G9991 */
