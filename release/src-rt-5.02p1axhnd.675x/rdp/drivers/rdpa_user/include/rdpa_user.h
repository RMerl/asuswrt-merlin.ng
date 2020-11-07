// <:copyright-BRCM:2013:DUAL/GPL:standard
// 
//    Copyright (c) 2013 Broadcom 
//    All Rights Reserved
// 
// Unless you and Broadcom execute a separate written software license
// agreement governing use of this software, this software is licensed
// to you under the terms of the GNU General Public License version 2
// (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
// with the following added to such license:
// 
//    As a special exception, the copyright holders of this software give
//    you permission to link this software with independent modules, and
//    to copy and distribute the resulting executable under terms of your
//    choice, provided that you also meet, for each linked independent
//    module, the terms and conditions of the license of that module.
//    An independent module is a module which is not derived from this
//    software.  The special exception does not apply to any modifications
//    of the software.
// 
// Not withstanding the above, under no circumstances may you combine
// this software in any way with any other Broadcom software provided
// under a license other than the GPL, without Broadcom's express prior
// written consent.
// 
// :>

#ifndef _RDPA_USER_H_
#define _RDPA_USER_H_

#include <bcmtypes.h> /* BCM_IOC_PTR */
#include <bdmf_system.h> /* bdmf_handles */
#include <bdmf_interface.h> /* bdmf_handles */

#define DRIVER_IOCTL 0x11
#define RDPA_USR_DEV_NAME "/dev/rdpa_user"


typedef struct {
    int32_t                         ret;     
    BCM_IOC_PTR(bdmf_type_handle,   drv);    
    BCM_IOC_PTR(bdmf_object_handle, mo);
    BCM_IOC_PTR(bdmf_object_handle, object);
    uint32_t                        cmd;
    bdmf_ptr                        ptr;
    uint64_t                        parm;
    bdmf_ptr                        ai_ptr;
    bdmf_index                      ai;
    uint32_t                        size;
        
} rdpa_ioctl_cmd_t;


typedef struct {
    int32_t                         ret;   
    BCM_IOC_PTR(bdmf_type_handle,   drv);
    BCM_IOC_PTR(bdmf_object_handle, mo_or_us);
    BCM_IOC_PTR(bdmf_object_handle, owner_or_ds);
    BCM_IOC_PTR(bdmf_mattr_handle,  mattr);
    BCM_IOC_PTR(bdmf_link_handle,   link);
    BCM_IOC_PTR(const char *,       str);
} bdmf_ioctl_t;


typedef union {
    bdmf_ioctl_t bdmf_pa;
    rdpa_ioctl_cmd_t rdpa_pa;
} ioctl_pa_t;

#define RDPA_PORT_IOCTL         _IOWR(DRIVER_IOCTL, 100, ioctl_pa_t)
#define RDPA_EGRESS_TM_IOCTL    _IOWR(DRIVER_IOCTL, 101, ioctl_pa_t)
#define RDPA_TCONT_IOCTL        _IOWR(DRIVER_IOCTL, 102, ioctl_pa_t)
#define RDPA_VLAN_IOCTL         _IOWR(DRIVER_IOCTL, 103, ioctl_pa_t)
#define RDPA_VLAN_ACTION_IOCTL  _IOWR(DRIVER_IOCTL, 104, ioctl_pa_t)
#define RDPA_SYSTEM_IOCTL       _IOWR(DRIVER_IOCTL, 105, ioctl_pa_t)
#define RDPA_IPTV_IOCTL         _IOWR(DRIVER_IOCTL, 106, ioctl_pa_t)
#define RDPA_TC_TO_QUEUE_IOCTL  _IOWR(DRIVER_IOCTL, 107, ioctl_pa_t)
#define RDPA_LLID_IOCTL         _IOWR(DRIVER_IOCTL, 108, ioctl_pa_t)
#define RDPA_INGRESS_CLASS_IOCTL  _IOWR(DRIVER_IOCTL, 109, ioctl_pa_t)
#define RDPA_POLICER_IOCTL        _IOWR(DRIVER_IOCTL, 110, ioctl_pa_t)
#define RDPA_PBIT_TO_QUEUE_IOCTL  _IOWR(DRIVER_IOCTL, 111, ioctl_pa_t)
#define RDPA_PBIT_TO_GEM_IOCTL    _IOWR(DRIVER_IOCTL, 112, ioctl_pa_t)
#define RDPA_DSCP_TO_PBIT_IOCTL   _IOWR(DRIVER_IOCTL, 113, ioctl_pa_t)
#define RDPA_UDPSPDTEST_IOCTL   _IOWR(DRIVER_IOCTL, 114, ioctl_pa_t)
#define RDPA_CAPWAP_IOCTL       _IOWR(DRIVER_IOCTL, 115, ioctl_pa_t)
#define RDPA_CAPWAP_REASSEMBLY_IOCTL _IOWR(DRIVER_IOCTL, 116, ioctl_pa_t)
#define RDPA_MLLID_IOCTL        _IOWR(DRIVER_IOCTL, 117, ioctl_pa_t)
#define RDPA_CAPWAP_FRAGMENTATION_IOCTL _IOWR(DRIVER_IOCTL, 118, ioctl_pa_t)

#define PORT_OBJECT 1
#define SYSTEM_OBJECT 1
#define EGRESS_TM_OBJECT 1
#define LLID_OBJECT 1
#define MLLID_OBJECT 1

#if defined(BCM_PON) || defined(CONFIG_BCM_PON)
#define INGRESS_CLASS_OBJECT 1
#define VLAN_OBJECT 1
#define VLAN_ACTION_OBJECT 1
#define IPTV_OBJECT 1
#define TC_TO_QUEUE_OBJECT 1
#define POLICER_OBJECT 1
#define PBIT_TO_QUEUE_OBJECT 1
#define PBIT_TO_GEM_OBJECT 1
#define DSCP_TO_PBIT_OBJECT 1
#endif /* defined(BCM_PON) || defined(CONFIG_BCM_PON)*/

#if defined(BCM_PON) || defined(CONFIG_BCM_PON) || defined(CONFIG_BCM963158)
#define TCONT_OBJECT 1
#endif /* defined(BCM_PON) || defined(CONFIG_BCM_PON) || defined(CONFIG_BCM963158) */

#if defined(CONFIG_BCM_UDPSPDTEST_SUPPORT) || defined(CONFIG_BCM_SPDTEST)
#define UDPSPDTEST_OBJECT 1
#endif

#if defined(CONFIG_BCM_CAPWAP)
#define CAPWAP_OBJECT 1
#endif

#ifndef __KERNEL__ 

#define CC_RDPA_USR_DEBUG 1 

#if defined(CC_RDPA_USR_DEBUG)
#define rdpa_usr_debug(fmt, arg...) printf("%s.%u: " fmt "\n", __FUNCTION__, __LINE__, ##arg)
#else
#define rdpa_usr_debug(fmt, arg...)
#endif

#define rdpa_usr_error(fmt, arg...) printf("ERROR[%s.%u]: " fmt "\n", __FUNCTION__, __LINE__, ##arg)

#endif /* __KERNEL__ */
#endif /* _RDPA_USER_H_ */
