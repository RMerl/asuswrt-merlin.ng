// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 * Authors: Timur Tabi <timur@freescale.com>
 *
 * FSL DIU Framebuffer driver
 */

#include <common.h>
#include <command.h>
#include <linux/ctype.h>
#include <asm/io.h>
#include <stdio_dev.h>
#include <video_fb.h>
#include "../common/ngpixis.h"
#include <fsl_diu_fb.h>

/* The CTL register is called 'csr' in the ngpixis_t structure */
#define PX_CTL_ALTACC		0x80

#define PX_BRDCFG0_ELBC_SPI_MASK	0xc0
#define PX_BRDCFG0_ELBC_SPI_ELBC	0x00
#define PX_BRDCFG0_ELBC_SPI_NULL	0xc0
#define PX_BRDCFG0_ELBC_DIU		0x02

#define PX_BRDCFG1_DVIEN	0x80
#define PX_BRDCFG1_DFPEN	0x40
#define PX_BRDCFG1_BACKLIGHT	0x20

#define PMUXCR_ELBCDIU_MASK	0xc0000000
#define PMUXCR_ELBCDIU_NOR16	0x80000000
#define PMUXCR_ELBCDIU_DIU	0x40000000

/*
 * DIU Area Descriptor
 *
 * Note that we need to byte-swap the value before it's written to the AD
 * register.  So even though the registers don't look like they're in the same
 * bit positions as they are on the MPC8610, the same value is written to the
 * AD register on the MPC8610 and on the P1022.
 */
#define AD_BYTE_F		0x10000000
#define AD_ALPHA_C_SHIFT	25
#define AD_BLUE_C_SHIFT		23
#define AD_GREEN_C_SHIFT	21
#define AD_RED_C_SHIFT		19
#define AD_PIXEL_S_SHIFT	16
#define AD_COMP_3_SHIFT		12
#define AD_COMP_2_SHIFT		8
#define AD_COMP_1_SHIFT		4
#define AD_COMP_0_SHIFT		0

/*
 * Variables used by the DIU/LBC switching code.  It's safe to makes these
 * global, because the DIU requires DDR, so we'll only run this code after
 * relocation.
 */
static u8 px_brdcfg0;
static u32 pmuxcr;
static void *lbc_lcs0_ba;
static void *lbc_lcs1_ba;
static u32 old_br0, old_or0, old_br1, old_or1;
static u32 new_br0, new_or0, new_br1, new_or1;

void diu_set_pixel_clock(unsigned int pixclock)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	unsigned long speed_ccb, temp;
	u32 pixval;

	speed_ccb = get_bus_freq(0);
	temp = 1000000000 / pixclock;
	temp *= 1000;
	pixval = speed_ccb / temp;
	debug("DIU pixval = %u\n", pixval);

	/* Modify PXCLK in GUTS CLKDVDR */
	temp = in_be32(&gur->clkdvdr) & 0x2000FFFF;
	out_be32(&gur->clkdvdr, temp);			/* turn off clock */
	out_be32(&gur->clkdvdr, temp | 0x80000000 | ((pixval & 0x1F) << 16));
}

int platform_diu_init(unsigned int xres, unsigned int yres, const char *port)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	const char *name;
	u32 pixel_format;
	u8 temp;
	phys_addr_t phys0, phys1; /* BR0/BR1 physical addresses */

	/*
	 * Indirect mode requires both BR0 and BR1 to be set to "GPCM",
	 * otherwise writes to these addresses won't actually appear on the
	 * local bus, and so the PIXIS won't see them.
	 *
	 * In FCM mode, writes go to the NAND controller, which does not pass
	 * them to the localbus directly.  So we force BR0 and BR1 into GPCM
	 * mode, since we don't care about what's behind the localbus any
	 * more.  However, we save those registers first, so that we can
	 * restore them when necessary.
	 */
	new_br0 = old_br0 = get_lbc_br(0);
	new_br1 = old_br1 = get_lbc_br(1);
	new_or0 = old_or0 = get_lbc_or(0);
	new_or1 = old_or1 = get_lbc_or(1);

	/*
	 * Use the existing BRx/ORx values if it's already GPCM. Otherwise,
	 * force the values to simple 32KB GPCM windows with the most
	 * conservative timing.
	 */
	if ((old_br0 & BR_MSEL) != BR_MS_GPCM) {
		new_br0 = (get_lbc_br(0) & BR_BA) | BR_V;
		new_or0 = OR_AM_32KB | 0xFF7;
		set_lbc_br(0, new_br0);
		set_lbc_or(0, new_or0);
	}
	if ((old_br1 & BR_MSEL) != BR_MS_GPCM) {
		new_br1 = (get_lbc_br(1) & BR_BA) | BR_V;
		new_or1 = OR_AM_32KB | 0xFF7;
		set_lbc_br(1, new_br1);
		set_lbc_or(1, new_or1);
	}

	/*
	 * Determine the physical addresses for Chip Selects 0 and 1.  The
	 * BR0/BR1 registers contain the truncated physical addresses for the
	 * chip selects, mapped via the localbus LAW.  Since the BRx registers
	 * only contain the lower 32 bits of the address, we have to determine
	 * the upper 4 bits some other way.  The proper way is to scan the LAW
	 * table looking for a matching localbus address. Instead, we cheat.
	 * We know that the upper bits are 0 for 32-bit addressing, or 0xF for
	 * 36-bit addressing.
	 */
#ifdef CONFIG_PHYS_64BIT
	phys0 = 0xf00000000ULL | (old_br0 & old_or0 & BR_BA);
	phys1 = 0xf00000000ULL | (old_br1 & old_or1 & BR_BA);
#else
	phys0 = old_br0 & old_or0 & BR_BA;
	phys1 = old_br1 & old_or1 & BR_BA;
#endif

	 /* Save the LBC LCS0 and LCS1 addresses for the DIU mux functions */
	lbc_lcs0_ba = map_physmem(phys0, 1, 0);
	lbc_lcs1_ba = map_physmem(phys1, 1, 0);

	pixel_format = cpu_to_le32(AD_BYTE_F | (3 << AD_ALPHA_C_SHIFT) |
		(0 << AD_BLUE_C_SHIFT) | (1 << AD_GREEN_C_SHIFT) |
		(2 << AD_RED_C_SHIFT) | (8 << AD_COMP_3_SHIFT) |
		(8 << AD_COMP_2_SHIFT) | (8 << AD_COMP_1_SHIFT) |
		(8 << AD_COMP_0_SHIFT) | (3 << AD_PIXEL_S_SHIFT));

	temp = in_8(&pixis->brdcfg1);

	if (strncmp(port, "lvds", 4) == 0) {
		/* Single link LVDS */
		temp &= ~PX_BRDCFG1_DVIEN;
		/*
		 * LVDS also needs backlight enabled, otherwise the display
		 * will be blank.
		 */
		temp |= (PX_BRDCFG1_DFPEN | PX_BRDCFG1_BACKLIGHT);
		name = "Single-Link LVDS";
	} else {	/* DVI */
		/* Enable the DVI port, disable the DFP and the backlight */
		temp &= ~(PX_BRDCFG1_DFPEN | PX_BRDCFG1_BACKLIGHT);
		temp |= PX_BRDCFG1_DVIEN;
		name = "DVI";
	}

	printf("DIU:   Switching to %s monitor @ %ux%u\n", name, xres, yres);
	out_8(&pixis->brdcfg1, temp);

	/*
	 * Enable PIXIS indirect access mode.  This is a hack that allows us to
	 * access PIXIS registers even when the LBC pins have been muxed to the
	 * DIU.
	 */
	setbits_8(&pixis->csr, PX_CTL_ALTACC);

	/*
	 * Route the LAD pins to the DIU.  This will disable access to the eLBC,
	 * which means we won't be able to read/write any NOR flash addresses!
	 */
	out_8(lbc_lcs0_ba, offsetof(ngpixis_t, brdcfg0));
	px_brdcfg0 = in_8(lbc_lcs1_ba);
	out_8(lbc_lcs1_ba, px_brdcfg0 | PX_BRDCFG0_ELBC_DIU);
	in_8(lbc_lcs1_ba);

	/* Set PMUXCR to switch the muxed pins from the LBC to the DIU */
	clrsetbits_be32(&gur->pmuxcr, PMUXCR_ELBCDIU_MASK, PMUXCR_ELBCDIU_DIU);
	pmuxcr = in_be32(&gur->pmuxcr);

	return fsl_diu_init(xres, yres, pixel_format, 0);
}

/*
 * set_mux_to_lbc - disable the DIU so that we can read/write to elbc
 *
 * On the Freescale P1022, the DIU video signal and the LBC address/data lines
 * share the same pins, which means that when the DIU is active (e.g. the
 * console is on the DVI display), NOR flash cannot be accessed.  So we use the
 * weak accessor feature of the CFI flash code to temporarily switch the pin
 * mux from DIU to LBC whenever we want to read or write flash.  This has a
 * significant performance penalty, but it's the only way to make it work.
 *
 * There are two muxes: one on the chip, and one on the board. The chip mux
 * controls whether the pins are used for the DIU or the LBC, and it is
 * set via PMUXCR.  The board mux controls whether those signals go to
 * the video connector or the NOR flash chips, and it is set via the ngPIXIS.
 */
static int set_mux_to_lbc(void)
{
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;

	/* Switch the muxes only if they're currently set to DIU mode */
	if ((in_be32(&gur->pmuxcr) & PMUXCR_ELBCDIU_MASK) !=
	    PMUXCR_ELBCDIU_NOR16) {
		/*
		 * In DIU mode, the PIXIS can only be accessed indirectly
		 * since we can't read/write the LBC directly.
		 */
		/* Set the board mux to LBC.  This will disable the display. */
		out_8(lbc_lcs0_ba, offsetof(ngpixis_t, brdcfg0));
		out_8(lbc_lcs1_ba, px_brdcfg0);
		in_8(lbc_lcs1_ba);

		/* Disable indirect PIXIS mode */
		out_8(lbc_lcs0_ba, offsetof(ngpixis_t, csr));
		clrbits_8(lbc_lcs1_ba, PX_CTL_ALTACC);

		/* Set the chip mux to LBC mode, so that writes go to flash. */
		out_be32(&gur->pmuxcr, (pmuxcr & ~PMUXCR_ELBCDIU_MASK) |
			 PMUXCR_ELBCDIU_NOR16);
		in_be32(&gur->pmuxcr);

		/* Restore the BR0 and BR1 settings */
		set_lbc_br(0, old_br0);
		set_lbc_or(0, old_or0);
		set_lbc_br(1, old_br1);
		set_lbc_or(1, old_or1);

		return 1;
	}

	return 0;
}

/*
 * set_mux_to_diu - re-enable the DIU muxing
 *
 * This function restores the chip and board muxing to point to the DIU.
 */
static void set_mux_to_diu(void)
{
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;

	/* Set BR0 and BR1 to GPCM mode */
	set_lbc_br(0, new_br0);
	set_lbc_or(0, new_or0);
	set_lbc_br(1, new_br1);
	set_lbc_or(1, new_or1);

	/* Enable indirect PIXIS mode */
	setbits_8(&pixis->csr, PX_CTL_ALTACC);

	/* Set the board mux to DIU.  This will enable the display. */
	out_8(lbc_lcs0_ba, offsetof(ngpixis_t, brdcfg0));
	out_8(lbc_lcs1_ba, px_brdcfg0 | PX_BRDCFG0_ELBC_DIU);
	in_8(lbc_lcs1_ba);

	/* Set the chip mux to DIU mode. */
	out_be32(&gur->pmuxcr, pmuxcr);
	in_be32(&gur->pmuxcr);
}

/*
 * pixis_read - board-specific function to read from the PIXIS
 *
 * This function overrides the generic pixis_read() function, so that it can
 * use PIXIS indirect mode if necessary.
 */
u8 pixis_read(unsigned int reg)
{
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;

	/* Use indirect mode if the mux is currently set to DIU mode */
	if ((in_be32(&gur->pmuxcr) & PMUXCR_ELBCDIU_MASK) !=
	    PMUXCR_ELBCDIU_NOR16) {
		out_8(lbc_lcs0_ba, reg);
		return in_8(lbc_lcs1_ba);
	} else {
		void *p = (void *)PIXIS_BASE;

		return in_8(p + reg);
	}
}

/*
 * pixis_write - board-specific function to write to the PIXIS
 *
 * This function overrides the generic pixis_write() function, so that it can
 * use PIXIS indirect mode if necessary.
 */
void pixis_write(unsigned int reg, u8 value)
{
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;

	/* Use indirect mode if the mux is currently set to DIU mode */
	if ((in_be32(&gur->pmuxcr) & PMUXCR_ELBCDIU_MASK) !=
	    PMUXCR_ELBCDIU_NOR16) {
		out_8(lbc_lcs0_ba, reg);
		out_8(lbc_lcs1_ba, value);
		/* Do a read-back to ensure the write completed */
		in_8(lbc_lcs1_ba);
	} else {
		void *p = (void *)PIXIS_BASE;

		out_8(p + reg, value);
	}
}

void pixis_bank_reset(void)
{
	/*
	 * For some reason, a PIXIS bank reset does not work if the PIXIS is
	 * in indirect mode, so switch to direct mode first.
	 */
	set_mux_to_lbc();

	out_8(&pixis->vctl, 0);
	out_8(&pixis->vctl, 1);

	while (1);
}

#ifdef CONFIG_CFI_FLASH_USE_WEAK_ACCESSORS

void flash_write8(u8 value, void *addr)
{
	int sw = set_mux_to_lbc();

	__raw_writeb(value, addr);
	if (sw) {
		/*
		 * To ensure the post-write is completed to eLBC, software must
		 * perform a dummy read from one valid address from eLBC space
		 * before changing the eLBC_DIU from NOR mode to DIU mode.
		 * set_mux_to_diu() includes a sync that will ensure the
		 * __raw_readb() completes before it switches the mux.
		 */
		__raw_readb(addr);
		set_mux_to_diu();
	}
}

void flash_write16(u16 value, void *addr)
{
	int sw = set_mux_to_lbc();

	__raw_writew(value, addr);
	if (sw) {
		/*
		 * To ensure the post-write is completed to eLBC, software must
		 * perform a dummy read from one valid address from eLBC space
		 * before changing the eLBC_DIU from NOR mode to DIU mode.
		 * set_mux_to_diu() includes a sync that will ensure the
		 * __raw_readb() completes before it switches the mux.
		 */
		__raw_readb(addr);
		set_mux_to_diu();
	}
}

void flash_write32(u32 value, void *addr)
{
	int sw = set_mux_to_lbc();

	__raw_writel(value, addr);
	if (sw) {
		/*
		 * To ensure the post-write is completed to eLBC, software must
		 * perform a dummy read from one valid address from eLBC space
		 * before changing the eLBC_DIU from NOR mode to DIU mode.
		 * set_mux_to_diu() includes a sync that will ensure the
		 * __raw_readb() completes before it switches the mux.
		 */
		__raw_readb(addr);
		set_mux_to_diu();
	}
}

void flash_write64(u64 value, void *addr)
{
	int sw = set_mux_to_lbc();
	uint32_t *p = addr;

	/*
	 * There is no __raw_writeq(), so do the write manually.  We don't trust
	 * the compiler, so we use inline assembly.
	 */
	__asm__ __volatile__(
		"stw%U0%X0 %2,%0;\n"
		"stw%U1%X1 %3,%1;\n"
		: "=m" (*p), "=m" (*(p + 1))
		: "r" ((uint32_t) (value >> 32)), "r" ((uint32_t) (value)));

	if (sw) {
		/*
		 * To ensure the post-write is completed to eLBC, software must
		 * perform a dummy read from one valid address from eLBC space
		 * before changing the eLBC_DIU from NOR mode to DIU mode.  We
		 * read addr+4 because we just wrote to addr+4, so that's how we
		 * maintain execution order.  set_mux_to_diu() includes a sync
		 * that will ensure the __raw_readb() completes before it
		 * switches the mux.
		 */
		__raw_readb(addr + 4);
		set_mux_to_diu();
	}
}

u8 flash_read8(void *addr)
{
	u8 ret;

	int sw = set_mux_to_lbc();

	ret = __raw_readb(addr);
	if (sw)
		set_mux_to_diu();

	return ret;
}

u16 flash_read16(void *addr)
{
	u16 ret;

	int sw = set_mux_to_lbc();

	ret = __raw_readw(addr);
	if (sw)
		set_mux_to_diu();

	return ret;
}

u32 flash_read32(void *addr)
{
	u32 ret;

	int sw = set_mux_to_lbc();

	ret = __raw_readl(addr);
	if (sw)
		set_mux_to_diu();

	return ret;
}

u64 flash_read64(void *addr)
{
	u64 ret;

	int sw = set_mux_to_lbc();

	/* There is no __raw_readq(), so do the read manually */
	ret = *(volatile u64 *)addr;
	if (sw)
		set_mux_to_diu();

	return ret;
}

#endif
