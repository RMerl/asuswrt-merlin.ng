/*
* <:copyright-BRCM:2022:DUAL/GPL:standard
* 
*    Copyright (c) 2022 Broadcom 
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
:>
*/

#ifndef _SW_GSO_HOOK_
#define _SW_GSO_HOOK_

#include <linux/nbuff.h>
#include <linux/bcm_log.h>

void* pkt_get_queue(void*);
void  pkt_set_queue(void*,void*);
int sw_gso_get_wait_queue_head_t_sz(void);
void* sw_gso_alloc_spinlock(void);
void sw_gso_free_spinlock(void* lock);
void sw_gso_spin_lock_bh(void* lock);
void sw_gso_spin_unlock_bh(void* lock);
#endif /* _SW_GSO_HOOK_ */
