/***********************************************************************
 *
 *  Copyright (c) 2021  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:DUAL/GPL:standard

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
 *
************************************************************************/

#ifndef MACSEC_MACROS_H
#define MACSEC_MACROS_H

/*
 * Generic Helpers 
 */
#define BITS2BYTES(x)   (((x) + 7) / 8)
#define BITS2WORDS(x)   (((x) + 31) / 32)
#define BYTES2BITS(x)   ((x) * 8)
#define BYTES2WORDS(x)  (((x) + 3) / 4)
#define WORDS2BITS(x)   ((x) * 32)
#define WORDS2BYTES(x)  ((x) * 4)

/*
 * Compiler abstractions
 */
#define COMPILER_64_TO_32_LO(dst, src)  ((dst) = (uint32) (src))
#define COMPILER_64_TO_32_HI(dst, src)  ((dst) = (uint32) ((src) >> 32))
#define COMPILER_64_HI(src)             ((uint32) ((src) >> 32))
#define COMPILER_64_LO(src)             ((uint32) (src))
#define COMPILER_64_ZERO(dst)           ((dst) = 0)
#define COMPILER_64_IS_ZERO(src)        ((src) == 0)

#define COMPILER_64_COPY(dst, src)      (dst = src)
#define COMPILER_REFERENCE(_a)          ((void)(_a))

#define COMPILER_64_SET(dst, src_hi, src_lo)                \
    ((dst) = (((uint64) ((uint32)(src_hi))) << 32) | ((uint64) ((uint32)(src_lo))))

/*
 * Generic operation macro on bit array _a, with bit _b
 */
#define SHR_BITWID              32
#define SHR_BITDCL              uint32
#define _SHR_BITDCLSIZE(_max)   (((_max) + SHR_BITWID - 1) / SHR_BITWID)

#define    _SHR_BITOP(_a, _b, _op)    \
        (((_a)[(_b) / SHR_BITWID]) _op (1U << ((_b) % SHR_BITWID)))

#define SHR_BITGET(_a, _b)          _SHR_BITOP(_a, _b, &)
#define SHR_BITSET(_a, _b)          _SHR_BITOP(_a, _b, |=)
#define SHR_BITCLR(_a, _b)          _SHR_BITOP(_a, _b, &= ~)
#define SHR_BITWRITE(_a, _b, _val)  ((_val) ? SHR_BITSET(_a, _b) : SHR_BITCLR(_a, _b))
#define SHR_IS_BITSET(_a, _b)       (_SHR_BITOP(_a, _b, &) ? TRUE : FALSE)

#define SHR_BIT_ITER(_a, _max, _b)               \
           for ((_b) = 0; (_b) < (_max); (_b)++) \
               if ((_a)[(_b) / SHR_BITWID] == 0) \
                   (_b) += (SHR_BITWID - 1);     \
               else if (SHR_BITGET((_a), (_b)))

/* 
 * MAC address helper macros
 */

/* Adjust justification for uint32 writes to fields */
/* dst is an array name of type uint32 [] */
#define SAL_MAC_ADDR_TO_UINT32(mac, dst) do {   \
        (dst)[0] = (((uint32)(mac)[2]) << 24 |  \
                  ((uint32)(mac)[3]) << 16 |    \
                  ((uint32)(mac)[4]) << 8 |     \
                  ((uint32)(mac)[5]));          \
        (dst)[1] = (((uint32)(mac)[0]) << 8 |   \
                  ((uint32)(mac)[1]));          \
    } while (0)

/* Adjust justification for uint32 writes to fields */
/* src is an array name of type uint32 [] */
#define SAL_MAC_ADDR_FROM_UINT32(mac, src) do {     \
        (mac)[0] = (uint8) ((src)[1] >> 8 & 0xff);  \
        (mac)[1] = (uint8) ((src)[1] & 0xff);       \
        (mac)[2] = (uint8) ((src)[0] >> 24);        \
        (mac)[3] = (uint8) ((src)[0] >> 16 & 0xff); \
        (mac)[4] = (uint8) ((src)[0] >> 8 & 0xff);  \
        (mac)[5] = (uint8) ((src)[0] & 0xff);       \
    } while (0)

/* dst is a uint64 */
#define SAL_MAC_ADDR_TO_UINT64(mac, dst) do {   \
        uint32 _val[2];                         \
        SAL_MAC_ADDR_TO_UINT32(mac, _val);      \
        COMPILER_64_SET(dst, _val[1], _val[0]); \
    } while (0)

/* src is a uint64 */
#define SAL_MAC_ADDR_FROM_UINT64(mac, src) do { \
        uint32 _val[2];                         \
        COMPILER_64_TO_32_LO(_val[0], src);     \
        COMPILER_64_TO_32_HI(_val[1], src);     \
        SAL_MAC_ADDR_FROM_UINT32(mac, _val);    \
    } while (0)

/****************************************************************
 * UNIT DRIVER ACCESS MACROS
 *
 *         MACRO                             EVALUATES TO
 *  ________________________________________________________________
 *      SOC_DRIVER(unit)                Chip driver structure
 *      SOC_INFO(unit)                  SOC Info structure
 *      SOC_MEM_INFO(unit,mem)          Memory info structure
 *      SOC_REG_INFO(unit,reg)          Register info structure
 *      SOC_BLOCK_INFO(unit,blk)        Block info structure
 *      SOC_PORT_INFO(unit,port)        Port info structure
 *      SOC_BLOCK2SCH(unit,blk)         Integer schan num for block
 *      SOC_BLOCK2OFFSET(unit,blk)      Block to idx for cmic cmds
 *      SOC_HAS_CTR_TYPE(unit, ctype)   Does the device have a given
 *                                      counter map defined?
 *      SOC_CTR_DMA_MAP(unit, ctype)    Return pointer to the counter
 *                                      map of the indicated type.
 *      SOC_CTR_TO_REG(unit, ctype, ind) Return the register index for
 *                                       a given counter index.
 *      SOC_CTR_MAP_SIZE(unit, ctype)   How many entries in a given
 *                                      counter map.
 ****************************************************************/

#define SOC_CONTROL(unit)               (soc_control[unit])
#define SOC_DRIVER(unit)                (SOC_CONTROL(unit)->chip_driver)
#define SOC_FUNCTIONS(unit)             (SOC_CONTROL(unit)->soc_functions)
#define SOC_LED_DRIVER(unit)            (SOC_CONTROL(unit)->soc_led_driver)
#define SOC_PORTCTRL_FUNCTIONS(unit)    (SOC_CONTROL(unit)->soc_portctrl_functions)
#define SOC_INFO(unit)                  (SOC_CONTROL(unit)->info)
#define SOC_STAT(unit)                  (&(SOC_CONTROL(unit)->stat))
#define SOC_REG_MASK_SUBSET(unit)       (SOC_CONTROL(unit)->reg_subset_mask)
#define DRV_SERVICES(unit)              (SOC_DRIVER(unit)->services)
#define SOC_MEM_INFO(unit, mem)         (*SOC_DRIVER(unit)->mem_info[mem])
#define SOC_MEM_AGGR(unit, index)       (SOC_DRIVER(unit)->mem_aggr[index])
#define SOC_MEM_PTR(unit, mem)          (SOC_DRIVER(unit)->mem_info[mem])
#define SOC_REG_INFO(unit, reg)         (*SOC_DRIVER(unit)->reg_info[reg])
#define SOC_REG_STAGE(unit, reg)        ((SOC_REG_INFO(unit, reg).offset >> 26) & 0x3F)
#define SOC_REG_UNIQUE_ACC(unit, reg)   (SOC_DRIVER(unit)->reg_unique_acc[reg])
#define SOC_REG_ABOVE_64_INFO(unit, reg) (*SOC_DRIVER(unit)->reg_above_64_info[reg])
#define SOC_REG_ARRAY_INFO(unit, reg)   (*SOC_DRIVER(unit)->reg_array_info[reg])
#define SOC_REG_ARRAY_INFOP(unit, reg)  (SOC_DRIVER(unit)->reg_array_info[reg])
#define SOC_MEM_ARRAY_INFO(unit, mem)   (*SOC_DRIVER(unit)->mem_array_info[mem])
#define SOC_MEM_ARRAY_INFOP(unit, mem)  (SOC_DRIVER(unit)->mem_array_info[mem])
#define SOC_REG_PTR(unit, reg)          (SOC_DRIVER(unit)->reg_info[reg])
#define SOC_BLOCK_INFO(unit, blk)       (SOC_DRIVER(unit)->block_info[blk])
#define SOC_BLOCK_INSTANCES(unit, type) (SOC_DRIVER(unit)->block_instances[type])
#define SOC_FORMAT_INFO(unit, format)   (*SOC_DRIVER(unit)->format_info[format])
#define SOC_FORMAT_PTR(unit, format)       (SOC_DRIVER(unit)->format_info[format])
#define SOC_DOP_INFO(unit)               (SOC_DRIVER(unit)->dop_info)
#define SOC_MEM_UNIQUE_ACC(unit, mem)   \
    (SOC_DRIVER(unit)->mem_unique_acc ? \
     (SOC_DRIVER(unit)->mem_unique_acc[mem]) : NULL)


/*
 * Internal memory table access macros
 */
#define SOC_MEM_IS_ARRAY(unit, mem) \
    (SOC_MEM_INFO(unit, mem).flags & SOC_MEM_FLAG_IS_ARRAY)

#define SOC_MEM_IS_ARRAY_SAFE(unit, mem)    \
    ( SOC_MEM_IS_ARRAY(unit, mem) && SOC_MEM_ARRAY_INFOP(unit, mem) )

#define SOC_MEM_ELEM_SKIP(unit, mem)    \
    ((SOC_IS_ARAD(unit) && (mem == NBI_TBINS_MEMm || mem == NBI_RBINS_MEMm))?     \
    (SOC_MEM_INFO(unit, mem).index_max - SOC_MEM_INFO(unit, mem).index_min + 1) : (SOC_MEM_ARRAY_INFO(unit, mem).element_skip))

#define SOC_MEM_NUMELS(unit, mem)   \
    (SOC_MEM_ARRAY_INFO(unit, mem).numels)

#define SOC_MEM_FIRST_ARRAY_INDEX(unit, mem)    \
    (SOC_MEM_ARRAY_INFO(unit, mem).first_array_index)

#define SOC_MEM_ELEM_SKIP_SAFE(unit, mem)   \
    ( SOC_MEM_ARRAY_INFOP(unit, mem) ? SOC_MEM_ARRAY_INFOP(unit, mem)->element_skip : 0 )

#define SOC_MEM_NUMELS_SAFE(unit, mem)  \
    ( SOC_MEM_ARRAY_INFOP(unit, mem) ? SOC_MEM_ARRAY_INFOP(unit, mem)->numels : 1 )

#define SOC_MEM_IS_VALID(unit, mem)         \
    ((mem >= 0 && mem < NUM_SOC_MEM) &&     \
     (SOC_CONTROL(unit) != NULL) &&         \
     (SOC_DRIVER(unit) != NULL) &&          \
     (SOC_MEM_PTR(unit, mem) != NULL) &&    \
     (SOC_MEM_INFO(unit, mem).flags & SOC_MEM_FLAG_VALID))

#define SOC_REG_IS_VALID(unit, reg)                            \
        ((reg >= 0 && reg < NUM_SOC_REG)  &&                   \
         (SOC_REG_PTR(unit, reg) != NULL) &&                   \
         (SOC_REG_INFO(unit, reg).regtype != soc_invalidreg))

#define SOC_REG_IS_ENABLED(unit, reg)           \
    (SOC_REG_IS_VALID(unit, reg) &&             \
     !(SOC_REG_INFO(unit, reg).flags &          \
       SOC_CONTROL(unit)->disabled_reg_flags))

#define SOC_MEM_BYTES(unit, mem)        (SOC_MEM_INFO(unit, mem).bytes)
#define SOC_MEM_WORDS(unit, mem)        (BYTES2WORDS(SOC_MEM_BYTES(unit, mem)))
#define SOC_MEM_BASE(unit, mem)         (SOC_MEM_INFO(unit, mem).base)

#define SOC_MEM_SIZE(unit, mem)             \
    (SOC_MEM_INFO(unit, mem).index_max -    \
     SOC_MEM_INFO(unit, mem).index_min + 1)
     
#define SOC_MEM_TABLE_BYTES(unit, mem)          \
    (4 * SOC_MEM_WORDS(unit, mem) * \
     (SOC_MEM_INFO(unit, mem).index_max -       \
      SOC_MEM_INFO(unit, mem).index_min + 1))

/*
 * Error and debug macros
 */

 #ifdef DEBUG_ERR_TRACE

#define _SHR_ERROR_TRACE(__errcode__)   LOG_ERROR(BSL_LS_SOC_DIAG, \
                                        (BSL_META("ERROR(%d)\n"), __errcode__))

#define _SHR_RETURN(__expr__)                                           \
                    do { int __errcode__ = (__expr__);                  \
                    if (__errcode__ < 0) _SHR_ERROR_TRACE(__errcode__); \
                    return (__errcode__); } while (0)

#else

#define _SHR_ERROR_TRACE(__errcode__)
#define _SHR_RETURN(__expr__)  return (__expr__)

#endif

#define _SHR_E_IF_ERROR_RETURN(op) \
    do { int __rv__; if ((__rv__ = (op)) < 0) { _SHR_ERROR_TRACE(__rv__);  return(__rv__); } } while(0)

#define _SHR_E_IF_ERROR_CLEAN_RETURN(op,exop) \
    do { int __rv__; if ((__rv__ = (op)) < 0) { _SHR_ERROR_TRACE(__rv__); (exop); return(__rv__); } } while(0)

#define _SHR_E_IF_ERROR_NOT_UNAVAIL_RETURN(op)                      \
    do {                                                            \
        int __rv__;                                                 \
        if (((__rv__ = (op)) < 0) && (__rv__ != _SHR_E_UNAVAIL)) {  \
            return(__rv__);                                         \
        }                                                           \
    } while(0)

#define BCM_FAILURE(rv) _SHR_E_FAILURE(rv) 
#define BCM_IF_ERROR_RETURN(op) _SHR_E_IF_ERROR_RETURN(op) 


/* Misc Registers */
#define BCHP_UINT64_C(hi, lo)   (((uint64)hi)<<32 | (lo))

/*
 * m = memory, c = core, r = register, f = field, d = data.
 */
#define BRCM_MASK(c,r,f)    c##_##r##_##f##_MASK
#define BRCM_SHIFT(c,r,f)   c##_##r##_##f##_SHIFT

#define MACSEC_XLMAC_PORT_REG(reg, port)            (BCHP_XPORT_XLMAC_CORE_0_PORT0_##reg + (port << 12))
#define MACSEC_XLMAC_PORT_FIELD_SHIFT(reg, field)   (BCHP_XPORT_XLMAC_CORE_0_PORT0_##reg##_##field##_SHIFT)
#define MACSEC_XLMAC_PORT_FIELD_MASK(reg, mask)     (BCHP_XPORT_XLMAC_CORE_0_PORT0_##reg##_##mask##_MASK)

#define MACSEC_ISEC_PORT_REG(reg, port)             (BCHP_ISEC_PORT_0_ISEC_##reg + (port << 12))
#define MACSEC_ISEC_PORT_FIELD_SHIFT(reg, field)    (BCHP_ISEC_PORT_0_ISEC_##reg##_##field##_SHIFT)
#define MACSEC_ISEC_PORT_FIELD_MASK(reg, mask)      (BCHP_ISEC_PORT_0_ISEC_##reg##_##mask##_MASK)

#define MACSEC_ESEC_PORT_REG(reg, port)             (BCHP_ESEC_PORT_0_##reg + (port << 12))
#define MACSEC_ESEC_PORT_FIELD_SHIFT(reg, field)    (BCHP_ESEC_PORT_0_##reg##_##field##_SHIFT)
#define MACSEC_ESEC_PORT_FIELD_MASK(reg, mask)      (BCHP_ESEC_PORT_0_##reg##_##mask##_MASK)

#define MACSEC_ISEC_GENERAL_REG(reg, port)          (BCHP_ISEC_GENERAL_##reg + (port << 12))
#define MACSEC_ISEC_GENERAL_FIELD_SHIFT(reg, field) (BCHP_ISEC_GENERAL_##reg##_##field##_SHIFT)
#define MACSEC_ISEC_GENERAL_FIELD_MASK(reg, mask)   (BCHP_ISEC_GENERAL_##reg##_##mask##_MASK)

#define MACSEC_ESEC_GENERAL_REG(reg, port)          (BCHP_ESEC_GENERAL_##reg + (port << 12))
#define MACSEC_ESEC_GENERAL_FIELD_SHIFT(reg, field) (BCHP_ESEC_GENERAL_##reg##_##field##_SHIFT)
#define MACSEC_ESEC_GENERAL_FIELD_MASK(reg, mask)   (BCHP_ESEC_GENERAL_##reg##_##mask##_MASK)

#define MACSEC_GENERAL_REG(reg, port)               (BCHP_MACSEC_GENERAL_##reg + (port << 12))
#define MACSEC_GENERAL_FIELD_SHIFT(reg, field)      (BCHP_MACSEC_GENERAL_##reg##_##field##_SHIFT)
#define MACSEC_GENERAL_FIELD_MASK(reg, mask)        (BCHP_MACSEC_GENERAL_##reg##_##mask##_MASK)

#define MACSEC_REG(reg, port)                       (BCHP_MACSEC_##reg + (port << 12))
#define MACSEC_FIELD_SHIFT(reg, field)              (BCHP_MACSEC_##reg##_##field##_SHIFT)
#define MACSEC_FIELD_MASK(reg, mask)                (BCHP_MACSEC_##reg##_##mask##_MASK)

#define ETH_PHY_TOP_REG_FIELD_SHIFT(reg, field)              (BCHP_ETH_PHY_TOP_REG_##reg##_##field##_SHIFT)
#define ETH_PHY_TOP_REG_FIELD_MASK(reg, mask)                (BCHP_ETH_PHY_TOP_REG_##reg##_##mask##_MASK)

#endif //MACSEC_MACROS_H
