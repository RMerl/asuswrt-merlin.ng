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

#ifndef _RDD_H
#define _RDD_H

#if defined(FIRMWARE_INIT)

#define CACHED_MALLOC(x)		((uint32_t) x)
#define CACHED_MALLOC_ATOMIC(x)		((uint32_t) x)
#define PHYS_TO_VIRT(_addr)		(_addr)

#else

#include "bl_os_wraper.h"
#include "access_macros.h"

#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
#include "bcm_mm.h"
#define PHYS_TO_VIRT(_addr)		PHYS_TO_CACHED(_addr)

#endif /* DSL_63138 || DSL_63148 || WL4908 */
#endif /* defined(FIRMWARE_INIT) */

#ifndef __PACKING_ATTRIBUTE_STRUCT_END__
#include "packing.h"
#endif
#include "rdpa_types.h"
#include "bdmf_system.h"
#include "bdmf_interface.h"

#include "rdp_runner.h"

#include "rdd_defs.h"
#include "rdd_runner_proj_defs.h"
#include "rdd_runner_defs_auto.h"
#include "rdd_runner_defs.h"
#include "rdd_data_structures_auto.h"
#include "rdd_data_structures.h"
#include "rdd_platform.h"
#include "rdd_common.h"
#include "rdd_init.h"
#include "rdd_proj_init.h"
#include "rdd_simulator.h"
#include "rdd_ic.h"
#include "rdd_cpu.h"
#include "rdd_tm.h"
#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
#include "rdd_router.h"
#else
#include "rdd_fc.h"
#endif
#include "rdd_wlan_mcast.h"

/* task addresses labels from fw compilation */
#include "rdd_runner_a_labels.h"
#include "rdd_runner_b_labels.h"
#include "rdd_runner_c_labels.h"
#include "rdd_runner_d_labels.h"

#endif /* _RDD_H */
