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



#include "rdd_ucast.h"

/* Wrapper mapping functions to convert rdpa_wan_type to
 * egress phy type. */
static int2int_map_t rdpa_wan_type_to_rdd_egress_phy[] =
{
    {rdpa_wan_gbe, rdd_egress_phy_eth_wan},
    {rdpa_wan_dsl, rdd_egress_phy_dsl},
    {rdpa_wan_gpon, rdd_egress_phy_gpon},
    {rdpa_wan_xgpon, rdd_egress_phy_gpon},
    {rdpa_wan_xepon, rdd_egress_phy_gpon},
    {rdpa_wan_epon, rdd_egress_phy_gpon},
    {BDMF_ERR_PARM, BDMF_ERR_PARM},
};

int rdpa_wan_type2rdd_egress_phy(rdpa_wan_type src)
{
    return int2int_map(rdpa_wan_type_to_rdd_egress_phy, src, BDMF_ERR_PARM);
}

rdpa_wan_type rdd_egress_phy2rdpa_wan_type(int src)
{
    return int2int_map_r(rdpa_wan_type_to_rdd_egress_phy, src, BDMF_ERR_PARM);
}

int rdd_fc_flow_ip_addresses_add(RDD_FC_FLOW_IP_ADDRESSES_ENTRY_DTS *ip_addresses_entry,
    bdmf_index *entry_index, uint16_t *entry_sram_address)
{

    bdmf_error_t rc = BDMF_ERR_NORES;
    RDD_FC_FLOW_IP_ADDRESSES_ENTRY_DTS curr, available;
    uint8_t *addr;
    uint32_t i, j;
    uint32_t core_index;

    *entry_index = *entry_sram_address = ip_addresses_entry->reference_count = 0;

    for (i = 0; i < RDD_FC_FLOW_IP_ADDRESSES_TABLE_SIZE; i++)
    {
        for (j = 0, addr = curr.sa_da_addresses; j < RDD_FC_FLOW_IP_ADDRESSES_ENTRY_SA_DA_ADDRESSES_NUMBER; j++, addr++)
            RDD_FC_FLOW_IP_ADDRESSES_ENTRY_SA_DA_ADDRESSES_READ_G(*addr, RDD_FC_FLOW_IP_ADDRESSES_TABLE_ADDRESS_ARR, i, j);

        if (!memcmp(curr.sa_da_addresses, ip_addresses_entry->sa_da_addresses, sizeof(curr.sa_da_addresses)))
        {
            /* Entry is already in the table, update the reference count and return the entry index and address. */
            RDD_FC_FLOW_IP_ADDRESSES_ENTRY_REFERENCE_COUNT_READ_G(ip_addresses_entry->reference_count, RDD_FC_FLOW_IP_ADDRESSES_TABLE_ADDRESS_ARR, i);
            ip_addresses_entry->reference_count++;
            RDD_FC_FLOW_IP_ADDRESSES_ENTRY_REFERENCE_COUNT_WRITE_G(ip_addresses_entry->reference_count, RDD_FC_FLOW_IP_ADDRESSES_TABLE_ADDRESS_ARR, i);

            *entry_index = i;

            for (core_index = 0; core_index < GROUPED_EN_SEGMENTS_NUM; core_index++)
            {
                if (RDD_FC_FLOW_IP_ADDRESSES_TABLE_ADDRESS_ARR[core_index] != INVALID_TABLE_ADDRESS)
                {
                    /* The SRAM address is configured to be the same for all cores. */
                    *entry_sram_address = RDD_FC_FLOW_IP_ADDRESSES_TABLE_ADDRESS_ARR[core_index] + (i * sizeof(RDD_FC_FLOW_IP_ADDRESSES_ENTRY_DTS));
                    break;
                }
            }

            rc = BDMF_ERR_OK;
            break;
        }
    }

    if (rc != BDMF_ERR_OK)
    {
        memset((uint8_t *) &available, 0x00, sizeof(RDD_FC_FLOW_IP_ADDRESSES_ENTRY_DTS));

        for (i = 0; i < RDD_FC_FLOW_IP_ADDRESSES_TABLE_SIZE; i++)
        {
            for (j = 0, addr = curr.sa_da_addresses; j < RDD_FC_FLOW_IP_ADDRESSES_ENTRY_SA_DA_ADDRESSES_NUMBER; j++, addr++)
                RDD_FC_FLOW_IP_ADDRESSES_ENTRY_SA_DA_ADDRESSES_READ_G(*addr, RDD_FC_FLOW_IP_ADDRESSES_TABLE_ADDRESS_ARR, i, j);

            if (!memcmp(curr.sa_da_addresses, available.sa_da_addresses, sizeof(curr.sa_da_addresses)))
            {
                /* Add new entry */
                for (j = 0, addr = ip_addresses_entry->sa_da_addresses; j < RDD_FC_FLOW_IP_ADDRESSES_ENTRY_SA_DA_ADDRESSES_NUMBER; j++, addr++)
                {
                    RDD_FC_FLOW_IP_ADDRESSES_ENTRY_SA_DA_ADDRESSES_WRITE_G(*addr, RDD_FC_FLOW_IP_ADDRESSES_TABLE_ADDRESS_ARR, i, j);
                }
                ip_addresses_entry->reference_count = 1;
                RDD_FC_FLOW_IP_ADDRESSES_ENTRY_REFERENCE_COUNT_WRITE_G(ip_addresses_entry->reference_count, RDD_FC_FLOW_IP_ADDRESSES_TABLE_ADDRESS_ARR, i);
                RDD_FC_FLOW_IP_ADDRESSES_ENTRY_IS_IPV6_ADDRESS_WRITE_G(ip_addresses_entry->is_ipv6_address, RDD_FC_FLOW_IP_ADDRESSES_TABLE_ADDRESS_ARR, i);
                *entry_index = i;

                for (core_index = 0; core_index < GROUPED_EN_SEGMENTS_NUM; core_index++)
                {
                    if (RDD_FC_FLOW_IP_ADDRESSES_TABLE_ADDRESS_ARR[core_index] != INVALID_TABLE_ADDRESS)
                    {
                        /* The SRAM address is configured to be the same for all cores. */
                        *entry_sram_address = RDD_FC_FLOW_IP_ADDRESSES_TABLE_ADDRESS_ARR[core_index] + (i * sizeof(RDD_FC_FLOW_IP_ADDRESSES_ENTRY_DTS));
                        break;
                    }
                }

                rc = BDMF_ERR_OK;
                break;
            }
        }
    }

    return rc;
}

int rdd_fc_flow_ip_addresses_get(bdmf_index entry_index, RDD_FC_FLOW_IP_ADDRESSES_ENTRY_DTS *ip_addresses_entry,
    uint16_t *entry_sram_address)
{
    bdmf_error_t rc = BDMF_ERR_NORES;
    uint8_t *addr;
    uint32_t j;
    uint32_t core_index;

    *entry_sram_address = 0;
    memset( (uint8_t *) ip_addresses_entry, 0x00, sizeof(RDD_FC_FLOW_IP_ADDRESSES_ENTRY_DTS));

    if (entry_index < RDD_FC_FLOW_IP_ADDRESSES_TABLE_SIZE)
    {
        RDD_FC_FLOW_IP_ADDRESSES_ENTRY_REFERENCE_COUNT_READ_G(ip_addresses_entry->reference_count, RDD_FC_FLOW_IP_ADDRESSES_TABLE_ADDRESS_ARR, entry_index);
        RDD_FC_FLOW_IP_ADDRESSES_ENTRY_IS_IPV6_ADDRESS_READ_G(ip_addresses_entry->is_ipv6_address, RDD_FC_FLOW_IP_ADDRESSES_TABLE_ADDRESS_ARR, entry_index);

        if (ip_addresses_entry->reference_count)
        {
            for (j = 0, addr = ip_addresses_entry->sa_da_addresses; j < RDD_FC_FLOW_IP_ADDRESSES_ENTRY_SA_DA_ADDRESSES_NUMBER; j++, addr++)
            {
                RDD_FC_FLOW_IP_ADDRESSES_ENTRY_SA_DA_ADDRESSES_READ_G(*addr, RDD_FC_FLOW_IP_ADDRESSES_TABLE_ADDRESS_ARR, entry_index, j);
            }

            for (core_index = 0; core_index < GROUPED_EN_SEGMENTS_NUM; core_index++)
            {
                if (RDD_FC_FLOW_IP_ADDRESSES_TABLE_ADDRESS_ARR[core_index] != INVALID_TABLE_ADDRESS)
                {
                    /* The SRAM address is configured to be the same for all cores. */
                    *entry_sram_address = RDD_FC_FLOW_IP_ADDRESSES_TABLE_ADDRESS_ARR[core_index] + (entry_index * sizeof(RDD_FC_FLOW_IP_ADDRESSES_ENTRY_DTS));
                    break;
                }
            }

            rc = BDMF_ERR_OK;
        }
    }

    return rc;
}

int rdd_fc_flow_ip_addresses_delete_by_index(bdmf_index entry_index)
{
    bdmf_error_t rc = BDMF_ERR_NORES;
    uint16_t reference_count;
    uint32_t i;

    if (entry_index < RDD_FC_FLOW_IP_ADDRESSES_TABLE_SIZE)
    {
        RDD_FC_FLOW_IP_ADDRESSES_ENTRY_REFERENCE_COUNT_READ_G(reference_count, RDD_FC_FLOW_IP_ADDRESSES_TABLE_ADDRESS_ARR, entry_index);
        if (reference_count > 0)
        {
            reference_count--;
            RDD_FC_FLOW_IP_ADDRESSES_ENTRY_REFERENCE_COUNT_WRITE_G(reference_count, RDD_FC_FLOW_IP_ADDRESSES_TABLE_ADDRESS_ARR, entry_index);
        }

        if (reference_count == 0)
        {
            for (i = 0; i < RDD_FC_FLOW_IP_ADDRESSES_ENTRY_SA_DA_ADDRESSES_NUMBER; i++)
            {
                RDD_FC_FLOW_IP_ADDRESSES_ENTRY_SA_DA_ADDRESSES_WRITE_G(0, RDD_FC_FLOW_IP_ADDRESSES_TABLE_ADDRESS_ARR, entry_index, i);
            }
        }

        rc = BDMF_ERR_OK;
    }

    return rc;
}

int rdpa_if_wan2rdd_egress_phy(rdpa_if src)
{
    return 0;
}

