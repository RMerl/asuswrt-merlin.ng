/*
 *  Copyright: (c) 2020 Broadcom.
 *  All rights reserved.
 *
 * <:label-BRCM:2022:DUAL/GPL:standard
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :>
 *
 * File:        memory.h
 * Purpose:     Base definitions for memory types
 * Requires:    
 */

#ifndef _SOC_MEMORY_H
#define _SOC_MEMORY_H

#include "soc/mem.h"
#include "soc/field.h"
#include "macsec_defs.h"
#include "macsec_types.h"
#include "soc/mcm/enum_max.h"
#include "soc/mcm/enum_types.h"

#include <soc/field.h>

#define SOC_MEM_FLAG_READONLY                0x00000001 /* True if table is read-only */
#define SOC_MEM_FLAG_VALID                   0x00000002 /* Some tables don't really exist */
#define SOC_MEM_FLAG_DEBUG                   0x00000004 /* Access requires CMIC DebugMode */
#define SOC_MEM_FLAG_SORTED                  0x00000008 /* Table is kept in sorted order */
#define SOC_MEM_FLAG_CBP                     0x00000010 /* Table is part of CBP */ /* Deprecated */
#define SOC_MEM_FLAG_TCAM_ENCODING_LPT       0x00000010 /* LPT-encoding for TCAM */
#define SOC_MEM_FLAG_CACHABLE                0x00000020 /* Reasonable to cache in S/W */
#define SOC_MEM_FLAG_BISTCBP                 0x00000040 /* CBP has BIST for this memory */ /* Deprecated */
#define SOC_MEM_FLAG_SER_ENTRY_CLEAR         0x00000040 /* Clear entry on error */
#define SOC_MEM_FLAG_BISTEPIC                0x00000080 /* EPIC has BIST for this memory */ /* Deprecated */
#define SOC_MEM_FLAG_SER_CACHE_RESTORE       0x00000080 /* Restore entry from a valid cache on error */
#define SOC_MEM_FLAG_BISTBIT                 0x00003f00 /* Bit pos for BIST reg (0-63) */ /* Deprecated */
#define SOC_MEM_FLAG_BISTBSHFT               8          /* Shift corresponding to BISTBIT */ /* Deprecated */
#define SOC_MEM_FLAG_BISTFFP                 0x00004000 /* Use FFPBIST for this memory */ /* Deprecated */
#define SOC_MEM_FLAG_SER_PARITY_ENABLE_SKIP  0x00004000 /* Skip parity enabling per memory */
#define SOC_MEM_FLAG_SER_ECC_CORRECTABLE     0x00000100 /* Issue read+write to correct error */
#define SOC_MEM_FLAG_SER_SPECIAL             0x00000200 /* Memory needs special correction handling */
#define SOC_MEM_FLAG_SER_WRITE_CACHE_RESTORE 0x00000400 /* Maintain cache for correction purpose even 
                                                           for h/w updated memory. Cache updated on writes only */
#define SOC_MEM_FLAG_SER_SRAM_CORRECTION     0x00000800 /* Do SRAM XOR correction */
#define SOC_MEM_FLAG_SER_ENGINE              0x00001000 /* Memory protected by SER engine */
#define SOC_MEM_FLAG_SCHAN_ERROR_RETURN      0x00002000 /* Return error on SCHAN NAK due to SER error */
#define SOC_MEM_FLAG_IS_ARRAY                0x00008000 /* Is this a memory array */
#define SOC_MEM_FLAG_UNIFIED                 0x00010000 /* Only one copy of table */
#define SOC_MEM_FLAG_HASHED                  0x00020000 /* Hashed table */
#define SOC_MEM_FLAG_WORDADR                 0x00040000 /* Requires special addressing */
#define SOC_MEM_FLAG_CAM                     0x00080000 /* Memory is a CAM */
#define SOC_MEM_FLAG_AGGR                    0x00100000 /* Memory is a AGGREGATE */
#define SOC_MEM_FLAG_CMD                     0x00200000 /* Memory allows CMDMEM access */
#define SOC_MEM_FLAG_DO_NOT_LOG_TIMEOUT      0x00400000 /* Allow timeout for the given memory */
#define SOC_MEM_FLAG_ACC_TYPE                0x07c00000 /* Access type */
#define SOC_MEM_FLAG_BE                      0x08000000 /* Big endian */
#define SOC_MEM_FLAG_MULTIVIEW               0x10000000 /* Multiview memories */
#define SOC_MEM_FLAG_DYNAMIC                 0x20000000 /* Memory field will be updated by hardware */
#define SOC_MEM_FLAG_EXT_CAM                 0x40000000 /* Memory is a external CAM */
#define SOC_MEM_FLAG_SIGNAL                  0x10000000 /* can be read/ can be write but not compare */
#define SOC_MEM_FLAG_WRITEONLY               0x80000000 /* True if table is write-only */


#define SOC_MEM_SER_FLAGS (SOC_MEM_FLAG_SER_ENTRY_CLEAR |\
                           SOC_MEM_FLAG_SER_CACHE_RESTORE |\
                           SOC_MEM_FLAG_SER_WRITE_CACHE_RESTORE |\
                           SOC_MEM_FLAG_SER_ECC_CORRECTABLE |\
                           SOC_MEM_FLAG_SER_SPECIAL)

#define SOC_MEM_SER_CORRECTION_TYPE(unit, mem) \
    (SOC_MEM_INFO(unit, mem).flags & SOC_MEM_SER_FLAGS)

#define SOC_MEM_FLAG_ACC_TYPE_MASK      0x1f
#define SOC_MEM_FLAG_ACC_TYPE_SHIFT     22
#define SOC_MEM_ACC_TYPE(unit, mem)  ((SOC_MEM_INFO(unit, mem).flags >> \
                                       SOC_MEM_FLAG_ACC_TYPE_SHIFT) &   \
                                      SOC_MEM_FLAG_ACC_TYPE_MASK)

#define SOC_MEM_BASE_TYPE(unit, mem)  ((SOC_MEM_INFO(unit, mem).base >> \
 	 	 	                          23) & 0x7)

#define SOC_MEM_RETURN_SCHAN_ERR(unit, mem) \
    (SOC_MEM_INFO(unit, mem).flags & SOC_MEM_FLAG_SCHAN_ERROR_RETURN)

#define SOC_MEM_RETURN_SCHAN_ERR_SET(unit, mem, enable) \
     if (enable) SOC_MEM_INFO(unit, mem).flags |= SOC_MEM_FLAG_SCHAN_ERROR_RETURN; \
     else SOC_MEM_INFO(unit, mem).flags &= ~SOC_MEM_FLAG_SCHAN_ERROR_RETURN

/* Flags for memory snooping callback register */
#define SOC_MEM_SNOOP_WRITE         0x00000001 /* snooping write operations */
#define SOC_MEM_SNOOP_READ          0x00000002 /* snooping read operations */
#define SOC_MEM_SNOOP_WRITE_COUNT   0x00000004 /* counting all memory write */
#define SOC_MEM_SNOOP_READ_COUNT    0x00000008 /* counting all memory read */
#define SOC_MEM_SNOOP_UNREGISTER    0x10000000 /* unregister callback */

/* Flags for showing memories that have been read/written */
#define SOC_MEM_WATCH_SHOW_READ         0x00000001 /* show rd memories */
#define SOC_MEM_WATCH_SHOW_WRITE        0x00000002 /* show wr memories */
#define SOC_MEM_WATCH_SHOW_READ_WRITE   0x00000003 /* show rd/wr memories */

/*
 * Declarations for null table entries.
 *
 * A null table entry is the data normally written to each entry when a
 * memory is cleared.  This is generally all zeroes, except for sorted
 * tables fields which make up the search key are generally all f's.
 */

extern uint32   _soc_mem_entry_null_zeroes[SOC_MAX_MEM_WORDS];
extern uint32   _soc_mem_entry_null_word0[SOC_MAX_MEM_WORDS];

#if defined(BCM_PETRA_SUPPORT)
/* DPP: Special entries format for specific tables */
extern uint32		_soc_mem_entry_multicast_null[SOC_MAX_MEM_WORDS];
extern uint32       _soc_mem_entry_jer_mcdb_null[SOC_MAX_MEM_WORDS];
extern uint32		_soc_mem_entry_llr_llvp_null[SOC_MAX_MEM_WORDS];
#endif /* BCM_PETRA_SUPPORT */

typedef int (*soc_mem_cmp_t)(int, void *, void *);
typedef void (*soc_mem_snoop_cb_t) (int unit, soc_mem_t mem, uint32 flags, 
                                    int copyno, int index_min, int index_max, 
                                    void *data_buffer, void *user_data);


typedef uint8 soc_mem_severe_t;
#define SOC_MEM_SEVERE_MISSING  0
#define SOC_MEM_SEVERE_LOW      1
#define SOC_MEM_SEVERE_MEDIUM   2
#define SOC_MEM_SEVERE_HIGH     3


/* 
* 1.	single port memory: 
* a memory that has one input port that can use for write or read. i.e, in each clock the user can access the memory only for one operation (write or read)
* possible access by the user: 
* idle
* write
* read
* 2.	two port memory: 
* a memory that has two ports. one port dedicated for write and one port dedicated for read. i.e, in each clock the user can access the memory for write and read (but not two writes/reads as ports are dedicated for specific operations)
* possible access by the user: 
* idle
* write
* read
* write + read
* 3.	dual port memory:
* a memory that has two ports, each port can use for write and read. i.e, in each clock the user can access for w and r/ w and w/ r and r 
* possible access by the user: 
* idle
* write
* read
* write + write 
* read + read
* write + read
* 4.	xor memory: 
* this memory has one port that dedicated to write operation and two ports that dedicated for read operations.
* The writing in opportunistic and not available in each clock due to the internal implementation. 
* This memory name is due to its internal implementation (it uses xor bank). it has also other specifications.
*/

typedef uint8 soc_mem_type_t;
#define SOC_MEM_TYPE_NORMAL      0
#define SOC_MEM_TYPE_ONE_PORT    1
#define SOC_MEM_TYPE_TWO_PORT    2
#define SOC_MEM_TYPE_XOR         3
#define SOC_MEM_TYPE_CUSTOM      4
 

typedef uint8 soc_mem_ecc_type_t;
#define SOC_MEM_ECC_TYPE_WIDE    0
#define SOC_MEM_ECC_TYPE_NORMAL  1
#define SOC_MEM_ECC_TYPE_IN      2
#define SOC_MEM_ECC_TYPE_OUT     3
#define SOC_MEM_ECC_TYPE_NONE    4


/*  Memory impementation type */
typedef uint8 soc_mem_impl_t;
#define SOC_MEM_IMPL_NORMAL      0
#define SOC_MEM_IMPL_FLIPFLOP    1
#define SOC_MEM_IMPL_EXTERNAL    2



#define SOC_MEM_SEVERITY(unit, mem) \
    (SOC_MEM_INFO(unit, mem).severity)

#define SOC_MEM_TYPE(unit, mem) \
    (SOC_MEM_INFO(unit, mem).type)


#define SOC_MEM_ECC_TYPE(unit, mem) \
    (SOC_MEM_INFO(unit, mem).ecc_type)


/* additional information for memories which are arrays */
/* To be used if in soc_mem_info_t, the SOC_MEM_FLAG_IS_ARRAY bit is set */
typedef struct soc_mem_array_info_t {
    unsigned int numels;       /* number of memory elements in the array. */
    uint32   element_skip; /* how many bytes to skip between array elements */
    uint32   first_array_index;
} soc_mem_array_info_t;


/* New unified soc memories structure */
typedef struct soc_mem_info_s {
    uint32  flags;  /* Logical OR of SOC_MEM_FLAG_xxx */
    soc_mem_cmp_t   cmp_fn; /* Function to compare two entries */
    void    *null_entry;    /* Value when table entry is empty */
    int     index_min;  /* Minimum table index */
    int     index_max;  /* Maximum table index */
    uint16  minblock;   /* first block instance */
    uint16  maxblock;   /* last block instance */
    uint32  blocks;     /* bitmask of valid blocks */
    uint32  blocks_hi;  /* bitmask 32:64 of valid blocks */
    uint32  base;       /* Includes region offset */
    uint32  gran;           /* Phys addr granularity by index */
    uint32  data_bits;  /* num of data bits all fields except ECC/Parity */
    uint16  bytes;
    uint16  nFields;
    uint32  mem_sel;
    uint32  mem_id;
    soc_field_info_t    *fields;
    char                **views;
    int                 *views_num;
    soc_mem_snoop_cb_t  snoop_cb; /* User call back for snooping memory */
    void                *snoop_user_data; /* User data for callback function */
    uint32              snoop_flags;    /* Snooping flags SOC_MEM_SNOOP_* */
    soc_mem_severe_t  severity; 
    soc_mem_type_t  type; 
    soc_mem_ecc_type_t  ecc_type; 
    soc_mem_impl_t      implementation; 
    uint32              cmic_mor_clks; /*  pending clock value*/
    uint32              access_latency_mode; /* accessibility in latency mode */
    int                 pipe_stage; /* pipe stage number / offset of the memory */
} soc_mem_info_t;

/* soc format structure */

typedef struct soc_format_info_s {
    uint16           nFields;
    soc_field_info_t *fields;
    uint16           bits; /* width of the format in number of bits */
    uint32           flags;
} soc_format_info_t;

typedef struct soc_dop_s {
    uint32             dop_id;
    uint16             n_data_phases;
    soc_format_t       dop_fmt;
} soc_dop_t;

#define SOC_MEM_IP6_FULL_ADDR       0   /* IPv6 address all 128 bits  */
#define SOC_MEM_IP6_UPPER_ONLY      1   /* IPv6 address upper 64 bits */
#define SOC_MEM_IP6_LOWER_ONLY      2   /* IPv6 address lower 64 bits */
#define SOC_MEM_IP6_UPPER_96BIT     4   /* IPv6 address upper 96 bits */
#define SOC_MEM_IP6_BITS_119_96     5   /* IPv6 address bits [119:96] */
#define SOC_MEM_IP6_LOWER_96BIT     6   /* IPv6 address lower 96 bits */
#define SOC_MEM_IP6_BITS_31_0       7   /* IPv6 address bits [31:0]   */
#define SOC_MEM_IP6_BITS_63_32      8   /* IPv6 address bits [63:32]   */

#define SOC_MEM_MAC_FULL_ADDR       0   /* Mac address all 48 bits  */
#define SOC_MEM_MAC_UPPER_ONLY      1   /* Mac address upper 24 bits */
#define SOC_MEM_MAC_LOWER_ONLY      2   /* Mac address lower 24 bits */

typedef enum soc_mem_base_to_wide_conv_type_e {
    TYPE_1_TO_TYPE_2,
    TYPE_1_TO_TYPE_4,
    TYPE_2_TO_TYPE_4,
    TYPE_2_TO_TYPE_1,
    TYPE_4_TO_TYPE_1,
    TYPE_4_TO_TYPE_2
} soc_mem_base_to_wide_conv_type_t;

uint32 soc_mem_addr(int unit, soc_mem_t mem, unsigned memory_index, int block, int index);
soc_mem_t soc_addr_to_mem(int unit, uint32 address, uint32 *mem_block);
soc_mem_t soc_addr_to_mem_extended(int unit, uint32 block, int acc_type, uint32 address);
int soc_mem_addr_to_array_element_and_index(int unit, soc_mem_t mem, uint32 address, unsigned* array_index, int* index);

uint32 *soc_mem_field_get(int unit, soc_mem_t mem, const uint32 *entbuf,
                          soc_field_t field, uint32 *fldbuf);
void soc_mem_field_set(int unit, soc_mem_t mem, uint32 *entbuf,
                       soc_field_t field, uint32 *fldbuf);
void soc_mem_field_width_fit_set(int unit, soc_mem_t mem, uint32 *entbuf,
                                 soc_field_t field, uint32 *fldbuf);

extern int soc_mem_field_pbmp_fit(int unit, soc_mem_t mem, 
                                  soc_field_t field, uint32 *value);
uint32 soc_mem_field32_get(int unit, soc_mem_t mem, const void *entbuf,
                           soc_field_t field);
uint32
soc_mem_field32_get_def(int         unit,
                        soc_mem_t   mem,
                        const void  *entbuf,
                        soc_field_t field,
                        uint32      def
                        );
void soc_mem_field32_set(int unit, soc_mem_t mem, void *entbuf,
                         soc_field_t field, uint32 value);
void soc_mem_field32_force(int unit, soc_mem_t mem, void *entbuf,
                           soc_field_t field, uint32 value);
uint32 *soc_mem_mask_field_get(int unit, soc_mem_t mem, const uint32 *entbuf,
                               soc_field_t field, uint32 *fldbuf);
void soc_mem_mask_field_set(int unit, soc_mem_t mem, uint32 *entbuf,
                            soc_field_t field, uint32 *fldbuf);
uint32 soc_mem_mask_field32_get(int unit, soc_mem_t mem, const void *entbuf,
                                soc_field_t field);
void soc_mem_mask_field32_set(int unit, soc_mem_t mem, void *entbuf,
                              soc_field_t field, uint32 value);
void soc_mem_mac_addr_set(int unit, soc_mem_t mem, void *entry,
                          soc_field_t field, const sal_mac_addr_t mac);
void soc_mem_mac_addr_get(int unit, soc_mem_t mem, const void *entry,
                          soc_field_t field, sal_mac_addr_t mac);
extern void soc_mem_field64_set(int unit, soc_mem_t mem, void *entry,
                                soc_field_t field, const uint64 val64);
extern void soc_mem_field64_get(int unit, soc_mem_t mem, void *entry,
                                soc_field_t field, uint64 *val64);
extern void soc_mem_datamask_get(int unit, soc_mem_t mem, uint32 *buf);
extern void soc_mem_datamask_rw_get(int unit, soc_mem_t mem, uint32 *buf);

extern void soc_mem_tcammask_get(int unit, soc_mem_t mem, uint32 *buf);
extern void soc_mem_eccmask_get(int unit, soc_mem_t mem, uint32 *buf);
extern void soc_mem_forcedata_get(int unit, soc_mem_t mem, uint32 *maskbuf,
                                  uint32 *databuf);

/* These functions do not require a unit number to manipulate mem vars */
uint32 *soc_meminfo_field_get(soc_mem_t mem, soc_mem_info_t *meminfo,
                              const uint32 *entbuf, soc_field_t field,
                              uint32 *fldbuf, uint32 fldbuf_size);
void soc_meminfo_mac_addr_set(soc_mem_t mem, soc_mem_info_t *meminfo,
                              void *entry, soc_field_t field,
                              const sal_mac_addr_t mac);
void soc_meminfo_field_set(soc_mem_t mem, soc_mem_info_t *meminfo,
                           uint32 *entbuf, soc_field_t field, uint32 *fldbuf);
void soc_meminfo_field32_force(soc_mem_t mem, soc_mem_info_t *meminfo,
                               void *entry, soc_field_t field, uint32 value);
void soc_mem_mac_address_set(int unit, soc_mem_t mem, void *entry,
                        soc_field_t field, const sal_mac_addr_t mac, int flags);
void soc_mem_mac_address_get(int unit, soc_mem_t mem, const void *entry,
                        soc_field_t field, sal_mac_addr_t mac, int flags);

void soc_mem_base_to_wide_entry_conv(int unit, soc_mem_t dest_mem, soc_mem_t src_mem, 
                                     uint32 *dest, uint32 *src[4], uint8 conv_type);

/* These functions accelerate bulk memory operations to a single table */
extern soc_field_info_t *soc_mem_fieldinfo_get(int unit, soc_mem_t mem,
                                               soc_field_t field);
extern uint32 *soc_meminfo_fieldinfo_field_get(const uint32 *entbuf,
                                               soc_mem_info_t *meminfo,
                                               soc_field_info_t *fieldinfo,
                                               uint32 *fldbuf);
extern uint32 soc_meminfo_fieldinfo_field32_get(soc_mem_info_t *meminfo,
                                                void *entry,
                                                soc_field_info_t *fieldinfo);
extern void soc_meminfo_fieldinfo_field64_get(soc_mem_info_t *meminfo,
                                              void *entry,
                                              soc_field_info_t *fieldinfo,
                                              uint64 *val64);

extern void soc_meminfo_fieldinfo_field_set(uint32 *entbuf,
                                               soc_mem_info_t *meminfo,
                                               soc_field_info_t *fieldinfo,
                                               uint32 *fldbuf);

extern void soc_meminfo_fieldinfo_field32_set(soc_mem_info_t *meminfo,
                                                void *entry,
                                                soc_field_info_t *fieldinfo, 
                                                uint32 value);

extern void soc_meminfo_fieldinfo_field64_set(soc_mem_info_t *meminfo,
                                                void *entry,
                                                soc_field_info_t *fieldinfo, 
                                                uint64 val64);

/* Compacted memory access functions for bulk operations */
typedef struct soc_memacc_s {
    soc_mem_info_t *minfo;
    soc_field_info_t *finfo;
} soc_memacc_t;

extern int soc_memacc_init(int unit, soc_mem_t mem, soc_field_t fld,
                           soc_memacc_t *memacc);

extern uint32 *soc_memacc_field_get(soc_memacc_t *memacc,
                                    const uint32 *entbuf,
                                    uint32 *fldbuf);
extern uint32 soc_memacc_field32_get(soc_memacc_t *memacc,
                                     void *entry);
extern void soc_memacc_field64_get(soc_memacc_t *memacc,
                                   void *entry,
                                   uint64 *val64);
extern void soc_memacc_mac_addr_get(soc_memacc_t *memacc, void *entry,
                                    sal_mac_addr_t mac);

extern void soc_memacc_field_set(soc_memacc_t *memacc,
                                    uint32 *entbuf,
                                    uint32 *fldbuf);
extern void soc_memacc_field32_set(soc_memacc_t *memacc,
                                     void *entry, uint32 value);
extern void soc_memacc_field64_set(soc_memacc_t *memacc,
                                   void *entry,
                                   uint64 val64);
extern void soc_memacc_mac_addr_set(soc_memacc_t *memacc, void *entry,
                                    const sal_mac_addr_t mac);

#define SOC_MEMACC_FIELD_LENGTH(_memacc_) ((_memacc_)->finfo->len)
#define SOC_MEMACC_VALID(_memacc_) \
        ((NULL != (_memacc_)->minfo) && (NULL != (_memacc_)->finfo))

#define SOC_MEMACC_INVALID_SET(_memacc_) \
        ((_memacc_)->minfo) = NULL; \
        ((_memacc_)->finfo) = NULL

/* Routines to register/unregister user call back routine for memory snooping */
void soc_mem_snoop_register(int unit, soc_mem_t mem, uint32 flags,
                           soc_mem_snoop_cb_t snoop_cb, void *user_data);
void soc_mem_snoop_unregister(int unit, soc_mem_t mem, uint32 flags);

#endif	/* !_SOC_MEMORY_H */
