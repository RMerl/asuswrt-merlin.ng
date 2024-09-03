/*
<:copyright-BRCM:2022:DUAL/GPL:standard

   Copyright (c) 2022 Broadcom 
   All Rights Reserved

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

/*****************************************************************************
 *  Description:
 *      This contains the PMC driver for Linux.
 *****************************************************************************/


#include "pmc_drv.h"

int GetDevPresence(int devAddr, int *value)
{
    return 0;
}

EXPORT_SYMBOL(GetDevPresence);

int GetSWStrap(int devAddr, int *value)
{
    return 0;
}

EXPORT_SYMBOL(GetSWStrap);

int GetHWRev(int devAddr, int *value)
{
    return 0;
}

EXPORT_SYMBOL(GetHWRev);

int GetNumZones(int devAddr, int *value)
{
    return 0;
}

EXPORT_SYMBOL(GetNumZones);

int GetAvsDisableState(int island, int *state)
{
    return 0;
}

EXPORT_SYMBOL(GetAvsDisableState);



int GetAllROs(uint32_t pa)
{
    return 0;
}

EXPORT_SYMBOL(GetAllROs);


void install_pmc_isr(uint32_t virq)
{
    
}

void install_pmc_temp_warn_isr(uint32_t virq)
{
    
}

int RecloseAVS(int iscold)
{
    return 0;
}

EXPORT_SYMBOL(RecloseAVS);


int pmc_init(void)
{
    return 0;
}
