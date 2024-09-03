/*
<:copyright-BRCM:2021:DUAL/GPL:standard 

   Copyright (c) 2021 Broadcom 
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

/*
 * The purpose of this interface is to allow modules to register a callback function
 * with the power management module. The power management module handles an
 * interrupt from the Battery Management Unit (BMU) which triggers each time
 * the system goes from Utility power to Battery power. The purpose of the
 * registering functions should be to power down certain interfaces immediately
 * to avoid exceeding the battery instantaneous current limits.
 * If the battery is capable of sustaining the system load for a long enough time,
 * then there is no need to register with the interrupt handler and instead
 * all the work can be done in userspace through the bmud application.
 * The interrupt does not trigger when going from battery power back to utility
 * power. This can be handled in userspace in collaboration with the bmud application.
*/
#ifndef BMU_H_INCLUDED__
#define BMU_H_INCLUDED__

#define DEVNAMSIZ  16

typedef void (*cb_bmu_t)(void *arg);

int bcm_bmu_is_battery_enabled(void);
void bcm_bmu_register_handler(char *devname, void *cbfn, void *context);
void bcm_bmu_deregister_handler(char *devname);

#endif
