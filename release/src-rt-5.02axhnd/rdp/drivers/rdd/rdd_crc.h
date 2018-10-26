/*
    <:copyright-BRCM:2015-2016:DUAL/GPL:standard
    
       Copyright (c) 2015-2016 Broadcom 
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

#ifndef __RDD_CRC_H
#define __RDD_CRC_H

#ifdef XRDP
#include "rdp_platform.h"
#else
#define HASH_NUM_OF_ENGINES  4
#endif
#include "rdd.h"

#define RDD_CRC_TYPE_16 0
#define RDD_CRC_TYPE_32 1

typedef enum {
    crc16_polynom_1021,
    crc16_polynom_8bb7,
    crc16_polynom_c867,
    crc16_polynom_0589,
    crc16_polynom_none
} crc16_polynom_t;

uint32_t rdd_crc_init_value_get(uint32_t crc_type);
#if defined(WL4908) || defined(XRDP)
uint32_t rdd_crc_bit_by_bit_natc(const uint8_t *p, uint32_t byte_len, uint32_t bit_len);
#endif

#if defined(XRDP)
uint32_t rdd_crc_bit_by_bit_hash(uint8_t *p, uint8_t key_bit_len, uint8_t key_byte_len, crc16_polynom_t eng_id);
#endif

uint32_t rdd_crc_bit_by_bit(const uint8_t *p, uint32_t byte_len, uint32_t bit_len, uint32_t crc_residue, uint32_t crc_type);
void rdd_crc_init(void);
void rdd_crc_ipv6_addr_calc(const bdmf_ip_t *ip_addr, uint32_t *ipv6_ip_crc);

uint32_t rdd_crc_buf_calc_crc32(const uint8_t *buf, int buf_size);

#endif
