/* SPDX-License-Identifier:     GPL-2.0+ */
/*
 * Copyright 2018 NXP
 */

#ifndef SC_RM_API_H
#define SC_RM_API_H

#include <asm/arch/sci/types.h>

/* Defines for type widths */
#define SC_RM_PARTITION_W   5U      /* Width of sc_rm_pt_t */
#define SC_RM_MEMREG_W      6U      /* Width of sc_rm_mr_t */
#define SC_RM_DID_W         4U      /* Width of sc_rm_did_t */
#define SC_RM_SID_W         6U      /* Width of sc_rm_sid_t */
#define SC_RM_SPA_W         2U      /* Width of sc_rm_spa_t */
#define SC_RM_PERM_W        3U      /* Width of sc_rm_perm_t */

/* Defines for ALL parameters */
#define SC_RM_PT_ALL        ((sc_rm_pt_t)UINT8_MAX)   /* All partitions */
#define SC_RM_MR_ALL        ((sc_rm_mr_t)UINT8_MAX)   /* All memory regions */

/* Defines for sc_rm_spa_t */
#define SC_RM_SPA_PASSTHRU  0U   /* Pass through (attribute driven by master) */
#define SC_RM_SPA_PASSSID   1U   /* Pass through and output on SID */
#define SC_RM_SPA_ASSERT    2U   /* Assert (force to be secure/privileged) */
#define SC_RM_SPA_NEGATE    3U   /* Negate (force to be non-secure/user) */

/* Defines for sc_rm_perm_t */
#define SC_RM_PERM_NONE         0U /* No access */
#define SC_RM_PERM_SEC_R        1U /* Secure RO */
#define SC_RM_PERM_SECPRIV_RW   2U /* Secure privilege R/W */
#define SC_RM_PERM_SEC_RW       3U /* Secure R/W */
#define SC_RM_PERM_NSPRIV_R     4U /* Secure R/W, non-secure privilege RO */
#define SC_RM_PERM_NS_R         5U /* Secure R/W, non-secure RO */
#define SC_RM_PERM_NSPRIV_RW    6U /* Secure R/W, non-secure privilege R/W */
#define SC_RM_PERM_FULL         7U /* Full access */

/* Types */

/*!
 * This type is used to declare a resource partition.
 */
typedef u8 sc_rm_pt_t;

/*!
 * This type is used to declare a memory region.
 */
typedef u8 sc_rm_mr_t;

/*!
 * This type is used to declare a resource domain ID used by the
 * isolation HW.
 */
typedef u8 sc_rm_did_t;

/*!
 * This type is used to declare an SMMU StreamID.
 */
typedef u16 sc_rm_sid_t;

/*!
 * This type is a used to declare master transaction attributes.
 */
typedef u8 sc_rm_spa_t;

typedef u8 sc_rm_perm_t;

#endif /* SC_RM_API_H */
