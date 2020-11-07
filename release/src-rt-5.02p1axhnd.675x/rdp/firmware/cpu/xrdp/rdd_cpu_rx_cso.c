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



#include "rdd.h"
#include "rdd_cpu_rx_cso.h"


typedef struct
{
    bdmf_ipv6_t ipv6_address;
    uint16_t    ref_count;
} ipv6_host_table_t;

static ipv6_host_table_t g_ipv6_host_table[RDD_IPV6_HOST_ADDRESS_CRC_TABLE_SIZE];
static uint16_t g_ipv4_host_ref_count_table[RDD_IPV4_HOST_ADDRESS_TABLE_SIZE];


int rdd_ipv4_host_address_table_set(uint32_t table_index, bdmf_ipv4 ipv4_host_addr, uint16_t ref_count)
{
    int ret = BDMF_ERR_OK;

    if ((ret = rdd_ag_cpu_rx_ipv4_host_address_table_set(table_index, ipv4_host_addr)) != BDMF_ERR_OK)
        return ret;

    g_ipv4_host_ref_count_table[table_index] = ref_count;

    return ret;
}

int rdd_ipv4_host_address_table_get(uint32_t table_index, bdmf_ipv4 *ipv4_host_addr, uint16_t *ref_count)
{
    int ret = BDMF_ERR_OK;
    if ((ret = rdd_ag_cpu_rx_ipv4_host_address_table_get(table_index, ipv4_host_addr)) != BDMF_ERR_OK)
        return ret;

    *ref_count = g_ipv4_host_ref_count_table[table_index];

    return ret;
}

int rdd_ipv6_host_address_table_set(uint32_t table_index, const bdmf_ipv6_t *ipv6_host_addr, uint16_t ref_count)
{
    int ret = BDMF_ERR_OK;
    uint32_t ipv6_crc;
    uint32_t crc_init_value;

    /* Reduce IPV6 address to a 32-bit value using CRC. This reduced value is what RDP FW will be using for lookup.*/
    /* rdd_crc_ipv6_addr_calc(ipv6_host_addr, &ipv6_crc); */
    crc_init_value = rdd_crc_init_value_get(RDD_CRC_TYPE_32);
    ipv6_crc = rdd_crc_bit_by_bit(ipv6_host_addr->data, 16, 0, crc_init_value, RDD_CRC_TYPE_32);

    /*Store the CRC in the RDP FW table*/
    if ((ret = rdd_ag_cpu_rx_ipv6_host_address_crc_table_set(table_index, ipv6_crc)) != BDMF_ERR_OK)
        return ret;

    /*Store ipv6 address in a local table so we can return in the get accessor*/
    g_ipv6_host_table[table_index].ipv6_address = *ipv6_host_addr;
    g_ipv6_host_table[table_index].ref_count = ref_count;

    return ret;
}

int rdd_ipv6_host_address_table_get(uint32_t table_index, bdmf_ipv6_t *ipv6_host_addr, uint16_t *ref_count)
{
    if (table_index >= RDD_IPV6_HOST_ADDRESS_CRC_TABLE_SIZE)
        return BDMF_ERR_PARM;

    /*Look up address in local table. The full IP address is not stored in an RDP table, only the CRC is.*/
    *ipv6_host_addr = g_ipv6_host_table[table_index].ipv6_address;
    *ref_count = g_ipv6_host_table[table_index].ref_count;

    return BDMF_ERR_OK;
}

