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
#include "XRDP_AG.h"
#include "rdd_tunnels_parsing.h"
#ifndef BCM63158
#include "xrdp_drv_psram_mem_ag.h"
#endif

void rdd_tunnels_parsing_enable(const rdd_module_t *module, bdmf_boolean enable)
{
    RDD_TUNNELS_PARSING_CFG_TUNNELING_ENABLE_WRITE_G(enable, RDD_TUNNELS_PARSING_CFG_ADDRESS_ARR, 0);
}

int rdd_tunnels_parsing_init(const rdd_module_t *module)
{
    RDD_TUNNELS_PARSING_CFG_DTS cfg_entry = {};

    cfg_entry.tunneling_enable = ((tunnels_parsing_params_t *)module->params)->tunneling_enable;
    cfg_entry.res_offset = module->res_offset;

    RDD_TUNNELS_PARSING_CFG_RES_OFFSET_WRITE_G(cfg_entry.res_offset, RDD_TUNNELS_PARSING_CFG_ADDRESS_ARR, 0);
    RDD_TUNNELS_PARSING_CFG_TUNNELING_ENABLE_WRITE_G(cfg_entry.tunneling_enable, RDD_TUNNELS_PARSING_CFG_ADDRESS_ARR, 0);
    return BDMF_ERR_OK;
}

#ifdef RDP_SIM
uint8_t temp_buff[512*1024]={};
#endif

#ifndef BCM63158
int rdd_tunnel_cfg_set(uint32_t tunnel_idx, RDD_TUNNEL_ENTRY_DTS *tunnel_entry, const psram_mem_memory_data *header_buffer)
{
    uint16_t l3_total_len = 0, l3_chsum = 0;
    uint8_t *buffer = (uint8_t *)header_buffer;

#ifdef RDP_SIM
    static int g_fd_psram = -1;
    int flags = O_RDWR;
    struct stat sbuf;
   
    if (stat("psram.mem", &sbuf) == -1)
        flags |= O_CREAT;
    if ((g_fd_psram = open("psram.mem", flags )) == -1)
    {
        bdmf_print("open psram.mem failed\n");
        return ( BDMF_ERR_OK );
    }
    memcpy(temp_buff + (128 * (TUNNEL_BN_FIRST + tunnel_idx)), header_buffer, tunnel_entry->tunnel_header_length);
    write(g_fd_psram, temp_buff, 512*1024);
    close(g_fd_psram);
#else
     /* copy tunnel header to PSRAM + tunnel_idx * 128*/
     ag_drv_psram_mem_memory_data_set(TUNNEL_BN_FIRST + tunnel_idx, header_buffer); 
#endif

    RDD_TUNNEL_ENTRY_LOCAL_IP_WRITE_G(tunnel_entry->local_ip, RDD_TUNNEL_TABLE_ADDRESS_ARR, tunnel_idx);
    RDD_TUNNEL_ENTRY_TUNNEL_TYPE_WRITE_G(tunnel_entry->tunnel_type, RDD_TUNNEL_TABLE_ADDRESS_ARR, tunnel_idx);
    RDD_TUNNEL_ENTRY_TUNNEL_HEADER_LENGTH_WRITE_G(tunnel_entry->tunnel_header_length, RDD_TUNNEL_TABLE_ADDRESS_ARR, tunnel_idx);
    RDD_TUNNEL_ENTRY_IP_FAMILY_WRITE_G(tunnel_entry->ip_family, RDD_TUNNEL_TABLE_ADDRESS_ARR, tunnel_idx);
    RDD_TUNNEL_ENTRY_LAYER3_OFFSET_WRITE_G(tunnel_entry->layer3_offset, RDD_TUNNEL_TABLE_ADDRESS_ARR, tunnel_idx);
    RDD_TUNNEL_ENTRY_GRE_PROTO_OFFSET_WRITE_G(tunnel_entry->gre_proto_offset, RDD_TUNNEL_TABLE_ADDRESS_ARR, tunnel_idx);

    /* extract fields from tunnel header */
    if(tunnel_entry->ip_family == bdmf_ip_family_ipv4)
    {
        memcpy( &l3_total_len, &buffer[tunnel_entry->layer3_offset + TUNNEL_L3_IPV4_TOTAL_LEN_OFFSET], 1);
        memcpy( &l3_chsum, &buffer[tunnel_entry->layer3_offset + TUNNEL_L3_IPV4_CHSUM_OFFSET], 2);
        l3_chsum = ( l3_chsum << 8 ) | ( l3_chsum >> 8 );
    }
    else
    {
        if (tunnel_entry->tunnel_type == TUNNEL_TYPE_DS_LITE)
            l3_total_len = 0;
        else
            memcpy( &l3_total_len, &buffer[tunnel_entry->layer3_offset + TUNNEL_L3_IPV6_TOTAL_LEN_OFFSET], 2);
        /* no checksum in IPv6 */
        l3_chsum = 0;
    }
    RDD_TUNNEL_ENTRY_LAYER3_TOTAL_LEN_WRITE_G(l3_total_len, RDD_TUNNEL_TABLE_ADDRESS_ARR, tunnel_idx);
    RDD_TUNNEL_ENTRY_LAYER3_CHKSUM_WRITE_G(l3_chsum, RDD_TUNNEL_TABLE_ADDRESS_ARR, tunnel_idx);
    /* set BN offset - FW will add according to tunnel index */
    RDD_TUNNEL_HEADER_PSRAM_BUFFER_NUMBER_WRITE_G(TUNNEL_BN_FIRST, RDD_TUNNEL_HEADER_PSRAM_BUFFER_ADDRESS_ARR, 0);

    return ( BDMF_ERR_OK );
}
#endif