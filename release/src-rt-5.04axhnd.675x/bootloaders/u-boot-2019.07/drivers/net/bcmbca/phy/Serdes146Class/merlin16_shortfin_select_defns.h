// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

*/
#ifndef MERLIN16_SHORTFIN_API_SELECT_DEFNS_H
#define MERLIN16_SHORTFIN_API_SELECT_DEFNS_H

//#include "merlin16_shortfin_ipconfig.h"
#include "merlin16_shortfin_field_access.h"

#   include "merlin16_api_uc_common.h"

/****************************************************************************
 * @name Register Access Macro Inclusions
 *
 * All cores provide access to hardware control/status registers.
 ****************************************************************************/
/**@{*/

/**
 * This build includes register access macros for the MERLIN16_SHORTFIN core.
 */
#include "merlin16_shortfin_fields.h"

/**@}*/


/****************************************************************************
 * @name RAM Access Macro Inclusions
 *
 * Some cores also provide access to firmware control/status RAM variables.
 ****************************************************************************/
/**@{*/

/**
 * This build includes macros to access Merlin16 microcode RAM variables.
 */
#include "merlin16_api_uc_vars_rdwr_defns.h"
/**@}*/

#endif
