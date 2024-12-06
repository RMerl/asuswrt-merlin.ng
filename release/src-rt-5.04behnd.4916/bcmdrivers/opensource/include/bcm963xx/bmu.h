/*
<:copyright-BRCM:2021:DUAL/GPL:standard 

   Copyright (c) 2021 Broadcom 
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
