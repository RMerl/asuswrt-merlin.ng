/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
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
#ifndef __ENUM_STR__H_
#define __ENUM_STR__H_

#include <linux/kernel.h>

typedef struct {
    int val;
    char *str;
} val_to_str_t;

static inline char *enum_val_to_str(int value, val_to_str_t *map)
{
    int i;

    for (i = 0; map[i].val != -1 && map[i].val != value; i++);
    
    return map[i].val == -1 ? NULL : map[i].str;
}

#endif
