// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Broadcom
 */
/*
    
*/

#ifndef _BL_LILAC_DRV_RUNNER_H
#define _BL_LILAC_DRV_RUNNER_H

#include "bl_os_wraper.h"
#include "access_macros.h"


#if defined(__UBOOT__)
#include "bcm_mm.h"
#define RDP_CPU_RING_MAX_QUEUES         1
#define RDP_WLAN_MAX_QUEUES             0

#elif defined(__KERNEL__)
#include "bcm_mm.h"
#define PHYS_TO_VIRT(_addr)             PHYS_TO_CACHED(_addr)
#define RDP_CPU_RING_MAX_QUEUES         RDPA_CPU_MAX_QUEUES
#define RDP_WLAN_MAX_QUEUES             RDPA_WLAN_MAX_QUEUES

#else
#define VIRT_TO_PHYS(_addr)             ((uint32_t)_addr)
#define PHYS_TO_VIRT(_addr)             (_addr)
#endif

#define RDD_VIRT_TO_PHYS(_addr) VIRT_TO_PHYS(_addr)
#define RDD_PHYS_TO_VIRT(_addr) PHYS_TO_VIRT(_addr)

#if defined(__UBOOT__)
#define RDD_RSV_VIRT_TO_PHYS(_vbase, _pbase, _addr) RDD_VIRT_TO_PHYS(_addr)
#define RDD_RSV_PHYS_TO_VIRT(_vbase, _pbase, _addr) RDD_PHYS_TO_VIRT(_addr)
#elif defined(__KERNEL__)
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

#include "rdp_drv_bpm.h"

/* temporary until complete stratosphere removal */
#define bdmf_fastlock int
#define DEFINE_BDMF_FASTLOCK(lock) int lock = 0
#define bdmf_fastlock_lock(_lock)       (*_lock = 0)
#define bdmf_fastlock_unlock(_lock)     (*_lock = 0)
#define bdmf_fastlock_lock_irq(_lock, _flag)   (_flag = *_lock)
#define bdmf_fastlock_unlock_irq(_lock, _flag) (*_lock = _flag)

#if !defined(BDMF_SYSTEM_SIM)
#define rdd_print(fmt, args...) printk(fmt, ##args)
#else
#define rdd_print(fmt, args...) printf(fmt, ##args)
#endif

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
#include "rdd_lookup_engine.h"
#include "rdd_tm.h"
#include "rdd_platform.h"

/* priority range according to classification type : acl 128-191, flow 64-127, qos 0-63, ip_flow 192-255  */
#define RDPA_QOS_PRTY_OFFSET 0
#define RDPA_FLOW_PRTY_OFFSET 64
#define RDPA_ACL_PRTY_OFFSET 128
#define RDPA_IP_FLOW_PRTY_OFFSET 192

#endif /* _BL_LILAC_DRV_RUNNER_H */
