/*
<:copyright-BRCM:2023:DUAL/GPL:standard 

   Copyright (c) 2023 Broadcom 
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

#ifndef PMC_SHUTDOWN_H
#define PMC_SHUTDOWN_H

#if defined(CONFIG_BRCM_SMC_BOOT)
typedef enum {
    WAKE_NONE,
    WAKE_XPORT,
    WAKE_IRQ,
    WAKE_TIMER,
    WAKE_IRQ_WOL,
    WAKE_LAST, //Must be last
} wake_type_t;

int pmc_deep_sleep(void);
int pmc_setup_wake_trig(wake_type_t wake_type, int param);
#else
#define pmc_deep_sleep() (0)
#define pmc_setup_wake_trig(x) (0)
#endif


#endif
