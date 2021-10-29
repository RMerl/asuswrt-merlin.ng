/*
    <:copyright-BRCM:2015-2016:DUAL/GPL:standard
    
       Copyright (c) 2015-2016 Broadcom 
       All Rights Reserved
    
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

#endif
