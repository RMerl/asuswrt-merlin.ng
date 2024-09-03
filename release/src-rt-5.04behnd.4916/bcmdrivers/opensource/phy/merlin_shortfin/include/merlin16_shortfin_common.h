/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
:>
*/

/********************************************************************************
 ********************************************************************************
 *                                                                              *
 *  Revision      :   *
 *                                                                              *
 *  Description   :  Defines and Enumerations required by Serdes APIs           *
 *                                                                              *
 ********************************************************************************
 ********************************************************************************/

/** @file merlin16_shortfin_common.h
 * Defines and Enumerations shared across M16/F16/BHK16 APIs BUT NOT MICROCODE
 */

#ifndef MERLIN16_SHORTFIN_API_COMMON_H
#define MERLIN16_SHORTFIN_API_COMMON_H

#include "merlin16_shortfin_ipconfig.h"

/** Macro to determine sign of a value */
#define sign(x) ((x>=0) ? 1 : -1)

#define UCODE_MAX_SIZE (84*1024)

/*
 * IP-Specific Iteration Bounds
 */

#endif
