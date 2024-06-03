// SPDX-License-Identifier: GPL-2.0+
/*
 * Keystone2: Architecture initialization
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#include <common.h>
#include <ns16550.h>
#include <asm/io.h>
#include <asm/arch/msmc.h>
#include <asm/arch/clock.h>
#include <asm/arch/hardware.h>
#include <asm/arch/psc_defs.h>

#define MAX_PCI_PORTS		2
enum pci_mode	{
	ENDPOINT,
	LEGACY_ENDPOINT,
	ROOTCOMPLEX,
};

#define DEVCFG_MODE_MASK		(BIT(2) | BIT(1))
#define DEVCFG_MODE_SHIFT		1

void chip_configuration_unlock(void)
{
	__raw_writel(KS2_KICK0_MAGIC, KS2_KICK0);
	__raw_writel(KS2_KICK1_MAGIC, KS2_KICK1);
}

#ifdef CONFIG_SOC_K2L
void osr_init(void)
{
	u32 i;
	u32 j;
	u32 val;
	u32 base = KS2_OSR_CFG_BASE;
	u32 ecc_ctrl[KS2_OSR_NUM_RAM_BANKS];

	/* Enable the OSR clock domain */
	psc_enable_module(KS2_LPSC_OSR);

	/* Disable OSR ECC check for all the ram banks */
	for (i = 0; i < KS2_OSR_NUM_RAM_BANKS; i++) {
		val = i | KS2_OSR_ECC_VEC_TRIG_RD |
			(KS2_OSR_ECC_CTRL << KS2_OSR_ECC_VEC_RD_ADDR_SH);

		writel(val , base + KS2_OSR_ECC_VEC);

		/**
		 * wait till read is done.
		 * Print should be added after earlyprintk support is added.
		 */
		for (j = 0; j < 10000; j++) {
			val = readl(base + KS2_OSR_ECC_VEC);
			if (val & KS2_OSR_ECC_VEC_RD_DONE)
				break;
		}

		ecc_ctrl[i] = readl(base + KS2_OSR_ECC_CTRL) ^
						KS2_OSR_ECC_CTRL_CHK;

		writel(ecc_ctrl[i], KS2_MSMC_DATA_BASE + i * 4);
		writel(ecc_ctrl[i], base + KS2_OSR_ECC_CTRL);
	}

	/* Reset OSR memory to all zeros */
	for (i = 0; i < KS2_OSR_SIZE; i += 4)
		writel(0, KS2_OSR_DATA_BASE + i);

	/* Enable OSR ECC check for all the ram banks */
	for (i = 0; i < KS2_OSR_NUM_RAM_BANKS; i++)
		writel(ecc_ctrl[i] |
		       KS2_OSR_ECC_CTRL_CHK, base + KS2_OSR_ECC_CTRL);
}
#endif

/* Function to set up PCIe mode */
static void config_pcie_mode(int pcie_port,  enum pci_mode mode)
{
	u32 val = __raw_readl(KS2_DEVCFG);

	if (pcie_port >= MAX_PCI_PORTS)
		return;

	/**
	 * each pci port has two bits for mode and it starts at
	 * bit 1. So use port number to get the right bit position.
	 */
	pcie_port <<= 1;
	val &= ~(DEVCFG_MODE_MASK << pcie_port);
	val |= ((mode << DEVCFG_MODE_SHIFT) << pcie_port);
	__raw_writel(val, KS2_DEVCFG);
}

static void msmc_k2hkle_common_setup(void)
{
	msmc_share_all_segments(KS2_MSMC_SEGMENT_C6X_0);
	msmc_share_all_segments(K2HKLE_MSMC_SEGMENT_ARM);
	msmc_share_all_segments(K2HKLE_MSMC_SEGMENT_NETCP);
	msmc_share_all_segments(K2HKLE_MSMC_SEGMENT_QM_PDSP);
	msmc_share_all_segments(K2HKLE_MSMC_SEGMENT_PCIE0);
	msmc_share_all_segments(KS2_MSMC_SEGMENT_DEBUG);
}

static void msmc_k2hk_setup(void)
{
	msmc_share_all_segments(KS2_MSMC_SEGMENT_C6X_1);
	msmc_share_all_segments(KS2_MSMC_SEGMENT_C6X_2);
	msmc_share_all_segments(KS2_MSMC_SEGMENT_C6X_3);
	msmc_share_all_segments(KS2_MSMC_SEGMENT_C6X_4);
	msmc_share_all_segments(KS2_MSMC_SEGMENT_C6X_5);
	msmc_share_all_segments(KS2_MSMC_SEGMENT_C6X_6);
	msmc_share_all_segments(KS2_MSMC_SEGMENT_C6X_7);
	msmc_share_all_segments(K2HKE_MSMC_SEGMENT_HYPERLINK);
}

static inline void msmc_k2l_setup(void)
{
	msmc_share_all_segments(KS2_MSMC_SEGMENT_C6X_1);
	msmc_share_all_segments(KS2_MSMC_SEGMENT_C6X_2);
	msmc_share_all_segments(KS2_MSMC_SEGMENT_C6X_3);
	msmc_share_all_segments(K2L_MSMC_SEGMENT_PCIE1);
}

static inline void msmc_k2e_setup(void)
{
	msmc_share_all_segments(K2E_MSMC_SEGMENT_PCIE1);
	msmc_share_all_segments(K2HKE_MSMC_SEGMENT_HYPERLINK);
	msmc_share_all_segments(K2E_MSMC_SEGMENT_TSIP);
}

static void msmc_k2g_setup(void)
{
	msmc_share_all_segments(KS2_MSMC_SEGMENT_C6X_0);
	msmc_share_all_segments(K2G_MSMC_SEGMENT_ARM);
	msmc_share_all_segments(K2G_MSMC_SEGMENT_ICSS0);
	msmc_share_all_segments(K2G_MSMC_SEGMENT_ICSS1);
	msmc_share_all_segments(K2G_MSMC_SEGMENT_NSS);
	msmc_share_all_segments(K2G_MSMC_SEGMENT_PCIE);
	msmc_share_all_segments(K2G_MSMC_SEGMENT_USB);
	msmc_share_all_segments(K2G_MSMC_SEGMENT_MLB);
	msmc_share_all_segments(K2G_MSMC_SEGMENT_PMMC);
	msmc_share_all_segments(K2G_MSMC_SEGMENT_DSS);
	msmc_share_all_segments(K2G_MSMC_SEGMENT_MMC);
	msmc_share_all_segments(KS2_MSMC_SEGMENT_DEBUG);
}

int arch_cpu_init(void)
{
	chip_configuration_unlock();
	icache_enable();

	if (cpu_is_k2g()) {
		msmc_k2g_setup();
	} else {
		msmc_k2hkle_common_setup();
		if (cpu_is_k2e())
			msmc_k2e_setup();
		else if (cpu_is_k2l())
			msmc_k2l_setup();
		else
			msmc_k2hk_setup();
	}

	/* Initialize the PCIe-0 to work as Root Complex */
	config_pcie_mode(0, ROOTCOMPLEX);
#if defined(CONFIG_SOC_K2E) || defined(CONFIG_SOC_K2L)
	/* Initialize the PCIe-1 to work as Root Complex */
	config_pcie_mode(1, ROOTCOMPLEX);
#endif
#ifdef CONFIG_SOC_K2L
	osr_init();
#endif

	/*
	 * just initialise the COM2 port so that TI specific
	 * UART register PWREMU_MGMT is initialized. Linux UART
	 * driver doesn't handle this.
	 */
#ifndef CONFIG_DM_SERIAL
	NS16550_init((NS16550_t)(CONFIG_SYS_NS16550_COM2),
		     CONFIG_SYS_NS16550_CLK / 16 / CONFIG_BAUDRATE);
#endif

	return 0;
}

void reset_cpu(ulong addr)
{
	volatile u32 *rstctrl = (volatile u32 *)(KS2_RSTCTRL);
	u32 tmp;

	tmp = *rstctrl & KS2_RSTCTRL_MASK;
	*rstctrl = tmp | KS2_RSTCTRL_KEY;

	*rstctrl &= KS2_RSTCTRL_SWRST;

	for (;;)
		;
}

void enable_caches(void)
{
#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
#endif
}

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	u16 cpu = get_part_number();
	u8 rev = cpu_revision();

	puts("CPU: ");
	switch (cpu) {
	case CPU_66AK2Hx:
		puts("66AK2Hx SR");
		break;
	case CPU_66AK2Lx:
		puts("66AK2Lx SR");
		break;
	case CPU_66AK2Ex:
		puts("66AK2Ex SR");
		break;
	case CPU_66AK2Gx:
		puts("66AK2Gx");
#ifdef CONFIG_SOC_K2G
		{
			int speed = get_max_arm_speed(speeds);
			if (speed == SPD1000)
				puts("-100 ");
			else if (speed == SPD600)
				puts("-60 ");
			else
				puts("-xx ");
		}
#endif
		puts("SR");
		break;
	default:
		puts("Unknown\n");
	}

	if (rev == 2)
		puts("2.0\n");
	else if (rev == 1)
		puts("1.1\n");
	else if (rev == 0)
		puts("1.0\n");
	else if (rev == 8)
		puts("1.0\n");
	return 0;
}
#endif
