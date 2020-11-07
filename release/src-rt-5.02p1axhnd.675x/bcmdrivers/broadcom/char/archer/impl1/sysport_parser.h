/*
<:copyright-BRCM:2017:proprietary:standard

   Copyright (c) 2017 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:> 
*/

/*
*******************************************************************************
* File Name  : sysport_parser.h
*
* Description: This file contains the System Port Parser API
*
*******************************************************************************
*/

#ifndef __SYSPORT_PARSER_H__
#define __SYSPORT_PARSER_H__

#if !defined(__KERNEL__)
#define ___constant_swab16(x) ((uint16_t)                               \
                               ((((uint16_t)(x) & (uint16_t)0x00ffU) << 8) | \
                                (((uint16_t)(x) & (uint16_t)0xff00U) >> 8)))
#define __constant_htons(x) ((uint16_t)___constant_swab16((x)))
#endif

/* IPv4 Multicast range: 224.0.0.0 to 239.255.255.255 (E0.*.*.* to EF.*.*.*) */
#define SYSPORT_PARSER_MCAST_IPV4_SHIFT  24
#define SYSPORT_PARSER_MCAST_IPV4_MASK   0xF0
#define SYSPORT_PARSER_MCAST_IPV4_VAL    0xE0

#define SYSPORT_PARSER_IS_MCAST_IPV4(_addr)                             \
    ( (((_addr) >> SYSPORT_PARSER_MCAST_IPV4_SHIFT) &                   \
       SYSPORT_PARSER_MCAST_IPV4_MASK) == SYSPORT_PARSER_MCAST_IPV4_VAL )

/* IPv6 Multicast range:  FF00::/8  */
#define SYSPORT_PARSER_MCAST_IPV6_VAL   0xFF

#define SYSPORT_PARSER_IS_MCAST_IPV6(_msb_addr)         \
    ( (_msb_addr)  == SYSPORT_PARSER_MCAST_IPV6_VAL)

#define SYSPORT_PARSER_IPV6_GET_VERSION(_ver_tos)       \
    ( ((_ver_tos) & 0xF000) >> 12 )

#define SYSPORT_PARSER_IPV6_GET_TOS(_ver_tos)   \
    ( ((_ver_tos) & 0x0FF0) >> 4 )

uint32_t sysport_crc32( const unsigned char *data, int size, uint32_t crc_prev);
uint16_t sysport_crc16( const unsigned char *data, int size, uint16_t crc_prev);
void sysport_crc32_init(void);
void sysport_crc16_init(void);

static inline uint32_t sysport_parser_crc32(void *buffer, int size)
{
    return sysport_crc32((const unsigned char *)buffer, size, (uint32_t)0xfffffffful);
}

#if defined(CC_ARCHER_PERFORMANCE) && !defined(CONFIG_BCM963178) && !defined(CONFIG_BCM947622)
static inline uint16_t sysport_parser_crc16(void *buffer, int size)
{
    uint32_t *u32_p = buffer;
    int size_32 = size >> 2;
    int hash_val = 0;
    int i;

    for(i=0; i<size_32; ++i)
    {
        hash_val += u32_p[i];
    }

    hash_val ^= ( hash_val >> 16 );
    hash_val ^= ( hash_val >>  8 );
    hash_val ^= ( hash_val >>  3 );

    return (uint16_t)hash_val;
}
#else
static inline uint16_t sysport_parser_crc16(void *buffer, int size)
{
    return sysport_crc16((const unsigned char *)buffer, size, (uint16_t)0xffffu);
}
#endif

static inline int sysport_parser_ucast_tuple_crc16(sysport_rsb_flow_tuple_t *tuple_p)
{
    return sysport_parser_crc16(tuple_p, sizeof(sysport_rsb_flow_tuple_t));
}

static inline int sysport_parser_mcast_tuple_crc16(sysport_rsb_flow_tuple_t *tuple_p)
{
    return sysport_parser_crc16(tuple_p, sizeof(sysport_rsb_flow_tuple_t) / 2);
}

static inline void sysport_parser_crc_init(void)
{
    sysport_crc32_init();
    sysport_crc16_init();
}

#endif  /* __SYSPORT_PARSER_H__ */
