/*---------------------------------------------------------------------------
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

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
