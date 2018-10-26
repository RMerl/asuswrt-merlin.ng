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

#include "rdd_crc.h"

static const int32_t order[2] = {16, 32};
static const int32_t direct[2] = {1, 1};
static const int32_t refin[2] = {0, 0};
static const int32_t refout[2] = {0, 0};
static const uint32_t polynom[2] = {0x1021, 0x04C11DB7};
#if (HASH_NUM_OF_ENGINES == 2)
static const uint32_t hash_polynom[HASH_NUM_OF_ENGINES] = {0x1021, 0xc867};
#else
static const uint32_t hash_polynom[HASH_NUM_OF_ENGINES] = {0x1021, 0xc867, 0x0589, 0x8bb7};
#endif
static const uint32_t crcinit[2] = {0xFFFF, 0xFFFFFFFF};
static const uint32_t crcxor[2] = {0x0, 0xFFFFFFFF};

static uint32_t crcmask[2];
static uint32_t crchighbit[2];
static uint32_t crcinit_direct[2];
static uint32_t crcinit_nondirect[2];
static uint32_t crctab[2][256];

static uint32_t reflect(uint32_t crc, int32_t bitnum)
{
    /* reflects the lower 'bitnum' bits of 'crc' */
    uint32_t i, j = 1, crcout = 0;

    for (i = (uint32_t)1 << (bitnum - 1); i; i >>= 1)
    {
        if (crc & i)
            crcout |= j;

        j <<= 1;
    }

    return crcout;
}

static uint32_t __crc_bit_by_bit(const uint8_t *p, uint32_t byte_len, uint32_t bit_len, uint32_t crc_residue, uint32_t crc_type, uint32_t crc16_poly_type)
{
    /* bit by bit algorithm with augmented zero bytes.
       does not use lookup table, suited for polynom orders between 1...32. */
    uint32_t i, j, c, bit;
    uint32_t crc = crc_residue;

    if (bit_len != 0)
    {
        c = *p++;

        if (refin[crc_type])
            c = reflect(c, 8);

        j = (1 << (bit_len - 1));

        for (; j; j >>= 1)
        {
            bit = crc & crchighbit[crc_type];
            crc <<= 1;

            if (c & j)
                bit ^= crchighbit[crc_type];

            if (bit)
            {
                if (crc_type == RDD_CRC_TYPE_16)
                    crc ^= hash_polynom[crc16_poly_type];
                else
                    crc ^= polynom[crc_type];
            }
        }
    }

    for (i = 0; i < byte_len; i++)
    {
        c = (uint32_t)*p++;

        if (refin[crc_type])
            c = reflect(c, 8);

        for (j = 0x80; j; j >>= 1)
        {
            bit = crc & crchighbit[crc_type];
            crc <<= 1;

            if (c & j)
                bit ^= crchighbit[crc_type];

            if (bit)
            {
                if (crc_type == RDD_CRC_TYPE_16)
                    crc ^= hash_polynom[crc16_poly_type];
                else
                    crc ^= polynom[crc_type];
            }
        }
    }

    if (refout[crc_type])
        crc = reflect(crc, order[crc_type]);


    return crc;
}

uint32_t rdd_crc_bit_by_bit(const uint8_t *p, uint32_t byte_len, uint32_t bit_len, uint32_t crc_residue, uint32_t crc_type)
{
    uint32_t crc = __crc_bit_by_bit(p, byte_len, bit_len, crc_residue, crc_type, 0);

    crc ^= crcxor[crc_type];
    crc &= crcmask[crc_type];

    return crc;
}

#if defined(XRDP) || defined(WL4908)
uint32_t rdd_crc_bit_by_bit_natc(const uint8_t *p, uint32_t byte_len, uint32_t bit_len)
{
    uint32_t crc;
    crc = __crc_bit_by_bit(p, byte_len, bit_len, 0, RDD_CRC_TYPE_32, crc16_polynom_none);
    crc &= crcmask[RDD_CRC_TYPE_32];

    return crc;
}
#endif

#if defined(XRDP)
uint32_t rdd_crc_bit_by_bit_hash(uint8_t *p, uint8_t key_bit_len, uint8_t key_byte_len, crc16_polynom_t eng_id)
{
    uint32_t crc;
    uint32_t bit_len = 8 - (key_bit_len % 8);
    crc = __crc_bit_by_bit(p, key_byte_len - 1, bit_len, 0xFFFF, RDD_CRC_TYPE_16, eng_id);
    crc ^= crcxor[RDD_CRC_TYPE_16];
    crc &= crcmask[RDD_CRC_TYPE_16];

    return crc;
}
#endif

static void generate_crc_table(void)
{
    /* make CRC lookup table used by table algorithms */
    int32_t i, j;
    uint32_t bit, crc, crc_type;

    for (crc_type = RDD_CRC_TYPE_16; crc_type <= RDD_CRC_TYPE_32; crc_type++)
    {
        for (i = 0; i < 256; i++)
        {
            crc = (uint32_t)i;

            if (refin[crc_type])
                crc = reflect(crc, 8);

            crc <<= order[crc_type] - 8;

            for (j = 0; j < 8; j++)
            {
                bit = crc & crchighbit[crc_type];
                crc <<= 1;

                if (bit)
                    crc ^= polynom[crc_type];
            }

            if (refin[crc_type])
                crc = reflect(crc, order[crc_type]);

            crc &= crcmask[crc_type];
            crctab[crc_type][i] = crc;
        }
    }
}

void rdd_crc_init(void)
{
    uint32_t i, bit, crc, crc_type;

    for (crc_type = RDD_CRC_TYPE_16; crc_type <= RDD_CRC_TYPE_32; crc_type++)
    {
        /* at first, compute constant bit masks for whole CRC and CRC high bit */
        crcmask[crc_type] = ((((uint32_t)1 << (order[crc_type]-1)) - 1) << 1) | 1;
        crchighbit[crc_type] = (uint32_t)1 << (order[crc_type]-1);

        generate_crc_table();

        /* compute missing initial CRC value */
        if (!direct[crc_type])
        {
            crcinit_nondirect[crc_type] = crcinit[crc_type];
            crc = crcinit[crc_type];

            for (i = 0; i < order[crc_type]; i++)
            {
                bit = crc & crchighbit[crc_type];
                crc <<= 1;

                if (bit)
                    crc ^= polynom[crc_type];
            }

            crc &= crcmask[crc_type];
            crcinit_direct[crc_type] = crc;
        }
        else
        {
            crcinit_direct[crc_type] = crcinit[crc_type];
            crc = crcinit[crc_type];

            for (i = 0; i < order[crc_type]; i++)
            {
                bit = crc & 1;

                if (bit)
                    crc ^= polynom[crc_type];

                crc >>= 1;

                if (bit)
                    crc |= crchighbit[crc_type];
            }

            crcinit_nondirect[crc_type] = crc;
        }
    }
}

uint32_t rdd_crc_init_value_get(uint32_t crc_type)
{
    return crcinit_direct[crc_type];
}

void rdd_crc_ipv6_addr_calc(const bdmf_ip_t *ip_addr, uint32_t *ipv6_ip_crc)
{
    uint32_t crc_init_value;

    crc_init_value = rdd_crc_init_value_get(RDD_CRC_TYPE_32);
    *ipv6_ip_crc = rdd_crc_bit_by_bit(ip_addr->addr.ipv6.data, 16, 0, crc_init_value, RDD_CRC_TYPE_32);
}

uint32_t rdd_crc_buf_calc_crc32(const uint8_t *buf, int buf_size)
{
    uint32_t crc_init_value;

    crc_init_value = rdd_crc_init_value_get(RDD_CRC_TYPE_32);
    return rdd_crc_bit_by_bit(buf, buf_size, 0, crc_init_value, RDD_CRC_TYPE_32);
}

