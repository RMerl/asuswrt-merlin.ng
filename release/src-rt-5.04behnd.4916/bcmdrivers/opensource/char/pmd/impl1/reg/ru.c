/*---------------------------------------------------------------------------
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
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
 ------------------------------------------------------------------------- */
/**
 * \brief Register Utilities functional implementation
 */
#include "ru.h"
#include "pmd_op.h"


void fld_set(uint32_t mask, uint32_t shift, uint32_t *rv, uint32_t fv)
{
    *rv = (*rv & ~mask) | (fv << shift);
}

uint32_t fld_get(uint32_t mask, uint32_t shift, uint32_t rv)
{
    return ((rv & mask) >> shift);
}

void fld_write(uint16_t addr, uint32_t mask, uint32_t shift, uint32_t fv)
{
    uint32_t rv;

    ru_reg_read(addr, &rv);
    fld_set(mask, shift, &rv, fv);
    ru_reg_write(addr, rv);
}


uint32_t fld_read(uint16_t addr, uint32_t mask, uint32_t shift)
{
    uint32_t rv;

    ru_reg_read(addr, &rv);
    return fld_get(mask, shift, rv);
}


/* End of file ru.c */
