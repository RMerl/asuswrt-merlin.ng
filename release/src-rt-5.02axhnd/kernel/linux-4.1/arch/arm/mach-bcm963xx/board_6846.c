#if defined(CONFIG_BCM_KF_ARM_BCM963XX)
/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
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

#include <linux/of.h>
#include <linux/of_platform.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>

static const char * const bcm63xx_dt_compat[] = {
        "brcm,bcm96846",
        NULL
};

MACHINE_START(BCM96846, "BCM96846")
    .dt_compat      = bcm63xx_dt_compat,
MACHINE_END

#endif //CONFIG_BCM_KF_ARM_BCM963XX
