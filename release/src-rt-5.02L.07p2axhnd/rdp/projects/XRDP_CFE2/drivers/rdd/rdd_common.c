/*
    <:copyright-BRCM:2014:DUAL/GPL:standard

       Copyright (c) 2014 Broadcom
       All Rights Reserved

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

#include "rdd.h"
#include "rdd_common.h"
#include "rdd_runner_proj_defs.h"

#define LAYER2_HEADER_COPY_DST_OFFSET_ARRAY(var, offset) \
    uint16_t var[] = { \
        [0] = offset - 2,  \
        [1] = offset - 10, \
        [2] = offset - 10, \
        [3] = offset - 18, \
        [4] = offset - 18, \
        [5] = offset - 2,  \
        [6] = offset - 2,  \
        [7] = offset - 10, \
        [8] = offset - 10, \
        [9] = offset - 18, \
        [10] = offset + 6,  \
        [11] = offset - 2,  \
        [12] = offset - 2,  \
        [13] = offset - 10, \
        [14] = offset - 10, \
        [15] = offset + 6,  \
        [16] = offset + 6,  \
        [17] = offset - 2,  \
        [18] = offset - 2,  \
        [19] = offset - 10, \
        [20] = offset + 14, \
        [21] = offset + 6,  \
        [22] = offset + 6,  \
        [23] = offset - 2,  \
        [24] = offset - 2,  \
        [25] = offset + 14, \
        [26] = offset + 14, \
        [27] = offset + 6,  \
        [28] = offset + 6,  \
        [29] = offset - 2,  \
    }




void rdd_fpm_pool_number_mapping_cfg(uint16_t fpm_base_token_size)
{
    uint8_t i;
    uint8_t exp_fpm_base;

    /* exponent of fpm_base_token_size { 2K, 1K, 512, 256 } */
    if (fpm_base_token_size == 2048)
        exp_fpm_base = 11;
    else
        exp_fpm_base = ((fpm_base_token_size/2) >> 8) + 8;

    /*
        This is a mapping function from packet size to FPM number according to configured fpm_base_token_size
        The pool number is resolved using the configured fpm base token size - usually 512 bytes
        From FW we get pool :
           fpm_pool = PM_POOL_NUMBER_MAPPING_TABLE [ find first MSB of packet_len ]
    */

    for (i = 0; i < exp_fpm_base; i++)
        RDD_FPM_POOL_NUMBER_POOL_NUMBER_WRITE_G(3, RDD_CPU_RX_FPM_POOL_NUMBER_MAPPING_TABLE_ADDRESS_ARR, i);

    RDD_FPM_POOL_NUMBER_POOL_NUMBER_WRITE_G(2, RDD_CPU_RX_FPM_POOL_NUMBER_MAPPING_TABLE_ADDRESS_ARR, exp_fpm_base);
    RDD_FPM_POOL_NUMBER_POOL_NUMBER_WRITE_G(1, RDD_CPU_RX_FPM_POOL_NUMBER_MAPPING_TABLE_ADDRESS_ARR, (exp_fpm_base + 1));

    for (i = exp_fpm_base + 2; i < RDD_CPU_RX_FPM_POOL_NUMBER_MAPPING_TABLE_SIZE; i++)
        RDD_FPM_POOL_NUMBER_POOL_NUMBER_WRITE_G(0, RDD_CPU_RX_FPM_POOL_NUMBER_MAPPING_TABLE_ADDRESS_ARR, i);

    /*  After table manager bug is fixed and will be possible to define same table on 2 different modules,
        belonging to the same core this 2 tables can be the same */
    for (i = 0; i < exp_fpm_base; i++)
        RDD_FPM_POOL_NUMBER_POOL_NUMBER_WRITE_G(3, RDD_CPU_TX_FPM_POOL_NUMBER_MAPPING_TABLE_ADDRESS_ARR, i);

    RDD_FPM_POOL_NUMBER_POOL_NUMBER_WRITE_G(2, RDD_CPU_TX_FPM_POOL_NUMBER_MAPPING_TABLE_ADDRESS_ARR, exp_fpm_base);
    RDD_FPM_POOL_NUMBER_POOL_NUMBER_WRITE_G(1, RDD_CPU_TX_FPM_POOL_NUMBER_MAPPING_TABLE_ADDRESS_ARR, (exp_fpm_base + 1));

    for (i = exp_fpm_base + 2; i < RDD_CPU_TX_FPM_POOL_NUMBER_MAPPING_TABLE_SIZE; i++)
        RDD_FPM_POOL_NUMBER_POOL_NUMBER_WRITE_G(0, RDD_CPU_TX_FPM_POOL_NUMBER_MAPPING_TABLE_ADDRESS_ARR, i);
}

static void __rdd_rx_flow_cfg(uint32_t flow_index, rdd_flow_dest destination, rdd_rdd_vport vport, uint32_t counter_id)
{
    RDD_RX_FLOW_ENTRY_FLOW_DEST_WRITE_G(destination, RDD_RX_FLOW_TABLE_ADDRESS_ARR, flow_index);
    RDD_RX_FLOW_ENTRY_VIRTUAL_PORT_WRITE_G(vport, RDD_RX_FLOW_TABLE_ADDRESS_ARR, flow_index);
    RDD_RX_FLOW_ENTRY_CNTR_ID_WRITE_G(counter_id, RDD_RX_FLOW_TABLE_ADDRESS_ARR, flow_index);
}

void rdd_rx_flow_cfg(uint32_t flow_index, rdd_flow_dest destination, rdd_rdd_vport vport, uint32_t counter_id)
{
    static uint8_t first_time = 1;

    RDD_BTRACE("flow_index = %d, destination = %d, vport = %d, counter_id = %d, first_time %d\n", flow_index,
        destination, vport, counter_id, first_time);

    if (first_time)
    {
        int i;

        first_time = 0;
        for (i = 0; i < RX_FLOW_CONTEXTS_NUMBER; i++)
            __rdd_rx_flow_cfg(i, FLOW_DEST_ETH_ID, RDD_VPORT_LAST, 256);
    }
    __rdd_rx_flow_cfg(flow_index, destination, vport, counter_id);
}
