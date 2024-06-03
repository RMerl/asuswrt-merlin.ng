/*
<:copyright-BRCM:2020:DUAL/GPL:standard

   Copyright (c) 2020 Broadcom 
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
