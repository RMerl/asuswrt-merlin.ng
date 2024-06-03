// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    
*/

/** @file merlin_mptwo_common.h
 * Defines and Enumerations shared across Eagle/Merlin/Falcon APIs BUT NOT MICROCODE
 */

#ifndef MERLIN_MPTWO_API_COMMON_H
#define MERLIN_MPTWO_API_COMMON_H
#include "common/srds_api_uc_common.h"

/** Macro to determine sign of a value */
#define sign(x) ((x>=0) ? 1 : -1)

#define UCODE_MAX_SIZE  32768

/*
 * Register Address Defines used by the API that are different between IPs
 */
#define DSC_A_DSC_UC_CTRL 0xD00D
#define TLB_RX_PRBS_CHK_ERR_CNT_MSB_STATUS 0xD0DA

/* PLL Lock and change Status Register define */
#define PLL_STATUS_ADDR 0xD128

/* PMD Lock and change Status Register define */
#define PMD_LOCK_STATUS_ADDR 0xD0DC

/* Sigdet and change Status Register define */
#define SIGDET_STATUS_ADDR 0xD0C8

#define MDIO_MMDSEL_AER_COM_MDIO_MASKDATA  0xFFDB

/* PCIe Gen3 Register Address defines*/

/*
 * Register Address Defines used by the API that are COMMON across IPs
 */


/*
 * IP-Specific Iteration Bounds
 */

#endif
