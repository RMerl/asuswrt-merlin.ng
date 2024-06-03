#ifndef __UAPI_BCM_ATMDEV_H__
#define __UAPI_BCM_ATMDEV_H__
/*
<:copyright-BRCM:2019:DUAL/GPL:standard 

   Copyright (c) 2019 Broadcom 
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

//#define ATM_EXTBACKENDIF        _IOW('a',ATMIOC_SPECIAL+6,atm_backend_t)
//#define ATM_SETEXTFILT          _IOW('a',ATMIOC_SPECIAL+7,atm_backend_t)

#define ATM_BACKEND_RT2684             3  /* Routed RFC1483/2684 */
#define ATM_BACKEND_BR2684_BCM         4  /* Bridged RFC1483/2684 uses Broadcom ATMAPI*/
#define ATM_BACKEND_PPP_BCM            5  /* PPPoA uses Broadcom bcmxtmrt driver */
#define ATM_BACKEND_PPP_BCM_DISCONN    6  /* PPPoA LCP disconnect */
#define ATM_BACKEND_PPP_BCM_CLOSE_DEV  7  /* PPPoA close device */

#endif  //__UAPI_BCM_ATMDEV_H__
