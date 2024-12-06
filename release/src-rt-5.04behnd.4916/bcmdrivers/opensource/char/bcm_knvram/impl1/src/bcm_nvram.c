/*
<:copyright-BRCM:2020:DUAL/GPL:standard

   Copyright (c) 2020 Broadcom 
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
