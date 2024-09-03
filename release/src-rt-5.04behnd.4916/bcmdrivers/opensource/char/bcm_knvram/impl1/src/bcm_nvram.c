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

/* kernel nvram API implementation file.
   Currently it is implemented as wrapper on top of wlcsm module.
   Plan to seperate out in future
*/
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>

#include <wlcsm_linux.h>
#include <bcm_nvram.h>

/* Search the given kernel nvram parameter (name) and returns the value */
char *nvram_k_get(char *name) {
	return wlcsm_nvram_k_get(name);
}
EXPORT_SYMBOL(nvram_k_get);

/* Sets the given value to the kernel nvram parameter (name) */
/* Returns 0 on success, else -ve value on failure */
int nvram_k_set(char *name, char *value) {
	return wlcsm_nvram_k_set(name, value);
}
EXPORT_SYMBOL(nvram_k_set);
