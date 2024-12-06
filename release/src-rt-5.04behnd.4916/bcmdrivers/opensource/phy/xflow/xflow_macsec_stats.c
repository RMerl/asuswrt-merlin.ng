/*
 *  Copyright: (c) 2020 Broadcom.
 *  All Rights Reserved
 *
 * <:label-BRCM:2022:DUAL/GPL:standard
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
 */
#include "soc/mcm/enum_max.h"
#include "soc/mcm/enum_types.h"
#include "soc/mem.h"
#include "soc/memory.h"
#include "soc/field.h"
#include <soc/mcm/allenum.h>
#include "soc/mcm/memregs.h"
#include "soc/feature.h"
#include "soc/68880/bchp_regs_int.h"
#include "xflow_macsec_defs.h"
#include "xflow_macsec.h"
#include "xflow_macsec_esw_defs.h"
#include "xflow_macsec_common.h"
#include "xflow_macsec_firelight.h"
#include "xflow_macsec_cfg_params.h"
#include "macsec_dev.h"
#include "macsec_defs.h"
#include "macsec_types.h"
#include "macsec_macros.h"

#define SAL_BOOT_BCMSIM 0
#define SAL_BOOT_XGSSIM 0
#define COUNTOF(ary)        ((int) (sizeof (ary) / sizeof ((ary)[0])))

#define _XFLOW_MACSEC_STAT_SPECIAL_MAP_NUM  8
#define PRINTVS(...)

extern soc_control_t *soc_control[SOC_MAX_NUM_DEVICES];

typedef struct _xflow_macsec_stats_special_map_s {
    xflow_macsec_stat_type_t    stat_from;
    xflow_macsec_stat_type_t    stat_to;
    int oper;
} _xflow_macsec_stats_special_map_t;

static _xflow_macsec_stats_special_map_t
    _xflow_macsec_stat_special_map[_XFLOW_MACSEC_STAT_SPECIAL_MAP_NUM] =
{
    {xflowMacsecSecyTxSCStatsProtectedPkts,
            xflowMacsecSecyTxSAStatsProtectedPkts,
            XFLOW_MACSEC_ENCRYPT},
    {xflowMacsecSecyTxSCStatsEncryptedPkts,
            xflowMacsecSecyTxSAStatsEncryptedPkts,
            XFLOW_MACSEC_ENCRYPT},
    {xflowMacsecSecyRxSCStatsUnusedSAPkts,
            xflowMacsecSecyRxSAStatsUnusedSAPkts,
            XFLOW_MACSEC_DECRYPT},
    {xflowMacsecSecyRxSCStatsNotUsingSAPkts,
            xflowMacsecSecyRxSAStatsNotUsingSAPkts,
            XFLOW_MACSEC_DECRYPT},
    {xflowMacsecSecyRxSCStatsNotValidPkts,
            xflowMacsecSecyRxSAStatsNotValidPkts,
            XFLOW_MACSEC_DECRYPT},
    {xflowMacsecSecyRxSCStatsInvalidPkts,
            xflowMacsecSecyRxSAStatsInvalidPkts,
            XFLOW_MACSEC_DECRYPT},
    {xflowMacsecSecyRxSCStatsOKPkts,
            xflowMacsecSecyRxSAStatsOKPkts,
            XFLOW_MACSEC_DECRYPT},
    {xflowMacsecBadOlpHdrCntr,
            xflowMacsecBadOlpHdrCntr,
            XFLOW_MACSEC_ENCRYPT_DECRYPT_NONE},
};

int
_xflow_macsec_stat_set(int unit, uint32 flags, xflow_macsec_id_t id,
                       xflow_macsec_stat_type_t  stat_type,
                       uint64 value);


/*
 * The function below is used to process the DMA'd buffer
 * and store values for each stat_type in its own buffer.
 * NOTE: This is required assuming Clear on Read. This is not
 * required if COR is false and can be read from DMA buffer.
 */
static int
_xflow_macsec_counters_process_buf(int unit)
{
    int i, j, is_sim, oper;
    int index_min, index_max;
    _xflow_macsec_counter_id_t counter_id;
    _xflow_macsec_counter_regs_t *counter;
    _xflow_macsec_stat_map_t stat_map, stat_map1;
    xflow_macsec_db_t *db;
    uint64 val;
    int index, num_index = 0;
    void *pentry;

    is_sim = (SAL_BOOT_BCMSIM || SAL_BOOT_XGSSIM) ? 1 : 0;

    PRINTVS("%s:\n", __FUNCTION__);
    BCM_IF_ERROR_RETURN
        (xflow_macsec_db_get(unit, &db));
    for (i = 0; i < xflowMacsecStatTypeCount; i++) {
        stat_map = db->_xflow_macsec_stat_map_list[i];

        /* extra protection for empty stat maps */
        if(stat_map.stat_type != i) {
            continue;
        }
#if 0
        if (xflow_macsec_driver[unit]->xflow_macsec_counters_stat_skip) {
            BCM_IF_ERROR_RETURN
                (xflow_macsec_driver[unit]->
                    xflow_macsec_counters_stat_skip(unit, i, &val32));
            if (val32) {
                continue;
            }
        }
#endif
        counter_id = stat_map.counter_id;
        if (counter_id == _xflowMacsecCidSpecial) {
            continue;
        }
        counter = &(db->_xflow_macsec_counter_array[counter_id]);
        index_min = counter->min_index;
        index_max = (counter->min_index + counter->num_entries - 1);
        PRINTVS("\tStat_type %d\n", i);
        if (counter->mem != INVALIDm) {
            for (j = index_min; j <= index_max; j++) {
                pentry = soc_mem_table_idx_to_pointer(unit, counter->mem,
                        void*,
                        counter->buf, j);
                soc_mem_field64_get(unit, counter->mem, pentry,
                        stat_map.field, &val);

                /*
                 * The code below assumes that the counters are
                 * clear on read (on device).
                 */
                if (is_sim) {
                    COMPILER_64_COPY(stat_map.sw_value[j], val);
                } else {
                    stat_map.sw_value[j] += val;
                }
                if (val || stat_map.sw_value[j]) {
                    PRINTVS("\t\tIndex %d, mem %d Stat type %d sw_value"\
                            " 0x%x%08x\n", j,
                            counter->mem,
                            stat_map.stat_type,
                            COMPILER_64_HI(stat_map.sw_value[j]),
                            COMPILER_64_LO(stat_map.sw_value[j]));
                }
            }
        }
    }
    for (i = 0; i < _XFLOW_MACSEC_STAT_SPECIAL_MAP_NUM; i++) {
        stat_map = db->_xflow_macsec_stat_map_list
                        [_xflow_macsec_stat_special_map[i].stat_from];
        stat_map1 = db->_xflow_macsec_stat_map_list
                        [_xflow_macsec_stat_special_map[i].stat_to];
        if (stat_map.id_type != xflowMacsecIdTypeSecureChan) {
            continue;
        }
        PRINTVS("\tSpecial_map stat_type %d\n",stat_map.stat_type);
        counter_id = stat_map1.counter_id;
        counter = &(db->_xflow_macsec_counter_array[counter_id]);
        index_min = counter->min_index;
        index_max = (counter->min_index + counter->num_entries - 1);
        oper = _xflow_macsec_stat_special_map[i].oper;
        if (XFLOW_MACSEC_4_SA_PER_SC(unit)) {
            num_index = 4;
            index_max = index_max  / 4;
        } else if (XFLOW_MACSEC_NUM_SA_PER_SC(unit, oper)) {
            num_index = XFLOW_MACSEC_NUM_SA_PER_SC(unit, oper);
            index_max = index_max  / XFLOW_MACSEC_NUM_SA_PER_SC(unit, oper);
        } else {
            num_index = 1;
        }
        for (j = index_min; j <= index_max; j++) {
            if (XFLOW_MACSEC_NUM_SA_PER_SC(unit, oper)) {
                index = j * XFLOW_MACSEC_NUM_SA_PER_SC(unit, oper);
            } else {
                index = (XFLOW_MACSEC_4_SA_PER_SC(unit)) ? (j * 4) : j;
            }
            pentry = soc_mem_table_idx_to_pointer(unit, counter->mem, void*,
                    counter->buf, index);
            soc_mem_field64_get(unit, counter->mem, pentry,
                    stat_map1.field, &val);

            if (is_sim) {
                COMPILER_64_COPY(stat_map.sw_value[j], val);
            } else {
                stat_map.sw_value[j] += val;
            }
            if (num_index > 1) {
                pentry = soc_mem_table_idx_to_pointer(unit, counter->mem, void*,
                        counter->buf, index + 1);
                soc_mem_field64_get(unit, counter->mem, pentry,
                        stat_map1.field, &val);
                stat_map.sw_value[j] += val;
                if (num_index > 2) {
                    pentry = soc_mem_table_idx_to_pointer(unit, counter->mem, void*,
                            counter->buf, index + 2);
                    soc_mem_field64_get(unit, counter->mem, pentry,
                            stat_map1.field, &val);
                    stat_map.sw_value[j] += val;
                    pentry = soc_mem_table_idx_to_pointer(unit, counter->mem, void*,
                            counter->buf, index + 3);
                    soc_mem_field64_get(unit, counter->mem, pentry,
                            stat_map1.field, &val);
                    stat_map.sw_value[j] += val;
                }
            }
            if (val || stat_map.sw_value[j]) {
                PRINTVS("\t\tIndex %d, mem %d stat type %d sw_value"\
                        " 0x%x%08x\n", j,
                        counter->mem,
                        stat_map.stat_type,
                        COMPILER_64_HI(stat_map.sw_value[j]),
                        COMPILER_64_LO(stat_map.sw_value[j]));
            }
        }
    }
    PRINTVS("\n");
    return BCM_E_NONE;
}

/*
 * The function below is used to DMA all the required tables
 * and store in a DMA buffer. Since multiple stat_type counters can
 * be present in a single entry, this DMA buffer later needs to be
 * processed on a per stat_type basis.
 */
void _xflow_macsec_counters_collect(int unit)
{
    int i, rv;
    _xflow_macsec_counter_regs_t *counter;
    xflow_macsec_db_t *db;

    /* XFLOW_MACSEC_LOCK(unit); */

    PRINTVS("%s:\n",__FUNCTION__);
    (void)(xflow_macsec_db_get(unit, &db));
    
    for (i = 0; i < _xflowMacsecCidCount; i++)
    {
        counter = &(db->_xflow_macsec_counter_array[i]);
        if (counter->id >= _xflowMacsecCidCount)
        {
            continue;
        }
        PRINTVS("\tRead counter_id %d\n",counter->id);
        if (counter->mem != INVALIDm)
        {
            PRINTVS("\tRead range %d to %d of mem %d\n",counter->min_index,
                    (counter->min_index + counter->num_entries - 1), counter->mem);
            
            rv = soc_mem_read_range(unit, counter->mem, MEM_BLOCK_ANY, counter->min_index,
                                    (counter->min_index + counter->num_entries - 1), counter->buf);
            
            if (BCM_FAILURE(rv))
            {
                goto collect_error;
            }
        }
#if 0
        if (counter->reg != INVALIDr)
        {
            PRINTVS("\tRead reg %d\n",counter->reg);
            soc_ubus_reg32_get(unit, counter->reg, REG_PORT_ANY, counter->buf);
        }
#endif
    }
collect_error:
    /*
     *     etime1 = sal_time_usecs();
     */
    _xflow_macsec_counters_process_buf(unit);
    
    /* XFLOW_MACSEC_UNLOCK(unit); */
}

int _xflow_macsec_stat_get(int unit, uint32 flags, xflow_macsec_id_t id, xflow_macsec_stat_type_t  stat_type,
                            uint64 *value)
{
    int dir, rv;
    int index, index1;
    int id_type, id_type1;
    _xflow_macsec_stat_map_t *stat_map;
    xflow_macsec_db_t *db;
    int macsec_port = 0;

    if ((stat_type <= xflowMacsecStatTypeInvalid) || (stat_type >= xflowMacsecStatTypeCount))
        return BCM_E_PARAM;
    
    if (value == NULL)
        return BCM_E_PARAM;

    *value = 0;

    BCM_IF_ERROR_RETURN(xflow_macsec_db_get(unit, &db));
    
    stat_map = &(db->_xflow_macsec_stat_map_list[stat_type]);

    if (stat_map->stat_type != stat_type)
        return BCM_E_INTERNAL;

    if (stat_map->id_type == xflowMacsecIdTypeSubportNum)
    {
        if (XFLOW_MACSEC_ID_TYPE_GET(id) == xflowMacsecIdTypeSubportNum)
        {
            /* no conversion */
        }
#if 0
        else if ((XFLOW_MACSEC_ID_TYPE_GET(id) == xflowMacsecIdTypeSecureChan) ||
                 (XFLOW_MACSEC_ID_TYPE_GET(id) == xflowMacsecIdTypePolicy))
        {
            /* derive the subport ID */
            BCM_IF_ERROR_RETURN(xflow_macsec_subport_id_get(unit, id, &macsec_subport_id));
            id = macsec_subport_id;
        }
#endif
        else
        {
            return BCM_E_PARAM;
        }
    }

    /*
     * Derive the index to be read.
     */
    if (stat_map->id_type == xflowMacsecIdTypePort)
    {
        index = macsec_port;
        id_type = stat_map->id_type;
        dir = stat_map->dir;
    }
    else
    {
        /*
         * Derive the index based on input ID.
         */
        id_type = XFLOW_MACSEC_ID_TYPE_GET(id);
        if ((id_type < 0) || (id_type >= xflowMacsecIdTypeCount))
        {
            return BCM_E_PARAM;
        }
        dir = XFLOW_MACSEC_DIR_TYPE_GET(id);
        if ((dir != XFLOW_MACSEC_ENCRYPT) && (dir != XFLOW_MACSEC_DECRYPT))
        {
           return BCM_E_PARAM;
        }
        index = XFLOW_MACSEC_INDEX_GET(id);
    }

    /* Special handling */
    if (stat_type == xflowMacsecSecyStatsRxOverrunPkts)
    {
        return BCM_E_NONE;
    }

    if (((stat_map->id_type != xflowMacsecIdTypeInvalid) && (stat_map->id_type != id_type)) ||
         ((stat_map->dir != XFLOW_MACSEC_ENCRYPT_DECRYPT_NONE) && (stat_map->dir != dir)))
    {
        return BCM_E_PARAM;
    }

    /* Set for memories where only index 0 is valid */
    if (stat_map->id_type == xflowMacsecIdTypeInvalid)
    {
        index = 0;
    }
    else
    {
        if (index >= _XFLOW_MACSEC_ID_MAX)
        {
            return BCM_E_PARAM;
        }

        if ((id_type != xflowMacsecIdTypeSubportNum) && (id_type != xflowMacsecIdTypePort))
        {

            id_type1 = id_type;
            index1 = index;

            if (id_type == xflowMacsecIdTypeSecureAssoc) {
                id_type1 = xflowMacsecIdTypeSecureChan;
                if (XFLOW_MACSEC_NUM_SA_PER_SC(unit, dir)) {
                    index1 = index / (XFLOW_MACSEC_NUM_SA_PER_SC(unit, dir));
                } else {
                    index1 = (XFLOW_MACSEC_4_SA_PER_SC(unit)) ?
                                        (index / 4) : index;
                }
            }

            if (dir == XFLOW_MACSEC_DECRYPT)
                rv = _xflow_macsec_sc_decrypt_index_hw_index_get(unit, index1, &index1, NULL);
            else
                rv = _xflow_macsec_sc_encrypt_index_hw_index_get(unit, index1, &index1, NULL);
//            rv = xflow_macsec_index_get(unit, dir, id_type1, index1, &index1);
            if (BCM_FAILURE(rv)) {
                return BCM_E_PARAM;
            }

            if (id_type == xflowMacsecIdTypeSecureAssoc) {
                if (XFLOW_MACSEC_NUM_SA_PER_SC(unit, dir)) {
                    index1 = (index1 * XFLOW_MACSEC_NUM_SA_PER_SC(unit, dir)) +
                             (index % XFLOW_MACSEC_NUM_SA_PER_SC(unit, dir));
                } else if (XFLOW_MACSEC_4_SA_PER_SC(unit)) {
                    index1 = (4 * index1) + (index & 3);
                }
            }
            index = index1;
        }
    }

    {
        COMPILER_64_COPY(*value,stat_map->sw_value[index]);
        PRINTVS("Get value stat_type %d index %d value %d %d %d\n",
                stat_type, index,
                COMPILER_64_HI(stat_map->sw_value[index]),
                COMPILER_64_LO(stat_map->sw_value[index]),
                COMPILER_64_LO(db->_xflow_macsec_stat_map_list[stat_type]
                                            .sw_value[index]));
    }

    return BCM_E_NONE;
}

int
xflow_macsec_stat_get(int unit, uint32 flags, xflow_macsec_id_t id,
                       xflow_macsec_stat_type_t  stat_type,
                       uint64 *value)
{
    BCM_IF_ERROR_RETURN
        (_xflow_macsec_stat_get(unit, flags, id, stat_type, value));
    return BCM_E_NONE;
}
#if 0
/*
 * The code currently does not support HW rollover for stats.
 */
static int
_xflow_macsec_stat_read_and_update(int unit, _xflow_macsec_counter_id_t id,
                    xflow_macsec_stat_type_t stat_type, int index, uint64 value)
{
    uint32 *buf;
    int alloc_size;
    soc_mem_t mem;
    _xflow_macsec_counter_regs_t *counter;
    _xflow_macsec_stat_map_t stat_map;
    xflow_macsec_db_t *db;
    int rv;

    BCM_IF_ERROR_RETURN
        (xflow_macsec_db_get(unit, &db));
    counter = &(db->_xflow_macsec_counter_array[id]);
    if ((id == _xflowMacsecCidSpecial) ||
        (id == _xflowMacsecCidInvalid)) {
        return BCM_E_PARAM;
    }
    if (counter->mem == INVALIDm) {
        return BCM_E_NONE;
    }
    mem = counter->mem;
    alloc_size = sizeof(uint32) * soc_mem_entry_words(unit, mem);
    buf = sal_alloc(alloc_size, "counter_buf");
    _XFLOW_MACSEC_MEM_NULL_CHECK_AND_SET_RV(buf, alloc_size, rv);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    stat_map = db->_xflow_macsec_stat_map_list[stat_type];

    COMPILER_64_COPY(stat_map.sw_value[index], value);
    PRINTVS("Set value stat_type %d index %d value %d %d %d\n",
           stat_type, index,
           COMPILER_64_HI(stat_map.sw_value[index]),
           COMPILER_64_LO(stat_map.sw_value[index]),
           COMPILER_64_LO
                (db->_xflow_macsec_stat_map_list[stat_type]
                                .sw_value[index]));

    if (SAL_BOOT_BCMSIM || SAL_BOOT_XGSSIM) {
        /* Need to check if the steps below can lead to
         * clearing of adjacent counters */
        rv = soc_mem_read(unit, mem, MEM_BLOCK_ALL, index, buf);
        if (BCM_FAILURE(rv)) {
            sal_free(buf);
            return rv;
        }

        soc_mem_field64_set(unit, counter->mem, buf, stat_map.field, value);
        rv = soc_mem_write(unit, mem, MEM_BLOCK_ALL, index, buf);
        if (BCM_FAILURE(rv)) {
            sal_free(buf);
            return rv;
        }
    }

    sal_free(buf);
    return BCM_E_NONE;
}

/*
 * Function to handle special stat set case where
 * to set one stat type, we need to set another stat type.
 * For example, to set SC stat we may have to set SA stat.
 */
int
_xflow_macsec_special_stat_set(int unit, uint32 flags, xflow_macsec_id_t id,
                                xflow_macsec_stat_type_t  stat_type_to,
                                uint64 value)
{
    int index, index1, iter;
    int dir;
    _xflow_macsec_stat_map_t *stat_map;
    xflow_macsec_secure_assoc_id_t assoc_id;
    xflow_macsec_db_t *db;
    int num_sa = 1;

    index = XFLOW_MACSEC_INDEX_GET(id);
    BCM_IF_ERROR_RETURN
        (xflow_macsec_db_get(unit, &db));
    stat_map = &(db->_xflow_macsec_stat_map_list[stat_type_to]);
    if (stat_map->id_type != xflowMacsecIdTypeSecureAssoc) {
        return BCM_E_NONE;
    }
    if (index >= _XFLOW_MACSEC_ID_MAX) {
        return BCM_E_PARAM;
    }
    dir = XFLOW_MACSEC_DIR_TYPE_GET(id);
    if ((dir != XFLOW_MACSEC_ENCRYPT) && (dir != XFLOW_MACSEC_DECRYPT)) {
        return BCM_E_PARAM;
    }
    if (stat_map->dir != dir) {
        return BCM_E_PARAM;
    }

    if (XFLOW_MACSEC_NUM_SA_PER_SC(unit, dir)) {
        num_sa = XFLOW_MACSEC_NUM_SA_PER_SC(unit, dir);
    } else {
        num_sa = (XFLOW_MACSEC_4_SA_PER_SC(unit)) ? 4 : 1;
    }
    index1 = index * num_sa;
    COMPILER_64_UDIV_32(value, num_sa);
    for (iter = 0; iter < num_sa; iter++) {
        assoc_id = XFLOW_MACSEC_SECURE_ASSOC_ID_CREATE(dir, index1);
        BCM_IF_ERROR_RETURN
            (_xflow_macsec_stat_set(unit, flags,
                    (xflow_macsec_id_t) assoc_id, stat_type_to, value));
        index1++;
    }
    return BCM_E_NONE;
}

int
_xflow_macsec_stat_set(int unit, uint32 flags, xflow_macsec_id_t id,
                       xflow_macsec_stat_type_t  stat_type,
                       uint64 value)
{
    int dir;
    int index, index1;
    int id_type, id_type1;
    int rv = BCM_E_NONE;
    _xflow_macsec_stat_map_t *stat_map;
    int s_stat = 0;
    xflow_macsec_db_t *db;
    int macsec_port = 0;
    int num_sa = 1;
    xflow_macsec_subport_id_t macsec_subport_id;

    /* Special handling */
    if (stat_type == xflowMacsecSecyStatsRxOverrunPkts) {
        return BCM_E_NONE;
    }

    if ((stat_type <= xflowMacsecStatTypeInvalid) ||
        (stat_type >= xflowMacsecStatTypeCount)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN
        (xflow_macsec_db_get(unit, &db));
    stat_map = &(db->_xflow_macsec_stat_map_list[stat_type]);

    if (stat_map->stat_type != stat_type) {
        return BCM_E_INTERNAL;
    }

    if (stat_map->id_type == xflowMacsecIdTypeSubportNum) {
        if (XFLOW_MACSEC_ID_TYPE_GET(id) == xflowMacsecIdTypeSubportNum) {
            /* no conversion */
        } else if ((XFLOW_MACSEC_ID_TYPE_GET(id) == xflowMacsecIdTypeSecureChan) ||
                (XFLOW_MACSEC_ID_TYPE_GET(id) == xflowMacsecIdTypePolicy)) {
            /* derive the subport ID */
            BCM_IF_ERROR_RETURN
                (xflow_macsec_subport_id_get(unit, id, &macsec_subport_id));
            id = macsec_subport_id;
        } else {
            return BCM_E_PARAM;
        }
    }

    /* For cases where single SC maps to multiple SA, the SA must
     * be set directly.
     */
    for (index = 0; index < _XFLOW_MACSEC_STAT_SPECIAL_MAP_NUM; index++) {
        if (_xflow_macsec_stat_special_map[index].stat_from == stat_type) {
            BCM_IF_ERROR_RETURN
                (_xflow_macsec_special_stat_set (unit, flags, id,
                    _xflow_macsec_stat_special_map[index].stat_to, value));
            s_stat = 1;
        }
    }

    /*
     * Derive the index to be set.
     */
    if ((stat_map->id_type == xflowMacsecIdTypePort)) {
        index = macsec_port;
        id_type = stat_map->id_type;
        dir = stat_map->dir;
    } else {
        dir = XFLOW_MACSEC_DIR_TYPE_GET(id);
        if ((dir != XFLOW_MACSEC_ENCRYPT) && (dir != XFLOW_MACSEC_DECRYPT)) {
            return BCM_E_PARAM;
        }
        id_type = XFLOW_MACSEC_ID_TYPE_GET(id);
        if ((id_type < 0) || (id_type >= xflowMacsecIdTypeCount)) {
            return BCM_E_PARAM;
        }
        index = XFLOW_MACSEC_INDEX_GET(id);
    }

    if (((stat_map->id_type != xflowMacsecIdTypeInvalid) &&
         (stat_map->id_type != id_type)) ||
         ((stat_map->dir != XFLOW_MACSEC_ENCRYPT_DECRYPT_NONE) &&
         (stat_map->dir != dir))) {
        return BCM_E_PARAM;
    }

    /* Set for memories where only index 0 is valid */
    if (stat_map->id_type == xflowMacsecIdTypeInvalid) {
        index = 0;
    } else {
        if (index >= _XFLOW_MACSEC_ID_MAX) {
            return BCM_E_PARAM;
        }

        if ((id_type != xflowMacsecIdTypeSubportNum) &&
            (id_type != xflowMacsecIdTypePort)) {

            id_type1 = id_type;
            index1 = index;
            if (XFLOW_MACSEC_NUM_SA_PER_SC(unit, dir)) {
                num_sa = XFLOW_MACSEC_NUM_SA_PER_SC(unit, dir);
            } else {
                num_sa = (XFLOW_MACSEC_4_SA_PER_SC(unit)) ? 4 : 1;
            }

            if (id_type == xflowMacsecIdTypeSecureAssoc) {
                id_type1 = xflowMacsecIdTypeSecureChan;
                index1 = index / num_sa;
            }

            rv = xflow_macsec_index_get(unit, dir, id_type1, index1, &index1);
            if (BCM_FAILURE(rv)) {
                return BCM_E_PARAM;
            }

            if (id_type == xflowMacsecIdTypeSecureAssoc) {
                index1 = (num_sa * index1) + (index % num_sa);
            }
            index = index1;
        }
    }

    if (s_stat == 1) {
        COMPILER_64_COPY(stat_map->sw_value[index], value);
    } else {
#ifdef BCM_HURRICANE4_SUPPORT
        if (SOC_IS_HURRICANE4(unit) &&
                ((stat_map->counter_id == _xflowMacsecCid9) ||
                 (stat_map->counter_id == _xflowMacsecCid11))) {
            counter = &(db->_xflow_macsec_counter_array[stat_map->counter_id]);
            /* HR4 operates in Dual-port (0 2 0 2) mode */
            for(index1 = counter->min_index; index1 < counter->num_entries; index1 += 2) {
                if (index1 == counter->min_index) {
                    rv =_xflow_macsec_stat_read_and_update(unit, stat_map->counter_id,
                                                           stat_type, index1, value);
                } else {
                    rv =_xflow_macsec_stat_read_and_update(unit, stat_map->counter_id,
                                                           stat_type, index1, 0);
                }
            }
        } else
#endif
        {
            rv =_xflow_macsec_stat_read_and_update(unit, stat_map->counter_id,
                    stat_type, index, value);
        }
    }

    return rv;
}

int
xflow_macsec_stat_set(int unit, uint32 flags, xflow_macsec_id_t id,
                       xflow_macsec_stat_type_t  stat_type,
                       uint64 value)
{
    BCM_IF_ERROR_RETURN
        (_xflow_macsec_stat_set(unit, flags, id, stat_type, value));

    /* DMA the stats again. */
    _xflow_macsec_counters_collect(unit);

    return BCM_E_NONE;
}

int
xflow_macsec_stat_multi_get(
    int unit,
    uint32 flags,
    xflow_macsec_id_t id,
    uint32 num_stats,
    xflow_macsec_stat_type_t  *stat_type_array,
    uint64 *value_array)
{
    int i;
    if (num_stats >= xflowMacsecStatTypeCount) {
        return BCM_E_PARAM;
    }

    if (value_array == NULL) {
        return BCM_E_PARAM;
    }
    for (i = 0; i < num_stats; i++) {
        if (stat_type_array[i] >= xflowMacsecStatTypeCount) {
            return BCM_E_PARAM;
        }
        BCM_IF_ERROR_RETURN
            (_xflow_macsec_stat_get(unit, flags, id, stat_type_array[i],
                                    &value_array[i]));
    }
    return BCM_E_NONE;
}

int
xflow_macsec_stat_multi_set(
    int unit,
    uint32 flags,
    xflow_macsec_id_t id,
    uint32 num_stats,
    xflow_macsec_stat_type_t  *stat_type_array,
    uint64 *value_array)
{
    int i;
    if (num_stats >= xflowMacsecStatTypeCount) {
        return BCM_E_PARAM;
    }

    if (value_array == NULL) {
        return BCM_E_PARAM;
    }
    for (i = 0; i < num_stats; i++) {
        if (stat_type_array[i] >= xflowMacsecStatTypeCount) {
            return BCM_E_PARAM;
        }
        BCM_IF_ERROR_RETURN
            (_xflow_macsec_stat_set(unit, flags, id, stat_type_array[i],
                                    value_array[i]));
    }
    /* DMA the stats again. */
    _xflow_macsec_counters_collect(unit);
    return BCM_E_NONE;
}
#endif

static int _xflow_macsec_counter_clear(int unit, uint8 dir, xflow_macsec_index_type_t id_type, int index,
                                       soc_mem_t *mem_arr, int mem_arr_count)
{
    int idx = 0;
    int rv = BCM_E_NONE;
    uint64 val64;
    int alloc_size = 0;
    uint32* buf = NULL;
    xflow_macsec_db_t *db;
    _xflow_macsec_stat_map_t *stat_map;

    if (mem_arr == NULL || mem_arr_count < 1)
        return BCM_E_PARAM;

    for (idx = 0 ; idx < mem_arr_count; idx++)
    {
        if (mem_arr[idx] == INVALIDm)
        {
            return BCM_E_PARAM;
        }
        if (alloc_size < soc_mem_entry_words(unit, mem_arr[idx]))
        {
            alloc_size = soc_mem_entry_words(unit, mem_arr[idx]);
        }
    }

    alloc_size = alloc_size * sizeof(uint32);

    buf = sal_alloc(alloc_size, "_xflow_macsec_counter_clear");
    _XFLOW_MACSEC_MEM_NULL_CHECK_AND_SET(buf, alloc_size);

    for (idx = 0 ; idx < mem_arr_count; idx++)
    {
        rv = soc_mem_read(unit, mem_arr[idx], MEM_BLOCK_ANY, index, buf);
        _XFLOW_MACSEC_BCM_FAILURE_GOTO_ERROR(rv, exit);
    }

    BCM_IF_ERROR_RETURN(xflow_macsec_db_get(unit, &db));
    COMPILER_64_SET(val64, 0, 0);
    
    for (idx = 0 ; idx < xflowMacsecStatTypeCount ; idx++)
    {
        if (dir != db->_xflow_macsec_stat_map_list[idx].dir)
        {
            continue;
        }
        if (id_type != db->_xflow_macsec_stat_map_list[idx].id_type)
        {
            continue;
        }

        stat_map = &(db->_xflow_macsec_stat_map_list[idx]);
        COMPILER_64_COPY(stat_map->sw_value[index], val64);
    }

exit:

    if (buf)
    {
        sal_free(buf);
    }
    return rv;
}

int
_xflow_macsec_sc_encrypt_counter_clear(int unit, int sc_index)
{
    int rv;
    int mem_count;
    soc_mem_t mem[] = {ESEC_MIB_SCm};

    mem_count = COUNTOF(mem);
    rv = _xflow_macsec_counter_clear(unit,
                                     XFLOW_MACSEC_ENCRYPT,
                                     xflowMacsecIdTypeSecureChan,
                                     sc_index,
                                     mem, mem_count - 1);
    rv = _xflow_macsec_counter_clear(unit,
                                     XFLOW_MACSEC_ENCRYPT,
                                     xflowMacsecIdTypeSubportNum,
                                     sc_index,
                                     &mem[mem_count - 1], 1);
    return rv;
}

int
_xflow_macsec_sa_encrypt_counter_clear(int unit, int sa_index)
{
    int rv;
    int mem_count;
    soc_mem_t mem[] = {ESEC_MIB_SAm};

    mem_count = COUNTOF(mem);
    rv = _xflow_macsec_counter_clear(unit,
                                     XFLOW_MACSEC_ENCRYPT,
                                     xflowMacsecIdTypeSecureAssoc,
                                     sa_index,
                                     mem, mem_count);
    return rv;
}

int
_xflow_macsec_sc_decrypt_counter_clear(int unit, int sc_index)
{
    int rv;
    int mem_count;
    soc_mem_t mem[] = {ISEC_MIB_SCm, ISEC_SCTCAM_HIT_COUNTm};

    mem_count = COUNTOF(mem);
    rv = _xflow_macsec_counter_clear(unit,
                                     XFLOW_MACSEC_DECRYPT,
                                     xflowMacsecIdTypeSecureChan,
                                     sc_index,
                                     mem, mem_count);
    return rv;
}

int
_xflow_macsec_sa_decrypt_counter_clear(int unit, int sa_index)
{
    int rv;
    int mem_count;
    soc_mem_t mem[] = {ISEC_MIB_SAm};

    mem_count = COUNTOF(mem);
    rv = _xflow_macsec_counter_clear(unit,
                                     XFLOW_MACSEC_DECRYPT,
                                     xflowMacsecIdTypeSecureAssoc,
                                     sa_index,
                                     mem, mem_count);
    return rv;
}

int
_xflow_macsec_flow_decrypt_counter_clear(int unit, int flow_index)
{
    int rv;
    int mem_count;
    soc_mem_t mem[] = {ISEC_SPTCAM_HIT_COUNTm};

    mem_count = COUNTOF(mem);
    rv = _xflow_macsec_counter_clear(unit,
                                     XFLOW_MACSEC_DECRYPT,
                                     xflowMacsecIdTypeSecureAssoc,
                                     flow_index,
                                     mem, mem_count);
    return rv;
}

int
_xflow_macsec_policy_decrypt_counter_clear(int unit, int policy_index)
{
    int rv;
    int mem_count;
    soc_mem_t mem[] = {ISEC_SCTCAM_HIT_COUNTm};

    mem_count = COUNTOF(mem);
    rv = _xflow_macsec_counter_clear(unit,
                                     XFLOW_MACSEC_DECRYPT,
                                     xflowMacsecIdTypeSubportNum,
                                     policy_index,
                                     mem, mem_count);
    return rv;
}

#if 0
/*
 * Update the SW counter cache for the id_type only for the
 * given index.
 */
static int
_xflow_macsec_counter_read_process_by_id_type(int unit, int oper,
                                   xflow_macsec_index_type_t id_type,
                                   int index)
{
    int i, j, rv = BCM_E_NONE;
    int s_idx, from_index;
    uint64 val64;
    int alloc_size = 0, num_sa, sa_index;
    uint32* buf = NULL;
    soc_mem_t mem;
    _xflow_macsec_stat_map_t *stat_map, *stat_map1;
    _xflow_macsec_counter_id_t counter_id;
    _xflow_macsec_counter_regs_t *counter;
    _xflow_macsec_counter_id_t last_read_id = _xflowMacsecCidInvalid;
    xflow_macsec_db_t *db;

    BCM_IF_ERROR_RETURN
        (xflow_macsec_db_get(unit, &db));

    if (XFLOW_MACSEC_NUM_SA_PER_SC(unit, oper)) {
        num_sa = XFLOW_MACSEC_NUM_SA_PER_SC(unit, oper);
    } else if (XFLOW_MACSEC_4_SA_PER_SC(unit)) {
        num_sa = 4;
    } else {
        num_sa = 1;
    }

    /*
     * Iterate through all stat_types. Match only for the input id_type.
     */
    for (s_idx = 0 ; s_idx < xflowMacsecStatTypeCount ; s_idx++) {
        stat_map = &(db->_xflow_macsec_stat_map_list[s_idx]);
        if (stat_map->id_type != id_type) {
            continue;
        }
        if (stat_map->dir != oper) {
            continue;
        }
        counter_id = stat_map->counter_id;

        /*
         * For certain special Stat_types, counter is collected from
         * elsewhere.
         * For example, for certain secure_chan types, counter is
         * collected from all the secure_assoc members.
         * The assumption is that SA counters were already cached.
         */
        if (counter_id == _xflowMacsecCidSpecial) {
            if (stat_map->id_type == xflowMacsecIdTypeSecureChan) {
                for (i = 0; i < _XFLOW_MACSEC_STAT_SPECIAL_MAP_NUM;
                                                                i++) {
                    if (_xflow_macsec_stat_special_map[i].stat_from !=
                        stat_map->stat_type) {
                        continue;
                    }
                    COMPILER_64_SET(stat_map->sw_value[index], 0, 0);
                    stat_map1 = &(db->_xflow_macsec_stat_map_list
                                    [_xflow_macsec_stat_special_map[i].stat_to]);
                    for (j = 0; j < num_sa; j++) {
                        sa_index = (index * num_sa) + j;
                        COMPILER_64_ADD_64(stat_map->sw_value[index],
                                        stat_map1->sw_value[sa_index]);
                    }
                }
            }
            continue;
        }
        /*
         * Prevent overwriting of buf with a different counter_id.
         */
        if ((last_read_id != _xflowMacsecCidInvalid) &&
            (last_read_id != counter_id)) {
            continue;
        }

        counter = &(db->_xflow_macsec_counter_array[counter_id]);
        mem = counter->mem;
        if (mem == INVALIDm) {
            continue;
        }

        from_index = index;
        if (stat_map->id_type == xflowMacsecIdTypeSecureAssoc) {
            from_index = num_sa * index;
        }
        /*
         * If the stat_type belongs to the same memory (same counter_id) as the
         * previous stat_type, reuse the buffer.
         */
        if (last_read_id != counter_id) {
            last_read_id = counter_id;
            if (buf == NULL) {
                alloc_size = num_sa * soc_mem_entry_words(unit, mem) * sizeof(uint32);
                buf = sal_alloc(alloc_size, "_xflow_macsec_counter_read_process_by_id_type");
                _XFLOW_MACSEC_MEM_NULL_CHECK_AND_SET(buf, alloc_size);
            }

            rv = soc_mem_read(unit, mem, MEM_BLOCK_ANY, from_index, &buf[0]);
            _XFLOW_MACSEC_BCM_FAILURE_GOTO_ERROR(rv, exit);

        }
        if (buf == NULL) {
            continue;
        }
        soc_mem_field64_get(unit, counter->mem, &buf[0],
                            stat_map->field, &val64);
        COMPILER_64_ADD_64(stat_map->sw_value[from_index], val64);
        /*
         * Read through the remaining num_sa entries for SA.
         */
        if (buf && stat_map->id_type == xflowMacsecIdTypeSecureAssoc) {
            for (j = 1; j < num_sa; j++) {
                from_index++;
                rv = soc_mem_read(unit, mem, MEM_BLOCK_ANY, from_index,
                                  &buf[j]);
                _XFLOW_MACSEC_BCM_FAILURE_GOTO_ERROR(rv, exit);
                soc_mem_field64_get(unit, counter->mem, &buf[j],
                                    stat_map->field, &val64);
                COMPILER_64_ADD_64(stat_map->sw_value[from_index], val64);
            }
        }
    }

exit:
    if (buf) {
        sal_free(buf);
    }
    return rv;
}

static int
_xflow_macsec_counter_read_process(int unit,
                                   int to_index,
                                   int index,
                                   soc_mem_t *mem_arr,
                                   int mem_arr_count)
{
    int rv = BCM_E_NONE;
    int m_idx, s_idx, is_sim;
    uint64 val64;
    int alloc_size = 0;
    uint32* buf = NULL;
    _xflow_macsec_stat_map_t *stat_map;
    _xflow_macsec_counter_id_t counter_id;
    _xflow_macsec_counter_regs_t *counter;
    xflow_macsec_db_t *db;

    if (mem_arr == NULL || mem_arr_count < 1) {
        return BCM_E_PARAM;
    }

    for (m_idx = 0 ; m_idx < mem_arr_count; m_idx++) {
        if (mem_arr[m_idx] == INVALIDm) {
            return BCM_E_PARAM;
        }
        if (alloc_size < soc_mem_entry_words(unit, mem_arr[m_idx])) {
            alloc_size = soc_mem_entry_words(unit, mem_arr[m_idx]);
        }
    }

    alloc_size = alloc_size * sizeof(uint32);

    buf = sal_alloc(alloc_size, "_xflow_macsec_counter_read_process");
    _XFLOW_MACSEC_MEM_NULL_CHECK_AND_SET(buf, alloc_size);

    is_sim = (SAL_BOOT_BCMSIM || SAL_BOOT_XGSSIM) ? 1 : 0;

    BCM_IF_ERROR_RETURN
        (xflow_macsec_db_get(unit, &db));
    for (m_idx = 0 ; m_idx < mem_arr_count; m_idx++) {
        rv = soc_mem_read(unit, mem_arr[m_idx], MEM_BLOCK_ANY, index, buf);
        _XFLOW_MACSEC_BCM_FAILURE_GOTO_ERROR(rv, exit);

        if (is_sim) {
            rv = soc_mem_write(unit, mem_arr[m_idx], MEM_BLOCK_ALL, index,
                               soc_mem_entry_null(unit, mem_arr[m_idx]));
            _XFLOW_MACSEC_BCM_FAILURE_GOTO_ERROR(rv, exit);
            rv = soc_mem_write(unit, mem_arr[m_idx], MEM_BLOCK_ALL,
                               to_index, buf);
            _XFLOW_MACSEC_BCM_FAILURE_GOTO_ERROR(rv, exit);
        }

        for (s_idx = 0 ; s_idx < xflowMacsecStatTypeCount ; s_idx++) {
            stat_map = &(db->_xflow_macsec_stat_map_list[s_idx]);
            counter_id = stat_map->counter_id;
            counter = &(db->_xflow_macsec_counter_array[counter_id]);

            if (counter->mem != mem_arr[m_idx]) {
                continue;
            }
            soc_mem_field64_get(unit, counter->mem, buf,
                                stat_map->field, &val64);
            if (is_sim) {
                COMPILER_64_COPY(stat_map->sw_value[to_index], val64);
            } else {
                COMPILER_64_ADD_64(stat_map->sw_value[to_index], val64);
            }
        }
    }

exit:
    if (buf) {
        sal_free(buf);
    }
    return rv;
}

static int
_xflow_macsec_counter_move(int unit, uint8 dir,
                           xflow_macsec_index_type_t id_type,
                           int to_index, int from_index)
{
    int j, idx = 0, num_sa, target_index, source_index;
    uint64 val64;
    _xflow_macsec_stat_map_t *stat_map;
    xflow_macsec_db_t *db;

    COMPILER_64_SET(val64, 0, 0);
    BCM_IF_ERROR_RETURN
        (xflow_macsec_db_get(unit, &db));

    if (XFLOW_MACSEC_NUM_SA_PER_SC(unit, dir)) {
        num_sa = XFLOW_MACSEC_NUM_SA_PER_SC(unit, dir);
    } else if (XFLOW_MACSEC_4_SA_PER_SC(unit)) {
        num_sa = 4;
    } else {
        num_sa = 1;
    }

    for (idx = 0 ; idx < xflowMacsecStatTypeCount ; idx++) {
        if (dir != db->_xflow_macsec_stat_map_list[idx].dir) {
            continue;
        }
        if (id_type != db->_xflow_macsec_stat_map_list[idx].id_type) {
            continue;
        }
        target_index = to_index;
        source_index = from_index;
        if (id_type == xflowMacsecIdTypeSecureAssoc) {
            target_index = num_sa * to_index;
            source_index = num_sa * from_index;
        }

        stat_map = &(db->_xflow_macsec_stat_map_list[idx]);
        if (SAL_BOOT_BCMSIM || SAL_BOOT_XGSSIM) {
            COMPILER_64_COPY(stat_map->sw_value[target_index],
                             stat_map->sw_value[source_index]);
        } else {
            COMPILER_64_ADD_64(stat_map->sw_value[target_index],
                               stat_map->sw_value[source_index]);
        }
        COMPILER_64_COPY(stat_map->sw_value[source_index], val64);
        if (id_type == xflowMacsecIdTypeSecureAssoc) {
            for (j = 1; j < num_sa; j++) {
                target_index++;
                source_index++;
                if (SAL_BOOT_BCMSIM || SAL_BOOT_XGSSIM) {
                    COMPILER_64_COPY(stat_map->sw_value[target_index],
                                    stat_map->sw_value[source_index]);
                } else {
                    COMPILER_64_ADD_64(stat_map->sw_value[target_index],
                                    stat_map->sw_value[source_index]);
                }
            }
        }
    }

    return BCM_E_NONE;
}

int
_xflow_macsec_tcam_ptr_counter_move_single_tcam_entry(int unit,
                                                      int to_index,
                                                      int from_index,
                                                      soc_mem_t *mem_arr,
                                                      int mem_arr_count)
{
    int rv = BCM_E_NONE;
    int mem_iter;
    int mem_count;
    soc_mem_t mem;
    soc_mem_t sa_mem[] = {ISEC_MIB2m};
    soc_mem_t sc_mem[] = {ISEC_MIB1m,
                          ISEC_SCTCAM_HIT_COUNTm};
    soc_mem_t flow_mem[] = {ISEC_SPTCAM_HIT_COUNTm};
    soc_mem_t *mem_ptr = NULL;
    xflow_macsec_index_type_t id_type;

    for (mem_iter = mem_arr_count - 1;
                            mem_iter >= 0; mem_iter--) {
        mem = mem_arr[mem_iter];
        if (mem == ISEC_SC_TCAMm) {
            mem_ptr = sc_mem;
            mem_count = COUNTOF(sc_mem);

            /* Copy SC counters to SW cache. */
            id_type = xflowMacsecIdTypeSecureChan;
        } else if (mem == ISEC_SA_TABLEm) {
            mem_ptr = sa_mem;
            mem_count = COUNTOF(sa_mem);

            /* Copy SA counters to SW cache. */
            id_type = xflowMacsecIdTypeSecureAssoc;
        } else if ((mem == ISEC_SP_TCAMm) ||
                   (mem == ISEC_SP_TCAM_KEYm)) {
            mem_ptr = flow_mem;
            mem_count = COUNTOF(flow_mem);

            /* Copy SP TCAM hit counters to SW cache. */
            id_type = xflowMacsecIdTypeFlow;
        } else {
            continue;
        }
        if (soc_feature(unit, soc_feature_xflow_macsec_inline)) {
            rv = _xflow_macsec_counter_read_process_by_id_type (unit,
                        XFLOW_MACSEC_DECRYPT, id_type, from_index);
        } else {
            rv = _xflow_macsec_counter_read_process(unit, to_index, from_index,
                                                mem_ptr, mem_count);
        }
        _XFLOW_MACSEC_BCM_FAILURE_GOTO_ERROR(rv, exit);
        rv = _xflow_macsec_counter_move(unit, XFLOW_MACSEC_DECRYPT,
                                        id_type, to_index, from_index);
        _XFLOW_MACSEC_BCM_FAILURE_GOTO_ERROR(rv, exit);
    }

exit:
    return rv;
}
#endif
