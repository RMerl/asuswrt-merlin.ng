/*
    <:copyright-BRCM:2015:DUAL/GPL:standard
    
       Copyright (c) 2015 Broadcom 
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

#ifndef RDP_PLATFORM_H_INCLUDED
#define RDP_PLATFORM_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

/* TODO: implement */
#define IS_WAN_PORT(bbh_id) (bbh_id == 7)

typedef enum bbh_id_e
{
    BBH_ID_XLMAC0_0_10G = 0,
    BBH_ID_XLMAC0_1_2p5G,
    BBH_ID_XLMAC0_2_1G,
    BBH_ID_XLMAC0_3_1G,
    BBH_ID_XLMAC1_0_RGMII,
    BBH_ID_XLMAC1_1_RGMII,
    BBH_ID_XLMAC1_2_RGMII,
    BBH_ID_PON,
    BBH_ID_NUM,
    BBH_ID_FIRST = BBH_ID_XLMAC0_0_10G,
    BBH_ID_LAST = BBH_ID_PON,
    BBH_ID_LAST_XLMAC = BBH_ID_XLMAC1_2_RGMII,
    BBH_ID_NULL = BBH_ID_NUM
} bbh_id_e;

typedef enum  ddr_buf_size_e
{
    BUF_256 = 0,
    BUF_512,
    BUF_1K,
    BUF_2K
} ddr_buf_size_e;

typedef enum rnr_quad_id_e
{
    RNR_QUAD0_ID = 0,
    RNR_QUAD1_ID,
    RNR_QUAD2_ID,
    RNR_QUAD3_ID,
    RNR_QUAD_ID_LAST = RNR_QUAD3_ID,
} rnr_quad_id_e;

#endif
