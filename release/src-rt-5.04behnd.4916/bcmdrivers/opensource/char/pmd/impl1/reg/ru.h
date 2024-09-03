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

#if !defined(ru_h)
#define ru_h

#include "bcm_OS_Deps.h"
#include <linux/types.h>

void fld_set(uint32_t mask, uint32_t shift, uint32_t *rv, uint32_t fv);
uint32_t fld_get(uint32_t mask, uint32_t shift, uint32_t rv);
void fld_write(uint16_t addr, uint32_t mask, uint32_t shift, uint32_t fv);
uint32_t fld_read(uint16_t addr, uint32_t mask, uint32_t shift);

#define RU_REG_READ(c,r,v)   ru_reg_read((c##_##r + c##_ADDR), v)

#define RU_REG_WRITE(c,r,v)    ru_reg_write((c##_##r + c##_ADDR), v)

#define RU_FIELD_READ(c,r,f) fld_read((c##_##r + c##_ADDR),\
                                 c##_##r##_##f##_MASK, c##_##r##_##f##_SHIFT)
#define RU_FIELD_WRITE(c,r,f,fv) fld_write((c##_##r + c##_ADDR),\
                                     c##_##r##_##f##_MASK, c##_##r##_##f##_SHIFT, fv)
#define RU_FIELD_GET(c,r,f,v) fld_get(c##_##r##_##f##_MASK, c##_##r##_##f##_SHIFT, v)

#define RU_FIELD_SET(c,r,f,rv,d) fld_set(c##_##r##_##f##_MASK, c##_##r##_##f##_SHIFT, &rv, d)

#endif /* End of file ru.h */
