/*
<:copyright-BRCM:2020:DUAL/GPL:standard

   Copyright (c) 2020 Broadcom 
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

#ifndef __WL_COMMON_DEFS_H__
#define __WL_COMMON_DEFS_H__

/* "WL_DEFAULT_NUM_SSID" is decided by "BRCM_DEFAULT_NUM_MBSS" defined in profile */
#if !defined WL_DEFAULT_NUM_SSID
#error WL_DEFAULT_NUM_SSID is not defined!!!!!
#endif

#define WL_MAX_NUM_RADIO  (4)

#define WL_MAX_NUM_SSID (WL_DEFAULT_NUM_SSID)

#endif

