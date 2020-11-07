/*
    <:copyright-BRCM:2013:DUAL/GPL:standard
    
       Copyright (c) 2013 Broadcom 
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

#ifndef _BL_LILAC_DRV_RUNNER_H
#define _BL_LILAC_DRV_RUNNER_H

#if defined(FIRMWARE_INIT)

#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "netinet/in.h"

#else

#include "bl_os_wraper.h"
#include "pmc_drv.h" 

#ifdef LINUX_KERNEL

#include <access_macros.h>
#include <linux/string.h>

#else

#include "access_macros.h"

#endif /* LINUX_KERNEL */


#endif /* defined(FIRMWARE_INIT) */

#ifdef _CFE_
#include "bcm_mm.h"
#define RDD_VIRT_TO_PHYS(_addr)                     K0_TO_PHYS((uint32_t)_addr)
#elif defined(__KERNEL__)
#include "bcm_mm.h"
#define RDD_VIRT_TO_PHYS(_addr)                     VIRT_TO_PHYS(_addr)
#else
#define RDD_VIRT_TO_PHYS(_addr)		            ((uint32_t)_addr)
#endif

#if defined(__KERNEL__)
#include "bcm_rsvmem.h"
#define RDD_RSV_VIRT_TO_PHYS(_vbase, _pbase, _addr) BcmMemReserveVirtToPhys(_vbase, _pbase, _addr)
#define RDD_RSV_PHYS_TO_VIRT(_vbase, _pbase, _addr) BcmMemReservePhysToVirt(_vbase, _pbase, _addr)
#else
#define RDD_RSV_VIRT_TO_PHYS(_vbase, _pbase, _addr) RDD_VIRT_TO_PHYS(_addr)
#define RDD_RSV_PHYS_TO_VIRT(_vbase, _pbase, _addr) RDD_PHYS_TO_VIRT(_addr)
#endif

#ifndef __PACKING_ATTRIBUTE_STRUCT_END__
#include "packing.h"
#endif
#include "rdp_runner.h"
#include "rdpa_types.h"

#ifndef RDD_BASIC
#if !defined(FIRMWARE_INIT) || defined(BDMF_SYSTEM_SIM)
#include "bdmf_system.h"
#else
#define bdmf_sysb void *
static inline void *bdmf_sysb_data(const bdmf_sysb sysb)
{
    return ((void *)sysb);
}
#endif
#include "rdpa_ip_class_basic.h"
#include "rdpa_ingress_class_basic.h"
#include "rdpa_cpu_basic.h"
#endif

#if !defined(FIRMWARE_INIT) || defined(BDMF_SYSTEM_SIM)

#include "rdp_drv_bpm.h"

/* temporary until complete stratosphere removal */

#ifndef RDD_BASIC

#undef malloc
/* Full RDD as part of RDPA */
#define malloc(s) bdmf_alloc(s)

#else

#define bdmf_fastlock int
#define DEFINE_BDMF_FASTLOCK(lock) int lock = 0

#if !defined _CFE_

#undef malloc
#undef printf
/* Minimal RDD for builtr-in kernel drivers */
#define malloc(s) kmalloc(s, GFP_KERNEL)
#define printf    printk

#endif /* !defined _CFE_ */

#endif /* RDD_BASIC */

#else

#define bdmf_fastlock int
#define DEFINE_BDMF_FASTLOCK(lock) int lock = 0

#endif /* !defined(FIRMWARE_INIT) */

/* task addresses labels from fw compilation */
#include "rdd_runner_a_labels.h"
#include "rdd_runner_b_labels.h"
#include "rdd_runner_c_labels.h"
#include "rdd_runner_d_labels.h"

#include "rdd_defs.h"
#include "rdd_runner_defs_auto.h"
#include "rdd_runner_defs.h"
#include "rdd_data_structures_auto.h"
#include "rdd_data_structures.h"
#include "rdd_common.h"
#include "rdd_cpu.h"
#include "rdd_init.h"
#ifndef G9991
#include "rdd_wlan_mcast.h"
#endif
#ifndef RDD_BASIC
#include "rdd_router.h"
#include "rdd_bridge.h"
#include "rdd_interworking.h"
#endif
#include "rdd_lookup_engine.h"
#include "rdd_tm.h"

/* priority range according to classification type : acl 128-191, flow 64-127, qos 0-63, ip_flow 192-255  */
#define RDPA_QOS_PRTY_OFFSET 0
#define RDPA_FLOW_PRTY_OFFSET 64
#define RDPA_ACL_PRTY_OFFSET 128
#define RDPA_IP_FLOW_PRTY_OFFSET 192


/* Wan flow header types, which vary for DSL case but not for PON modes */
#define US_WANFLOW_HT_TYPE_NONE                 0 

#endif /* _BL_LILAC_DRV_RUNNER_H */

