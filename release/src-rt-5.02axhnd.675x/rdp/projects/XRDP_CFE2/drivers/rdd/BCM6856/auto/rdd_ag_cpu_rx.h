/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
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



/* This is an automated file. Do not edit its contents. */


#ifndef _RDD_AG_CPU_RX_H_
#define _RDD_AG_CPU_RX_H_

#include "rdd.h"

int rdd_ag_cpu_rx_cpu_reason_to_tc_set(uint32_t _entry, uint8_t bits);
int rdd_ag_cpu_rx_cpu_reason_to_tc_get(uint32_t _entry, uint8_t *bits);

#endif /* _RDD_AG_CPU_RX_H_ */
