/*
<:copyright-BRCM:2020:DUAL/GPL:standard

   Copyright (c) 2020 Broadcom 
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

#ifndef __WLCSM_NVRAM_H_
#define __WLCSM_NVRAM_H_

#include "wlcsm_linux.h"
#include <linux/list.h>
typedef struct {
    unsigned int pid;
    char name[256];
    struct list_head list;
} PROCESS_REG_LIST;

typedef struct wlcsm_nvram_tuple {
    struct rb_node node;
    t_WLCSM_NAME_VALUEPAIR *tuple;
} t_WLCSM_NVRAM_TUPLE;

int wlcsm_nvram_set(char *buf, int len);
int wlcsm_nvram_unset (char *buf );
int wlcsm_nvram_get(char *name,char *result);
int wlcsm_nvram_getall(char *buf,int count);
int wlcsm_nvram_getall_pos(char *buf,int count,int pos);
#endif
