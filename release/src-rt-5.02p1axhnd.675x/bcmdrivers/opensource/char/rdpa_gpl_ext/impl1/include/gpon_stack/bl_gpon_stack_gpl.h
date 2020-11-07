/*
* <:copyright-BRCM:2013:GPL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
*    All Rights Reserved
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
*/


#ifndef BL_GPON_STACK_GPL_H_INCLUDED
#define BL_GPON_STACK_GPL_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

/* Definitions */

/******************************************************************************/
/* This type defines the rogue detection types                                */
/******************************************************************************/
typedef enum
{
    CE_BL_TX_MONITOR_ROGUE_MODE,
    CE_BL_TX_FAULT_ROGUE_MODE,
    CE_BL_TX_INVALID_ROGUE_MODE
}
BL_ROGUE_ONU_MODE_DTE ;

#ifdef __cplusplus
}
#endif

#endif /* BL_GPON_STACK_GPL_H_INCLUDED */

