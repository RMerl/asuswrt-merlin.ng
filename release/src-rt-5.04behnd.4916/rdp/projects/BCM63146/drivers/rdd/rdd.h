/*
    <:copyright-BRCM:2015:DUAL/GPL:standard
    
       Copyright (c) 2015 Broadcom 
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

#ifndef _RDD_H_
#define _RDD_H_

#ifndef __PACKING_ATTRIBUTE_STRUCT_END__
#include "packing.h"
#endif

#include "rdpa_types.h"
#include "rdpa_ip_class_basic.h"
#include "rdpa_cpu_basic.h"
#include "rdpa_ingress_class_basic.h"
#include "rdpa_iptv.h"
#include "bdmf_system.h"
#include "bdmf_interface.h"
#include "access_macros.h"
#include "rdd_runner_defs_auto.h"
#include "rdd_data_structures_auto.h"
#include "rdd_platform.h"
/* task addresses labels from fw compilation */
#include "rdd_runner_labels.h"


#include "rdd_scheduling.h"
/* rdd autogen files */
#include "rdd_ag_update_fifo.h"
#include "rdd_ag_processing.h"
#if defined(BCM63146)
#include "rdd_ag_service_queues.h"
#include "rdd_ag_ds_tm.h"
#else
#include "rdd_ag_ds_tm.h"
#include "rdd_ag_service_queues.h"
#endif
#include "rdd_ag_us_tm.h"

#endif /* _RDD_H_ */

