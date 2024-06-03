// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    
*/

/*********************************************************************************
 *********************************************************************************
 *  File Name  :  merlin16_shortfin_select_defns.h
 *  Created On :  29 Sep 2015
 *  Created By :  Brent Roberts
 *  Description:  Select header files for IP-specific definitions
 *  Revision   :  
 *  
 *********************************************************************************
 ********************************************************************************/

 /** @file
 * Select IP files to include for API
 */

#ifndef MERLIN16_SHORTFIN_API_SELECT_DEFNS_H
#define MERLIN16_SHORTFIN_API_SELECT_DEFNS_H

#include "merlin16_shortfin_ipconfig.h"
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
