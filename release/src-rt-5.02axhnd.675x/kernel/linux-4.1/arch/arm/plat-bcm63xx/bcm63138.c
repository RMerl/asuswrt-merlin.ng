#if defined(CONFIG_BCM_KF_ARM_BCM963XX)
/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
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
 * BCM63138 SoC main platform file.
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/clkdev.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/stop_machine.h>
#include <linux/bug.h>
#ifdef CONFIG_PLAT_BCM63XX_AMBA_PL011
#include <linux/amba/bus.h>
#include <linux/amba/serial.h>
#endif
#ifdef CONFIG_PLAT_BCM63XX_AMBA_PL081
#include <linux/amba/bus.h>
#include <linux/amba/pl08x.h>
#include <asm/hardware/pl080.h>
#include <linux/dmaengine.h>
#endif
#include <asm/hardware/cache-l2x0.h>
#include <asm/mach/map.h>
#include <asm/cp15.h>
#include <asm/system_misc.h>
#include <mach/hardware.h>
#include <mach/smp.h>
#include <plat/bsp.h>
#include <plat/ca9mpcore.h>
#include <bcm_map_part.h>
#include <bcm_intr.h>
#include <pmc_drv.h>
#include <pmc_cpu_core.h>
#include <pmc_neon.h>
#ifdef CONFIG_PLAT_BCM63XX_ACP
#include <mach/memory.h>
#endif

extern unsigned long getMemorySize(void);

#ifdef CONFIG_PLAT_BCM63XX_AMBA_PL081
static struct clk pl081dmac_clk = {
	.name = "pl081dmac",
	.rate = FREQ_MHZ(50),
	.fixed = 1,
	.type = CLK_DMAC,
};
#endif

#define IO_DESC(pa, sz) { \
		.virtual = IO_ADDRESS(pa), \
		.pfn = __phys_to_pfn(pa), \
		.length = sz, \
		.type = MT_DEVICE, \
	}

#define MEM_DESC(pa, sz) { \
		.virtual = IO_ADDRESS(pa), \
		.pfn = __phys_to_pfn(pa), \
		.length = sz, \
		.type = MT_MEMORY_RWX_NONCACHED, \
	}
#ifdef CONFIG_PLAT_BCM63XX_ACP
#define ACP_MEM_DESC(pa, sz) { \
		.virtual = ACP_ADDRESS(pa), \
		.pfn = __phys_to_pfn(pa), \
		.length = sz, \
		.type = MT_MEMORY_NONSECURED, \
	}
#endif


static struct map_desc bcm63138_io_desc[] __initdata = {
	IO_DESC(USB_CTL_PHYS_BASE, SZ_4K),
	IO_DESC(MEMC_PHYS_BASE, SZ_4K),
	IO_DESC(DDRPHY_PHYS_BASE, SZ_4K),
	IO_DESC(SAR_PHYS_BASE, SZ_16K),
	IO_DESC(SATA_PHYS_BASE, SZ_16K),
	IO_DESC(USBH_PHYS_BASE, SZ_8K),
	IO_DESC(ERROR_PORT_PHYS_BASE, SZ_4K),
	IO_DESC(AIP_PHYS_BASE, SZ_4K),
#ifdef CONFIG_PLAT_BCM63XX_AMBA_PL011
	IO_DESC(ARM_UART_PHYS_BASE, SZ_4K),
#endif
	IO_DESC(L2C_PHYS_BASE, SZ_4K),
	IO_DESC(ARMCFG_PHYS_BASE, SZ_4K),
	IO_DESC(DECT_PHYS_BASE, SZ_128K),
	IO_DESC(SWITCH_PHYS_BASE, SZ_512K),
	IO_DESC(APM_PHYS_BASE, SZ_128K),
	IO_DESC(RDP_PHYS_BASE, SZ_1M),
	IO_DESC(PMC_PHYS_BASE, SZ_512K),
	IO_DESC(PROC_MON_PHYS_BASE, SZ_4K),
	IO_DESC(DSLPHY_PHYS_BASE, SZ_1M),
	IO_DESC(DSLLMEM_PHYS_BASE, SZ_1M),
	IO_DESC(PERF_PHYS_BASE, SZ_32K),
	IO_DESC(BOOTLUT_PHYS_BASE, SZ_4K),
	IO_DESC(SPIFLASH_PHYS_BASE, SZ_128K),
	IO_DESC(NANDFLASH_PHYS_BASE, SZ_128K),
};

/* any fixup that has to be performed in the early stage of
 * kernel booting */
void __init soc_fixup(void)
{
	ca9mp_fixup();
}

/*
 * Map fix-mapped I/O that is needed before full MMU operation
 */
void __init soc_map_io(void)
{
#ifdef CONFIG_PLAT_BCM63XX_ACP
	struct map_desc acp_desc;
#endif
	ca9mp_map_io();

	iotable_init(bcm63138_io_desc, ARRAY_SIZE(bcm63138_io_desc));

#ifdef CONFIG_PLAT_BCM63XX_ACP
	acp_desc.virtual = ACP_ADDRESS(PLAT_PHYS_OFFSET);
	acp_desc.pfn = __phys_to_pfn(PLAT_PHYS_OFFSET);
	acp_desc.length = getMemorySize();
	acp_desc.type = MT_MEMORY_NONSECURED;
	iotable_init(&acp_desc, 1);
#endif
}

#if 0
static int soc_abort_handler(unsigned long addr, unsigned int fsr,
		struct pt_regs *regs)
{
	/*
	 * These happen for no good reason
	 * possibly left over from CFE
	 */
	printk(KERN_WARNING "External imprecise Data abort at "
			"addr=%#lx, fsr=%#x ignored.\n", addr, fsr);

	/* Returning non-zero causes fault display and panic */
	return 0;
}
#endif

static void soc_aborts_enable(void)
{
#if 0
	u32 x;

	/* Install our hook */
	hook_fault_code(16 + 6, soc_abort_handler, SIGBUS, 0,
			"imprecise external abort");

	/* Enable external aborts - clear "A" bit in CPSR */

	/* Read CPSR */
	asm( "mrs	%0,cpsr": "=&r" (x) : : );

	x &= ~ PSR_A_BIT;

	/* Update CPSR, affect bits 8-15 */
	asm( "msr	cpsr_x,%0; nop; nop": : "r" (x) : "cc" );
#endif
}

/*
 * This SoC relies on MPCORE GIC interrupt controller
 */
void __init soc_init_irq(void)
{
	ca9mp_init_gic();
	soc_aborts_enable();
}

#if !defined (CONFIG_OF)
#ifdef CONFIG_CACHE_L2X0
/*
 * SoC initialization that need to be done early,
 * e.g. L2 cache, clock, I/O pin mux, power management
 */
void  __init soc_l2_cache_init(void)
{
	u32 auxctl_val = 0, auxctl_msk = ~0UL;

	auxctl_val = BCM_L2C_AUX_VAL;
	auxctl_msk = BCM_L2C_AUX_MSK;

	/* Configure using default aux control value */
	l2x0_init(__io_address(L2C_PHYS_BASE), auxctl_val, auxctl_msk);

	return;
}
#endif
#endif

void __init soc_init_early(void)
{
	ca9mp_init_early();

	pmc_init();

	/* TODO: can we use the following for reserving DMA memory for ACP?
	 * Will it maintain the same region of memory all the time? */
	//init_consistent_dma_size(SZ_128M);
}

/*
 * Install all other SoC device drivers
 * that are not automatically discoverable.
 */

 
#ifdef CONFIG_PLAT_BCM63XX_AMBA_PL081
struct pl08x_channel_data pl081_dmac0_channeldata [] = 
{
	/* HS_UART HOFIFO Channel */
	{
		.bus_id          = PL081_DMA_CHAN_HS_UART_RX,
		.min_signal      = 0,
		.max_signal      = 0,
		.muxval          = 0,
		.circular_buffer = false,
		.single          = false,
		.periph_buses    = PL08X_AHB1,
	},
	
	/* HS_UART HIFIFO Channel */
	{
		.bus_id          = PL081_DMA_CHAN_HS_UART_TX,
		.min_signal      = 1,
		.max_signal      = 1,
		.muxval          = 0,
		.circular_buffer = false,
		.single          = false,
		.periph_buses    = PL08X_AHB1,
	}	
};

struct pl08x_channel_data pl081_dmac0_memcp_chdata [] = 
{
	{
		.bus_id          = "DMA_MTOM",
		.min_signal      = 2,
		.max_signal      = 2,
		.muxval          = 0,
		.circular_buffer = false,
		.single          = false,
		.periph_buses    = PL08X_AHB1,
	},
};

struct pl08x_platform_data pl081dmac0_pdata;
static AMBA_AHB_DEVICE(pl081dmac0, "pl08xdmac.0", 0x00041081, PL081_DMA_PHYS_BASE, {INTERRUPT_ID_PL081}, &pl081dmac0_pdata);
#endif

#ifdef CONFIG_PLAT_BCM63XX_AMBA_PL011
static AMBA_APB_DEVICE(uart0, "uart0", 0, ARM_UART_PHYS_BASE, { INTERRUPT_ID_UART2 }, NULL);
static struct amba_device *amba_devs[] __initdata = {
	&uart0_device,
};
#endif

#ifdef CONFIG_PLAT_BCM63XX_AMBA_PL081
/*
 * Get PL081 periperal DMA request signal number
 * PL081 has 16 DMA request signals. This function
 * returns thhe DMA request signal number associated
 * with the specified dma channel
 */
static int get_signal(struct pl08x_dma_chan * dma_chan)
{
	int signal = -1;
	
	/* Just return min_signal as dma request lines are not muxed */
	if( dma_chan && dma_chan->cd )
		signal = dma_chan->cd->min_signal;

	return signal;		
}

/*
 * Release PL081 periperal DMA request signal number
 */
static void put_signal(struct pl08x_dma_chan * dma_chan)
{
	/* Do nothing as dma request lines are not muxed */	
}
#endif

#ifdef CONFIG_PLAT_BCM63XX_EMMC  /* Arasan emmc SD */
static struct resource bcm63xx_emmc_resources[] = {
	[0] = {
				.start = EMMC_HOSTIF_PHYS_BASE,
				.end = EMMC_HOSTIF_PHYS_BASE + SZ_256 - 1,  /* we only need this area */
				/* the memory map actually makes SZ_4K available  */
				.flags = IORESOURCE_MEM,
			},
	[1] =	{
				.start = INTERRUPT_ID_EMMC,
				.end = INTERRUPT_ID_EMMC,
				.flags = IORESOURCE_IRQ,
			},
};

static u64 bcm63xx_emmc_dmamask = 0xffffffffUL;

struct platform_device bcm63xx_emmc_device = {
	.name = "sdhci-bcm63xx",
	.id = 0,
	.num_resources = ARRAY_SIZE(bcm63xx_emmc_resources),
	.resource = bcm63xx_emmc_resources,
	.dev = {
		.dma_mask = &bcm63xx_emmc_dmamask,
		.coherent_dma_mask = 0xffffffffUL},
};
#endif

void __init soc_add_devices(void)
{
#ifdef CONFIG_PLAT_BCM63XX_AMBA_PL011
	{
		int i;

		/* init uart (AMBA device) here */
		for (i = 0; i < ARRAY_SIZE(amba_devs); i++) {
			struct amba_device *d = amba_devs[i];
			int ret;

			ret = amba_device_register(d, &iomem_resource);
			if (ret)
				printk("%s:%d:amba device[%d] registered failed, err = %d",
					   __func__, __LINE__, i, ret);
		}
	}
#endif

#ifdef CONFIG_PLAT_BCM63XX_AMBA_PL081
	{
		pl081dmac0_pdata.slave_channels = &pl081_dmac0_channeldata[0];
		pl081dmac0_pdata.num_slave_channels = 2;
// 		pl081dmac0_pdata.memcpy_channel = NULL;
		pl081dmac0_pdata.get_signal = get_signal;
		pl081dmac0_pdata.put_signal = put_signal;
		pl081dmac0_pdata.lli_buses = PL08X_AHB1;
		pl081dmac0_pdata.mem_buses = PL08X_AHB1;
				
		/* Register AMBA device */
		amba_device_register(&pl081dmac0_device, &iomem_resource);			
	}
#endif

#ifdef CONFIG_PLAT_BCM63XX_EMMC
	{
		/* Only register EMMC device if NAND i/f is NOT active */
		if ( MISC->miscStrapBus & MISC_STRAP_BUS_SW_BOOT_SPI_SPINAND_EMMC_MASK ) {
			platform_device_register(&bcm63xx_emmc_device);
		}
	}
#endif
}

static void __init neon_enable(void *data)
{
	u32 access;
	(void)data;

	access = get_copro_access();

	/*
	 * Enable full access to VFP (cp10 and cp11)
	 */
	set_copro_access(access | CPACC_FULL(10) | CPACC_FULL(11));

	/* mov r0, 0x40000000; vmsr fpexc, r0 */
	asm volatile ("mov r0, #0x40000000; .word 0xeee80a10" : : : "r0" );
}

static int __init bcm63138_neon_fixup(void)
{
	pmc_neon_power_up();
	smp_call_function_single(0, neon_enable, NULL, 1);
	return 0;
}
late_initcall(bcm63138_neon_fixup);

#endif /* CONFIG_BCM_KF_ARM_BCM963XX */
