/*
    Copyright 221 Broadcom Corporation

    <:label-BRCM:2021:DUAL/GPL:standard
    
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

#ifndef _BOARD_DT_H
#define _BOARD_DT_H

#include <asm/io.h>

void *bcm_get_ioreg(const char *compat, const char *name);
static inline void bcm_unmap_ioreg(void *addr)
{
	iounmap(addr);
}
bool bcm_get_prop32(const char *compat, const char *name, uint32_t *value);

#endif /* _BOARD_DT_H */

