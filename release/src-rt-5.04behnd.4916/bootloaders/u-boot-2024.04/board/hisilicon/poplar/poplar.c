// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2017 Linaro
 * Jorge Ramirez-Ortiz <jorge.ramirez-ortiz@linaro.org>
 */

#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <init.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <dm/platform_data/serial_pl01x.h>
#include <asm/arch/hi3798cv200.h>
#include <asm/armv8/mmu.h>
#include <linux/bitops.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

static struct mm_region poplar_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0x80000000UL,
		.phys = 0x80000000UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		0,
	}
};

struct mm_region *mem_map = poplar_mem_map;

#if !CONFIG_IS_ENABLED(OF_CONTROL)
static const struct pl01x_serial_plat serial_plat = {
	.base = REG_BASE_UART0,
	.type = TYPE_PL010,
	.clock = 75000000,
};

U_BOOT_DRVINFO(poplar_serial) = {
	.name = "serial_pl01x",
	.plat = &serial_plat,
};
#endif

int checkboard(void)
{
	puts("BOARD: Hisilicon HI3798cv200 Poplar\n");

	return 0;
}

void reset_cpu(void)
{
	psci_system_reset();
}

int dram_init(void)
{
	gd->ram_size = get_ram_size(NULL, 0x80000000);

	return 0;
}

/*
 * Some linux kernel versions don't use memory before its load address, so to
 * be generic we just pretend it isn't there.  In previous uboot versions we
 * carved the space used by BL31 (runs in DDR on this platfomr) so the PSCI code
 * could persist in memory and be left alone by the kernel.
 *
 * That led to a problem when mapping memory in older kernels.  That PSCI code
 * now lies in memory below the kernel load offset; it therefore won't be
 * touched by the kernel, and by not specially reserving it we avoid the mapping
 * problem as well.
 *
 */
#define KERNEL_TEXT_OFFSET	0x00080000

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = KERNEL_TEXT_OFFSET;
	gd->bd->bi_dram[0].size = gd->ram_size - gd->bd->bi_dram[0].start;

	return 0;
}

static void usb2_phy_config(void)
{
	const u32 config[] = {
		/* close EOP pre-emphasis. open data pre-emphasis */
		0xa1001c,
		/* Rcomp = 150mW, increase DC level */
		0xa00607,
		/* keep Rcomp working */
		0xa10700,
		/* Icomp = 212mW, increase current drive */
		0xa00aab,
		/* EMI fix: rx_active not stay 1 when error packets received */
		0xa11140,
		/* Comp mode select */
		0xa11041,
		/* adjust eye diagram */
		0xa0098c,
		/* adjust eye diagram */
		0xa10a0a,
	};
	int i;

	for (i = 0; i < ARRAY_SIZE(config); i++) {
		writel(config[i], PERI_CTRL_USB0);
		clrsetbits_le32(PERI_CTRL_USB0, BIT(21), BIT(20) | BIT(22));
		udelay(20);
	}
}

static void usb2_phy_init(void)
{
	/* reset usb2 controller bus/utmi/roothub */
	setbits_le32(PERI_CRG46, USB2_BUS_SRST_REQ | USB2_UTMI0_SRST_REQ |
			USB2_HST_PHY_SYST_REQ | USB2_OTG_PHY_SYST_REQ);
	udelay(200);

	/* reset usb2 phy por/utmi */
	setbits_le32(PERI_CRG47, USB2_PHY01_SRST_REQ | USB2_PHY01_SRST_TREQ1);
	udelay(200);

	/* open usb2 ref clk */
	setbits_le32(PERI_CRG47, USB2_PHY01_REF_CKEN);
	udelay(300);

	/* cancel usb2 power on reset */
	clrbits_le32(PERI_CRG47, USB2_PHY01_SRST_REQ);
	udelay(500);

	usb2_phy_config();

	/* cancel usb2 port reset, wait comp circuit stable */
	clrbits_le32(PERI_CRG47, USB2_PHY01_SRST_TREQ1);
	mdelay(10);

	/* open usb2 controller clk */
	setbits_le32(PERI_CRG46, USB2_BUS_CKEN | USB2_OHCI48M_CKEN |
			USB2_OHCI12M_CKEN | USB2_OTG_UTMI_CKEN |
			USB2_HST_PHY_CKEN | USB2_UTMI0_CKEN);
	udelay(200);

	/* cancel usb2 control reset */
	clrbits_le32(PERI_CRG46, USB2_BUS_SRST_REQ | USB2_UTMI0_SRST_REQ |
			USB2_HST_PHY_SYST_REQ | USB2_OTG_PHY_SYST_REQ);
	udelay(200);
}

#if defined(CONFIG_USB_GADGET) && defined(CONFIG_USB_GADGET_DWC2_OTG)
#include <env.h>
#include <usb.h>
#include <usb/dwc2_udc.h>
#include <g_dnl.h>

static struct dwc2_plat_otg_data poplar_otg_data = {
	.regs_otg = HIOTG_BASE_ADDR
};

static void set_usb_to_device(void)
{
	setbits_le32(PERI_CTRL_USB3, USB2_2P_CHIPID);
}

int board_usb_init(int index, enum usb_init_type init)
{
	set_usb_to_device();
	return dwc2_udc_probe(&poplar_otg_data);
}

int g_dnl_bind_fixup(struct usb_device_descriptor *dev, const char *name)
{
	if (!env_get("serial#"))
		g_dnl_set_serialnumber("0123456789POPLAR");
	return 0;
}
#endif

int board_init(void)
{
	usb2_phy_init();

	return 0;
}
