/*
<:copyright-BRCM:2015:GPL/GPL:standard

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
#include <linux/init.h>
#include <linux/string.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/of_fdt.h>
#include <linux/sched.h>
#include <linux/sizes.h>
#include <asm/pgtable.h>
#include <asm/system_misc.h>
#include <board.h>
#include <bcm_rsvmem.h>
#include "pmc_core_api.h"
#if !defined(CONFIG_BCM94908)
#include <bcm_ubus4.h>
#endif
#include <bcm_pinmux.h>

#if defined(CONFIG_BCM_BOOTSTATE)
#include <bcm_bootstate.h>
#endif

#if defined(CONFIG_BCM_GLB_COHERENCY)
extern void bcm_coherency_init(void);
#endif
extern void check_if_rootfs_is_set(char *cmdline);
extern void check_if_ikosboot(char *cmdline);
extern int bcm_early_scan_dt(unsigned long node, const char *uname, int depth, void *data);
#ifdef CONFIG_BCM_STRAP
extern int __init init_strap_register(void);
#endif

static struct notifier_block restart_handler_struct;

static void bcm_sys_restart(enum reboot_mode reboot_mode, const char *cmd)
{
#if !defined(CONFIG_BRCM_IKOS)
    kerSysSoftReset();
#endif
}

static int _reset_restart_handler(struct notifier_block *nb,
    unsigned long reboot_mode, void *data)
{
    bcm_sys_restart(reboot_mode, data);
    return NOTIFY_DONE;
}

#if defined(CONFIG_BCM_BOOTSTATE)
static int reboot_handler(struct notifier_block *self, unsigned long val, void*data)
{

    if((bcmbca_get_boot_reason() & BCM_BOOT_REASON_ACTIVATE) == 0)
    {
        bcmbca_set_boot_reason(0);
    }
    return 0;
}
static struct notifier_block reboot_notifier = {
     .notifier_call = reboot_handler,
};
#endif

static int __init bcm_arch_early_init(void)
{
    memset(&restart_handler_struct, '\0', sizeof(restart_handler_struct));
    restart_handler_struct.notifier_call = _reset_restart_handler;
    restart_handler_struct.priority = 255;
    register_restart_handler(&restart_handler_struct);

#if defined(CONFIG_BCM_BOOTSTATE)
    register_reboot_notifier(&reboot_notifier);
#endif


    memset((void*)bcm_get_blparms(), 0x0, bcm_get_blparms_size());
#ifdef CONFIG_OF
    of_scan_flat_dt(bcm_early_scan_dt, NULL);
#ifdef CONFIG_BCM_STRAP
    init_strap_register();
#endif
#endif
    check_if_rootfs_is_set(boot_command_line);
    check_if_ikosboot(boot_command_line);

#if defined(CONFIG_BCM_GLB_COHERENCY)
    bcm_coherency_init();
#endif

    /* 68360 arch should be modified after PMC support!!! */
#if !defined(CONFIG_BRCM_IKOS)
    kerSysEarlyFlashInit();
    kerSysFlashInit();

#endif

    return 0;
}

early_initcall(bcm_arch_early_init);
