/*
    <:copyright-BRCM:2015:DUAL/GPL:standard
    
       Copyright (c) 2015 Broadcom 
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

#ifndef _RDD_H_
#define _RDD_H_

#ifndef __PACKING_ATTRIBUTE_STRUCT_END__
#include "packing.h"
#endif
#include "rdpa_types.h"
#include "rdpa_ip_class_basic.h"
#include "rdpa_cpu_basic.h"
#ifndef _CFE_
#include "rdpa_iptv.h"
#include "bdmf_system.h"
#include "bdmf_interface.h"
#else
#include "lib_printf.h"
#define bdmf_trace xprintf
#endif
#include "access_macros.h"
#include "rdd_runner_defs_auto.h"
#include "rdd_data_structures_auto.h"
#include "rdd_platform.h"
#ifdef RDP_SIM
#include "rdd_simulator.h"
#endif
/* task addresses labels from fw compilation */
#include "rdd_runner_labels.h"

/* rdd autogen files */
#include "rdd_ag_service_queues.h"
#include "rdd_ag_cpu_rx.h"

#endif /* _RDD_H_ */

