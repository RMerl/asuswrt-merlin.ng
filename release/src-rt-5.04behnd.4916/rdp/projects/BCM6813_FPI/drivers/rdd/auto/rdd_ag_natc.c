/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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



/* This is an automated file. Do not edit its contents. */


#include "rdd.h"

#include "rdd_ag_natc.h"

int rdd_ag_natc_natc_l2_tos_mask_set(uint8_t bits)
{
    RDD_BYTE_1_BITS_WRITE_G(bits, RDD_NATC_L2_TOS_MASK_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_natc_natc_l2_tos_mask_set_core(uint8_t bits, int core_id)
{
    RDD_BYTE_1_BITS_WRITE_CORE(bits, RDD_NATC_L2_TOS_MASK_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_natc_natc_l2_tos_mask_get(uint8_t *bits)
{
    RDD_BYTE_1_BITS_READ_G(*bits, RDD_NATC_L2_TOS_MASK_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_natc_natc_l2_tos_mask_get_core(uint8_t *bits, int core_id)
{
    RDD_BYTE_1_BITS_READ_CORE(*bits, RDD_NATC_L2_TOS_MASK_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_natc_natc_l2_vlan_key_mask_set(uint16_t bits)
{
    RDD_BYTES_2_BITS_WRITE_G(bits, RDD_NATC_L2_VLAN_KEY_MASK_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_natc_natc_l2_vlan_key_mask_set_core(uint16_t bits, int core_id)
{
    RDD_BYTES_2_BITS_WRITE_CORE(bits, RDD_NATC_L2_VLAN_KEY_MASK_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_natc_natc_l2_vlan_key_mask_get(uint16_t *bits)
{
    RDD_BYTES_2_BITS_READ_G(*bits, RDD_NATC_L2_VLAN_KEY_MASK_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_natc_natc_l2_vlan_key_mask_get_core(uint16_t *bits, int core_id)
{
    RDD_BYTES_2_BITS_READ_CORE(*bits, RDD_NATC_L2_VLAN_KEY_MASK_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_natc_nat_cache_key0_mask_set(uint32_t bits)
{
    RDD_BYTES_4_BITS_WRITE_G(bits, RDD_NAT_CACHE_KEY0_MASK_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_natc_nat_cache_key0_mask_set_core(uint32_t bits, int core_id)
{
    RDD_BYTES_4_BITS_WRITE_CORE(bits, RDD_NAT_CACHE_KEY0_MASK_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_natc_nat_cache_key0_mask_get(uint32_t *bits)
{
    RDD_BYTES_4_BITS_READ_G(*bits, RDD_NAT_CACHE_KEY0_MASK_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_natc_nat_cache_key0_mask_get_core(uint32_t *bits, int core_id)
{
    RDD_BYTES_4_BITS_READ_CORE(*bits, RDD_NAT_CACHE_KEY0_MASK_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

