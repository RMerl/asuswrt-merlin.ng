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
 */

#ifndef XFLOW_MACSEC_COMMON_H
#define XFLOW_MACSEC_COMMON_H

#include "macsec_defs.h"
#include "soc/feature.h"
#define BCM_MAX_NUM_UNITS 4

#if 0
/* Structure to store user callback information. */
typedef struct {
    xflow_macsec_event_cb cb;
    void *user_data;
} xflow_macsec_callback_data_t;

extern xflow_macsec_callback_data_t _xflow_macsec_callback_data[BCM_MAX_NUM_UNITS];
#endif
/*******************************************************************************
 * Macro definitions
 */
/* To be chaned if table size changes. */
#define _XFLOW_MACSEC_ID_MAX            1024
#define _XFLOW_MACSEC_SC_ENCRYPT_MAX    1024
#define _XFLOW_MACSEC_SC_DECRYPT_MAX    1024
#define _XFLOW_MACSEC_SA_ENCRYPT_MAX    1024
#define _XFLOW_MACSEC_SA_DECRYPT_MAX    1024
#define _XFLOW_MACSEC_POLICY_MAX        1024

#define _XFLOW_MACSEC_TFLAGS_VALID  1 /* Thread valid. */
#define _XFLOW_MACSEC_UDF_BIT_INFO_MAX  10

#ifdef BCM_HURRICANE4_SUPPORT
/* Lower 4 bits of macsec sub-port number for the ingress management packets.*/
#define _XFLOW_MACSEC_SUBPORT_NUM_MGMT_PKT_OFFSET  0xF
#define _XFLOW_MACSEC_SUBPORT_NUM_PER_PORT  0x10
#endif

#ifdef BCM_WARM_BOOT_SUPPORT
#define XFLOW_MACSEC_WB_VERSION_1_0         SOC_SCACHE_VERSION(1,0)
#define XFLOW_MACSEC_WB_DEFAULT_VERSION     XFLOW_MACSEC_WB_VERSION_1_0
#endif

#define XFLOW_MACSEC_4_SA_PER_SC(_u) (num_sa_per_sc[_u] == 4)
#define XFLOW_MACSEC_NUM_SA_PER_SC(_u, dir)   \
        ((soc_feature(_u, soc_feature_xflow_macsec_inline) == 0) ? 0 : \
                                                                (dir * 2))
#define XFLOW_MACSEC_NUM_RSVD_SC        48
/* Macros */

#define XFLOW_MACSEC_PBMP_COUNT(_p) _shr_popcount(_p)
#define _XFLOW_MACSEC_THREAD_INFO(unit) _xflow_macsec_thread_info[unit]
#define _XFLOW_MACSEC_THREAD_IS_VALID(thread_info)     \
        (((thread_info)->tflags & _XFLOW_MACSEC_TFLAGS_VALID) ? 1 : 0)

#define _XFLOW_MACSEC_ENCRYPT_DECRYPT_CHECK(flags)      \
    if ((flags & XFLOW_MACSEC_ENCRYPT) &&               \
        (flags & XFLOW_MACSEC_DECRYPT)) {               \
        return BCM_E_PARAM;                             \
    }

#define _BCM_FIELD32_LEN_CHECK(_unit, _mem, _fld, _val)             \
    do {                                                            \
        int f_rv = soc_mem_field32_fit(_unit, _mem, _fld, _val);    \
        if (f_rv != SOC_E_NONE) {                                   \
            return f_rv;                                            \
        }                                                           \
    } while (0)

#define _BCM_FIELD32_LEN_CHECK_WITH_RV(_unit, _mem, _fld, _val, _rv)        \
    do {                                                                    \
        int f_rv = soc_mem_field32_fit(_unit, _mem, _fld, _val);            \
        if (f_rv != SOC_E_NONE) {                                           \
            _rv = f_rv;                                                     \
        }                                                                   \
    } while (0)

#define _SHR_BIT_FIND_FREE(_a, max, _b)                         \
    do {                                                        \
        for ((_b) = 0; (_b) < max; (_b) = (_b) + SHR_BITWID) {  \
            if ((_a)[(_b) / SHR_BITWID] != 0xffffffff)          \
            break;                                              \
        }                                                       \
        while ((_b) < max) {                                    \
            if (SHR_BITGET((_a),(_b))) {                        \
                (_b)++;                                         \
            } else {                                            \
                break;                                          \
            }                                                   \
        }                                                       \
    } while(0)

#define _XFLOW_MACSEC_MEM_NULL_CHECK_AND_SET(_a, _s)        \
    do {                                                    \
        if (_a == NULL) {                                   \
            return BCM_E_MEMORY;                            \
        } else {                                            \
            sal_memset(_a, 0, _s);                          \
        }                                                   \
    } while(0)

#define _XFLOW_MACSEC_MEM_NULL_CHECK_AND_SET_RV(_a, _s, rv) \
    do {                                                    \
        rv = BCM_E_NONE;                                    \
        if (_a == NULL) {                                   \
            rv = BCM_E_MEMORY;                              \
        } else {                                            \
            sal_memset(_a, 0, _s);                          \
        }                                                   \
    } while(0)

#define _XFLOW_MACSEC_MEM_NULL_CHECK_SET_RV_GOTO_ERROR(_a, _s, rv, _e)  \
    do {                                                                \
        rv = BCM_E_NONE;                                                \
        if (_a == NULL) {                                               \
            rv = BCM_E_MEMORY;                                          \
            goto _e;                                                    \
        } else {                                                        \
            sal_memset(_a, 0, _s);                                      \
        }                                                               \
    } while(0)

#define _XFLOW_MACSEC_BCM_FAILURE_GOTO_ERROR(_rv, _e)       \
    do {                                                    \
        if (BCM_FAILURE(rv)) {                              \
            goto _e;                                        \
        }                                                   \
    } while(0)

#define _BCM_MAC_ADDR_TO_UINT32_ARRAY(_dst32, _mac_addr)        \
        SAL_MAC_ADDR_TO_UINT32(_mac_addr, _dst32)

#define _BCM_UINT32_ARRAY_TO_MAC_ADDR(_mac_addr, _src32)        \
        SAL_MAC_ADDR_FROM_UINT32(_mac_addr, _src32)

#define XFLOW_MAC_ADDR_TO_UDF_UINT32(mac, dst) do {     \
        (dst)[1] = (((uint32)(mac)[0]) << 24 |          \
                  ((uint32)(mac)[1]) << 16 |            \
                  ((uint32)(mac)[2]) << 8 |             \
                  ((uint32)(mac)[3]));                  \
        (dst)[0] = ((((uint32)(mac)[4]) << 24 |         \
                  ((uint32)(mac)[5]) << 16)  &          \
                  0xffff0000);                          \
    } while (0)

#define XFLOW_MAC_ADDR_FROM_UDF_UINT32(src, mac) do {   \
        (mac)[0] = (uint8) (((src)[1] >> 24) & 0xff);   \
        (mac)[1] = (uint8) (((src)[1] >> 16) & 0xff);   \
        (mac)[2] = (uint8) (((src)[1] >> 8) & 0xff);    \
        (mac)[3] = (uint8) (((src)[1]) & 0xff);    \
        (mac)[4] = (uint8) (((src)[0] >> 24) & 0xff);   \
        (mac)[5] = (uint8) (((src)[0] >> 16) & 0xff);   \
    } while (0)

#define XFLOW_MACSEC_CALLBACK_SET(_u, _cb, _ud)         \
    do {                                                \
        _xflow_macsec_callback_data[_u].cb = _cb;       \
        _xflow_macsec_callback_data[_u].user_data = _ud;\
    } while(0)

#define XFLOW_MACSEC_CB_DATA(unit)  _xflow_macsec_callback_data[unit]
#define XFLOW_MACSEC_DB_DATA_GET_CB(_u)       \
        _xflow_macsec_callback_data[_u].cb
#define XFLOW_MACSEC_DB_DATA_GET_USER_DATA(_u)       \
        _xflow_macsec_callback_data[_u].user_data

/*******************************************************************************
 * Structures and Enums
 */

extern uint8* sc_table_dma;
extern uint8* sa_table_dma;

extern int num_sa_per_sc[BCM_MAX_NUM_UNITS];
extern int inline_xflow_macsec;

//sal_mutex_t xflow_macsec_lock[BCM_MAX_NUM_UNITS];

/* The logical map provides a mapping between the logical priority of
 * a TCAM entry and its actual index in the Hardware. The input priority
 * during reserve(add) will be used for lookup.
 */
typedef struct xflow_macsec_logical_tcam_map_s {
    int in_use;
    int priority;
    uint32 hw_index;
} xflow_macsec_tcam_logical_map_t;

/* The TCAM PTR structure will provide a software map of the hardware index
 * allocations. This will be indexed by the TCAM HW index and will be used for
 * priority based search.
 */
typedef struct xflow_macsec_tcam_ptr_s {
    int in_use;
    int priority;
    uint32 tcam_logical_map_id;
} xflow_macsec_tcam_ptr_t;

typedef struct _xflow_macsec_sc_tcam_map_s {
    int reserved_index; /* One reserved index for shifting entries in HW */
    int free_count;     /* Total number of free entries. */
    int tcam_mem_count; /* Total number of memories to modify when a TCAM
                           modification occurs. */
    soc_mem_t *tcam_mem; /* Head node should always be a TCAM. */

    int tcam_index_count; /* Total number of entries in the map. */
    xflow_macsec_tcam_logical_map_t  *tcam_logical_map;
    xflow_macsec_tcam_ptr_t          *tcam_ptr;
} xflow_macsec_tcam_map_t;

typedef enum _xflow_macsec_tcam_shift_e {
    _XFLOW_MACSEC_TCAM_SHIFT_UP,
    _XFLOW_MACSEC_TCAM_SHIFT_DOWN,
    _XFLOW_MACSEC_TCAM_COUNT
} _xflow_macsec_tcam_shift_t;

/* Enums for SA expiration events. */
typedef enum {
    _xflowMacsecIntrEgrSoftSAExpired = 0,
    _xflowMacsecIntrEgrSAExpired,
    _xflowMacsecIntrIngSoftSAExpired,
    _xflowMacsecIntrIngSAExpired,
    _xflowMacsecIntrCount,
} intr_status_flags_t;

/* Structure for macsec event generation. */
typedef struct _xflow_macsec_thread_info_s {
    uint32 tflags;              /* Flags */
//    sal_sem_t macsec_trigger;   /* Notify the thread of interrupt. */
//    sal_sem_t macsec_thread;    /* Notify main thread during exit. */

    /* WAR CRMACSEC-383 */
    uint32 poll_interval;       /* Duration of each poll. */
} _xflow_macsec_thread_info_t;

#ifdef BCM_HURRICANE4_SUPPORT
typedef struct _xflow_macsec_sc_dgpp_map_s {
    int is_gport;
    bcm_gport_t gport;
} _xflow_macsec_sc_dgpp_map_t;
_xflow_macsec_sc_dgpp_map_t sc_dgpp_map [BCM_MAX_NUM_UNITS][_XFLOW_MACSEC_SC_ENCRYPT_MAX];
#endif

typedef enum xflow_macsec_flow_udf_pkt_type_e{
    _xflowMacsecUdfPktVlan1     = 0,
    _xflowMacsecUdfPktVlan2     = 1,
    _xflowMacsecUdfPktMpls1     = 2,
    _xflowMacsecUdfPktMpls2     = 3,
    _xflowMacsecUdfPktOther     = 4,
    _xflowMacsecUdfPktEII       = 5,
    _xflowMacsecUdfPktMpls3     = 6,
    _xflowMacsecUdfPktPbb       = 7,
    _xflowMacsecUdfPktVntag     = 8,
    _xflowMacsecUdfPktEtag      = 9,
    _xflowMacsecUdfPktIPv4      = 10,
    _xflowMacsecUdfPktIPv6      = 11,
    _xflowMacsecUdfPktL4IPv4    = 12,
    _xflowMacsecUdfPktL4IPv6    = 13,
    _xflowMacsecUdfPktTypeCount = 14,

} xflow_macsec_flow_udf_pkt_type_t;

typedef enum xflow_macsec_flow_udf_param_type_e {
    _xflowMacsecUdfInvalid          = 0,
    _xflowMacsecUdfEthertype        = 1,
    _xflowMacsecUdfFirstVlan        = 2,
    _xflowMacsecUdfSecondVlan       = 3,
    _xflowMacsecUdfThirdVlan        = 4,
    _xflowMacsecUdfFourthVlan       = 5,
    _xflowMacsecUdfProtocolId       = 6,
    _xflowMacsecUdfFirstMpls        = 7,
    _xflowMacsecUdfSecondMpls       = 8,
    _xflowMacsecUdfFirstMplsSbit    = 9,
    _xflowMacsecUdfSecondMplsSbit   = 10,
    _xflowMacsecUdfThirdMplsSbit    = 11,
    _xflowMacsecUdfFourthMplsSbit   = 12,
    _xflowMacsecUdfSipAddr          = 13,
    _xflowMacsecUdfDipAddr          = 14,
    _xflowMacsecUdfSourcePort       = 15,
    _xflowMacsecUdfDestPort         = 16,
    _xflowMacsecUdfOuterSrcMac      = 17,
    _xflowMacsecUdfOuterDstMac      = 18,
    _xflowMacsecUdfInnerSrcMac      = 19,
    _xflowMacsecUdfInnerDstMac      = 20,
    _xflowMacsecUdfInnerFirstVlan   = 21,
    _xflowMacsecUdfInnerSecondVlan  = 22,
    _xflowMacsecUdfBbtagVidPCP      = 23,
    _xflowMacsecUdfItagPcpIsid      = 24,
    _xflowMacsecUdfEtagTci          = 25,
    _xflowMacsecUdfPayload          = 26,
    _xflowMacsecUdfend              = 27
} xflow_macsec_flow_udf_param_type_t;

typedef struct xflow_macsec_flow_udf_byte_location_s {
    xflow_macsec_flow_udf_param_type_t udf_type;
    uint32 num_bits;
} xflow_macsec_flow_udf_bit_info_t;

typedef struct xflow_macsec_flow_udf_map_s {
    xflow_macsec_flow_udf_pkt_type_t udf_pkt_type;
    xflow_macsec_flow_udf_bit_info_t bit_info[_XFLOW_MACSEC_UDF_BIT_INFO_MAX];
} xflow_macsec_flow_udf_map_t;

typedef struct xflow_macsec_udf_populate_s {
    /* Width of UDF field. */
    int udf_num_bits;

    /* Pointer to UDF_MAP array. */
    const xflow_macsec_flow_udf_map_t *udf_map;

    /* Number of elements in the UDF_MAP array. */
    int udf_map_count;

    /* Generic UDF type. */
    xflow_macsec_flow_udf_pkt_type_t udf_pkt_type;
} xflow_macsec_udf_populate_t;

/* Statistics */

typedef enum _xflow_macsec_counter_id_e {
    _xflowMacsecCid0         = 0,
    _xflowMacsecCid1         = 1,
    _xflowMacsecCid2         = 2,
    _xflowMacsecCid3         = 3,
    _xflowMacsecCid4         = 4,
    _xflowMacsecCid5         = 5,
    _xflowMacsecCid6         = 6,
    _xflowMacsecCid7         = 7,
    _xflowMacsecCid8         = 8,
    _xflowMacsecCid9         = 9,
    _xflowMacsecCid10        = 10,
    _xflowMacsecCid11        = 11,
    _xflowMacsecCid12        = 12,
    _xflowMacsecCid13        = 13,
    _xflowMacsecCidCount     = 14,
    _xflowMacsecCidInvalid   = 15,
    _xflowMacsecCidSpecial   = 16
} _xflow_macsec_counter_id_t;

typedef struct _xflow_macsec_stat_map_s {

    xflow_macsec_stat_type_t stat_type;

    _xflow_macsec_counter_id_t counter_id;

    soc_field_t field;

    uint8 dir;

    xflow_macsec_index_type_t id_type;

    _xflow_macsec_counter_id_t rollover_reg_id;

    /* Rollover not implemented. */
    uint32  rollover_bit;

    /* sw_value: The actual 64bit value.
     * Default setting is to clear on read in Macsec.
     */
    uint64 *sw_value;
} _xflow_macsec_stat_map_t;

typedef struct _xflow_macsec_counter_regs_s {
    _xflow_macsec_counter_id_t id;
    soc_mem_t mem;
    soc_reg_t reg;
    int min_index;
    int num_entries;
    uint32 *buf;
} _xflow_macsec_counter_regs_t;

typedef struct xflow_macsec_db_s {
    /* Reserved policy index for no flow match. */
    int reserved_policy_index;

    /* Port based macsec enabled for at least one port. */
    int port_based_macsec;

    /*
     * Counter array. Each entry represents a counter
     * register/table entry.
     * Used to DMA the tables from HW.
     */
    _xflow_macsec_counter_regs_t
        *_xflow_macsec_counter_array;

    /*
     * Stats map table. This table is used to associate
     * a statistics to an entry in the counter array along
     * with the register.
     * Used to read individual stats and present it to user.
     */
    _xflow_macsec_stat_map_t
        *_xflow_macsec_stat_map_list;

} xflow_macsec_db_t;

/*******************************************************************************
 * Function declarations
 */

#ifdef BCM_WARM_BOOT_SUPPORT

extern int
_xflow_macsec_wb_resource_reinit_version_1_0(int unit, uint8 **scache_ptr);
extern int
_xflow_macsec_wb_resource_sync_version_1_0(int unit, uint8 **scache_ptr);
#endif /* BCM_WARM_BOOT_SUPPORT */

extern
void uint32_to_byte_array(int num_bytes, uint32 *val32, uint8 *byte_array);
extern
void byte_array_to_uint32(int num_bytes, uint8 *byte_array, uint32 *val32);
extern int
_xflow_macsec_counters_init (int unit);
extern int
_xflow_macsec_counters_deinit (int unit);
extern void
_xflow_macsec_counters_collect(int unit);
extern int
_xflow_macsec_resource_init(int unit);
extern int
_xflow_macsec_resource_deinit (int unit);

extern int
xflow_macsec_secure_chan_enable_get(int unit,
                                    xflow_macsec_secure_chan_id_t chan_id,
                                    int *enable);
extern int
xflow_macsec_secure_chan_enable_set(int unit,
                                    xflow_macsec_secure_chan_id_t chan_id,
                                    int enable);
extern int
_xflow_macsec_sc_create_encrypt(int unit, int sc_index,
                                xflow_macsec_secure_chan_info_t *chan);
extern int
_xflow_macsec_sc_get_encrypt(int unit, int sc_index,
                                xflow_macsec_secure_chan_info_t *chan_info);
extern int
_xflow_macsec_sc_destroy_encrypt(int unit, int sc_index);

extern int
_xflow_macsec_sc_enable_set_encrypt(int unit,
                                    int sc_index, int enable);

extern int
_xflow_macsec_sc_enable_get_encrypt(int unit,
                                    int sc_index, int *enable);
extern int
_xflow_macsec_sa_create_encrypt(int unit, int sa_index,
                                xflow_macsec_secure_assoc_info_t *assoc_info,
                                xflow_macsec_secure_chan_info_t *chan_info);

extern int
_xflow_macsec_sa_get_encrypt(int unit, int sa_index,
                             xflow_macsec_secure_assoc_info_t *assoc_info,
                             xflow_macsec_secure_chan_info_t *chan_info);

extern int
_xflow_macsec_sa_destroy_encrypt(int unit, int sa_index);

extern int
_xflow_macsec_sc_create_decrypt(int unit, int sc_index,
                                xflow_macsec_secure_chan_info_t *chan);
extern int
_xflow_macsec_sc_get_decrypt(int unit, int sc_index,
                                xflow_macsec_secure_chan_info_t *chan_info);
extern int
_xflow_macsec_sc_destroy_decrypt(int unit, int sc_index);

extern int
_xflow_macsec_sc_enable_set_decrypt(int unit,
                                    int sc_index, int enable);
extern int
_xflow_macsec_sc_enable_get_decrypt(int unit,
                                    int sc_index, int *enable);
extern int
_xflow_macsec_sa_create_decrypt(int unit, int sa_index,
                                xflow_macsec_secure_assoc_info_t *assoc_info,
                                xflow_macsec_secure_chan_info_t *chan_info);
extern int
_xflow_macsec_sa_get_decrypt(int unit, int sa_index,
                                xflow_macsec_secure_assoc_info_t *assoc_info,
                                xflow_macsec_secure_chan_info_t *chan_info);
extern int
_xflow_macsec_sa_destroy_decrypt(int unit, int sa_index);

extern int
_xflow_macsec_decrypt_policy_create (int unit,
                        xflow_macsec_policy_info_t *policy_info,
                        int policy_index);
extern int
_xflow_macsec_decrypt_policy_get(int unit, int policy_index,
                        xflow_macsec_policy_info_t *policy_info);
extern int
_xflow_macsec_decrypt_policy_destroy(int unit, int policy_index);

extern int
_xflow_macsec_decrypt_subport_flow_create(int unit,
                        xflow_macsec_flow_info_t *flow_info,
                        int sp_index);
extern int
_xflow_macsec_decrypt_subport_flow_get (int unit,
                        xflow_macsec_flow_info_t *flow_info,
                        int sp_index);
extern int
_xflow_macsec_decrypt_subport_flow_destroy (int unit, int sp_index);

extern int
_xflow_macsec_decrypt_subport_flow_enable_set (int unit, int sp_index,
                                               int enable);
extern int
_xflow_macsec_decrypt_subport_flow_enable_get (int unit, int sp_index,
                                               int *enable);
extern int
_xflow_macsec_decrypt_sc_subport_get (int unit, int sc_index,
                                      int *subport_id);

int
_xflow_macsec_sc_index_resolve(int unit, int flags, int *sc_index);

int
_xflow_macsec_flow_index_resolve(int unit, int *sp_tcam_index);

extern int
_xflow_macsec_decrypt_policy_index_resolve(int unit, uint32 flags,
                        int *policy_index);

extern int
xflow_macsec_stat_get(int unit, uint32 flags, xflow_macsec_id_t id,
                       xflow_macsec_stat_type_t  stat_type,
                       uint64 *value);
extern int
xflow_macsec_stat_set(int unit, uint32 flags, xflow_macsec_id_t id,
                       xflow_macsec_stat_type_t  stat_type,
                       uint64 value);
extern int
xflow_macsec_stat_multi_get(
    int unit,
    uint32 flags,
    xflow_macsec_id_t id,
    uint32 num_stats,
    xflow_macsec_stat_type_t  *stat_type_array,
    uint64 *value_array);
extern int
xflow_macsec_stat_multi_set(
    int unit,
    uint32 flags,
    xflow_macsec_id_t id,
    uint32 num_stats,
    xflow_macsec_stat_type_t  *stat_type_array,
    uint64 *value_array);

extern int
xflow_macsec_index_reserve(int unit, int dir,
                            xflow_macsec_index_type_t type,
                            int prio,
                            int *index,
                            uint8 flag);
extern int
xflow_macsec_index_free(int unit, int dir,
                         xflow_macsec_index_type_t type,
                         int index);
extern int
xflow_macsec_index_get(int unit, int dir,
                        xflow_macsec_index_type_t type,
                        int index, int *hw_index);
extern int
xflow_macsec_prio_get(int unit, int dir,
                        xflow_macsec_index_type_t type,
                        int index, int *prio);
extern int
xflow_macsec_logical_index_get(int unit, int dir,
                        xflow_macsec_index_type_t type,
                        int hw_index, int *index);

extern int
_xflow_macsec_wb_resource_alloc_size(int unit, int *scache_size);

extern int
xflow_macsec_index_update_priority(int unit, xflow_macsec_index_type_t type,
                                   int index, int priority);
extern int
_xflow_macsec_sc_encrypt_counter_clear(int unit, int sc_index);
extern int
_xflow_macsec_sa_encrypt_counter_clear(int unit, int sa_index);
extern int
_xflow_macsec_sc_decrypt_counter_clear(int unit, int sc_index);
extern int
_xflow_macsec_sa_decrypt_counter_clear(int unit, int sa_index);
extern int
_xflow_macsec_flow_decrypt_counter_clear(int unit, int flow_index);
extern int
_xflow_macsec_policy_decrypt_counter_clear(int unit, int policy_index);
extern int
_xflow_macsec_tcam_ptr_counter_move_single_tcam_entry(int unit,
                                                      int to_index,
                                                      int from_index,
                                                      soc_mem_t *mem_arr,
                                                      int mem_arr_count);
extern int
xflow_macsec_flow_udf_populate(int unit,
                               xflow_macsec_flow_udf_param_t *udf_param,
                               xflow_macsec_udf_populate_t *udf_populate,
                               uint32 *udf_words);
extern int
xflow_macsec_flow_udf_retrieve(int unit, uint32 *udf_words,
                               xflow_macsec_udf_populate_t *udf_retrieve,
                               xflow_macsec_flow_udf_param_t *udf_param);
extern void
xflow_macsec_db_init (int unit);

extern int
xflow_macsec_db_set (int unit, xflow_macsec_db_t *db);

extern int
xflow_macsec_db_get (int unit, xflow_macsec_db_t **db);
#endif /* XFLOW_MACSEC_COMMON_H */
