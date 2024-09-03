/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

#include <common.h>
#include <linux/ctype.h>
#include <linux/io.h>
#include <spl.h>
#include <asm/arch/misc.h>
#include "asm/arch/ddr.h"
#include "bcm_secure.h"
#include "bcm_otp.h"
#if defined(CONFIG_BCMBCA_ITC_RPC) && !defined(CONFIG_BCMBCA_IKOS)
#include "itc_rpc.h"
#endif
#include "bca_common.h"

DECLARE_GLOBAL_DATA_PTR;

void spl_board_deinit(void);

__weak void arch_cpu_deinit(void)
{

}

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_NONE;
}
__weak void spl_ddrinit_prepare(void)
{
}

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
int reserve_mmu(void)
{
#ifdef CONFIG_BCMBCA_PGTBL_IN_MEMC_SRAM
	bcmbca_enable_memc_sram(CONFIG_SYS_PAGETBL_BASE, CONFIG_SYS_PAGETBL_SIZE);
#endif
	gd->arch.tlb_addr = CONFIG_SYS_PAGETBL_BASE;
	gd->arch.tlb_size = CONFIG_SYS_PAGETBL_SIZE;

	return 0;
}
#endif

extern int sec_tk(void);
extern void sec_tk_find_keystore(void);
void board_init_f(ulong dummy)
{
#if defined(CONFIG_ARCH_CPU_INIT)
	arch_cpu_init();
#endif
#if defined(CONFIG_SYS_ARCH_TIMER)
	timer_init();
#endif
       if (spl_early_init())
                hang();


	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();
	printf(" TK OTP/SOTP \n");

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
	reserve_mmu();
	enable_caches();
#endif
	sec_tk_find_keystore();
}

void spl_board_deinit(void)
{
	/* 
	 * even thought the name says linux but it does everything needed for
	 * boot to the next image: flush and disable cache, disable mmu
	 */
	cleanup_before_linux();

	arch_cpu_deinit();

#ifdef CONFIG_BCMBCA_PGTBL_IN_MEMC_SRAM
	bcmbca_disable_memc_sram();
#endif
}

void spl_board_init(void)
{
#if defined(CONFIG_BCMBCA_ITC_RPC) && !defined(CONFIG_BCMBCA_IKOS)
	rpc_tunnel_init(RPC_TUNNEL_ARM_SMC_NS, true);
	rpc_tunnel_init(RPC_TUNNEL_VFLASH_SMC_NS, true);
	rpc_tunnel_init(RPC_TUNNEL_AVS_SMC_NS, true);
#endif
	bcm_otp_init();
	bcm_sec_init();
	bcm_sec_cb_arg_t cb_args[SEC_CTRL_ARG_MAX] = {0};
	cb_args[SEC_CTRL_ARG_KEY].arg[0].ctrl = SEC_CTRL_KEY_GET;
	bcm_sec_do(SEC_SET, cb_args);
	sec_tk();
	bcm_sec_deinit();
	while(1);
}
