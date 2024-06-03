// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#include <common.h>
#include <netdev.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <mvebu_mmc.h>

void reset_cpu(unsigned long ignored)
{
	struct kwcpu_registers *cpureg =
	    (struct kwcpu_registers *)KW_CPU_REG_BASE;

	writel(readl(&cpureg->rstoutn_mask) | (1 << 2),
		&cpureg->rstoutn_mask);
	writel(readl(&cpureg->sys_soft_rst) | 1,
		&cpureg->sys_soft_rst);
	while (1) ;
}

/*
 * Window Size
 * Used with the Base register to set the address window size and location.
 * Must be programmed from LSB to MSB as sequence of ones followed by
 * sequence of zeros. The number of ones specifies the size of the window in
 * 64 KByte granularity (e.g., a value of 0x00FF specifies 256 = 16 MByte).
 * NOTE: A value of 0x0 specifies 64-KByte size.
 */
unsigned int kw_winctrl_calcsize(unsigned int sizeval)
{
	int i;
	unsigned int j = 0;
	u32 val = sizeval >> 1;

	for (i = 0; val >= 0x10000; i++) {
		j |= (1 << i);
		val = val >> 1;
	}
	return (0x0000ffff & j);
}

static struct mbus_win windows[] = {
	/* Window 0: PCIE MEM address space */
	{ KW_DEFADR_PCI_MEM, 1024 * 1024 * 256,
	  KWCPU_TARGET_PCIE, KWCPU_ATTR_PCIE_MEM },

	/* Window 1: PCIE IO address space */
	{ KW_DEFADR_PCI_IO, 1024 * 64,
	  KWCPU_TARGET_PCIE, KWCPU_ATTR_PCIE_IO },

	/* Window 2: NAND Flash address space */
	{ KW_DEFADR_NANDF, 1024 * 1024 * 128,
	  KWCPU_TARGET_MEMORY, KWCPU_ATTR_NANDFLASH },

	/* Window 3: SPI Flash address space */
	{ KW_DEFADR_SPIF, 1024 * 1024 * 128,
	  KWCPU_TARGET_MEMORY, KWCPU_ATTR_SPIFLASH },

	/* Window 4: BOOT Memory address space */
	{ KW_DEFADR_BOOTROM, 1024 * 1024 * 128,
	  KWCPU_TARGET_MEMORY, KWCPU_ATTR_BOOTROM },

	/* Window 5: Security SRAM address space */
	{ KW_DEFADR_SASRAM, 1024 * 64,
	  KWCPU_TARGET_SASRAM, KWCPU_ATTR_SASRAM },
};

/*
 * SYSRSTn Duration Counter Support
 *
 * Kirkwood SoC implements a hardware-based SYSRSTn duration counter.
 * When SYSRSTn is asserted low, a SYSRSTn duration counter is running.
 * The SYSRSTn duration counter is useful for implementing a manufacturer
 * or factory reset. Upon a long reset assertion that is greater than a
 * pre-configured environment variable value for sysrstdelay,
 * The counter value is stored in the SYSRSTn Length Counter Register
 * The counter is based on the 25-MHz reference clock (40ns)
 * It is a 29-bit counter, yielding a maximum counting duration of
 * 2^29/25 MHz (21.4 seconds). When the counter reach its maximum value,
 * it remains at this value until counter reset is triggered by setting
 * bit 31 of KW_REG_SYSRST_CNT
 */
static void kw_sysrst_action(void)
{
	int ret;
	char *s = env_get("sysrstcmd");

	if (!s) {
		debug("Error.. %s failed, check sysrstcmd\n",
			__FUNCTION__);
		return;
	}

	debug("Starting %s process...\n", __FUNCTION__);
	ret = run_command(s, 0);
	if (ret != 0)
		debug("Error.. %s failed\n", __FUNCTION__);
	else
		debug("%s process finished\n", __FUNCTION__);
}

static void kw_sysrst_check(void)
{
	u32 sysrst_cnt, sysrst_dly;
	char *s;

	/*
	 * no action if sysrstdelay environment variable is not defined
	 */
	s = env_get("sysrstdelay");
	if (s == NULL)
		return;

	/* read sysrstdelay value */
	sysrst_dly = (u32) simple_strtoul(s, NULL, 10);

	/* read SysRst Length counter register (bits 28:0) */
	sysrst_cnt = (0x1fffffff & readl(KW_REG_SYSRST_CNT));
	debug("H/w Rst hold time: %d.%d secs\n",
		sysrst_cnt / SYSRST_CNT_1SEC_VAL,
		sysrst_cnt % SYSRST_CNT_1SEC_VAL);

	/* clear the counter for next valid read*/
	writel(1 << 31, KW_REG_SYSRST_CNT);

	/*
	 * sysrst_action:
	 * if H/w Reset key is pressed and hold for time
	 * more than sysrst_dly in seconds
	 */
	if (sysrst_cnt >= SYSRST_CNT_1SEC_VAL * sysrst_dly)
		kw_sysrst_action();
}

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	char *rev = "??";
	u16 devid = (readl(KW_REG_PCIE_DEVID) >> 16) & 0xffff;
	u8 revid = readl(KW_REG_PCIE_REVID) & 0xff;

	if ((readl(KW_REG_DEVICE_ID) & 0x03) > 2) {
		printf("Error.. %s:Unsupported Kirkwood SoC 88F%04x\n", __FUNCTION__, devid);
		return -1;
	}

	switch (revid) {
	case 0:
		if (devid == 0x6281)
			rev = "Z0";
		else if (devid == 0x6282)
			rev = "A0";
		break;
	case 1:
		rev = "A1";
		break;
	case 2:
		rev = "A0";
		break;
	case 3:
		rev = "A1";
		break;
	default:
		break;
	}

	printf("SoC:   Kirkwood 88F%04x_%s\n", devid, rev);
	return 0;
}
#endif /* CONFIG_DISPLAY_CPUINFO */

#ifdef CONFIG_ARCH_CPU_INIT
int arch_cpu_init(void)
{
	u32 reg;
	struct kwcpu_registers *cpureg =
		(struct kwcpu_registers *)KW_CPU_REG_BASE;

	/* Linux expects the internal registers to be at 0xf1000000 */
	writel(KW_REGS_PHY_BASE, KW_OFFSET_REG);

	/* Enable and invalidate L2 cache in write through mode */
	writel(readl(&cpureg->l2_cfg) | 0x18, &cpureg->l2_cfg);
	invalidate_l2_cache();

#ifdef CONFIG_KIRKWOOD_RGMII_PAD_1V8
	/*
	 * Configures the I/O voltage of the pads connected to Egigabit
	 * Ethernet interface to 1.8V
	 * By default it is set to 3.3V
	 */
	reg = readl(KW_REG_MPP_OUT_DRV_REG);
	reg |= (1 << 7);
	writel(reg, KW_REG_MPP_OUT_DRV_REG);
#endif
#ifdef CONFIG_KIRKWOOD_EGIGA_INIT
	/*
	 * Set egiga port0/1 in normal functional mode
	 * This is required becasue on kirkwood by default ports are in reset mode
	 * OS egiga driver may not have provision to set them in normal mode
	 * and if u-boot is build without network support, network may fail at OS level
	 */
	reg = readl(KWGBE_PORT_SERIAL_CONTROL1_REG(0));
	reg &= ~(1 << 4);	/* Clear PortReset Bit */
	writel(reg, (KWGBE_PORT_SERIAL_CONTROL1_REG(0)));
	reg = readl(KWGBE_PORT_SERIAL_CONTROL1_REG(1));
	reg &= ~(1 << 4);	/* Clear PortReset Bit */
	writel(reg, (KWGBE_PORT_SERIAL_CONTROL1_REG(1)));
#endif
#ifdef CONFIG_KIRKWOOD_PCIE_INIT
	/*
	 * Enable PCI Express Port0
	 */
	reg = readl(&cpureg->ctrl_stat);
	reg |= (1 << 0);	/* Set PEX0En Bit */
	writel(reg, &cpureg->ctrl_stat);
#endif
	return 0;
}
#endif /* CONFIG_ARCH_CPU_INIT */

/*
 * SOC specific misc init
 */
#if defined(CONFIG_ARCH_MISC_INIT)
int arch_misc_init(void)
{
	volatile u32 temp;

	/*CPU streaming & write allocate */
	temp = readfr_extra_feature_reg();
	temp &= ~(1 << 28);	/* disable wr alloc */
	writefr_extra_feature_reg(temp);

	temp = readfr_extra_feature_reg();
	temp &= ~(1 << 29);	/* streaming disabled */
	writefr_extra_feature_reg(temp);

	/* L2Cache settings */
	temp = readfr_extra_feature_reg();
	/* Disable L2C pre fetch - Set bit 24 */
	temp |= (1 << 24);
	/* enable L2C - Set bit 22 */
	temp |= (1 << 22);
	writefr_extra_feature_reg(temp);

	/* Change reset vector to address 0x0 */
	temp = get_cr();
	set_cr(temp & ~CR_V);

	/* Configure mbus windows */
	mvebu_mbus_probe(windows, ARRAY_SIZE(windows));

	/* checks and execute resset to factory event */
	kw_sysrst_check();

	return 0;
}
#endif /* CONFIG_ARCH_MISC_INIT */

#ifdef CONFIG_MVGBE
int cpu_eth_init(bd_t *bis)
{
	mvgbe_initialize(bis);
	return 0;
}
#endif

#ifdef CONFIG_MVEBU_MMC
int board_mmc_init(bd_t *bis)
{
	mvebu_mmc_init(bis);
	return 0;
}
#endif /* CONFIG_MVEBU_MMC */
