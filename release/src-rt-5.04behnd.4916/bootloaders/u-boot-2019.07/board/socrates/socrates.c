// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Sergei Poselenov, Emcraft Systems, sposelenov@emcraft.com.
 *
 * Copyright 2004 Freescale Semiconductor.
 * (C) Copyright 2002,2003, Motorola Inc.
 * Xianghua Xiao, (X.Xiao@motorola.com)
 *
 * (C) Copyright 2002 Scott McNutt <smcnutt@artesyncp.com>
 */

#include <common.h>
#include <pci.h>
#include <asm/processor.h>
#include <asm/immap_85xx.h>
#include <ioports.h>
#include <flash.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <asm/io.h>
#include <i2c.h>
#include <mb862xx.h>
#include <video_fb.h>
#include "upm_table.h"

DECLARE_GLOBAL_DATA_PTR;

extern flash_info_t flash_info[];	/* FLASH chips info */
extern GraphicDevice mb862xx;

void local_bus_init (void);
ulong flash_get_size (ulong base, int banknum);

int checkboard (void)
{
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	char buf[64];
	int f;
	int i = env_get_f("serial#", buf, sizeof(buf));
#ifdef CONFIG_PCI
	char *src;
#endif

	puts("Board: Socrates");
	if (i > 0) {
		puts(", serial# ");
		puts(buf);
	}
	putc('\n');

#ifdef CONFIG_PCI
	/* Check the PCI_clk sel bit */
	if (in_be32(&gur->porpllsr) & (1<<15)) {
		src = "SYSCLK";
		f = CONFIG_SYS_CLK_FREQ;
	} else {
		src = "PCI_CLK";
		f = CONFIG_PCI_CLK_FREQ;
	}
	printf ("PCI1:  32 bit, %d MHz (%s)\n",	f/1000000, src);
#else
	printf ("PCI1:  disabled\n");
#endif

	/*
	 * Initialize local bus.
	 */
	local_bus_init ();
	return 0;
}

int misc_init_r (void)
{
	/*
	 * Adjust flash start and offset to detected values
	 */
	gd->bd->bi_flashstart = 0 - gd->bd->bi_flashsize;
	gd->bd->bi_flashoffset = 0;

	/*
	 * Check if boot FLASH isn't max size
	 */
	if (gd->bd->bi_flashsize < (0 - CONFIG_SYS_FLASH0)) {
		set_lbc_or(0, gd->bd->bi_flashstart |
			   (CONFIG_SYS_OR0_PRELIM & 0x00007fff));
		set_lbc_br(0, gd->bd->bi_flashstart |
			   (CONFIG_SYS_BR0_PRELIM & 0x00007fff));

		/*
		 * Re-check to get correct base address
		 */
		flash_get_size(gd->bd->bi_flashstart, CONFIG_SYS_MAX_FLASH_BANKS - 1);
	}

	/*
	 * Check if only one FLASH bank is available
	 */
	if (gd->bd->bi_flashsize != CONFIG_SYS_MAX_FLASH_BANKS * (0 - CONFIG_SYS_FLASH0)) {
		set_lbc_or(1, 0);
		set_lbc_br(1, 0);

		/*
		 * Re-do flash protection upon new addresses
		 */
		flash_protect (FLAG_PROTECT_CLEAR,
			       gd->bd->bi_flashstart, 0xffffffff,
			       &flash_info[CONFIG_SYS_MAX_FLASH_BANKS - 1]);

		/* Monitor protection ON by default */
		flash_protect (FLAG_PROTECT_SET,
			       CONFIG_SYS_MONITOR_BASE, CONFIG_SYS_MONITOR_BASE + monitor_flash_len - 1,
			       &flash_info[CONFIG_SYS_MAX_FLASH_BANKS - 1]);

		/* Environment protection ON by default */
		flash_protect (FLAG_PROTECT_SET,
			       CONFIG_ENV_ADDR,
			       CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE - 1,
			       &flash_info[CONFIG_SYS_MAX_FLASH_BANKS - 1]);

		/* Redundant environment protection ON by default */
		flash_protect (FLAG_PROTECT_SET,
			       CONFIG_ENV_ADDR_REDUND,
			       CONFIG_ENV_ADDR_REDUND + CONFIG_ENV_SECT_SIZE - 1,
			       &flash_info[CONFIG_SYS_MAX_FLASH_BANKS - 1]);
	}

	return 0;
}

/*
 * Initialize Local Bus
 */
void local_bus_init (void)
{
	volatile fsl_lbc_t *lbc = LBC_BASE_ADDR;
	volatile ccsr_local_ecm_t *ecm = (void *)(CONFIG_SYS_MPC85xx_ECM_ADDR);
	sys_info_t sysinfo;
	uint clkdiv;
	uint lbc_mhz;
	uint lcrr = CONFIG_SYS_LBC_LCRR;

	get_sys_info (&sysinfo);
	clkdiv = lbc->lcrr & LCRR_CLKDIV;
	lbc_mhz = sysinfo.freq_systembus / 1000000 / clkdiv;

	/* Disable PLL bypass for Local Bus Clock >= 66 MHz */
	if (lbc_mhz >= 66)
		lcrr &= ~LCRR_DBYP;	/* DLL Enabled */
	else
		lcrr |= LCRR_DBYP;	/* DLL Bypass */

	out_be32 (&lbc->lcrr, lcrr);
	asm ("sync;isync;msync");

	out_be32 (&lbc->ltesr, 0xffffffff);	/* Clear LBC error interrupts */
	out_be32 (&lbc->lteir, 0xffffffff);	/* Enable LBC error interrupts */
	out_be32 (&ecm->eedr, 0xffffffff);	/* Clear ecm errors */
	out_be32 (&ecm->eeer, 0xffffffff);	/* Enable ecm errors */

	/* Init UPMA for FPGA access */
	out_be32 (&lbc->mamr, 0x44440); /* Use a customer-supplied value */
	upmconfig (UPMA, (uint *)UPMTableA, sizeof(UPMTableA)/sizeof(int));

	/* Init UPMB for Lime controller access */
	out_be32 (&lbc->mbmr, 0x444440); /* Use a customer-supplied value */
	upmconfig (UPMB, (uint *)UPMTableB, sizeof(UPMTableB)/sizeof(int));
}

#if defined(CONFIG_PCI)
/*
 * Initialize PCI Devices, report devices found.
 */

#ifndef CONFIG_PCI_PNP
static struct pci_config_table pci_mpc85xxads_config_table[] = {
	{PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID,
	 PCI_IDSEL_NUMBER, PCI_ANY_ID,
	 pci_cfgfunc_config_device, {PCI_ENET0_IOADDR,
				     PCI_ENET0_MEMADDR,
				     PCI_COMMAND_MEMORY |
				     PCI_COMMAND_MASTER}},
	{}
};
#endif


static struct pci_controller hose = {
#ifndef CONFIG_PCI_PNP
	config_table:pci_mpc85xxads_config_table,
#endif
};

#endif /* CONFIG_PCI */


void pci_init_board (void)
{
#ifdef CONFIG_PCI
	pci_mpc85xx_init (&hose);
#endif /* CONFIG_PCI */
}

#ifdef CONFIG_BOARD_EARLY_INIT_R
int board_early_init_r (void)
{
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

	/* set and reset the GPIO pin 2 which will reset the W83782G chip */
	out_8((unsigned char*)&gur->gpoutdr, 0x3F );
	out_be32((unsigned int*)&gur->gpiocr, 0x200 );	/* enable GPOut */
	udelay(200);
	out_8( (unsigned char*)&gur->gpoutdr, 0x1F );

	return (0);
}
#endif /* CONFIG_BOARD_EARLY_INIT_R */

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, bd_t *bd)
{
	u32 val[12];
	int rc, i = 0;

	ft_cpu_setup(blob, bd);

	/* Fixup NOR FLASH mapping */
	val[i++] = 0;				/* chip select number */
	val[i++] = 0;				/* always 0 */
	val[i++] = gd->bd->bi_flashstart;
	val[i++] = gd->bd->bi_flashsize;

	if (mb862xx.frameAdrs == CONFIG_SYS_LIME_BASE) {
		/* Fixup LIME mapping */
		val[i++] = 2;			/* chip select number */
		val[i++] = 0;			/* always 0 */
		val[i++] = CONFIG_SYS_LIME_BASE;
		val[i++] = CONFIG_SYS_LIME_SIZE;
	}

	/* Fixup FPGA mapping */
	val[i++] = 3;				/* chip select number */
	val[i++] = 0;				/* always 0 */
	val[i++] = CONFIG_SYS_FPGA_BASE;
	val[i++] = CONFIG_SYS_FPGA_SIZE;

	rc = fdt_find_and_setprop(blob, "/localbus", "ranges",
				  val, i * sizeof(u32), 1);
	if (rc)
		printf("Unable to update localbus ranges, err=%s\n",
		       fdt_strerror(rc));

	return 0;
}
#endif /* CONFIG_OF_BOARD_SETUP */

#define DEFAULT_BRIGHTNESS	25
#define BACKLIGHT_ENABLE	(1 << 31)

static const gdc_regs init_regs [] =
{
	{0x0100, 0x00010f00},
	{0x0020, 0x801901df},
	{0x0024, 0x00000000},
	{0x0028, 0x00000000},
	{0x002c, 0x00000000},
	{0x0110, 0x00000000},
	{0x0114, 0x00000000},
	{0x0118, 0x01df0320},
	{0x0004, 0x041f0000},
	{0x0008, 0x031f031f},
	{0x000c, 0x017f0349},
	{0x0010, 0x020c0000},
	{0x0014, 0x01df01e9},
	{0x0018, 0x00000000},
	{0x001c, 0x01e00320},
	{0x0100, 0x80010f00},
	{0x0, 0x0}
};

const gdc_regs *board_get_regs (void)
{
	return init_regs;
}

int lime_probe(void)
{
	uint cfg_br2;
	uint cfg_or2;
	int type;

	cfg_br2 = get_lbc_br(2);
	cfg_or2 = get_lbc_or(2);

	/* Configure GPCM for CS2 */
	set_lbc_br(2, 0);
	set_lbc_or(2, 0xfc000410);
	set_lbc_br(2, (CONFIG_SYS_LIME_BASE) | 0x00001901);

	/* Get controller type */
	type = mb862xx_probe(CONFIG_SYS_LIME_BASE);

	/* Restore previous CS2 configuration */
	set_lbc_br(2, 0);
	set_lbc_or(2, cfg_or2);
	set_lbc_br(2, cfg_br2);

	return (type == MB862XX_TYPE_LIME) ? 1 : 0;
}

/* Returns Lime base address */
unsigned int board_video_init (void)
{
	if (!lime_probe())
		return 0;

	mb862xx.winSizeX = 800;
	mb862xx.winSizeY = 480;
	mb862xx.gdfIndex = GDF_15BIT_555RGB;
	mb862xx.gdfBytesPP = 2;

	return CONFIG_SYS_LIME_BASE;
}

#define W83782D_REG_CFG		0x40
#define W83782D_REG_BANK_SEL	0x4e
#define W83782D_REG_ADCCLK	0x4b
#define W83782D_REG_BEEP_CTRL	0x4d
#define W83782D_REG_BEEP_CTRL2	0x57
#define W83782D_REG_PWMOUT1	0x5b
#define W83782D_REG_VBAT	0x5d

static int w83782d_hwmon_init(void)
{
	u8 buf;

	if (i2c_read(CONFIG_SYS_I2C_W83782G_ADDR, W83782D_REG_CFG, 1, &buf, 1))
		return -1;

	i2c_reg_write(CONFIG_SYS_I2C_W83782G_ADDR, W83782D_REG_CFG, 0x80);
	i2c_reg_write(CONFIG_SYS_I2C_W83782G_ADDR, W83782D_REG_BANK_SEL, 0);
	i2c_reg_write(CONFIG_SYS_I2C_W83782G_ADDR, W83782D_REG_ADCCLK, 0x40);

	buf = i2c_reg_read(CONFIG_SYS_I2C_W83782G_ADDR, W83782D_REG_BEEP_CTRL);
	i2c_reg_write(CONFIG_SYS_I2C_W83782G_ADDR, W83782D_REG_BEEP_CTRL,
		      buf | 0x80);
	i2c_reg_write(CONFIG_SYS_I2C_W83782G_ADDR, W83782D_REG_BEEP_CTRL2, 0);
	i2c_reg_write(CONFIG_SYS_I2C_W83782G_ADDR, W83782D_REG_PWMOUT1, 0x47);
	i2c_reg_write(CONFIG_SYS_I2C_W83782G_ADDR, W83782D_REG_VBAT, 0x01);

	buf = i2c_reg_read(CONFIG_SYS_I2C_W83782G_ADDR, W83782D_REG_CFG);
	i2c_reg_write(CONFIG_SYS_I2C_W83782G_ADDR, W83782D_REG_CFG,
		      (buf & 0xf4) | 0x01);
	return 0;
}

static void board_backlight_brightness(int br)
{
	u32 reg;
	u8 buf;
	u8 old_buf;

	/* Select bank 0 */
	if (i2c_read(CONFIG_SYS_I2C_W83782G_ADDR, 0x4e, 1, &old_buf, 1))
		goto err;
	else
		buf = old_buf & 0xf8;

	if (i2c_write(CONFIG_SYS_I2C_W83782G_ADDR, 0x4e, 1, &buf, 1))
		goto err;

	if (br > 0) {
		/* PWMOUT1 duty cycle ctrl */
		buf = 255 / (100 / br);
		if (i2c_write(CONFIG_SYS_I2C_W83782G_ADDR, 0x5b, 1, &buf, 1))
			goto err;

		/* LEDs on */
		reg = in_be32((void *)(CONFIG_SYS_FPGA_BASE + 0x0c));
		if (!(reg & BACKLIGHT_ENABLE))
			out_be32((void *)(CONFIG_SYS_FPGA_BASE + 0x0c),
				 reg | BACKLIGHT_ENABLE);
	} else {
		buf = 0;
		if (i2c_write(CONFIG_SYS_I2C_W83782G_ADDR, 0x5b, 1, &buf, 1))
			goto err;

		/* LEDs off */
		reg = in_be32((void *)(CONFIG_SYS_FPGA_BASE + 0x0c));
		reg &= ~BACKLIGHT_ENABLE;
		out_be32((void *)(CONFIG_SYS_FPGA_BASE + 0x0c), reg);
	}
	/* Restore previous bank setting */
	if (i2c_write(CONFIG_SYS_I2C_W83782G_ADDR, 0x4e, 1, &old_buf, 1))
		goto err;

	return;
err:
	printf("W83782G I2C access failed\n");
}

void board_backlight_switch (int flag)
{
	char * param;
	int rc;

	if (w83782d_hwmon_init())
		printf ("hwmon IC init failed\n");

	if (flag) {
		param = env_get("brightness");
		rc = param ? simple_strtol(param, NULL, 10) : -1;
		if (rc < 0)
			rc = DEFAULT_BRIGHTNESS;
	} else {
		rc = 0;
	}
	board_backlight_brightness(rc);
}

#if defined(CONFIG_CONSOLE_EXTRA_INFO)
/*
 * Return text to be printed besides the logo.
 */
void video_get_info_str (int line_number, char *info)
{
	if (line_number == 1) {
		strcpy (info, " Board: Socrates");
	} else {
		info [0] = '\0';
	}
}
#endif
