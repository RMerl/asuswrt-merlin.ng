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

#include "macsec_types.h"
#include "soc/drv.h"

#ifndef MACSEC_DEFS_H
#define MACSEC_DEFS_H

#define NUM_SA_PER_PHY 2
#define INGRESS     0
#define EGRESS      1

#define SOC_MAX_NUM_DEVICES	4
#define REG_PORT_ANY    0xff
#define BCM_PORT_INVALID    0xff

/* used as the block parameter in memory access */
#define SOC_BLOCK_ANY                   -1      /* for reading */
#define SOC_BLOCK_ALL                   -1      /* for writing */

#define    MEM_BLOCK_ANY        SOC_BLOCK_ANY    /* for reading */
#define    MEM_BLOCK_ALL        SOC_BLOCK_ALL    /* for writing */
#define    COPYNO_ALL           SOC_BLOCK_ALL    /* historical */

#define SOC_IS_FIRELIGHT(unit)    (1)

#define XFLOW_MACSEC_TX_THRESHOLD   0x8

#define _sal_assert assert
#define sal_usecs_t int
#define sal_time_t int
#define SAL_ALLOC_F_ZERO     0x01

#define NUM_MACSEC_UNITS                1
#define CMBB_FL_MACSEC_MAX_PORT_NUM     4
#define CMBB_XFLOW_MACSEC_NUM_RSVD_SC   0 
#define CMBB_SVTAG_TPID                 0xEC

#define EXTERN  extern

#define INVALID_fld 255 
#define INVALIDreg NULL

#define sal_memset  memset
#define sal_memcmp  memcmp

#define SOC_UBUS_REG_IS_64   soc_ubus_reg_is_64
#define soc_ubus_reg_t   soc_ubus_reg_s *
#define soc_ubus_reg64_field_set    soc_ubus_reg64_field32_set
#define soc_ubus_reg64_field32_get  soc_ubus_reg64_field_get
#define soc_ubus_reg_field_get      soc_ubus_reg64_field_get

extern unsigned char *virt_base;

extern int  _soc_mem_cmp_undef(int, void *, void *);
extern int  (*soc_mem_config_set)(char *name, char *value);


uint32  soc_ubus_xlmac_reg_get(int unit, soc_ubus_reg_t reg, int port, uint64 *rval);
uint32  soc_ubus_xlmac_reg_set(int unit, soc_ubus_reg_t reg, int port, uint64 rval);
int     soc_ubus_xlmac_reg_fields32_modify(int unit, soc_ubus_reg_t reg, int port, soc_ubus_field_t *fld, uint32 *rval);

int     soc_ubus_reg_is_64(int unit, soc_ubus_reg_s *reg);
uint32  soc_ubus_reg_get(int unit, soc_ubus_reg_t reg, int port, uint64 *rval);
uint32  soc_ubus_reg_set(int unit, soc_ubus_reg_t reg, int port, uint64 rval);
uint32  soc_ubus_reg32_set(int unit, soc_ubus_reg_t reg, int port, uint32 rval);
uint32  soc_ubus_reg32_get(int unit, soc_ubus_reg_t reg, int port, uint32 *rval);
uint32  soc_ubus_reg32_field_get(int unit, soc_ubus_reg_t reg, uint32 rval32, soc_ubus_field_t fld);
int     SOC_DIRECT_REG_IS_64(int unit, soc_ubus_reg_t reg);
int     soc_ubus_reg_field_length(int unit, soc_ubus_reg_t reg, uint32 fld);
uint32  soc_ubus_reg64_field32_set(int unit, soc_ubus_reg_t reg, uint64 *rval64, soc_ubus_field_t fld, uint64 val);
uint32  soc_ubus_reg_field32_set(int unit, soc_ubus_reg_t reg, uint32 *rval32, soc_ubus_field_t fld, uint32 val);
int  soc_ubus_reg_field_set(int unit, soc_ubus_reg_t reg, uint32 *rval, soc_ubus_field_t fld, uint32 val);
uint64  soc_ubus_reg64_field_get(int unit, soc_ubus_reg_t reg, uint64 rval64, soc_ubus_field_t fld);

int          soc_feature(int unit, int feature);
unsigned int cmbb_soc_property_get(int unit, int property);
unsigned int cmbb_soc_property_port_get(int unit, int phy_port, int property);

void *soc_cm_salloc(int unit, int size, char *name);
void soc_cm_sfree(int unit, void *mem);
void *sal_alloc(int size, char *name);
void sal_free(void *mem);

int _xflow_macsec_sc_encrypt_index_hw_index_get(int unit, int index, int *hw_index, int *prio);
int _xflow_macsec_policy_index_hw_index_get(int unit, int index, int *hw_index, int *prio);
int _xflow_macsec_flow_index_hw_index_get(int unit, int index, int *hw_index, int *prio);
int _xflow_macsec_sc_decrypt_index_hw_index_get(int unit, int index, int *hw_index,int *prio);
int _xflow_macsec_sc_encrypt_index_logical_index_get(int unit, int index, int *hw_index);
int _xflow_macsec_sc_decrypt_index_logical_index_get(int unit, int sc_hw_index, int *sc_logical_index);
int _xflow_macsec_sc_encrypt_index_reserve(int unit, int prio, int *index, uint8 flag);
int _xflow_macsec_sc_decrypt_index_reserve(int unit, int prio, int *index, uint8 flag);
int _xflow_macsec_policy_index_reserve(int unit, int prio, int *index, uint8 flag);
int _xflow_macsec_flow_index_reserve(int unit, int prio, int *index, uint8 flag);

int _xflow_macsec_sc_encrypt_index_free(int unit, int index);
int _xflow_macsec_policy_index_free(int unit, int index);
int _xflow_macsec_flow_index_free(int unit, int index);
int _xflow_macsec_sc_decrypt_index_free(int unit, int index);

#endif //MACSEC_DEFS_H

