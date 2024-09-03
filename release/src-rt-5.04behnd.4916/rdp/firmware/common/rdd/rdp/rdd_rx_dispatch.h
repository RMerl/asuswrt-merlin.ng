/*
   <:copyright-BRCM:2014-2016:DUAL/GPL:standard
   
      Copyright (c) 2014-2016 Broadcom 
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

#ifndef _RDD_RX_DISPATCH_H
#define _RDD_RX_DISPATCH_H

#ifdef DS_DYNAMIC_DISPATCH
void rdd_ds_dynamic_dispatcher_init(uint16_t *processing_tasks_arr, uint32_t num_of_tasks);
#endif
#ifdef US_DYNAMIC_DISPATCH
void rdd_us_dynamic_dispatcher_init(uint16_t *processing_tasks_arr, uint32_t num_of_tasks);
#endif
void rdd_parallel_processing_init(void);

#endif /* _RDD_RX_DISPATCH_H */
