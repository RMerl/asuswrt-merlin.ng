/***********************************************************************
 *
 *  Copyright (c) 2021  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2012:DUAL/GPL:standard
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 *
 ************************************************************************/

#include "soc/mcm/enum_max.h"
#include "soc/mcm/enum_types.h"
#include "soc/mem.h"
#include "soc/memory.h"
#include "soc/field.h"
#include <soc/mcm/allenum.h>
#include "soc/mcm/memregs.h"
#include "soc/feature.h"
#include "xflow_macsec_defs.h"
#include "xflow_macsec.h"
#include "xflow_macsec_common.h"
#include "xflow_macsec_firelight.h"
#include "xflow_macsec_cfg_params.h"
#include "macsec_dev.h"
#include "macsec_types.h"
#include "macsec_macros.h"

#define XFLOW_MACSEC_FLOW_UDF_PARAM_MAX_WORDS   8
#define XFLOW_MACSEC_FLOW_UDF_NUM_BITS          416

/*
 * Return the number of bits set in a unsigned int
 */

int _shr_popcount(unsigned int n)
{
    n = (n & 0x55555555) + ((n >> 1) & 0x55555555);
    n = (n & 0x33333333) + ((n >> 2) & 0x33333333);
    n = (n + (n >> 4)) & 0x0f0f0f0f;
    n = n + (n >> 8);

    return (n + (n >> 16)) & 0xff;
}

xflow_macsec_db_t *xflow_macsec_db[BCM_MAX_NUM_UNITS];

void xflow_macsec_db_init (int unit)
{
    xflow_macsec_db[unit] = NULL;
}

int xflow_macsec_db_set (int unit, xflow_macsec_db_t *db)
{
    xflow_macsec_db[unit] = db;
    return BCM_E_NONE;
}

int xflow_macsec_db_get (int unit, xflow_macsec_db_t **db)
{
    *db = xflow_macsec_db[unit];
    return BCM_E_NONE;
}

/*
 * Function: xflow_macsec_udf_param_to_bytes
 * Description: Based on the UDF param type, the function copies
 *              the user configured udf_param elements to the array
 *              out_words. The function copies from MSB the number
 *              of bits given.
 */
int xflow_macsec_udf_param_to_bytes(int unit, xflow_macsec_flow_udf_param_type_t udf_param_type,
                                    xflow_macsec_flow_udf_pkt_type_t udf_pkt_type, int num_bits,
                                    xflow_macsec_flow_udf_param_t *udf_param, uint32 *out_words)
{
    int m, max_bytes;
    int iter;
    int offset, shift;
    int inline_xflow = 0;
    uint8 *temp_words;
    uint32 mac[2];

    if (soc_feature(unit, soc_feature_xflow_macsec_inline))
    {
        inline_xflow = 1;
    }

    /*
     * Write the user configured parameters to out_words[].
     * The values need to be MSB aligned.
     */
    m = ((num_bits - 1)/ 32);
    shift = (32 - (num_bits % 32));
    if (shift == 32) {
        shift = 0;
    }

    /* Values less than a word should be aligned to MSB. */
    switch (udf_param_type)
    {
        case _xflowMacsecUdfEthertype:
            out_words[m] = (udf_param->ethertype << shift);
            break;
        case _xflowMacsecUdfFirstVlan:
            if (inline_xflow)
            {
                out_words[m] = (uint32)((udf_param->first_vlan & 0xfff) |
                            ((udf_param->first_vlan_cfi & 0x1) << 12)   |
                            ((udf_param->first_vlan_priority & 0x7) << 13))
                                                                << shift;
            }
            else
            {
                out_words[m] = (uint32)((udf_param->first_vlan << 3) |
                                (udf_param->first_vlan_priority & 0x7))
                                                                << shift;
            }
            break;
        case _xflowMacsecUdfSecondVlan:
            if (inline_xflow)
            {
                out_words[m] = (uint32)((udf_param->second_vlan & 0xfff) |
                            ((udf_param->second_vlan_cfi & 0x1) << 12)   |
                            ((udf_param->second_vlan_priority & 0x7) << 13))
                                                                << shift;
            }
            else
            {
                out_words[m] = (uint32)((udf_param->second_vlan << 3) |
                                (udf_param->second_vlan_priority & 0x7))
                                                                << shift;
            }
            break;
        case _xflowMacsecUdfThirdVlan:
            out_words[m] = (uint32)((udf_param->third_vlan & 0xfff) |
                        ((udf_param->third_vlan_cfi & 0x1) << 12)   |
                        ((udf_param->third_vlan_priority & 0x7) << 13))
                                                                << shift;
            break;
        case _xflowMacsecUdfFourthVlan:
            out_words[m] = (uint32)((udf_param->fourth_vlan & 0xfff) |
                        ((udf_param->fourth_vlan_cfi & 0x1) << 12)   |
                        ((udf_param->fourth_vlan_priority & 0x7) << 13))
                                                                << shift;
            break;
        case _xflowMacsecUdfFirstMpls:
            out_words[m] =
                ((udf_param->first_mpls.mpls_label << 3) |
                 (udf_param->first_mpls.mpls_exp & 0x7)) << shift;
            break;
        case _xflowMacsecUdfSecondMpls:
            out_words[m] =
                ((udf_param->second_mpls.mpls_label << 3) |
                 (udf_param->second_mpls.mpls_exp & 0x7)) << shift;
            break;
        case _xflowMacsecUdfProtocolId:
            out_words[m] = (udf_param->protocol_id << shift);
            break;
        case _xflowMacsecUdfSipAddr:
            if ((udf_pkt_type == _xflowMacsecUdfPktIPv4) ||
                (udf_pkt_type == _xflowMacsecUdfPktL4IPv4))
            {
                out_words[m] = (udf_param->sip_addr.ipv4_addr);
            }
            else

            if ((udf_pkt_type == _xflowMacsecUdfPktIPv6) ||
                (udf_pkt_type == _xflowMacsecUdfPktL4IPv6))
            {
                temp_words = (uint8 *)&out_words;
                for (iter = 0; iter < 16; iter++)
                {
                    temp_words[iter] =
                        udf_param->sip_addr.ipv6_addr[iter];
                }
            }
            break;
        case _xflowMacsecUdfDipAddr:
            if ((udf_pkt_type == _xflowMacsecUdfPktIPv4) ||
                (udf_pkt_type == _xflowMacsecUdfPktL4IPv4))
            {
                out_words[m] = (udf_param->dip_addr.ipv4_addr);
            }
            else
            if ((udf_pkt_type == _xflowMacsecUdfPktIPv6) ||
                (udf_pkt_type == _xflowMacsecUdfPktL4IPv6))
            {
                temp_words = (uint8 *)&out_words;
                for (iter = 0; iter < 16; iter++)
                {
                    temp_words[iter] = udf_param->dip_addr.ipv6_addr[iter];
                }
            }
            break;
        case _xflowMacsecUdfSourcePort:
            out_words[m] = (udf_param->source_port << shift);
            break;
        case _xflowMacsecUdfDestPort:
            out_words[m] = (udf_param->dest_port << shift);
            break;
        case _xflowMacsecUdfOuterSrcMac:
            XFLOW_MAC_ADDR_TO_UDF_UINT32(udf_param->outer_src_mac, mac);
            out_words[m] = mac[1];
            out_words[m - 1] = mac[0];
            break;
        case _xflowMacsecUdfOuterDstMac:
            XFLOW_MAC_ADDR_TO_UDF_UINT32(udf_param->outer_dst_mac, mac);
            out_words[m] = mac[1];
            out_words[m - 1] = mac[0];
            break;
        case _xflowMacsecUdfInnerSrcMac:
            XFLOW_MAC_ADDR_TO_UDF_UINT32(udf_param->inner_src_mac, mac);
            out_words[m] = mac[1];
            out_words[m - 1] = (mac[0] << 16);
            break;
        case _xflowMacsecUdfInnerDstMac:
            XFLOW_MAC_ADDR_TO_UDF_UINT32(udf_param->inner_dst_mac, mac);
            out_words[m] = mac[1];
            out_words[m - 1] = (mac[0] << 16);
            break;
        case _xflowMacsecUdfInnerFirstVlan:
            out_words[m] = (uint32)((udf_param->inner_first_vlan & 0xfff) |
                        ((udf_param->inner_first_vlan_cfi & 0x1) << 12)   |
                        ((udf_param->inner_first_vlan_priority & 0x7) << 13))
                                                                    << shift;
            break;
        case _xflowMacsecUdfInnerSecondVlan:
            out_words[m] = (uint32)((udf_param->inner_second_vlan & 0xfff) |
                        ((udf_param->inner_second_vlan_cfi & 0x1) << 12)   |
                        ((udf_param->inner_second_vlan_priority & 0x7) << 13))
                                                                    << shift;
            break;
        case _xflowMacsecUdfBbtagVidPCP:
            out_words[m] = (udf_param->bbtag_vid_pcp[1] << 24 |
                           udf_param->bbtag_vid_pcp[0]) << 16;
            break;
        case _xflowMacsecUdfItagPcpIsid:
            out_words[m] = (udf_param->itag_pcp_isid[1] << 24 |
                           udf_param->itag_pcp_isid[0]) << 16;
            break;
        case _xflowMacsecUdfEtagTci:
            out_words[m] = (udf_param->etag_tci[2] |
                           udf_param->etag_tci[3] << 8 |
                           udf_param->etag_tci[4] << 16 |
                           udf_param->etag_tci[5] << 24);
            out_words[m - 1] = (udf_param->etag_tci[0] << 16 |
                               udf_param->etag_tci[1] << 24);
            break;
        case _xflowMacsecUdfFirstMplsSbit:
            out_words[m] =
                (udf_param->first_mpls.mpls_label << 8|
                 udf_param->first_mpls.mpls_exp << 28 |
                 udf_param->first_mpls.mpls_sbit << 31);
            break;
        case _xflowMacsecUdfSecondMplsSbit:
            out_words[m] =
                (udf_param->second_mpls.mpls_label << 8|
                 udf_param->second_mpls.mpls_exp << 28 |
                 udf_param->second_mpls.mpls_sbit << 31);
            break;
        case _xflowMacsecUdfThirdMplsSbit:
            out_words[m] =
                (udf_param->third_mpls.mpls_label << 8|
                 udf_param->third_mpls.mpls_exp << 28 |
                 udf_param->third_mpls.mpls_sbit << 31);
            break;
        case _xflowMacsecUdfFourthMplsSbit:
            out_words[m] =
                (udf_param->fourth_mpls.mpls_label << 8|
                 udf_param->fourth_mpls.mpls_exp << 28 |
                 udf_param->fourth_mpls.mpls_sbit << 31);
            break;
        case _xflowMacsecUdfPayload:
            offset = 0;
            iter = 0;
            temp_words = (uint8 *)out_words;

            /* Offset is to align to word boundary in bytes. */
            if (num_bits % 32)
            {
                offset = (4 - (((num_bits % 32) + 7) / 8));
            }

            max_bytes = (num_bits - 1) / 8;
            for (iter = 0; iter <= max_bytes; iter++)
            {
                temp_words[iter + offset] =
                    udf_param->payload[max_bytes - iter];
            }
            break;
        default:
            return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}


int xflow_macsec_flow_udf_populate(int unit, xflow_macsec_flow_udf_param_t *udf_param,
                                   xflow_macsec_udf_populate_t *udf_populate, uint32 *udf_words)
{
    uint32 temp_word[XFLOW_MACSEC_FLOW_UDF_PARAM_MAX_WORDS];
    xflow_macsec_flow_udf_pkt_type_t udf_pkt_type_l;
    uint32 udf_map_count, iter_map, iter_udf_param;
    uint32 num_bits, num_bits_t, shift;
    xflow_macsec_flow_udf_param_type_t udf_param_type;
    xflow_macsec_flow_udf_param_t null_param;
    const xflow_macsec_flow_udf_map_t *udf_map = udf_populate->udf_map;
    int bp, iter;

    udf_pkt_type_l = udf_populate->udf_pkt_type;

    /* Set base pointer to the MSB bit of the UDF field. */
    bp = udf_populate->udf_num_bits - 1;

    /* Skip the iterations if user has not configured UDF. */
    sal_memset(&null_param, 0, sizeof(xflow_macsec_flow_udf_param_t));

    if (!memcmp(udf_param, &null_param, sizeof(xflow_macsec_flow_udf_param_t)))
    {
        return BCM_E_NONE;
    }

    udf_map_count = udf_populate->udf_map_count;

    /* Iterate over all packet types. */
    for (iter_map = 0; iter_map < udf_map_count; iter_map++)
    {
        if (udf_map[iter_map].udf_pkt_type != udf_pkt_type_l)
        {
            continue;
        }

        /* For the given packet type, iterate over all UDF parameters. */
        for (iter_udf_param = 0; iter_udf_param < 10; iter_udf_param++)
        {
            udf_param_type =
                udf_map[iter_map].bit_info[iter_udf_param].udf_type;
            if (udf_param_type == _xflowMacsecUdfInvalid)
            {
                continue;
            }
            if (bp < 0)
            {
                /* num_bits total exceeds the udf_num_bits. */
                return BCM_E_PARAM;
            }
            sal_memset(temp_word, 0, sizeof(temp_word));
            num_bits = udf_map[iter_map].bit_info[iter_udf_param].num_bits;
            num_bits_t = num_bits;

            /*
             * Convert the user configured paramter and copy to temp_word[].
             * temp_word[] will be populated from the MSB defined by num_bits.
             * For example, if num_bits is 48,
             * temp_word[1] valid range is [31 - 0].
             * temp_word[0] valid range is [31 -16].
             */
            BCM_IF_ERROR_RETURN(xflow_macsec_udf_param_to_bytes(unit,
                            udf_param_type, udf_populate->udf_pkt_type, num_bits,
                            udf_param, temp_word));

            /* Iterate over all bits in the temp_word starting from num_bits. */
            for (iter = ((num_bits - 1) / 32); (iter >= 0); iter--)
            {
                shift = ((bp + 1) % 32);

                /* Shift is non-zero when base pointer is not aligned to 32. */
                if (shift)
                {
                    shift = 32 - shift;
                }

                /* Write to the higher word. */
                udf_words[bp / 32] |= (temp_word[iter] >> shift);
                if ((shift) && (bp / 32))
                {
                    /* If spillover, write to lower word. */
                    udf_words[(bp / 32) - 1] |=
                        (temp_word[iter] << (32 - shift));
                }

                /* Adjust base pointer for word boundary. */
                if (num_bits_t > 32) {
                    bp -= 32;
                    num_bits_t -= 32;
                }
            }
            if (num_bits_t > 0) {
                bp -= num_bits_t;
            }
        }
        break;
    }

    return BCM_E_NONE;
}
