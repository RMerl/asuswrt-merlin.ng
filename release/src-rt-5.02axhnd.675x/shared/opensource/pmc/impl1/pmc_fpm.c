/*
<:copyright-BRCM:2015:DUAL/GPL:standard

   Copyright (c) 2015 Broadcom 
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
#ifndef _CFE_
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#endif

#include "pmc_drv.h"
#include "pmc_fpm.h"
#include "BPCM.h"

int pmc_fpm_power_up(void)
{
	return PowerOnDevice(PMB_ADDR_FPM);
}

int pmc_fpm_power_down(void)
{
	return PowerOffDevice(PMB_ADDR_FPM, 0);
}

#ifndef _CFE_
EXPORT_SYMBOL(pmc_fpm_power_up);
EXPORT_SYMBOL(pmc_fpm_power_down);
#endif

