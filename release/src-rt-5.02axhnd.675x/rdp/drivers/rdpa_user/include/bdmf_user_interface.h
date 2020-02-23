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

#ifndef _BDMF_USER_INTERFACE_H_
#define _BDMF_USER_INTERFACE_H_

#include "rdpa_user.h"

#define BDMF_NEW_AND_SET        _IOWR(DRIVER_IOCTL, 0, ioctl_pa_t)
#define BDMF_DESTROY            _IOWR(DRIVER_IOCTL, 1, ioctl_pa_t)
#define BDMF_MATTR_ALLOC        _IOWR(DRIVER_IOCTL, 2, ioctl_pa_t)
#define BDMF_MATTR_FREE         _IOWR(DRIVER_IOCTL, 3, ioctl_pa_t)
#define BDMF_GET                _IOWR(DRIVER_IOCTL, 4, ioctl_pa_t)
#define BDMF_PUT                _IOWR(DRIVER_IOCTL, 5, ioctl_pa_t)
#define BDMF_GET_NEXT           _IOWR(DRIVER_IOCTL, 6, ioctl_pa_t)
#define BDMF_LINK               _IOWR(DRIVER_IOCTL, 7, ioctl_pa_t)
#define BDMF_UNLINK             _IOWR(DRIVER_IOCTL, 8, ioctl_pa_t)
#define BDMF_GET_NEXT_US_LINK   _IOWR(DRIVER_IOCTL, 9, ioctl_pa_t)
#define BDMF_GET_NEXT_DS_LINK   _IOWR(DRIVER_IOCTL, 10, ioctl_pa_t)
#define BDMF_US_LINK_TO_OBJECT  _IOWR(DRIVER_IOCTL, 11, ioctl_pa_t)
#define BDMF_DS_LINK_TO_OBJECT  _IOWR(DRIVER_IOCTL, 12, ioctl_pa_t)
#define BDMF_GET_OWNER          _IOWR(DRIVER_IOCTL, 13, ioctl_pa_t)

#endif /*_BDMF_USER_INTERFACE_H_*/
