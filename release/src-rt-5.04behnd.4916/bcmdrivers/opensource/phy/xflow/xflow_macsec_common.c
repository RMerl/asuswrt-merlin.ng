/***********************************************************************
 *
 *  Copyright (c) 2021  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2012:DUAL/GPL:standard
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
 ************************************************************************/

#include "macsec_defs.h"
#include "macsec_dev.h"
#include "macsec_types.h"
#include "soc/mcm/enum_max.h"
#include "soc/mcm/enum_types.h"
#include "soc/mem.h"
#include "soc/mcm/memregs.h"
#include "soc/memory.h"
#include "soc/field.h"
#include <soc/mcm/allenum.h>
#include "soc/feature.h"
#include "xflow_macsec_defs.h"
#include "xflow_macsec.h"
#include "xflow_macsec_esw_defs.h"
#include "xflow_macsec_common.h"
#include "xflow_macsec_firelight.h"
#include "xflow_macsec_cfg_params.h"

extern      soc_control_t    *soc_control[SOC_MAX_NUM_DEVICES];

int num_sa_per_sc[BCM_MAX_NUM_UNITS];
uint8 xflow_macsec_init_done[BCM_MAX_NUM_UNITS];

void uint32_to_byte_array(int num_bytes,
                          uint32 *val32,
                          uint8 *byte_array)
{
    int i;

    for (i = 0; i < num_bytes; i++)
    {
        byte_array[i] = (val32[i / 4] >> ((i % 4) * 8)) & 0xff;
    }
}

void byte_array_to_uint32(int num_bytes, uint8 *byte_array, uint32 *val32)
{
    int i;
    for (i = 0; i < num_bytes; i++)
    {
        if ((i % 4) == 0)
        {
            val32[i / 4] = 0;
        }
        val32[i / 4] |= (byte_array[i] << ((i % 4) * 8));
    }
}

int xflow_macsec_flow_create(int unit,
                             uint32 flags,
                             xflow_macsec_flow_info_t *flow_info,
                             int priority,
                             xflow_macsec_flow_id_t *flow_id)
{
    int rv;
    uint8 flag = FALSE;
    int policy_index;
    int sp_index, sp_tcam_index;
    sub_port_data_entry_t subport_data_entry;
    soc_mem_t mem = INVALIDm;

    sal_memset(&subport_data_entry, 0, sizeof(sub_port_data_entry_t));

    if ((flow_info == NULL) || (flow_id == NULL))
        return BCM_E_PARAM;

    /* Assumes that caller always pass XFLOW_MACSEC_DECRYPT */
    _XFLOW_MACSEC_ENCRYPT_DECRYPT_CHECK(flags);

    policy_index = XFLOW_MACSEC_POLICY_ID_INDEX_GET(flow_info->policy_id);
    BCM_IF_ERROR_RETURN(policy_index);

    if (XFLOW_MACSEC_POLICY_ID_IS_ENCRYPT(flow_info->policy_id))
        return BCM_E_PARAM;

    BCM_IF_ERROR_RETURN(_xflow_macsec_policy_index_hw_index_get(unit, policy_index, &policy_index, NULL));

    if (flags & XFLOW_MACSEC_FLOW_WITH_ID)
    {
        if (XFLOW_MACSEC_DIR_TYPE_GET(*flow_id) != XFLOW_MACSEC_DECRYPT)
            return BCM_E_PARAM;

        sp_index = XFLOW_MACSEC_FLOW_ID_INDEX_GET(*flow_id);
        BCM_IF_ERROR_RETURN(sp_index);
        flag = TRUE;
    }

    BCM_IF_ERROR_RETURN(_xflow_macsec_flow_index_reserve( unit, priority, &sp_index, flag));

    rv = _xflow_macsec_flow_index_hw_index_get(unit, sp_index, &sp_tcam_index, NULL);

    if (BCM_FAILURE(rv))
    {
        BCM_IF_ERROR_RETURN(_xflow_macsec_flow_index_free(unit, sp_index));
        return rv;
    }

    printk("xflow_macsec_flow_create sp_index=%d, sp_tcam_index=%d\n", sp_index, sp_tcam_index);

    rv = xflow_macsec_firelight_flow_set(unit, sp_tcam_index, flow_info);
    if (BCM_FAILURE(rv))
    {
        BCM_IF_ERROR_RETURN(_xflow_macsec_flow_index_free(unit, sp_index));
        return rv;
    }

    if (!(flags & XFLOW_MACSEC_FLOW_WITH_ID))
    {
        *flow_id = XFLOW_MACSEC_FLOW_ID_CREATE(XFLOW_MACSEC_DECRYPT, sp_index);
    }

    /* Write to TCAM DATA. */
    if (SOC_MEM_IS_VALID(unit, SUB_PORT_MAP_TABLEm))
    {
        mem = SUB_PORT_MAP_TABLEm;
    }
    else
    {
        return BCM_E_PARAM;
    }

    soc_mem_field32_set(unit, mem, &subport_data_entry, MS_SUBPORT_NUMf, policy_index);

    if (flow_info->set_management_pkt)
    {
        soc_mem_field32_set(unit, mem, &subport_data_entry, MANAGEMENT_PACKETf, 1);
    }

    rv = soc_mem_write(unit, mem, MEM_BLOCK_ALL, sp_tcam_index, &subport_data_entry);

    if (BCM_FAILURE(rv))
    {
        BCM_IF_ERROR_RETURN(xflow_macsec_flow_destroy(unit, *flow_id));
        BCM_IF_ERROR_RETURN(_xflow_macsec_flow_index_free(unit, sp_index));
        return rv;
    }

    return BCM_E_NONE;
}
/* Xflow macsec control */
int _xflow_macsec_sc_enable_set_encrypt(int unit,
                                        int sc_index,
                                        int enable)
{
    esec_sc_table_entry_t sc_entry;
    int valid;

    if (SOC_MEM_FIELD_VALID(unit, ESEC_SC_TABLEm, VALIDf))
    {
        BCM_IF_ERROR_RETURN(READ_ESEC_SC_TABLEm(unit, MEM_BLOCK_ALL, sc_index, &sc_entry));

        valid = soc_mem_field32_get(unit, ESEC_SC_TABLEm, &sc_entry, VALIDf);

        if ((enable && valid) || (!enable && !valid))
        {
            return BCM_E_PARAM;
        }
        soc_mem_field32_set(unit, ESEC_SC_TABLEm, &sc_entry, VALIDf, !!enable);
        BCM_IF_ERROR_RETURN(WRITE_ESEC_SC_TABLEm(unit, MEM_BLOCK_ALL, sc_index, &sc_entry));
    }
    else
    {
        return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}

int _xflow_macsec_sc_enable_get_encrypt(int unit,
                                        int sc_index,
                                        int *enable)
{
    esec_sc_table_entry_t sc_entry;

    if (SOC_MEM_FIELD_VALID(unit, ESEC_SC_TABLEm, VALIDf))
    {
        BCM_IF_ERROR_RETURN(READ_ESEC_SC_TABLEm(unit, MEM_BLOCK_ALL, sc_index, &sc_entry));
        *enable = soc_mem_field32_get(unit, ESEC_SC_TABLEm, &sc_entry, VALIDf);
    }
    else
    {
        return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}

int _xflow_macsec_sa_create_encrypt(int unit,
                                    int sa_index,
                                    xflow_macsec_secure_assoc_info_t *assoc_info,
                                    xflow_macsec_secure_chan_info_t *chan_info)
{
    esec_sa_table_entry_t sa_entry;
    esec_sa_hash_table_entry_t sa_hash_entry;
    uint32              salt_field[3];
    uint64              next_pn;
    uint32              val256_field[8];
    uint32              sa_hash_field[4];

    if ((sa_index < 0) || (sa_index > soc_mem_index_max(unit, ESEC_SA_TABLEm)))
        return BCM_E_PARAM;

    sal_memset(&sa_entry, 0, sizeof(esec_sa_table_entry_t));
    sal_memset(val256_field, 0, sizeof(uint32) * 8);
    sal_memset(&sa_hash_entry, 0, sizeof(esec_sa_hash_table_entry_t));
    sal_memset(sa_hash_field, 0, sizeof(uint32) * 4);

    if (SOC_MEM_FIELD_VALID(unit, ESEC_SA_TABLEm, AN_CONTROLf))
    {
        _BCM_FIELD32_LEN_CHECK(unit, ESEC_SA_TABLEm, AN_CONTROLf, assoc_info->an_control);
        soc_mem_field32_set(unit, ESEC_SA_TABLEm, &sa_entry, AN_CONTROLf, assoc_info->an_control);
    }

    if (!soc_feature(unit, soc_feature_xflow_macsec_inline))
    {
        soc_mem_field32_set(unit, ESEC_SA_TABLEm, &sa_entry, STATUSf, 2);
    }

    if (assoc_info->flags & XFLOW_MACSEC_SECURE_ASSOC_INFO_SET_NEXT_PKT_NUM)
    {
        COMPILER_64_SET(next_pn, assoc_info->next_pkt_num_upper, assoc_info->next_pkt_num);
        if (SOC_MEM_FIELD_VALID(unit, ESEC_SA_TABLEm, NEXT_PNf))
        {
            soc_mem_field64_set(unit, ESEC_SA_TABLEm, &sa_entry, NEXT_PNf, next_pn);
        }
        else if (SOC_MEM_FIELD_VALID(unit, ESEC_SA_TABLEm, NEXTPNf))
        {
            soc_mem_field64_set(unit, ESEC_SA_TABLEm, &sa_entry, NEXTPNf, next_pn);
        }
        else
        {
            return BCM_E_INTERNAL;
        }
    }

    if (soc_feature(unit, soc_feature_xflow_macsec_crypto_aes_256))
    {
        if (chan_info->crypto >= xflowMacsecCryptoAes256GcmIntegrityOnly)
        {
            byte_array_to_uint32(XFLOW_MACSEC_CRYPTO_AES256GCM_KEY_SIZE, (uint8 *)&assoc_info->aes_256.key,
                                val256_field);
        }
        else
        {
            byte_array_to_uint32(XFLOW_MACSEC_CRYPTO_AES128GCM_KEY_SIZE, (uint8 *)&assoc_info->aes.key, val256_field);
        }
    }
    else
    {
        byte_array_to_uint32(XFLOW_MACSEC_CRYPTO_AES128GCM_KEY_SIZE, (uint8 *)&assoc_info->aes.key, val256_field);
    }
    soc_mem_field_set(unit, ESEC_SA_TABLEm, (uint32 *)&sa_entry, SAKf, val256_field);

    soc_mem_field32_set(unit, ESEC_SA_TABLEm, &sa_entry, SSCIf, assoc_info->ssci);

    byte_array_to_uint32(12, (uint8 *)&assoc_info->salt, salt_field);
    soc_mem_field_set(unit, ESEC_SA_TABLEm, (uint32 *)&sa_entry, SALTf, salt_field);

    if (SOC_MEM_FIELD_VALID(unit, ESEC_SA_TABLEm, VALIDf))
    {
        soc_mem_field32_set(unit, ESEC_SA_TABLEm, &sa_entry, VALIDf, assoc_info->enable);
    }
    BCM_IF_ERROR_RETURN(WRITE_ESEC_SA_TABLEm(unit, MEM_BLOCK_ALL, sa_index, &sa_entry));

    byte_array_to_uint32(16, (uint8 *)&assoc_info->hash, sa_hash_field);
    soc_mem_field_set(unit, ESEC_SA_HASH_TABLEm, (uint32 *)&sa_hash_entry, HASHf, sa_hash_field);
    BCM_IF_ERROR_RETURN(WRITE_ESEC_SA_HASH_TABLEm(unit, MEM_BLOCK_ALL, sa_index, &sa_hash_entry));

    return BCM_E_NONE;
}

int xflow_macsec_secure_chan_enable_set(int unit,
                                        xflow_macsec_secure_chan_id_t chan_id,
                                        int enable)
{
    int sc_index;
    int oper;

    if (enable < 0)
        return BCM_E_PARAM;

    sc_index = XFLOW_MACSEC_SECURE_CHAN_ID_INDEX_GET(chan_id);
    BCM_IF_ERROR_RETURN(sc_index);

    oper = XFLOW_MACSEC_DIR_TYPE_GET(chan_id);

    if(oper == XFLOW_MACSEC_DECRYPT)
        BCM_IF_ERROR_RETURN(_xflow_macsec_sc_decrypt_index_hw_index_get(unit, sc_index, &sc_index, NULL));
    else
        BCM_IF_ERROR_RETURN(_xflow_macsec_sc_encrypt_index_hw_index_get(unit, sc_index, &sc_index, NULL));

    if (oper == XFLOW_MACSEC_ENCRYPT)
    {
        BCM_IF_ERROR_RETURN(_xflow_macsec_sc_enable_set_encrypt(unit, sc_index, enable));
    }
    else
    {
        BCM_IF_ERROR_RETURN(_xflow_macsec_sc_enable_set_decrypt(unit, sc_index, enable));
    }
    return BCM_E_NONE;
}

/*
 * Get a Secure channel info.
 */
int xflow_macsec_secure_chan_get(int unit,
                                 xflow_macsec_secure_chan_id_t chan_id,
                                 xflow_macsec_secure_chan_info_t *chan_info,
                                 int *priority)
{
    int sc_index, sc_index_orig;
    int oper;
    int hw_index;


    if ((chan_info == NULL))
        return BCM_E_PARAM;

    sc_index = XFLOW_MACSEC_SECURE_CHAN_ID_INDEX_GET(chan_id);
    BCM_IF_ERROR_RETURN(sc_index);

    oper = XFLOW_MACSEC_DIR_TYPE_GET(chan_id);
    sc_index_orig = sc_index;

    if(oper == XFLOW_MACSEC_DECRYPT)
    {
        BCM_IF_ERROR_RETURN(_xflow_macsec_sc_decrypt_index_hw_index_get(unit, sc_index, &sc_index, NULL));
    }
    else
    {
        BCM_IF_ERROR_RETURN(_xflow_macsec_sc_encrypt_index_hw_index_get(unit, sc_index, &sc_index, NULL));
    }

    BCM_IF_ERROR_RETURN(xflow_macsec_firelight_sc_get(unit, oper, sc_index, chan_info));

    if ((oper == XFLOW_MACSEC_DECRYPT) && priority != NULL)
    {
        /* Get the flow priority for the SC index */
        BCM_IF_ERROR_RETURN(_xflow_macsec_flow_index_hw_index_get(unit, sc_index_orig, &hw_index, priority));
    }

    return BCM_E_NONE;
}

int _xflow_macsec_sc_enable_set_decrypt(int unit,
                                        int sc_index,
                                        int enable)
{
    isec_sc_tcam_entry_t sc_tcam_entry;
    int valid;

    if ((sc_index < 0) || (sc_index > soc_mem_index_max(unit, ISEC_SC_TCAMm)))
        return BCM_E_PARAM;

    sal_memset(&sc_tcam_entry, 0, sizeof(isec_sc_tcam_entry_t));

    BCM_IF_ERROR_RETURN(READ_ISEC_SC_TCAMm(unit, MEM_BLOCK_ALL, sc_index, &sc_tcam_entry));

    valid = soc_mem_field32_get(unit, ISEC_SC_TCAMm, &sc_tcam_entry, VALIDf);

    if (enable && valid)
        return BCM_E_EXISTS;

    if (!enable && !valid)
        return BCM_E_NONE;

    soc_mem_field32_set(unit, ISEC_SC_TCAMm, &sc_tcam_entry,  VALIDf, !!enable);
    BCM_IF_ERROR_RETURN(WRITE_ISEC_SC_TCAMm(unit, MEM_BLOCK_ALL, sc_index, &sc_tcam_entry));

    return BCM_E_NONE;
}

int _xflow_macsec_sa_create_decrypt(int unit,
                                    int sa_index,
                                    xflow_macsec_secure_assoc_info_t *assoc_info,
                                    xflow_macsec_secure_chan_info_t *chan_info)
{
    isec_sa_table_entry_t sa_entry;
    uint32              salt_field[3];
    uint64              next_pn;
    uint32              val256_field[8];

    if ((sa_index < 0) || (sa_index > soc_mem_index_max(unit, ISEC_SA_TABLEm)))
        return BCM_E_PARAM;

    sal_memset(&sa_entry, 0, sizeof(isec_sa_table_entry_t));
    sal_memset(val256_field, 0, sizeof(uint32) * 8);

    if (assoc_info->flags & XFLOW_MACSEC_SECURE_ASSOC_INFO_SET_NEXT_PKT_NUM)
    {
        COMPILER_64_SET(next_pn, assoc_info->next_pkt_num_upper, assoc_info->next_pkt_num);

        if (SOC_MEM_FIELD_VALID(unit, ISEC_SA_TABLEm, NEXT_PNf))
        {
            soc_mem_field64_set(unit, ISEC_SA_TABLEm, &sa_entry, NEXT_PNf, next_pn);
        }
        else if (SOC_MEM_FIELD_VALID(unit, ISEC_SA_TABLEm, NEXTPNf))
        {
            soc_mem_field64_set(unit, ISEC_SA_TABLEm, &sa_entry, NEXTPNf, next_pn);
        }
        else
        {
            return BCM_E_INTERNAL;
        }
    }

    if (soc_feature(unit, soc_feature_xflow_macsec_crypto_aes_256))
    {
        if (chan_info->crypto >= xflowMacsecCryptoAes256GcmIntegrityOnly)
        {
            byte_array_to_uint32(XFLOW_MACSEC_CRYPTO_AES256GCM_KEY_SIZE, (uint8 *)&assoc_info->aes_256.key,
                                 val256_field);
        }
        else
        {
            byte_array_to_uint32(XFLOW_MACSEC_CRYPTO_AES128GCM_KEY_SIZE, (uint8 *)&assoc_info->aes.key, val256_field);
        }
    }
    else
    {
        byte_array_to_uint32(XFLOW_MACSEC_CRYPTO_AES128GCM_KEY_SIZE, (uint8 *)&assoc_info->aes.key, val256_field);
    }

    soc_mem_field_set(unit, ISEC_SA_TABLEm, (uint32 *)&sa_entry, SAKf, val256_field);

    soc_mem_field32_set(unit, ISEC_SA_TABLEm, &sa_entry, SSCIf, assoc_info->ssci);

    byte_array_to_uint32(12, (uint8 *)&assoc_info->salt, salt_field);

    soc_mem_field_set(unit, ISEC_SA_TABLEm, (uint32 *)&sa_entry, SALTf, salt_field);

    if (!soc_feature(unit, soc_feature_xflow_macsec_inline))
    {
        soc_mem_field32_set(unit, ISEC_SA_TABLEm, &sa_entry, STATUSf, 2);
    }

    if (SOC_MEM_FIELD_VALID(unit, ISEC_SA_TABLEm, VALIDf))
    {
        soc_mem_field32_set(unit, ISEC_SA_TABLEm, &sa_entry, VALIDf, assoc_info->enable);
    }
    BCM_IF_ERROR_RETURN(WRITE_ISEC_SA_TABLEm(unit, MEM_BLOCK_ALL, sa_index, &sa_entry));

    return BCM_E_NONE;
}

int _xflow_macsec_sc_enable_get_decrypt(int unit,
                                        int sc_index,
                                        int *enable)
{
    isec_sc_tcam_entry_t sc_tcam_entry;

    if ((sc_index < 0) || (sc_index > soc_mem_index_max(unit, ISEC_SC_TCAMm)))
        return BCM_E_PARAM;

    if (enable == NULL)
        return BCM_E_PARAM;

    sal_memset(&sc_tcam_entry, 0, sizeof(isec_sc_tcam_entry_t));

    BCM_IF_ERROR_RETURN(READ_ISEC_SC_TCAMm(unit, MEM_BLOCK_ALL, sc_index, &sc_tcam_entry));

    *enable = soc_mem_field32_get(unit, ISEC_SC_TCAMm, &sc_tcam_entry, VALIDf);

    return BCM_E_NONE;
}

/*    Security Association    */

int xflow_macsec_secure_assoc_create(int unit,
                                     uint32 flags,
                                     xflow_macsec_secure_chan_id_t chan_id,
                                     xflow_macsec_secure_assoc_info_t *assoc_info,
                                     xflow_macsec_secure_assoc_id_t *assoc_id)
{
    int oper;
    int sc_index, sc_hw_index;
    xflow_macsec_secure_chan_info_t chan_info;
    int sa_index = 0, sa_hw_index;

    if ((assoc_info == NULL) || (assoc_id == NULL))
        return BCM_E_PARAM;

    if (assoc_info->an > 3)
        return BCM_E_PARAM;

    _XFLOW_MACSEC_ENCRYPT_DECRYPT_CHECK(flags);

    sc_index = XFLOW_MACSEC_SECURE_CHAN_ID_INDEX_GET(chan_id);
    BCM_IF_ERROR_RETURN(sc_index);

    memset(&chan_info, 0, sizeof(xflow_macsec_secure_chan_info_t));

    BCM_IF_ERROR_RETURN(xflow_macsec_secure_chan_get(unit, chan_id, &chan_info, NULL));

    oper = XFLOW_MACSEC_DIR_TYPE_GET(chan_id);

    if(oper == XFLOW_MACSEC_DECRYPT)
        BCM_IF_ERROR_RETURN(_xflow_macsec_sc_decrypt_index_hw_index_get(unit, sc_index, &sc_hw_index, NULL));
    else
        BCM_IF_ERROR_RETURN(_xflow_macsec_sc_encrypt_index_hw_index_get(unit, sc_index, &sc_hw_index, NULL));

    if (XFLOW_MACSEC_4_SA_PER_SC(unit))
    {
        sa_hw_index = (4 * sc_hw_index) + assoc_info->an;
    }
    else if (XFLOW_MACSEC_NUM_SA_PER_SC(unit, oper))
    {
        if(assoc_info->an >= XFLOW_MACSEC_NUM_SA_PER_SC(unit, oper))
        {
            return BCM_E_PARAM;
        }
        sa_hw_index = ((XFLOW_MACSEC_NUM_SA_PER_SC(unit, oper) * sc_hw_index)) + assoc_info->an;
    }
    else
    {
        sa_hw_index = sc_hw_index;
    }

    if (oper == XFLOW_MACSEC_ENCRYPT)
    {
        BCM_IF_ERROR_RETURN(_xflow_macsec_sa_create_encrypt(unit, sa_hw_index, assoc_info, &chan_info));
    }
    else
    {
        BCM_IF_ERROR_RETURN(_xflow_macsec_sa_create_decrypt(unit, sa_hw_index, assoc_info, &chan_info));
    }

    if (XFLOW_MACSEC_4_SA_PER_SC(unit))
    {
        sa_index = (4 * sc_index) + assoc_info->an;
    }
    else if (XFLOW_MACSEC_NUM_SA_PER_SC(unit, oper))
    {
        sa_index = (XFLOW_MACSEC_NUM_SA_PER_SC(unit, oper) * sc_index) + assoc_info->an;
    }
    else
    {
        sa_index = sc_index;
    }

    printk("xflow_macsec_secure_assoc_create chan_id=0x%X, sc_index=%d, oper=%d, sc_hw_index=%d, sa_hw_index=%d, sa_index=%d, assoc_info->an=%d\n", 
        chan_id, sc_index, oper, sc_hw_index, sa_hw_index, sa_index, assoc_info->an);

    /* Set ID as per SC logical index */
    *assoc_id = XFLOW_MACSEC_SECURE_ASSOC_ID_CREATE(oper, sa_index);
    return BCM_E_NONE;
}

int xflow_macsec_port_control_set(int unit,
                                  uint32 flags,
                                  bcm_gport_t port,
                                  xflow_macsec_port_control_t type,
                                  xflow_macsec_port_info_t *port_info)
{
    bcm_port_t      local_port = BCM_PORT_INVALID;
//    int             is_local = FALSE, id;

    if (port_info == NULL)
        return BCM_E_PARAM;

    /* TODO: REVISIT */
#if 0
    if (BCM_GPORT_IS_SET(port))
    {
        /* GPORT Validations */
        BCM_IF_ERROR_RETURN(_bcm_esw_gport_resolve(unit, port, &modid, &local_port, &tgid, &id));

        if (-1 == local_port)
            return BCM_E_PARAM;

        BCM_IF_ERROR_RETURN(_bcm_esw_modid_is_local(unit, modid, &is_local));
        if (!is_local)
            return BCM_E_PARAM;
    }
    else
    {
        BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &local_port));
    }
#else
    local_port = port;
#endif

    return (xflow_macsec_firelight_port_control_set(unit, flags, local_port, type, port_info->value));
}

int xflow_macsec_port_control_get(int unit,
                                  uint32 flags,
                                  bcm_gport_t port,
                                  xflow_macsec_port_control_t type,
                                  xflow_macsec_port_info_t *port_info)
{
    bcm_port_t      local_port = 0;
//    int             is_local = FALSE, id;

    if (port_info == NULL)
        return BCM_E_PARAM;

    /* TODO: REVISIT */
#if 0
    if (BCM_GPORT_IS_SET(port))
    {
        /* GPORT Validations */
        BCM_IF_ERROR_RETURN(_bcm_esw_gport_resolve(unit, port, &modid, &local_port, &tgid, &id));

        if (-1 == local_port)
            return BCM_E_PARAM;

        BCM_IF_ERROR_RETURN(_bcm_esw_modid_is_local(unit, modid, &is_local));

        if (!is_local)
            return BCM_E_PARAM;
    }
    else
    {
        BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &local_port));
    }
#endif

    return (xflow_macsec_firelight_port_control_get(unit, flags, local_port, type, &port_info->value));
}

/*    Security Channel    */

/*
 * Create a Secure channel.
 */
int xflow_macsec_secure_chan_create(int unit,
                                    uint32 flags,
                                    xflow_macsec_secure_chan_info_t *chan,
                                    int priority,
                                    xflow_macsec_secure_chan_id_t *chan_id)
{
    int rv;
    uint8 flag = FALSE;
    int sc_index, sc_hw_index;
    int oper = XFLOW_MACSEC_ENCRYPT_DECRYPT_NONE;

    if ((chan == NULL) || (chan_id == NULL))
    {
        return BCM_E_PARAM;
    }

    _XFLOW_MACSEC_ENCRYPT_DECRYPT_CHECK(flags);

    if (flags & XFLOW_MACSEC_ENCRYPT)
    {
        oper = XFLOW_MACSEC_ENCRYPT;
    }
    else if (flags & XFLOW_MACSEC_DECRYPT)
    {
        oper = XFLOW_MACSEC_DECRYPT;
    }
    else
    {
        return BCM_E_PARAM;
    }

    if (flags & XFLOW_MACSEC_SECURE_CHAN_WITH_ID)
    {
        if (XFLOW_MACSEC_DIR_TYPE_GET(*chan_id) != oper)
        {
            return BCM_E_PARAM;
        }
        sc_index = XFLOW_MACSEC_SECURE_CHAN_ID_INDEX_GET(*chan_id);
        BCM_IF_ERROR_RETURN(sc_index);
        flag = TRUE;
    }

    if(oper == XFLOW_MACSEC_DECRYPT)
    {
        BCM_IF_ERROR_RETURN(_xflow_macsec_sc_decrypt_index_reserve(unit, priority, &sc_index, flag));
        rv = _xflow_macsec_sc_decrypt_index_hw_index_get(unit, sc_index, &sc_hw_index, NULL);
    }
    else
    {
        BCM_IF_ERROR_RETURN(_xflow_macsec_sc_encrypt_index_reserve(unit, priority, &sc_index, flag));
        rv = _xflow_macsec_sc_encrypt_index_hw_index_get(unit, sc_index, &sc_hw_index, NULL);
    }

    if (BCM_FAILURE(rv))
    {
        if(oper == XFLOW_MACSEC_DECRYPT)
            BCM_IF_ERROR_RETURN(_xflow_macsec_sc_decrypt_index_free(unit, sc_index));
        else
            BCM_IF_ERROR_RETURN(_xflow_macsec_sc_encrypt_index_free(unit, sc_index));

        return rv;
    }

    printk("xflow_macsec_secure_chan_create oper=%d, sc_hw_index=%d\n", oper, sc_hw_index);

    rv = xflow_macsec_firelight_sc_set(unit, oper, sc_hw_index, chan);
    if (BCM_FAILURE(rv))
    {
        if(oper == XFLOW_MACSEC_DECRYPT)
            BCM_IF_ERROR_RETURN(_xflow_macsec_sc_decrypt_index_free(unit, sc_index));
        else
            BCM_IF_ERROR_RETURN(_xflow_macsec_sc_encrypt_index_free(unit, sc_index));

        return rv;
    }

    if (!(flags & XFLOW_MACSEC_SECURE_CHAN_WITH_ID))
    {
        *chan_id = XFLOW_MACSEC_SECURE_CHAN_ID_CREATE(oper, sc_index);
    }

    return BCM_E_NONE;
}

static int _bcm_xflow_macsec_default_sc_setup (int unit,
                                               int flags,
                                               int sc_count)
{
    int oper, sc_index;
    xflow_macsec_secure_chan_info_t chan_info;
    xflow_macsec_secure_chan_id_t chan_id;

    memset(&chan_info, 0, sizeof(xflow_macsec_secure_chan_info_t));

    _XFLOW_MACSEC_ENCRYPT_DECRYPT_CHECK(flags);

    if (flags & XFLOW_MACSEC_ENCRYPT)
        oper = XFLOW_MACSEC_ENCRYPT;
    else
        /* We don't support default SC for decrypt for now. */
        return BCM_E_PARAM;

    flags = (XFLOW_MACSEC_SECURE_CHAN_WITH_ID | oper);

    for (sc_index = 0; sc_index < sc_count; sc_index++)
    {
        chan_info.sectag_offset = MACSEC_CONFIG_DEFAULT_SECTAG_OFFSET;
        chan_info.crypto = MACSEC_CONFIG_DEFAULT_CRYPTO_ALG;
        chan_id = XFLOW_MACSEC_SECURE_CHAN_ID_CREATE(XFLOW_MACSEC_ENCRYPT, sc_index);

        BCM_IF_ERROR_RETURN(xflow_macsec_secure_chan_create(unit, flags, &chan_info, 0, &chan_id));
    }

    return BCM_E_NONE;
}

int _bcm_xflow_macsec_deinit(int unit)
{

    /* TODO: REVISIT */
#if 0
    if (soc_feature(unit, soc_feature_xflow_macsec_poll_intr))
    {
        BCM_IF_ERROR_RETURN(_bcm_xflow_macsec_thread_stop(unit));
    }

    BCM_IF_ERROR_RETURN(_xflow_macsec_firelight_counters_cleanup(unit));
    BCM_IF_ERROR_RETURN(xflow_macsec_firelight_cleanup(unit));
    BCM_IF_ERROR_RETURN(_xflow_macsec_resource_deinit(unit));
#endif

    xflow_macsec_init_done[unit] = FALSE;
    return BCM_E_NONE;
}

int _bcm_xflow_macsec_init(int unit)
{
    xflow_macsec_db_t *macsec_db;

    if (xflow_macsec_init_done[unit] == TRUE)
        BCM_IF_ERROR_RETURN(_bcm_xflow_macsec_deinit(unit));

    xflow_macsec_db_init(unit);

    BCM_IF_ERROR_RETURN(_xflow_macsec_resource_init(unit));
    BCM_IF_ERROR_RETURN(_xflow_macsec_firelight_counters_init(unit));
    BCM_IF_ERROR_RETURN(xflow_macsec_firelight_init(unit));

    BCM_IF_ERROR_RETURN(xflow_macsec_db_get(unit, &macsec_db));

    if (macsec_db->port_based_macsec)
    {
        BCM_IF_ERROR_RETURN(_bcm_xflow_macsec_default_sc_setup(unit, XFLOW_MACSEC_ENCRYPT,
                            CMBB_XFLOW_MACSEC_NUM_RSVD_SC));
    }

    /* TODO: REVISIT */
#if 0
    if (soc_feature(unit, soc_feature_xflow_macsec_poll_intr))
    {
        BCM_IF_ERROR_RETURN(_bcm_xflow_macsec_thread_start(unit));
    }
    XFLOW_MACSEC_CALLBACK_SET(unit, NULL, NULL);
#endif

    xflow_macsec_init_done[unit] = TRUE;
    return BCM_E_NONE;
}

int xflow_macsec_flow_destroy(int unit, xflow_macsec_flow_id_t flow_id)
{
    int sp_index, sp_tcam_index;
    soc_mem_t mem;

    sub_port_data_entry_t subport_data_entry;
    sal_memset(&subport_data_entry, 0, sizeof(sub_port_data_entry_t));

    sp_index = XFLOW_MACSEC_FLOW_ID_INDEX_GET(flow_id);
    BCM_IF_ERROR_RETURN(sp_index);

    if (XFLOW_MACSEC_FLOW_ID_IS_ENCRYPT(flow_id))
        return BCM_E_PARAM;

    BCM_IF_ERROR_RETURN(_xflow_macsec_flow_index_hw_index_get(unit, sp_index, &sp_tcam_index, NULL));

    /* Write to TCAM DATA. */
    if (SOC_MEM_IS_VALID(unit, SUB_PORT_MAP_TABLEm))
    {
        mem = SUB_PORT_MAP_TABLEm;
    }
    else
    {
        return BCM_E_INTERNAL;
    }

    BCM_IF_ERROR_RETURN(xflow_macsec_fl_flow_destroy(unit, sp_tcam_index));

    BCM_IF_ERROR_RETURN(soc_mem_write(unit, mem, MEM_BLOCK_ALL, sp_tcam_index, &subport_data_entry));

    BCM_IF_ERROR_RETURN(_xflow_macsec_flow_index_free(unit, sp_index));

    return BCM_E_NONE;
}

int _xflow_macsec_sa_get_decrypt(int unit, int sa_index,
                                xflow_macsec_secure_assoc_info_t *assoc_info,
                                xflow_macsec_secure_chan_info_t *chan_info)
{
    isec_sa_table_entry_t sa_entry;
    uint32      salt_field[3];
    uint64      next_pn;
    uint32      val256_field[8];

    sal_memset(&sa_entry, 0, sizeof(isec_sa_table_entry_t));
    sal_memset(val256_field, 0, sizeof(uint32) * 8);
    BCM_IF_ERROR_RETURN
        (READ_ISEC_SA_TABLEm(unit, MEM_BLOCK_ALL, sa_index, &sa_entry));

    /* Check if Entry is valid */
    if (sal_memcmp(&sa_entry, soc_mem_entry_null(unit, ISEC_SA_TABLEm), soc_mem_entry_words(unit, ISEC_SA_TABLEm) *
                   sizeof(uint32)) == 0)
    {
        return BCM_E_NOT_FOUND;
    }

    if (SOC_MEM_FIELD_VALID(unit, ISEC_SA_TABLEm, NEXT_PNf))
        soc_mem_field64_get(unit, ISEC_SA_TABLEm, (uint32 *)&sa_entry, NEXT_PNf, &next_pn);
    else if (SOC_MEM_FIELD_VALID(unit, ISEC_SA_TABLEm, NEXTPNf))
        soc_mem_field64_get(unit, ISEC_SA_TABLEm, (uint32 *)&sa_entry, NEXTPNf, &next_pn);
    else
        return BCM_E_INTERNAL;

    COMPILER_64_TO_32_LO(assoc_info->next_pkt_num, next_pn);
    COMPILER_64_TO_32_HI(assoc_info->next_pkt_num_upper, next_pn);

    if (assoc_info->next_pkt_num || assoc_info->next_pkt_num_upper)
        assoc_info->flags |= XFLOW_MACSEC_SECURE_ASSOC_INFO_SET_NEXT_PKT_NUM;

    if (XFLOW_MACSEC_4_SA_PER_SC(unit))
        assoc_info->an = (sa_index % 4);
    else
        assoc_info->an = 0;

    soc_mem_field_get(unit, ISEC_SA_TABLEm, (uint32 *)&sa_entry,
                      SAKf, val256_field);

    sal_memset(&assoc_info->aes.key, 0, sizeof(assoc_info->aes.key));
    sal_memset(&assoc_info->aes_256.key, 0, sizeof(assoc_info->aes_256.key));

    if (soc_feature(unit, soc_feature_xflow_macsec_crypto_aes_256))
    {
        if (chan_info->crypto >= xflowMacsecCryptoAes256GcmIntegrityOnly)
        {
            uint32_to_byte_array(XFLOW_MACSEC_CRYPTO_AES256GCM_KEY_SIZE,
                                 val256_field, (uint8 *)&assoc_info->aes_256.key);
        }
        else
            uint32_to_byte_array(XFLOW_MACSEC_CRYPTO_AES128GCM_KEY_SIZE, val256_field, (uint8 *)&assoc_info->aes.key);
    }
    else
        uint32_to_byte_array(XFLOW_MACSEC_CRYPTO_AES128GCM_KEY_SIZE, val256_field, (uint8 *)&assoc_info->aes.key);

    assoc_info->ssci = soc_mem_field32_get(unit, ISEC_SA_TABLEm, &sa_entry, SSCIf);

    soc_mem_field_get(unit, ISEC_SA_TABLEm, (uint32 *)&sa_entry, SALTf, salt_field);

    uint32_to_byte_array(12, salt_field, (uint8 *)&assoc_info->salt);

    if (SOC_MEM_FIELD_VALID(unit, ISEC_SA_TABLEm, VALIDf))
        assoc_info->enable = soc_mem_field32_get(unit, ISEC_SA_TABLEm, &sa_entry, VALIDf);

    return BCM_E_NONE;
}

int _xflow_macsec_sa_get_encrypt(int unit, int sa_index,
                             xflow_macsec_secure_assoc_info_t *assoc_info,
                             xflow_macsec_secure_chan_info_t *chan_info)
{
    esec_sa_table_entry_t sa_entry;
    uint32      salt_field[3];
    uint64      next_pn = 0;
    uint32      val256_field[8];

    sal_memset(&sa_entry, 0, sizeof(esec_sa_table_entry_t));
    sal_memset(val256_field, 0, sizeof(uint32) * 8);
    BCM_IF_ERROR_RETURN
        (READ_ESEC_SA_TABLEm(unit, MEM_BLOCK_ALL, sa_index, &sa_entry));

    /* Check if Entry is valid */
    if (sal_memcmp(&sa_entry, soc_mem_entry_null(unit, ESEC_SA_TABLEm),
                   soc_mem_entry_words(unit, ESEC_SA_TABLEm) *
                   sizeof(uint32)) == 0)
        return BCM_E_NOT_FOUND;

    if (SOC_MEM_FIELD_VALID(unit, ESEC_SA_TABLEm, AN_CONTROLf))
    {
        assoc_info->an_control = soc_mem_field32_get(unit, ESEC_SA_TABLEm,
                                                     &sa_entry,
                                                     AN_CONTROLf);
    }

    if (SOC_MEM_FIELD_VALID(unit, ESEC_SA_TABLEm, NEXT_PNf))
    {
        soc_mem_field64_get(unit, ESEC_SA_TABLEm, (uint32 *)&sa_entry,
                            NEXT_PNf, &next_pn);
    }
    else if (SOC_MEM_FIELD_VALID(unit, ESEC_SA_TABLEm, NEXTPNf))
    {
        soc_mem_field64_get(unit, ESEC_SA_TABLEm, (uint32 *)&sa_entry,
                            NEXTPNf, &next_pn);
    }

    COMPILER_64_TO_32_LO(assoc_info->next_pkt_num, next_pn);
    COMPILER_64_TO_32_HI(assoc_info->next_pkt_num_upper, next_pn);

    if (assoc_info->next_pkt_num || assoc_info->next_pkt_num_upper)
        assoc_info->flags |= XFLOW_MACSEC_SECURE_ASSOC_INFO_SET_NEXT_PKT_NUM;

    if (soc_feature(unit, soc_feature_xflow_macsec_inline))
        assoc_info->an = (sa_index % 2);
    else
    {
        if (XFLOW_MACSEC_4_SA_PER_SC(unit))
            assoc_info->an = (sa_index % 4);
        else
            assoc_info->an = 0;
    }

    soc_mem_field_get(unit, ESEC_SA_TABLEm, (uint32 *)&sa_entry, SAKf, val256_field);
    sal_memset(&assoc_info->aes.key, 0, sizeof(assoc_info->aes.key));
    sal_memset(&assoc_info->aes_256.key, 0, sizeof(assoc_info->aes_256.key));

    if (soc_feature(unit, soc_feature_xflow_macsec_crypto_aes_256))
    {
        if (chan_info->crypto >= xflowMacsecCryptoAes256GcmIntegrityOnly)
        {
            uint32_to_byte_array(XFLOW_MACSEC_CRYPTO_AES256GCM_KEY_SIZE,
                    val256_field, (uint8 *)&assoc_info->aes_256.key);
        } else
        {
            uint32_to_byte_array(XFLOW_MACSEC_CRYPTO_AES128GCM_KEY_SIZE,
                    val256_field, (uint8 *)&assoc_info->aes.key);
        }
    }
    else
    {
        uint32_to_byte_array(XFLOW_MACSEC_CRYPTO_AES128GCM_KEY_SIZE,
                val256_field, (uint8 *)&assoc_info->aes.key);
    }
    assoc_info->ssci = soc_mem_field32_get(unit, ESEC_SA_TABLEm, &sa_entry, SSCIf);

    soc_mem_field_get(unit, ESEC_SA_TABLEm, (uint32 *)&sa_entry, SALTf, salt_field);

    uint32_to_byte_array(12, salt_field, (uint8 *)&assoc_info->salt);

    if (SOC_MEM_FIELD_VALID(unit, ESEC_SA_TABLEm, VALIDf))
        assoc_info->enable = soc_mem_field32_get(unit, ESEC_SA_TABLEm, &sa_entry, VALIDf);

    return BCM_E_NONE;
}

int
xflow_macsec_secure_assoc_get(int unit,
                             xflow_macsec_secure_assoc_id_t assoc_id,
                             xflow_macsec_secure_assoc_info_t *assoc_info,
                             xflow_macsec_secure_chan_id_t *chan_id)
{
    int oper;
    int an = 0;
    int sc_index, sc_hw_index;
    int sa_index, sa_hw_index;
    xflow_macsec_secure_chan_info_t chan_info;

    if ((assoc_info == NULL) || (chan_id == NULL))
        return BCM_E_PARAM;

    sa_index = XFLOW_MACSEC_SECURE_ASSOC_ID_INDEX_GET(assoc_id);
    BCM_IF_ERROR_RETURN(sa_index);

    oper = XFLOW_MACSEC_DIR_TYPE_GET(assoc_id);

    if (XFLOW_MACSEC_4_SA_PER_SC(unit))
    {
        sc_index = sa_index / 4;
        an = sa_index & 3;
    }
    else if (XFLOW_MACSEC_NUM_SA_PER_SC(unit, oper))
    {
        sc_index = sa_index / (XFLOW_MACSEC_NUM_SA_PER_SC(unit, oper));
        an = sa_index % XFLOW_MACSEC_NUM_SA_PER_SC(unit, oper);
    }
    else
        sc_index = sa_index;

    BCM_IF_ERROR_RETURN(xflow_macsec_index_get(unit, oper, xflowMacsecIdTypeSecureChan, sc_index, &sc_hw_index));

    if (XFLOW_MACSEC_4_SA_PER_SC(unit))
        sa_hw_index = (4 * sc_hw_index) + an;
    else if (XFLOW_MACSEC_NUM_SA_PER_SC(unit, oper))
        sa_hw_index = (XFLOW_MACSEC_NUM_SA_PER_SC(unit, oper) * sc_hw_index) + an;
    else
        sa_hw_index = sc_hw_index;

    *chan_id = XFLOW_MACSEC_SECURE_CHAN_ID_CREATE(oper, sc_index);

    BCM_IF_ERROR_RETURN(xflow_macsec_secure_chan_get(unit, *chan_id, &chan_info, NULL));
    if (oper == XFLOW_MACSEC_ENCRYPT)
        BCM_IF_ERROR_RETURN(_xflow_macsec_sa_get_encrypt(unit, sa_hw_index, assoc_info, &chan_info));
    else
        BCM_IF_ERROR_RETURN(_xflow_macsec_sa_get_decrypt(unit, sa_hw_index, assoc_info, &chan_info));

    return BCM_E_NONE;
}

int _xflow_macsec_sa_destroy_decrypt(int unit, int sa_index)
{
    isec_sa_table_entry_t sa_entry = {};

    BCM_IF_ERROR_RETURN(WRITE_ISEC_SA_TABLEm(unit, MEM_BLOCK_ALL, sa_index, &sa_entry));
    BCM_IF_ERROR_RETURN(_xflow_macsec_sa_decrypt_counter_clear(unit, sa_index));

    return BCM_E_NONE;
}

int _xflow_macsec_sa_destroy_encrypt(int unit, int sa_index)
{
    esec_sa_table_entry_t sa_entry = {};
//    esec_sa_hash_table_entry_t sa_hash_entry = {};

    BCM_IF_ERROR_RETURN(WRITE_ESEC_SA_TABLEm(unit, MEM_BLOCK_ALL, sa_index, &sa_entry));
//    BCM_IF_ERROR_RETURN(WRITE_ESEC_SA_HASH_TABLEm(unit, MEM_BLOCK_ALL, sa_index, &sa_hash_entry));
    BCM_IF_ERROR_RETURN(_xflow_macsec_sa_encrypt_counter_clear(unit, sa_index));

    return BCM_E_NONE;
}

int xflow_macsec_secure_assoc_destroy(int unit,
                                      xflow_macsec_secure_assoc_id_t assoc_id)
{
    int oper;
    int an = 0;
    int sc_index, sc_hw_index;
    int sa_index, sa_hw_index;
    xflow_macsec_secure_chan_id_t chan_id = 0;
    xflow_macsec_secure_assoc_info_t assoc_info;

    /* Validate if SA exists */
    memset((void *)&assoc_info, 0, sizeof(xflow_macsec_secure_assoc_info_t));
    BCM_IF_ERROR_RETURN(xflow_macsec_secure_assoc_get(unit, assoc_id, &assoc_info, &chan_id));

    sa_index = XFLOW_MACSEC_SECURE_ASSOC_ID_INDEX_GET(assoc_id);
    BCM_IF_ERROR_RETURN(sa_index);

    oper = XFLOW_MACSEC_DIR_TYPE_GET(assoc_id);

    if (XFLOW_MACSEC_4_SA_PER_SC(unit))
    {
        sc_index = sa_index / 4;
        an = sa_index & 3;
    }
    else if (XFLOW_MACSEC_NUM_SA_PER_SC(unit, oper))
    {
        sc_index = (sa_index / XFLOW_MACSEC_NUM_SA_PER_SC(unit, oper));
        an = sa_index % XFLOW_MACSEC_NUM_SA_PER_SC(unit, oper);
    }
    else
    {
        sc_index = sa_index;
    }

    BCM_IF_ERROR_RETURN(xflow_macsec_index_get(unit, oper, xflowMacsecIdTypeSecureChan, sc_index, &sc_hw_index));

    if (XFLOW_MACSEC_4_SA_PER_SC(unit))
    {
        sa_hw_index = (4 * sc_hw_index) + an;
    }
    else if (XFLOW_MACSEC_NUM_SA_PER_SC(unit, oper))
    {
        sa_hw_index = (XFLOW_MACSEC_NUM_SA_PER_SC(unit, oper) * sc_hw_index) + an;
    }
    else
    {
        sa_hw_index = sc_hw_index;
    }

    if (oper == XFLOW_MACSEC_ENCRYPT)
    {
        BCM_IF_ERROR_RETURN(_xflow_macsec_sa_destroy_encrypt(unit, sa_hw_index));
    }
    else
    {
        BCM_IF_ERROR_RETURN(_xflow_macsec_sa_destroy_decrypt(unit, sa_hw_index));
    }

    return BCM_E_NONE;
}

int _xflow_macsec_sc_destroy_decrypt(int unit, int sc_index)
{
    isec_sc_table_entry_t sc_table_entry;
    isec_sc_tcam_entry_t sc_tcam_entry;

    if ((sc_index < 0) ||
        (sc_index > soc_mem_index_max(unit, ISEC_SC_TABLEm)))
    {
        return BCM_E_PARAM;
    }

    memset(&sc_table_entry, 0, sizeof(isec_sc_table_entry_t));
    memset(&sc_tcam_entry, 0, sizeof(isec_sc_tcam_entry_t));

    BCM_IF_ERROR_RETURN(WRITE_ISEC_SC_TABLEm(unit, MEM_BLOCK_ALL, sc_index, &sc_table_entry));
    BCM_IF_ERROR_RETURN(WRITE_ISEC_SC_TCAMm(unit, MEM_BLOCK_ALL, sc_index, &sc_tcam_entry));
    BCM_IF_ERROR_RETURN(_xflow_macsec_sc_decrypt_counter_clear(unit, sc_index));

    return BCM_E_NONE;
}

int _xflow_macsec_sc_destroy_encrypt(int unit, int sc_index)
{
    esec_sc_table_entry_t sc_entry;
    memset(&sc_entry, 0, sizeof(esec_sc_table_entry_t));

    BCM_IF_ERROR_RETURN(WRITE_ESEC_SC_TABLEm(unit, MEM_BLOCK_ALL, sc_index, &sc_entry));

    BCM_IF_ERROR_RETURN(_xflow_macsec_sc_encrypt_counter_clear(unit, sc_index));

    return BCM_E_NONE;
}

int xflow_macsec_secure_chan_destroy(int unit,
                                     xflow_macsec_secure_chan_id_t chan_id)
{
    int sc_index, sc_hw_index;
    int oper;

    sc_index = XFLOW_MACSEC_SECURE_CHAN_ID_INDEX_GET(chan_id);
    BCM_IF_ERROR_RETURN(sc_index);

    oper = XFLOW_MACSEC_DIR_TYPE_GET(chan_id);

    BCM_IF_ERROR_RETURN(xflow_macsec_index_get(unit, oper, xflowMacsecIdTypeSecureChan, sc_index, &sc_hw_index));

    if (oper == XFLOW_MACSEC_ENCRYPT)
    {
        BCM_IF_ERROR_RETURN(_xflow_macsec_sc_destroy_encrypt(unit, sc_hw_index));
    }
    else
    {
        BCM_IF_ERROR_RETURN(_xflow_macsec_sc_destroy_decrypt(unit, sc_hw_index));
    }

    BCM_IF_ERROR_RETURN(xflow_macsec_index_free(unit, oper, xflowMacsecIdTypeSecureChan, sc_index));
    return BCM_E_NONE;
}

int xflow_macsec_secure_chan_enable_get(int unit,
                                        xflow_macsec_secure_chan_id_t chan_id,
                                        int *enable)
{
    int sc_index;
    int oper;

    sc_index = XFLOW_MACSEC_SECURE_CHAN_ID_INDEX_GET(chan_id);
    BCM_IF_ERROR_RETURN(sc_index);

    oper = XFLOW_MACSEC_DIR_TYPE_GET(chan_id);

    if(oper == XFLOW_MACSEC_DECRYPT)
        BCM_IF_ERROR_RETURN(_xflow_macsec_sc_decrypt_index_hw_index_get(unit, sc_index, &sc_index, NULL));
    else
        BCM_IF_ERROR_RETURN(_xflow_macsec_sc_encrypt_index_hw_index_get(unit, sc_index, &sc_index, NULL));

    if (oper == XFLOW_MACSEC_ENCRYPT)
    {
        BCM_IF_ERROR_RETURN(_xflow_macsec_sc_enable_get_encrypt(unit, sc_index, enable));
    }
    else
    {
        BCM_IF_ERROR_RETURN(_xflow_macsec_sc_enable_get_decrypt(unit, sc_index, enable));
    }
    return BCM_E_NONE;
}

int xflow_macsec_flow_enable_set(int unit, xflow_macsec_flow_id_t flow_id, int enable)
{
    int sp_index, sp_tcam_index;
    sub_port_data_entry_t subport_data_entry;
    sal_memset(&subport_data_entry, 0, sizeof(sub_port_data_entry_t));

    if (enable < 0)
        return BCM_E_PARAM;

    sp_index = XFLOW_MACSEC_FLOW_ID_INDEX_GET(flow_id);
    BCM_IF_ERROR_RETURN(sp_index);

    if (XFLOW_MACSEC_FLOW_ID_IS_ENCRYPT(flow_id))
        return BCM_E_PARAM;

    BCM_IF_ERROR_RETURN(_xflow_macsec_flow_index_hw_index_get( unit, sp_index, &sp_tcam_index, NULL));

    BCM_IF_ERROR_RETURN(xflow_macsec_firelight_flow_enable_set(unit, sp_tcam_index, enable));

    return BCM_E_NONE;
}

int xflow_macsec_flow_enable_get(int unit, xflow_macsec_flow_id_t flow_id, int *enable)
{
    int sp_index, sp_tcam_index;
    sub_port_data_entry_t subport_data_entry;
    sal_memset(&subport_data_entry, 0, sizeof(sub_port_data_entry_t));

    if (enable == NULL)
        return BCM_E_PARAM;

    if (XFLOW_MACSEC_FLOW_ID_IS_ENCRYPT(flow_id))
        return BCM_E_PARAM;

    sp_index = XFLOW_MACSEC_FLOW_ID_INDEX_GET(flow_id);
    BCM_IF_ERROR_RETURN(sp_index);

    BCM_IF_ERROR_RETURN(_xflow_macsec_flow_index_hw_index_get(unit, sp_index, &sp_tcam_index, NULL));

    BCM_IF_ERROR_RETURN(xflow_macsec_firelight_flow_enable_get(unit, sp_tcam_index, enable));

    return BCM_E_NONE;
}


/* Decrypt Policy */
int xflow_macsec_policy_create(int unit, uint32 flags, xflow_macsec_policy_info_t *policy_info, xflow_macsec_policy_id_t *policy_id)
{
    int rv;
    uint8 flag = FALSE;
    int policy_index;
    int policy_hw_index;

    if ((policy_info == NULL) || (policy_id == NULL))
        return BCM_E_PARAM;

    /* Assumes that caller always pass XFLOW_MACSEC_DECRYPT */
    _XFLOW_MACSEC_ENCRYPT_DECRYPT_CHECK(flags);

    if (flags & XFLOW_MACSEC_POLICY_WITH_ID)
    {
        if (XFLOW_MACSEC_DIR_TYPE_GET(*policy_id) != XFLOW_MACSEC_DECRYPT)
            return BCM_E_PARAM;

        policy_index = XFLOW_MACSEC_POLICY_ID_INDEX_GET(*policy_id);
        BCM_IF_ERROR_RETURN(policy_index);

        if (XFLOW_MACSEC_POLICY_ID_IS_ENCRYPT(*policy_id))
            return BCM_E_PARAM;

        flag = TRUE;
    }

    /* This reserves a policy index */
    BCM_IF_ERROR_RETURN(_xflow_macsec_policy_index_reserve(unit, 0, &policy_index, flag));

    rv = _xflow_macsec_policy_index_hw_index_get(unit, policy_index, &policy_hw_index, NULL);
    if (BCM_FAILURE(rv))
    {
        BCM_IF_ERROR_RETURN(_xflow_macsec_policy_index_free(unit, policy_index));
        return rv;
    }

    rv = xflow_macsec_firelight_policy_set(unit, policy_hw_index, policy_info);

    if (BCM_FAILURE(rv))
    {
        BCM_IF_ERROR_RETURN(_xflow_macsec_policy_index_free(unit, policy_index));
        return rv;
    }

    if (!(flags & XFLOW_MACSEC_POLICY_WITH_ID))
    {
        *policy_id = XFLOW_MACSEC_POLICY_ID_CREATE(XFLOW_MACSEC_DECRYPT, policy_index);
    }

    return BCM_E_NONE;
}

int _xflow_macsec_decrypt_policy_destroy(int unit,
                                         int policy_index)
{
    sub_port_policy_table_entry_t policy_entry;

    if ((policy_index < 0) ||
        (policy_index > soc_mem_index_max(unit, SUB_PORT_POLICY_TABLEm)))
    {
        return BCM_E_PARAM;
    }

    memset(&policy_entry, 0, sizeof(sub_port_policy_table_entry_t));

    BCM_IF_ERROR_RETURN(WRITE_SUB_PORT_POLICY_TABLEm(unit, MEM_BLOCK_ALL,
                                             policy_index, &policy_entry));

    BCM_IF_ERROR_RETURN(_xflow_macsec_policy_decrypt_counter_clear(unit, policy_index));

    return BCM_E_NONE;
}

int xflow_macsec_policy_destroy (int unit,
                                 xflow_macsec_policy_id_t policy_id)
{
    int policy_index;

    policy_index = XFLOW_MACSEC_POLICY_ID_INDEX_GET(policy_id);
    BCM_IF_ERROR_RETURN(policy_index);

    if (XFLOW_MACSEC_POLICY_ID_IS_DECRYPT(policy_id))
    {
        BCM_IF_ERROR_RETURN(_xflow_macsec_policy_index_hw_index_get(unit, policy_index, &policy_index, NULL));

        BCM_IF_ERROR_RETURN(_xflow_macsec_decrypt_policy_destroy(unit, policy_index));

        BCM_IF_ERROR_RETURN(xflow_macsec_index_free(unit, XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypePolicy,
                                                    policy_index));
    }
    else
        return BCM_E_PARAM;

    return BCM_E_NONE;
}
