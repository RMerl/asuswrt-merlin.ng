#ifndef __UAPI_BCM_ATMDEV_H__
#define __UAPI_BCM_ATMDEV_H__
/*
<:copyright-BRCM:2019:DUAL/GPL:standard 

   Copyright (c) 2019 Broadcom 
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

//#define ATM_EXTBACKENDIF        _IOW('a',ATMIOC_SPECIAL+6,atm_backend_t)
//#define ATM_SETEXTFILT          _IOW('a',ATMIOC_SPECIAL+7,atm_backend_t)

#define ATM_BACKEND_RT2684             3  /* Routed RFC1483/2684 */
#define ATM_BACKEND_BR2684_BCM         4  /* Bridged RFC1483/2684 uses Broadcom ATMAPI*/
#define ATM_BACKEND_PPP_BCM            5  /* PPPoA uses Broadcom bcmxtmrt driver */
#define ATM_BACKEND_PPP_BCM_DISCONN    6  /* PPPoA LCP disconnect */
#define ATM_BACKEND_PPP_BCM_CLOSE_DEV  7  /* PPPoA close device */

#endif  //__UAPI_BCM_ATMDEV_H__
