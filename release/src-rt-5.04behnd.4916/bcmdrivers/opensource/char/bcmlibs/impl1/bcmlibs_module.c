/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
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
 *******************************************************************************
 * File Name  : bcmlibs_module.c
 *
 *******************************************************************************
 */

#include <linux/module.h>
#include <linux/of_address.h>
#include <bcm_pktfwd.h>

#define BCMLIBS_VERSION              "0.1"
#define BCMLIBS_VER_STR              "v" BCMLIBS_VERSION
#define BCMLIBS_MODDES              "Broadcom Library of Utilities (bcmlibs)"

int bcm_of_address_to_resource(struct device_node *dev, int index,
                struct resource *r)
{
    return of_address_to_resource(dev, index, r);
}
EXPORT_SYMBOL(bcm_of_address_to_resource);

static int __init bcmlibs_module_init( void )
{
    printk("BCMLIBS loaded...\n");

#if defined(BCM_PKTFWD)
    /* Instantiate the singleton bcm_pktfwd global */
    bcm_pktfwd_sys_init();
#endif /* BCM_PKTFWD */

    return 0;
}
static void __exit bcmlibs_module_exit( void )
{
    printk("BCMLIBS unloaded...\n");

#if defined(BCM_PKTFWD)
    /* Destruct the singleton bcm_pktfwd global */
    bcm_pktfwd_sys_fini();
#endif /* BCM_PKTFWD */

    return;
}

module_init( bcmlibs_module_init );
module_exit(bcmlibs_module_exit);


MODULE_DESCRIPTION(BCMLIBS_MODDES);
MODULE_VERSION(BCMLIBS_VER_STR);
MODULE_LICENSE("GPL");

