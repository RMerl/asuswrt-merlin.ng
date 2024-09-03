/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
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

