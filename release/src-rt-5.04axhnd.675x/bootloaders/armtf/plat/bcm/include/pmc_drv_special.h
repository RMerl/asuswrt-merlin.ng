/*
<:copyright-BRCM:2019:DUAL/GPL:standard 

   Copyright (c) 2019 Broadcom 
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

/*****************************************************************************
 *  Description:
 *      This contains special header for different flavors of PMC drivers.
 *****************************************************************************/

#ifndef PMC_DRV_SPECIAL_H__
#define PMC_DRV_SPECIAL_H__

#if defined(__KERNEL__) || defined(_ATF_)
#define CFG_RAMAPP
#endif

#include "bcm_map_part.h"
#include "bcm_mem_reserve.h"

#ifndef IS_BCMCHIP
#define IS_BCMCHIP(num) (defined(_BCM9##num##_) || \
		defined(CONFIG_BCM9##num) || defined(CONFIG_BCM##num))
#endif

#if IS_BCMCHIP(63138)
#include "pmc_addr_63138.h"
#elif IS_BCMCHIP(63148)
#include "pmc_addr_63148.h"
#elif IS_BCMCHIP(6858)
#include "pmc_addr_6858.h"
#elif IS_BCMCHIP(6846)
#include "pmc_addr_6846.h"
#elif IS_BCMCHIP(6878)
#include "pmc_addr_6878.h"
#elif IS_BCMCHIP(4908)
#include "pmc_addr_4908.h"
#elif IS_BCMCHIP(63158)
#include "pmc_addr_63158.h"
#elif IS_BCMCHIP(63178)
#include "pmc_addr_63178.h"
#elif IS_BCMCHIP(47622)
#include "pmc_addr_47622.h"
#elif IS_BCMCHIP(6856)
#include "pmc_addr_6856.h"
#elif IS_BCMCHIP(63146)
#include "pmc_addr_63146.h"
#elif IS_BCMCHIP(4912)
#include "pmc_addr_4912.h"
#elif IS_BCMCHIP(6813)
#include "pmc_addr_6813.h"
#elif IS_BCMCHIP(6855)
#include "pmc_addr_6855.h"
#elif IS_BCMCHIP(6756)
#include "pmc_addr_6756.h"
#endif

#ifndef __ASSEMBLER__
#if defined(_CFE_) && !defined(_ATF_)
#include "lib_printf.h"
#include "lib_types.h"
#include "lib_string.h"
#include "cfe_iocb.h"
#include "bsp_config.h"
#include "bcm63xx_util.h"
#include "shared_utils.h"

#define MAX_PMC_ROM_SIZE    0x8000
#define MAX_PMC_LOG_SIZE    0x8000

#if defined(_BCM963158_) && !defined(CONFIG_BRCM_IKOS)
#define PMC_SHARED_MEMORY   0x80204000

#if MAX_PMC_ROM_SIZE + MAX_PMC_LOG_SIZE > CFG_BOOT_PMC_SIZE
#error ROM and LOG buffer size needs to be re-adjusted
#endif
#endif /* _BCM963158_ */

extern void _cfe_flushcache(int, uint8_t *, uint8_t *);
extern int getAVSConfig(void);
#define  is_pmcfw_code_loaded(void) (0)
#define  is_pmcfw_data_loaded(void) (0)
#endif /* _CFE_ */

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/delay.h>
void pmc_spin_lock(void);
void pmc_spin_unlock(void);
#if defined(CONFIG_BCM96878) || defined(CONFIG_BCM96855)
void keyhole_spin_lock(void);
void keyhole_spin_unlock(void);
#endif
#else // #ifdef __KERNEL__
#ifndef EXPORT_SYMBOL
#define EXPORT_SYMBOL(a)
#endif
#define pmc_spin_lock(...) do { } while (0)
#define pmc_spin_unlock(...) do { } while (0)
#if defined(CONFIG_BCM96878) || defined(CONFIG_BCM96855)
#define keyhole_spin_lock(...) do { } while (0)
#define keyhole_spin_unlock(...) do { } while (0)
#endif
#define printk	printf

#ifndef phys_to_virt
#define phys_to_virt(a) (a)
#endif
#endif // #ifdef __KERNEL__
#endif // #ifndef __ASSEMBLER__

#endif // #ifndef PMC_DRV_SPECIAL_H__
