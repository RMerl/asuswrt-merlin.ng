// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2024 Broadcom Corporation
   All Rights Reserved
*/

/*********************************************************************************
 *********************************************************************************
 *  File Name  :  merlin8_phy1_select_defns.h
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

#ifndef MERLIN8_PHY1_API_SELECT_DEFNS_H
#define MERLIN8_PHY1_API_SELECT_DEFNS_H

#include "merlin8_phy1_ipconfig.h"
#include "merlin8_phy1_field_access.h"

#   include "merlin7_api_uc_common.h"



/****************************************************************************
 * @name Register Access Macro Inclusions
 *
 * All cores provide access to hardware control/status registers.
 ****************************************************************************/
/**@{*/

/**
 * This build includes register access macros for the MERLIN8_PCIEG3 core.
 */
#include "merlin8_phy1_fields.h"

/**@}*/


/****************************************************************************
 * @name RAM Access Macro Inclusions
 *
 * Some cores also provide access to firmware control/status RAM variables.
 ****************************************************************************/
/**@{*/

/**
 * This build includes macros to access Merlin7 microcode RAM variables.
 */
#include "merlin8_api_uc_vars_rdwr_defns.h"
/**@}*/

#endif
