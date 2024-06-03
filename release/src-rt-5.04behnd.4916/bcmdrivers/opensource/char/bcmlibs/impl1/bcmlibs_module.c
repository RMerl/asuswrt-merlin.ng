/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
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
 *******************************************************************************
 * File Name  : bcmlibs_module.c
 *
 *******************************************************************************
 */

#include <linux/module.h>
#include <bcm_pktfwd.h>

#define BCMLIBS_VERSION              "0.1"
#define BCMLIBS_VER_STR              "v" BCMLIBS_VERSION
#define BCMLIBS_MODDES              "Broadcom Library of Utilities (bcmlibs)"

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

