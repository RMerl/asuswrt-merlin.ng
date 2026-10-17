// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020 Nuvoton Technology Corp.
 * Author: Joseph Liu <kwliu@nuvoton.com>
 * Author: Tomer Maimon <tomer.maimon@nuvoton.com>
 */

#include <dm.h>
#include <errno.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <dm/pinctrl.h>

/* GCR registers */
#define NPCM7XX_GCR_PDID		0x00
#define NPCM7XX_GCR_MFSEL1		0x0C
#define NPCM7XX_GCR_MFSEL2		0x10
#define NPCM7XX_GCR_MFSEL3		0x64
#define NPCM7XX_GCR_MFSEL4		0xb0
#define NPCM7XX_GCR_CPCTL		0xD0
#define NPCM7XX_GCR_CP2BST		0xD4
#define NPCM7XX_GCR_B2CPNT		0xD8
#define NPCM7XX_GCR_I2CSEGSEL		0xE0
#define NPCM7XX_GCR_I2CSEGCTL		0xE4
#define NPCM7XX_GCR_INTCR2		0x60
#define NPCM7XX_GCR_SRCNT		0x68
#define NPCM7XX_GCR_RESSR		0x6C
#define NPCM7XX_GCR_FLOCKR1		0x74
#define NPCM7XX_GCR_DSCNT		0x78

#define SRCNT_ESPI			BIT(3)

/* reset registers */
#define NPCM7XX_RST_WD0RCR		0x38
#define NPCM7XX_RST_WD1RCR		0x3C
#define NPCM7XX_RST_WD2RCR		0x40
#define NPCM7XX_RST_SWRSTC1		0x44
#define NPCM7XX_RST_SWRSTC2		0x48
#define NPCM7XX_RST_SWRSTC3		0x4C
#define NPCM7XX_RST_SWRSTC4		0x50
#define NPCM7XX_RST_CORSTC		0x5C

#define PORST				BIT(31)
#define CORST				BIT(30)
#define WD0RST				BIT(29)
#define WD1RST				BIT(24)
#define WD2RST				BIT(23)
#define SWRST1				BIT(28)
#define SWRST2				BIT(27)
#define SWRST3				BIT(26)
#define SW4RST				BIT(25)

#define GPIOX_MODULE_RESET		16
#define CA9C_RESET			BIT(0)

/* GPIO registers */
#define NPCM7XX_GP_N_TLOCK1	0x00
#define NPCM7XX_GP_N_DIN	0x04 /* Data IN */
#define NPCM7XX_GP_N_POL	0x08 /* Polarity */
#define NPCM7XX_GP_N_DOUT	0x0c /* Data OUT */
#define NPCM7XX_GP_N_OE		0x10 /* Output Enable */
#define NPCM7XX_GP_N_OTYP	0x14
#define NPCM7XX_GP_N_MP		0x18
#define NPCM7XX_GP_N_PU		0x1c /* Pull-up */
#define NPCM7XX_GP_N_PD		0x20 /* Pull-down */
#define NPCM7XX_GP_N_DBNC	0x24 /* Debounce */
#define NPCM7XX_GP_N_EVTYP	0x28 /* Event Type */
#define NPCM7XX_GP_N_EVBE	0x2c /* Event Both Edge */
#define NPCM7XX_GP_N_OBL0	0x30
#define NPCM7XX_GP_N_OBL1	0x34
#define NPCM7XX_GP_N_OBL2	0x38
#define NPCM7XX_GP_N_OBL3	0x3c
#define NPCM7XX_GP_N_EVEN	0x40 /* Event Enable */
#define NPCM7XX_GP_N_EVENS	0x44 /* Event Set (enable) */
#define NPCM7XX_GP_N_EVENC	0x48 /* Event Clear (disable) */
#define NPCM7XX_GP_N_EVST	0x4c /* Event Status */
#define NPCM7XX_GP_N_SPLCK	0x50
#define NPCM7XX_GP_N_MPLCK	0x54
#define NPCM7XX_GP_N_IEM	0x58 /* Input Enable */
#define NPCM7XX_GP_N_OSRC	0x5c
#define NPCM7XX_GP_N_ODSC	0x60
#define NPCM7XX_GP_N_DOS	0x68 /* Data OUT Set */
#define NPCM7XX_GP_N_DOC	0x6c /* Data OUT Clear */
#define NPCM7XX_GP_N_OES	0x70 /* Output Enable Set */
#define NPCM7XX_GP_N_OEC	0x74 /* Output Enable Clear */
#define NPCM7XX_GP_N_TLOCK2	0x7c

#define NPCM7XX_GPIO_BANK_OFFSET 0x1000
#define NPCM7XX_GPIO_PER_BITS	32
#define NPCM7XX_GPIO_PER_BANK	32
#define NPCM7XX_GPIO_BANK_NUM	8
#define NPCM7XX_GCR_NONE	0

/* pinmux handing in the pinctrl driver*/
static const int smb0_pins[]  = { 115, 114 };
static const int smb0b_pins[] = { 195, 194 };
static const int smb0c_pins[] = { 202, 196 };
static const int smb0d_pins[] = { 198, 199 };
static const int smb0den_pins[] = { 197 };

static const int smb1_pins[]  = { 117, 116 };
static const int smb1b_pins[] = { 126, 127 };
static const int smb1c_pins[] = { 124, 125 };
static const int smb1d_pins[] = { 4, 5 };

static const int smb2_pins[]  = { 119, 118 };
static const int smb2b_pins[] = { 122, 123 };
static const int smb2c_pins[] = { 120, 121 };
static const int smb2d_pins[] = { 6, 7 };

static const int smb3_pins[]  = { 30, 31 };
static const int smb3b_pins[] = { 39, 40 };
static const int smb3c_pins[] = { 37, 38 };
static const int smb3d_pins[] = { 59, 60 };

static const int smb4_pins[]  = { 28, 29 };
static const int smb4b_pins[] = { 18, 19 };
static const int smb4c_pins[] = { 20, 21 };
static const int smb4d_pins[] = { 22, 23 };
static const int smb4den_pins[] = { 17 };

static const int smb5_pins[]  = { 26, 27 };
static const int smb5b_pins[] = { 13, 12 };
static const int smb5c_pins[] = { 15, 14 };
static const int smb5d_pins[] = { 94, 93 };
static const int ga20kbc_pins[] = { 94, 93 };

static const int smb6_pins[]  = { 172, 171 };
static const int smb7_pins[]  = { 174, 173 };
static const int smb8_pins[]  = { 129, 128 };
static const int smb9_pins[]  = { 131, 130 };
static const int smb10_pins[] = { 133, 132 };
static const int smb11_pins[] = { 135, 134 };
static const int smb12_pins[] = { 221, 220 };
static const int smb13_pins[] = { 223, 222 };
static const int smb14_pins[] = { 22, 23 };
static const int smb15_pins[] = { 20, 21 };

static const int fanin0_pins[] = { 64 };
static const int fanin1_pins[] = { 65 };
static const int fanin2_pins[] = { 66 };
static const int fanin3_pins[] = { 67 };
static const int fanin4_pins[] = { 68 };
static const int fanin5_pins[] = { 69 };
static const int fanin6_pins[] = { 70 };
static const int fanin7_pins[] = { 71 };
static const int fanin8_pins[] = { 72 };
static const int fanin9_pins[] = { 73 };
static const int fanin10_pins[] = { 74 };
static const int fanin11_pins[] = { 75 };
static const int fanin12_pins[] = { 76 };
static const int fanin13_pins[] = { 77 };
static const int fanin14_pins[] = { 78 };
static const int fanin15_pins[] = { 79 };
static const int faninx_pins[] = { 175, 176, 177, 203 };

static const int pwm0_pins[] = { 80 };
static const int pwm1_pins[] = { 81 };
static const int pwm2_pins[] = { 82 };
static const int pwm3_pins[] = { 83 };
static const int pwm4_pins[] = { 144 };
static const int pwm5_pins[] = { 145 };
static const int pwm6_pins[] = { 146 };
static const int pwm7_pins[] = { 147 };

static const int uart1_pins[] = { 43, 44, 45, 46, 47, 61, 62, 63 };
static const int uart2_pins[] = { 48, 49, 50, 51, 52, 53, 54, 55 };

/* RGMII 1 pin group */
static const int rg1_pins[] = { 96, 97, 98, 99, 100, 101, 102, 103, 104, 105,
	106, 107 };
/* RGMII 1 MD interface pin group */
static const int rg1mdio_pins[] = { 108, 109 };

/* RGMII 2 pin group */
static const int rg2_pins[] = { 110, 111, 112, 113, 208, 209, 210, 211, 212,
	213, 214, 215 };
/* RGMII 2 MD interface pin group */
static const int rg2mdio_pins[] = { 216, 217 };

static const int ddr_pins[] = { 110, 111, 112, 113, 208, 209, 210, 211, 212,
	213, 214, 215, 216, 217 };
/* Serial I/O Expander 1 */
static const int iox1_pins[] = { 0, 1, 2, 3 };
/* Serial I/O Expander 2 */
static const int iox2_pins[] = { 4, 5, 6, 7 };
/* Host Serial I/O Expander 2 */
static const int ioxh_pins[] = { 10, 11, 24, 25 };

static const int mmc_pins[] = { 152, 154, 156, 157, 158, 159 };
static const int mmcwp_pins[] = { 153 };
static const int mmccd_pins[] = { 155 };
static const int mmcrst_pins[] = { 155 };
static const int mmc8_pins[] = { 148, 149, 150, 151 };

/* RMII 1 pin groups */
static const int r1_pins[] = { 178, 179, 180, 181, 182, 193, 201 };
static const int r1err_pins[] = { 56 };
static const int r1md_pins[] = { 57, 58 };

/* RMII 2 pin groups */
static const int r2_pins[] = { 84, 85, 86, 87, 88, 89, 200 };
static const int r2err_pins[] = { 90 };
static const int r2md_pins[] = { 91, 92 };

static const int sd1_pins[] = { 136, 137, 138, 139, 140, 141, 142, 143 };
static const int sd1pwr_pins[] = { 143 };

static const int wdog1_pins[] = { 218 };
static const int wdog2_pins[] = { 219 };

/* BMC serial port 0 */
static const int bmcuart0a_pins[] = { 41, 42 };
static const int bmcuart0b_pins[] = { 48, 49 };

static const int bmcuart1_pins[] = { 43, 44, 62, 63 };

/* System Control Interrupt and Power Management Event pin group */
static const int scipme_pins[] = { 169 };
/* System Management Interrupt pin group */
static const int sci_pins[] = { 170 };
/* Serial Interrupt Line pin group */
static const int serirq_pins[] = { 162 };

static const int clkout_pins[] = { 160 };
static const int clkreq_pins[] = { 231 };

static const int jtag2_pins[] = { 43, 44, 45, 46, 47 };
/* Graphics SPI Clock pin group */
static const int gspi_pins[] = { 12, 13, 14, 15 };

static const int spix_pins[] = { 224, 225, 226, 227, 229, 230 };
static const int spixcs1_pins[] = { 228 };

static const int pspi1_pins[] = { 175, 176, 177 };
static const int pspi2_pins[] = { 17, 18, 19 };

static const int spi0cs1_pins[] = { 32 };

static const int spi3_pins[] = { 183, 184, 185, 186 };
static const int spi3cs1_pins[] = { 187 };
static const int spi3quad_pins[] = { 188, 189 };
static const int spi3cs2_pins[] = { 188 };
static const int spi3cs3_pins[] = { 189 };

static const int ddc_pins[] = { 204, 205, 206, 207 };

static const int lpc_pins[] = { 95, 161, 163, 164, 165, 166, 167 };
static const int lpcclk_pins[] = { 168 };
static const int espi_pins[] = { 95, 161, 163, 164, 165, 166, 167, 168 };

static const int lkgpo0_pins[] = { 16 };
static const int lkgpo1_pins[] = { 8 };
static const int lkgpo2_pins[] = { 9 };

static const int nprd_smi_pins[] = { 190 };

static const int hgpio0_pins[] = { 20 };
static const int hgpio1_pins[] = { 21 };
static const int hgpio2_pins[] = { 22 };
static const int hgpio3_pins[] = { 23 };
static const int hgpio4_pins[] = { 24 };
static const int hgpio5_pins[] = { 25 };
static const int hgpio6_pins[] = { 59 };
static const int hgpio7_pins[] = { 60 };

/*
 * pin:	     name, number
 * group:    name, npins,   pins
 * function: name, ngroups, groups
 */
struct npcm7xx_group {
	const char *name;
	const int *pins;
	int npins;
};

#define NPCM7XX_GRPS \
	NPCM7XX_GRP(smb0), \
	NPCM7XX_GRP(smb0b), \
	NPCM7XX_GRP(smb0c), \
	NPCM7XX_GRP(smb0d), \
	NPCM7XX_GRP(smb0den), \
	NPCM7XX_GRP(smb1), \
	NPCM7XX_GRP(smb1b), \
	NPCM7XX_GRP(smb1c), \
	NPCM7XX_GRP(smb1d), \
	NPCM7XX_GRP(smb2), \
	NPCM7XX_GRP(smb2b), \
	NPCM7XX_GRP(smb2c), \
	NPCM7XX_GRP(smb2d), \
	NPCM7XX_GRP(smb3), \
	NPCM7XX_GRP(smb3b), \
	NPCM7XX_GRP(smb3c), \
	NPCM7XX_GRP(smb3d), \
	NPCM7XX_GRP(smb4), \
	NPCM7XX_GRP(smb4b), \
	NPCM7XX_GRP(smb4c), \
	NPCM7XX_GRP(smb4d), \
	NPCM7XX_GRP(smb4den), \
	NPCM7XX_GRP(smb5), \
	NPCM7XX_GRP(smb5b), \
	NPCM7XX_GRP(smb5c), \
	NPCM7XX_GRP(smb5d), \
	NPCM7XX_GRP(ga20kbc), \
	NPCM7XX_GRP(smb6), \
	NPCM7XX_GRP(smb7), \
	NPCM7XX_GRP(smb8), \
	NPCM7XX_GRP(smb9), \
	NPCM7XX_GRP(smb10), \
	NPCM7XX_GRP(smb11), \
	NPCM7XX_GRP(smb12), \
	NPCM7XX_GRP(smb13), \
	NPCM7XX_GRP(smb14), \
	NPCM7XX_GRP(smb15), \
	NPCM7XX_GRP(fanin0), \
	NPCM7XX_GRP(fanin1), \
	NPCM7XX_GRP(fanin2), \
	NPCM7XX_GRP(fanin3), \
	NPCM7XX_GRP(fanin4), \
	NPCM7XX_GRP(fanin5), \
	NPCM7XX_GRP(fanin6), \
	NPCM7XX_GRP(fanin7), \
	NPCM7XX_GRP(fanin8), \
	NPCM7XX_GRP(fanin9), \
	NPCM7XX_GRP(fanin10), \
	NPCM7XX_GRP(fanin11), \
	NPCM7XX_GRP(fanin12), \
	NPCM7XX_GRP(fanin13), \
	NPCM7XX_GRP(fanin14), \
	NPCM7XX_GRP(fanin15), \
	NPCM7XX_GRP(faninx), \
	NPCM7XX_GRP(pwm0), \
	NPCM7XX_GRP(pwm1), \
	NPCM7XX_GRP(pwm2), \
	NPCM7XX_GRP(pwm3), \
	NPCM7XX_GRP(pwm4), \
	NPCM7XX_GRP(pwm5), \
	NPCM7XX_GRP(pwm6), \
	NPCM7XX_GRP(pwm7), \
	NPCM7XX_GRP(rg1), \
	NPCM7XX_GRP(rg1mdio), \
	NPCM7XX_GRP(rg2), \
	NPCM7XX_GRP(rg2mdio), \
	NPCM7XX_GRP(ddr), \
	NPCM7XX_GRP(uart1), \
	NPCM7XX_GRP(uart2), \
	NPCM7XX_GRP(bmcuart0a), \
	NPCM7XX_GRP(bmcuart0b), \
	NPCM7XX_GRP(bmcuart1), \
	NPCM7XX_GRP(iox1), \
	NPCM7XX_GRP(iox2), \
	NPCM7XX_GRP(ioxh), \
	NPCM7XX_GRP(gspi), \
	NPCM7XX_GRP(mmc), \
	NPCM7XX_GRP(mmcwp), \
	NPCM7XX_GRP(mmccd), \
	NPCM7XX_GRP(mmcrst), \
	NPCM7XX_GRP(mmc8), \
	NPCM7XX_GRP(r1), \
	NPCM7XX_GRP(r1err), \
	NPCM7XX_GRP(r1md), \
	NPCM7XX_GRP(r2), \
	NPCM7XX_GRP(r2err), \
	NPCM7XX_GRP(r2md), \
	NPCM7XX_GRP(sd1), \
	NPCM7XX_GRP(sd1pwr), \
	NPCM7XX_GRP(wdog1), \
	NPCM7XX_GRP(wdog2), \
	NPCM7XX_GRP(scipme), \
	NPCM7XX_GRP(sci), \
	NPCM7XX_GRP(serirq), \
	NPCM7XX_GRP(jtag2), \
	NPCM7XX_GRP(spix), \
	NPCM7XX_GRP(spixcs1), \
	NPCM7XX_GRP(pspi1), \
	NPCM7XX_GRP(pspi2), \
	NPCM7XX_GRP(ddc), \
	NPCM7XX_GRP(clkreq), \
	NPCM7XX_GRP(clkout), \
	NPCM7XX_GRP(spi3), \
	NPCM7XX_GRP(spi3cs1), \
	NPCM7XX_GRP(spi3quad), \
	NPCM7XX_GRP(spi3cs2), \
	NPCM7XX_GRP(spi3cs3), \
	NPCM7XX_GRP(spi0cs1), \
	NPCM7XX_GRP(lpc), \
	NPCM7XX_GRP(lpcclk), \
	NPCM7XX_GRP(espi), \
	NPCM7XX_GRP(lkgpo0), \
	NPCM7XX_GRP(lkgpo1), \
	NPCM7XX_GRP(lkgpo2), \
	NPCM7XX_GRP(nprd_smi), \
	NPCM7XX_GRP(hgpio0), \
	NPCM7XX_GRP(hgpio1), \
	NPCM7XX_GRP(hgpio2), \
	NPCM7XX_GRP(hgpio3), \
	NPCM7XX_GRP(hgpio4), \
	NPCM7XX_GRP(hgpio5), \
	NPCM7XX_GRP(hgpio6), \
	NPCM7XX_GRP(hgpio7), \
	\

enum {
#define NPCM7XX_GRP(x) fn_ ## x
	NPCM7XX_GRPS
	/* add placeholder for none/gpio */
	NPCM7XX_GRP(none),
	NPCM7XX_GRP(gpio),
#undef NPCM7XX_GRP
};

static struct npcm7xx_group npcm7xx_groups[] = {
#define NPCM7XX_GRP(x) { .name = #x, .pins = x ## _pins, \
			.npins = ARRAY_SIZE(x ## _pins) }
	NPCM7XX_GRPS
#undef NPCM7XX_GRP
};

#define NPCM7XX_SFUNC(a) NPCM7XX_FUNC(a, #a)
#define NPCM7XX_FUNC(a, b...) static const char *a ## _grp[] = { b }
#define NPCM7XX_MKFUNC(nm) { .name = #nm, .ngroups = ARRAY_SIZE(nm ## _grp), \
			.groups = nm ## _grp }
struct npcm7xx_func {
	const char *name;
	const unsigned int ngroups;
	const char *const *groups;
};

NPCM7XX_SFUNC(smb0);
NPCM7XX_SFUNC(smb0b);
NPCM7XX_SFUNC(smb0c);
NPCM7XX_SFUNC(smb0d);
NPCM7XX_SFUNC(smb0den);
NPCM7XX_SFUNC(smb1);
NPCM7XX_SFUNC(smb1b);
NPCM7XX_SFUNC(smb1c);
NPCM7XX_SFUNC(smb1d);
NPCM7XX_SFUNC(smb2);
NPCM7XX_SFUNC(smb2b);
NPCM7XX_SFUNC(smb2c);
NPCM7XX_SFUNC(smb2d);
NPCM7XX_SFUNC(smb3);
NPCM7XX_SFUNC(smb3b);
NPCM7XX_SFUNC(smb3c);
NPCM7XX_SFUNC(smb3d);
NPCM7XX_SFUNC(smb4);
NPCM7XX_SFUNC(smb4b);
NPCM7XX_SFUNC(smb4c);
NPCM7XX_SFUNC(smb4d);
NPCM7XX_SFUNC(smb4den);
NPCM7XX_SFUNC(smb5);
NPCM7XX_SFUNC(smb5b);
NPCM7XX_SFUNC(smb5c);
NPCM7XX_SFUNC(smb5d);
NPCM7XX_SFUNC(ga20kbc);
NPCM7XX_SFUNC(smb6);
NPCM7XX_SFUNC(smb7);
NPCM7XX_SFUNC(smb8);
NPCM7XX_SFUNC(smb9);
NPCM7XX_SFUNC(smb10);
NPCM7XX_SFUNC(smb11);
NPCM7XX_SFUNC(smb12);
NPCM7XX_SFUNC(smb13);
NPCM7XX_SFUNC(smb14);
NPCM7XX_SFUNC(smb15);
NPCM7XX_SFUNC(fanin0);
NPCM7XX_SFUNC(fanin1);
NPCM7XX_SFUNC(fanin2);
NPCM7XX_SFUNC(fanin3);
NPCM7XX_SFUNC(fanin4);
NPCM7XX_SFUNC(fanin5);
NPCM7XX_SFUNC(fanin6);
NPCM7XX_SFUNC(fanin7);
NPCM7XX_SFUNC(fanin8);
NPCM7XX_SFUNC(fanin9);
NPCM7XX_SFUNC(fanin10);
NPCM7XX_SFUNC(fanin11);
NPCM7XX_SFUNC(fanin12);
NPCM7XX_SFUNC(fanin13);
NPCM7XX_SFUNC(fanin14);
NPCM7XX_SFUNC(fanin15);
NPCM7XX_SFUNC(faninx);
NPCM7XX_SFUNC(pwm0);
NPCM7XX_SFUNC(pwm1);
NPCM7XX_SFUNC(pwm2);
NPCM7XX_SFUNC(pwm3);
NPCM7XX_SFUNC(pwm4);
NPCM7XX_SFUNC(pwm5);
NPCM7XX_SFUNC(pwm6);
NPCM7XX_SFUNC(pwm7);
NPCM7XX_SFUNC(rg1);
NPCM7XX_SFUNC(rg1mdio);
NPCM7XX_SFUNC(rg2);
NPCM7XX_SFUNC(rg2mdio);
NPCM7XX_SFUNC(ddr);
NPCM7XX_SFUNC(uart1);
NPCM7XX_SFUNC(uart2);
NPCM7XX_SFUNC(bmcuart0a);
NPCM7XX_SFUNC(bmcuart0b);
NPCM7XX_SFUNC(bmcuart1);
NPCM7XX_SFUNC(iox1);
NPCM7XX_SFUNC(iox2);
NPCM7XX_SFUNC(ioxh);
NPCM7XX_SFUNC(gspi);
NPCM7XX_SFUNC(mmc);
NPCM7XX_SFUNC(mmcwp);
NPCM7XX_SFUNC(mmccd);
NPCM7XX_SFUNC(mmcrst);
NPCM7XX_SFUNC(mmc8);
NPCM7XX_SFUNC(r1);
NPCM7XX_SFUNC(r1err);
NPCM7XX_SFUNC(r1md);
NPCM7XX_SFUNC(r2);
NPCM7XX_SFUNC(r2err);
NPCM7XX_SFUNC(r2md);
NPCM7XX_SFUNC(sd1);
NPCM7XX_SFUNC(sd1pwr);
NPCM7XX_SFUNC(wdog1);
NPCM7XX_SFUNC(wdog2);
NPCM7XX_SFUNC(scipme);
NPCM7XX_SFUNC(sci);
NPCM7XX_SFUNC(serirq);
NPCM7XX_SFUNC(jtag2);
NPCM7XX_SFUNC(spix);
NPCM7XX_SFUNC(spixcs1);
NPCM7XX_SFUNC(pspi1);
NPCM7XX_SFUNC(pspi2);
NPCM7XX_SFUNC(ddc);
NPCM7XX_SFUNC(clkreq);
NPCM7XX_SFUNC(clkout);
NPCM7XX_SFUNC(spi3);
NPCM7XX_SFUNC(spi3cs1);
NPCM7XX_SFUNC(spi3quad);
NPCM7XX_SFUNC(spi3cs2);
NPCM7XX_SFUNC(spi3cs3);
NPCM7XX_SFUNC(spi0cs1);
NPCM7XX_SFUNC(lpc);
NPCM7XX_SFUNC(lpcclk);
NPCM7XX_SFUNC(espi);
NPCM7XX_SFUNC(lkgpo0);
NPCM7XX_SFUNC(lkgpo1);
NPCM7XX_SFUNC(lkgpo2);
NPCM7XX_SFUNC(nprd_smi);
NPCM7XX_SFUNC(hgpio0);
NPCM7XX_SFUNC(hgpio1);
NPCM7XX_SFUNC(hgpio2);
NPCM7XX_SFUNC(hgpio3);
NPCM7XX_SFUNC(hgpio4);
NPCM7XX_SFUNC(hgpio5);
NPCM7XX_SFUNC(hgpio6);
NPCM7XX_SFUNC(hgpio7);

/* Function names */
static struct npcm7xx_func npcm7xx_funcs[] = {
	NPCM7XX_MKFUNC(smb0),
	NPCM7XX_MKFUNC(smb0b),
	NPCM7XX_MKFUNC(smb0c),
	NPCM7XX_MKFUNC(smb0d),
	NPCM7XX_MKFUNC(smb0den),
	NPCM7XX_MKFUNC(smb1),
	NPCM7XX_MKFUNC(smb1b),
	NPCM7XX_MKFUNC(smb1c),
	NPCM7XX_MKFUNC(smb1d),
	NPCM7XX_MKFUNC(smb2),
	NPCM7XX_MKFUNC(smb2b),
	NPCM7XX_MKFUNC(smb2c),
	NPCM7XX_MKFUNC(smb2d),
	NPCM7XX_MKFUNC(smb3),
	NPCM7XX_MKFUNC(smb3b),
	NPCM7XX_MKFUNC(smb3c),
	NPCM7XX_MKFUNC(smb3d),
	NPCM7XX_MKFUNC(smb4),
	NPCM7XX_MKFUNC(smb4b),
	NPCM7XX_MKFUNC(smb4c),
	NPCM7XX_MKFUNC(smb4d),
	NPCM7XX_MKFUNC(smb4den),
	NPCM7XX_MKFUNC(smb5),
	NPCM7XX_MKFUNC(smb5b),
	NPCM7XX_MKFUNC(smb5c),
	NPCM7XX_MKFUNC(smb5d),
	NPCM7XX_MKFUNC(ga20kbc),
	NPCM7XX_MKFUNC(smb6),
	NPCM7XX_MKFUNC(smb7),
	NPCM7XX_MKFUNC(smb8),
	NPCM7XX_MKFUNC(smb9),
	NPCM7XX_MKFUNC(smb10),
	NPCM7XX_MKFUNC(smb11),
	NPCM7XX_MKFUNC(smb12),
	NPCM7XX_MKFUNC(smb13),
	NPCM7XX_MKFUNC(smb14),
	NPCM7XX_MKFUNC(smb15),
	NPCM7XX_MKFUNC(fanin0),
	NPCM7XX_MKFUNC(fanin1),
	NPCM7XX_MKFUNC(fanin2),
	NPCM7XX_MKFUNC(fanin3),
	NPCM7XX_MKFUNC(fanin4),
	NPCM7XX_MKFUNC(fanin5),
	NPCM7XX_MKFUNC(fanin6),
	NPCM7XX_MKFUNC(fanin7),
	NPCM7XX_MKFUNC(fanin8),
	NPCM7XX_MKFUNC(fanin9),
	NPCM7XX_MKFUNC(fanin10),
	NPCM7XX_MKFUNC(fanin11),
	NPCM7XX_MKFUNC(fanin12),
	NPCM7XX_MKFUNC(fanin13),
	NPCM7XX_MKFUNC(fanin14),
	NPCM7XX_MKFUNC(fanin15),
	NPCM7XX_MKFUNC(faninx),
	NPCM7XX_MKFUNC(pwm0),
	NPCM7XX_MKFUNC(pwm1),
	NPCM7XX_MKFUNC(pwm2),
	NPCM7XX_MKFUNC(pwm3),
	NPCM7XX_MKFUNC(pwm4),
	NPCM7XX_MKFUNC(pwm5),
	NPCM7XX_MKFUNC(pwm6),
	NPCM7XX_MKFUNC(pwm7),
	NPCM7XX_MKFUNC(rg1),
	NPCM7XX_MKFUNC(rg1mdio),
	NPCM7XX_MKFUNC(rg2),
	NPCM7XX_MKFUNC(rg2mdio),
	NPCM7XX_MKFUNC(ddr),
	NPCM7XX_MKFUNC(uart1),
	NPCM7XX_MKFUNC(uart2),
	NPCM7XX_MKFUNC(bmcuart0a),
	NPCM7XX_MKFUNC(bmcuart0b),
	NPCM7XX_MKFUNC(bmcuart1),
	NPCM7XX_MKFUNC(iox1),
	NPCM7XX_MKFUNC(iox2),
	NPCM7XX_MKFUNC(ioxh),
	NPCM7XX_MKFUNC(gspi),
	NPCM7XX_MKFUNC(mmc),
	NPCM7XX_MKFUNC(mmcwp),
	NPCM7XX_MKFUNC(mmccd),
	NPCM7XX_MKFUNC(mmcrst),
	NPCM7XX_MKFUNC(mmc8),
	NPCM7XX_MKFUNC(r1),
	NPCM7XX_MKFUNC(r1err),
	NPCM7XX_MKFUNC(r1md),
	NPCM7XX_MKFUNC(r2),
	NPCM7XX_MKFUNC(r2err),
	NPCM7XX_MKFUNC(r2md),
	NPCM7XX_MKFUNC(sd1),
	NPCM7XX_MKFUNC(sd1pwr),
	NPCM7XX_MKFUNC(wdog1),
	NPCM7XX_MKFUNC(wdog2),
	NPCM7XX_MKFUNC(scipme),
	NPCM7XX_MKFUNC(sci),
	NPCM7XX_MKFUNC(serirq),
	NPCM7XX_MKFUNC(jtag2),
	NPCM7XX_MKFUNC(spix),
	NPCM7XX_MKFUNC(spixcs1),
	NPCM7XX_MKFUNC(pspi1),
	NPCM7XX_MKFUNC(pspi2),
	NPCM7XX_MKFUNC(ddc),
	NPCM7XX_MKFUNC(clkreq),
	NPCM7XX_MKFUNC(clkout),
	NPCM7XX_MKFUNC(spi3),
	NPCM7XX_MKFUNC(spi3cs1),
	NPCM7XX_MKFUNC(spi3quad),
	NPCM7XX_MKFUNC(spi3cs2),
	NPCM7XX_MKFUNC(spi3cs3),
	NPCM7XX_MKFUNC(spi0cs1),
	NPCM7XX_MKFUNC(lpc),
	NPCM7XX_MKFUNC(lpcclk),
	NPCM7XX_MKFUNC(espi),
	NPCM7XX_MKFUNC(lkgpo0),
	NPCM7XX_MKFUNC(lkgpo1),
	NPCM7XX_MKFUNC(lkgpo2),
	NPCM7XX_MKFUNC(nprd_smi),
	NPCM7XX_MKFUNC(hgpio0),
	NPCM7XX_MKFUNC(hgpio1),
	NPCM7XX_MKFUNC(hgpio2),
	NPCM7XX_MKFUNC(hgpio3),
	NPCM7XX_MKFUNC(hgpio4),
	NPCM7XX_MKFUNC(hgpio5),
	NPCM7XX_MKFUNC(hgpio6),
	NPCM7XX_MKFUNC(hgpio7),
};

#define NPCM7XX_PINCFG(a, b, c, d, e, f, g, h, i, j, k) \
	[a] { .fn0 = fn_ ## b, .reg0 = NPCM7XX_GCR_ ## c, .bit0 = d, \
			.fn1 = fn_ ## e, .reg1 = NPCM7XX_GCR_ ## f, .bit1 = g, \
			.fn2 = fn_ ## h, .reg2 = NPCM7XX_GCR_ ## i, .bit2 = j, \
			.flag = k }

/* Drive strength controlled by NPCM7XX_GP_N_ODSC */
#define DRIVE_STRENGTH_LO_SHIFT		8
#define DRIVE_STRENGTH_HI_SHIFT		12
#define DRIVE_STRENGTH_MASK		0x0000FF00

#define DS(lo, hi)	(((lo) << DRIVE_STRENGTH_LO_SHIFT) | \
			 ((hi) << DRIVE_STRENGTH_HI_SHIFT))
#define DSLO(x)		(((x) >> DRIVE_STRENGTH_LO_SHIFT) & 0xF)
#define DSHI(x)		(((x) >> DRIVE_STRENGTH_HI_SHIFT) & 0xF)

#define GPI		0x1 /* Not GPO */
#define GPO		0x2 /* Not GPI */
#define SLEW		0x4 /* Has Slew Control, NPCM7XX_GP_N_OSRC */
#define SLEWLPC		0x8 /* Has Slew Control, SRCNT.3 */

struct npcm7xx_pincfg {
	int flag;
	int fn0, reg0, bit0;
	int fn1, reg1, bit1;
	int fn2, reg2, bit2;
};

static const struct npcm7xx_pincfg pincfgs[] = {
	/*	PIN	  FUNCTION 1		   FUNCTION 2		  FUNCTION 3	    FLAGS */
	NPCM7XX_PINCFG(0,	 iox1, MFSEL1, 30,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(1,	 iox1, MFSEL1, 30,	  none, NONE, 0,	none, NONE, 0,	     DS(8, 12)),
	NPCM7XX_PINCFG(2,	 iox1, MFSEL1, 30,	  none, NONE, 0,	none, NONE, 0,	     DS(8, 12)),
	NPCM7XX_PINCFG(3,	 iox1, MFSEL1, 30,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(4,	 iox2, MFSEL3, 14,	 smb1d, I2CSEGSEL, 7,	none, NONE, 0,	     SLEW),
	NPCM7XX_PINCFG(5,	 iox2, MFSEL3, 14,	 smb1d, I2CSEGSEL, 7,	none, NONE, 0,	     SLEW),
	NPCM7XX_PINCFG(6,	 iox2, MFSEL3, 14,	 smb2d, I2CSEGSEL, 10,  none, NONE, 0,       SLEW),
	NPCM7XX_PINCFG(7,	 iox2, MFSEL3, 14,	 smb2d, I2CSEGSEL, 10,  none, NONE, 0,       SLEW),
	NPCM7XX_PINCFG(8,      lkgpo1, FLOCKR1, 4,        none, NONE, 0,	none, NONE, 0,	     DS(8, 12)),
	NPCM7XX_PINCFG(9,      lkgpo2, FLOCKR1, 8,        none, NONE, 0,	none, NONE, 0,	     DS(8, 12)),
	NPCM7XX_PINCFG(10,	 ioxh, MFSEL3, 18,	  none, NONE, 0,	none, NONE, 0,	     DS(8, 12)),
	NPCM7XX_PINCFG(11,	 ioxh, MFSEL3, 18,	  none, NONE, 0,	none, NONE, 0,	     DS(8, 12)),
	NPCM7XX_PINCFG(12,	 gspi, MFSEL1, 24,	 smb5b, I2CSEGSEL, 19,  none, NONE, 0,	     SLEW),
	NPCM7XX_PINCFG(13,	 gspi, MFSEL1, 24,	 smb5b, I2CSEGSEL, 19,  none, NONE, 0,	     SLEW),
	NPCM7XX_PINCFG(14,	 gspi, MFSEL1, 24,	 smb5c, I2CSEGSEL, 20,	none, NONE, 0,	     SLEW),
	NPCM7XX_PINCFG(15,	 gspi, MFSEL1, 24,	 smb5c, I2CSEGSEL, 20,	none, NONE, 0,	     SLEW),
	NPCM7XX_PINCFG(16,     lkgpo0, FLOCKR1, 0,        none, NONE, 0,	none, NONE, 0,	     DS(8, 12)),
	NPCM7XX_PINCFG(17,      pspi2, MFSEL3, 13,     smb4den, I2CSEGSEL, 23,  none, NONE, 0,       DS(8, 12)),
	NPCM7XX_PINCFG(18,      pspi2, MFSEL3, 13,	 smb4b, I2CSEGSEL, 14,  none, NONE, 0,	     DS(8, 12)),
	NPCM7XX_PINCFG(19,      pspi2, MFSEL3, 13,	 smb4b, I2CSEGSEL, 14,  none, NONE, 0,	     DS(8, 12)),
	NPCM7XX_PINCFG(20,	hgpio0, MFSEL2, 24,      smb15, MFSEL3, 8,      smb4c, I2CSEGSEL, 15,	     0),
	NPCM7XX_PINCFG(21,	hgpio1, MFSEL2, 25,      smb15, MFSEL3, 8,      smb4c, I2CSEGSEL, 15,	     0),
	NPCM7XX_PINCFG(22,      hgpio2, MFSEL2, 26,	 smb14, MFSEL3, 7,      smb4d, I2CSEGSEL, 16,	     0),
	NPCM7XX_PINCFG(23,      hgpio3, MFSEL2, 27,	 smb14, MFSEL3, 7,      smb4d, I2CSEGSEL, 16,	     0),
	NPCM7XX_PINCFG(24,	 hgpio4, MFSEL2, 28,	ioxh, MFSEL3, 18,	none, NONE, 0,	     DS(8, 12)),
	NPCM7XX_PINCFG(25,	 hgpio5, MFSEL2, 29,	ioxh, MFSEL3, 18,	none, NONE, 0,	     DS(8, 12)),
	NPCM7XX_PINCFG(26,	 smb5, MFSEL1, 2,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(27,	 smb5, MFSEL1, 2,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(28,	 smb4, MFSEL1, 1,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(29,	 smb4, MFSEL1, 1,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(30,	 smb3, MFSEL1, 0,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(31,	 smb3, MFSEL1, 0,	  none, NONE, 0,	none, NONE, 0,	     0),

	NPCM7XX_PINCFG(32,    spi0cs1, MFSEL1, 3,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(33,   none, NONE, 0,     none, NONE, 0,	none, NONE, 0,	     SLEW),
	NPCM7XX_PINCFG(34,   none, NONE, 0,     none, NONE, 0,	none, NONE, 0,	     SLEW),
	NPCM7XX_PINCFG(37,	smb3c, I2CSEGSEL, 12,	  none, NONE, 0,	none, NONE, 0,	     SLEW),
	NPCM7XX_PINCFG(38,	smb3c, I2CSEGSEL, 12,	  none, NONE, 0,	none, NONE, 0,	     SLEW),
	NPCM7XX_PINCFG(39,	smb3b, I2CSEGSEL, 11,	  none, NONE, 0,	none, NONE, 0,	     SLEW),
	NPCM7XX_PINCFG(40,	smb3b, I2CSEGSEL, 11,	  none, NONE, 0,	none, NONE, 0,	     SLEW),
	NPCM7XX_PINCFG(41,  bmcuart0a, MFSEL1, 9,         none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(42,  bmcuart0a, MFSEL1, 9,         none, NONE, 0,	none, NONE, 0,	     DS(2, 4) | GPO),
	NPCM7XX_PINCFG(43,      uart1, MFSEL1, 10,	 jtag2, MFSEL4, 0,  bmcuart1, MFSEL3, 24,    0),
	NPCM7XX_PINCFG(44,      uart1, MFSEL1, 10,	 jtag2, MFSEL4, 0,  bmcuart1, MFSEL3, 24,    0),
	NPCM7XX_PINCFG(45,      uart1, MFSEL1, 10,	 jtag2, MFSEL4, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(46,      uart1, MFSEL1, 10,	 jtag2, MFSEL4, 0,	none, NONE, 0,	     DS(2, 8)),
	NPCM7XX_PINCFG(47,      uart1, MFSEL1, 10,	 jtag2, MFSEL4, 0,	none, NONE, 0,	     DS(2, 8)),
	NPCM7XX_PINCFG(48,	uart2, MFSEL1, 11,   bmcuart0b, MFSEL4, 1,      none, NONE, 0,	     GPO),
	NPCM7XX_PINCFG(49,	uart2, MFSEL1, 11,   bmcuart0b, MFSEL4, 1,      none, NONE, 0,	     0),
	NPCM7XX_PINCFG(50,	uart2, MFSEL1, 11,	  none, NONE, 0,        none, NONE, 0,	     0),
	NPCM7XX_PINCFG(51,	uart2, MFSEL1, 11,	  none, NONE, 0,        none, NONE, 0,	     GPO),
	NPCM7XX_PINCFG(52,	uart2, MFSEL1, 11,	  none, NONE, 0,        none, NONE, 0,	     0),
	NPCM7XX_PINCFG(53,	uart2, MFSEL1, 11,	  none, NONE, 0,        none, NONE, 0,	     GPO),
	NPCM7XX_PINCFG(54,	uart2, MFSEL1, 11,	  none, NONE, 0,        none, NONE, 0,	     0),
	NPCM7XX_PINCFG(55,	uart2, MFSEL1, 11,	  none, NONE, 0,        none, NONE, 0,	     0),
	NPCM7XX_PINCFG(56,	r1err, MFSEL1, 12,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(57,       r1md, MFSEL1, 13,        none, NONE, 0,        none, NONE, 0,       DS(2, 4)),
	NPCM7XX_PINCFG(58,       r1md, MFSEL1, 13,        none, NONE, 0,	none, NONE, 0,	     DS(2, 4)),
	NPCM7XX_PINCFG(59,	hgpio6, MFSEL2, 30,	  smb3d, I2CSEGSEL, 13,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(60,	hgpio7, MFSEL2, 31,	  smb3d, I2CSEGSEL, 13,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(61,      uart1, MFSEL1, 10,	  none, NONE, 0,	none, NONE, 0,     GPO),
	NPCM7XX_PINCFG(62,      uart1, MFSEL1, 10,    bmcuart1, MFSEL3, 24,	none, NONE, 0,     GPO),
	NPCM7XX_PINCFG(63,      uart1, MFSEL1, 10,    bmcuart1, MFSEL3, 24,	none, NONE, 0,     GPO),

	NPCM7XX_PINCFG(64,    fanin0, MFSEL2, 0,          none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(65,    fanin1, MFSEL2, 1,          none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(66,    fanin2, MFSEL2, 2,          none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(67,    fanin3, MFSEL2, 3,          none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(68,    fanin4, MFSEL2, 4,          none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(69,    fanin5, MFSEL2, 5,          none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(70,    fanin6, MFSEL2, 6,          none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(71,    fanin7, MFSEL2, 7,          none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(72,    fanin8, MFSEL2, 8,          none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(73,    fanin9, MFSEL2, 9,          none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(74,    fanin10, MFSEL2, 10,        none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(75,    fanin11, MFSEL2, 11,        none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(76,    fanin12, MFSEL2, 12,        none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(77,    fanin13, MFSEL2, 13,        none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(78,    fanin14, MFSEL2, 14,        none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(79,    fanin15, MFSEL2, 15,        none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(80,	 pwm0, MFSEL2, 16,        none, NONE, 0,	none, NONE, 0,	     DS(4, 8)),
	NPCM7XX_PINCFG(81,	 pwm1, MFSEL2, 17,        none, NONE, 0,	none, NONE, 0,	     DS(4, 8)),
	NPCM7XX_PINCFG(82,	 pwm2, MFSEL2, 18,        none, NONE, 0,	none, NONE, 0,	     DS(4, 8)),
	NPCM7XX_PINCFG(83,	 pwm3, MFSEL2, 19,        none, NONE, 0,	none, NONE, 0,	     DS(4, 8)),
	NPCM7XX_PINCFG(84,         r2, MFSEL1, 14,        none, NONE, 0,        none, NONE, 0,	     DS(8, 12) | SLEW),
	NPCM7XX_PINCFG(85,         r2, MFSEL1, 14,        none, NONE, 0,        none, NONE, 0,	     DS(8, 12) | SLEW),
	NPCM7XX_PINCFG(86,         r2, MFSEL1, 14,        none, NONE, 0,        none, NONE, 0,	     DS(8, 12) | SLEW),
	NPCM7XX_PINCFG(87,         r2, MFSEL1, 14,        none, NONE, 0,        none, NONE, 0,	     0),
	NPCM7XX_PINCFG(88,         r2, MFSEL1, 14,        none, NONE, 0,        none, NONE, 0,	     0),
	NPCM7XX_PINCFG(89,         r2, MFSEL1, 14,        none, NONE, 0,        none, NONE, 0,	     0),
	NPCM7XX_PINCFG(90,      r2err, MFSEL1, 15,        none, NONE, 0,        none, NONE, 0,       0),
	NPCM7XX_PINCFG(91,       r2md, MFSEL1, 16,	  none, NONE, 0,        none, NONE, 0,	     DS(2, 4)),
	NPCM7XX_PINCFG(92,       r2md, MFSEL1, 16,	  none, NONE, 0,        none, NONE, 0,	     DS(2, 4)),
	NPCM7XX_PINCFG(93,    ga20kbc, MFSEL1, 17,	 smb5d, I2CSEGSEL, 21,  none, NONE, 0,	     0),
	NPCM7XX_PINCFG(94,    ga20kbc, MFSEL1, 17,	 smb5d, I2CSEGSEL, 21,  none, NONE, 0,	     0),
	NPCM7XX_PINCFG(95,	  lpc, NONE, 0,		  espi, MFSEL4, 8,      gpio, MFSEL1, 26,    0),

	NPCM7XX_PINCFG(96,	  rg1, MFSEL4, 22,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(97,	  rg1, MFSEL4, 22,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(98,	  rg1, MFSEL4, 22,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(99,	  rg1, MFSEL4, 22,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(100,	  rg1, MFSEL4, 22,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(101,	  rg1, MFSEL4, 22,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(102,	  rg1, MFSEL4, 22,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(103,	  rg1, MFSEL4, 22,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(104,	  rg1, MFSEL4, 22,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(105,	  rg1, MFSEL4, 22,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(106,	  rg1, MFSEL4, 22,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(107,	  rg1, MFSEL4, 22,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(108,   rg1mdio, MFSEL4, 21,        none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(109,   rg1mdio, MFSEL4, 21,        none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(110,       rg2, MFSEL4, 24,         ddr, MFSEL3, 26,     none, NONE, 0,       0),
	NPCM7XX_PINCFG(111,       rg2, MFSEL4, 24,         ddr, MFSEL3, 26,     none, NONE, 0,       0),
	NPCM7XX_PINCFG(112,       rg2, MFSEL4, 24,         ddr, MFSEL3, 26,     none, NONE, 0,       0),
	NPCM7XX_PINCFG(113,       rg2, MFSEL4, 24,         ddr, MFSEL3, 26,     none, NONE, 0,       0),
	NPCM7XX_PINCFG(114,	 smb0, MFSEL1, 6,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(115,	 smb0, MFSEL1, 6,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(116,	 smb1, MFSEL1, 7,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(117,	 smb1, MFSEL1, 7,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(118,	 smb2, MFSEL1, 8,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(119,	 smb2, MFSEL1, 8,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(120,	smb2c, I2CSEGSEL, 9,	  none, NONE, 0,	none, NONE, 0,	     SLEW),
	NPCM7XX_PINCFG(121,	smb2c, I2CSEGSEL, 9,      none, NONE, 0,	none, NONE, 0,	     SLEW),
	NPCM7XX_PINCFG(122,	smb2b, I2CSEGSEL, 8,	  none, NONE, 0,	none, NONE, 0,	     SLEW),
	NPCM7XX_PINCFG(123,	smb2b, I2CSEGSEL, 8,	  none, NONE, 0,	none, NONE, 0,	     SLEW),
	NPCM7XX_PINCFG(124,	smb1c, I2CSEGSEL, 6,	  none, NONE, 0,	none, NONE, 0,	     SLEW),
	NPCM7XX_PINCFG(125,	smb1c, I2CSEGSEL, 6,	  none, NONE, 0,	none, NONE, 0,	     SLEW),
	NPCM7XX_PINCFG(126,	smb1b, I2CSEGSEL, 5,	  none, NONE, 0,	none, NONE, 0,	     SLEW),
	NPCM7XX_PINCFG(127,	smb1b, I2CSEGSEL, 5,	  none, NONE, 0,	none, NONE, 0,	     SLEW),

	NPCM7XX_PINCFG(128,	 smb8, MFSEL4, 11,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(129,	 smb8, MFSEL4, 11,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(130,	 smb9, MFSEL4, 12,        none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(131,	 smb9, MFSEL4, 12,        none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(132,	smb10, MFSEL4, 13,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(133,	smb10, MFSEL4, 13,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(134,	smb11, MFSEL4, 14,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(135,	smb11, MFSEL4, 14,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(136,	  sd1, MFSEL3, 12,	  none, NONE, 0,	none, NONE, 0,	     DS(8, 12) | SLEW),
	NPCM7XX_PINCFG(137,	  sd1, MFSEL3, 12,	  none, NONE, 0,	none, NONE, 0,	     DS(8, 12) | SLEW),
	NPCM7XX_PINCFG(138,	  sd1, MFSEL3, 12,	  none, NONE, 0,	none, NONE, 0,	     DS(8, 12) | SLEW),
	NPCM7XX_PINCFG(139,	  sd1, MFSEL3, 12,	  none, NONE, 0,	none, NONE, 0,	     DS(8, 12) | SLEW),
	NPCM7XX_PINCFG(140,	  sd1, MFSEL3, 12,	  none, NONE, 0,	none, NONE, 0,	     DS(8, 12) | SLEW),
	NPCM7XX_PINCFG(141,	  sd1, MFSEL3, 12,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(142,	  sd1, MFSEL3, 12,	  none, NONE, 0,	none, NONE, 0,	     DS(8, 12) | SLEW),
	NPCM7XX_PINCFG(143,       sd1, MFSEL3, 12,      sd1pwr, MFSEL4, 5,      none, NONE, 0,       0),
	NPCM7XX_PINCFG(144,	 pwm4, MFSEL2, 20,	  none, NONE, 0,	none, NONE, 0,	     DS(4, 8)),
	NPCM7XX_PINCFG(145,	 pwm5, MFSEL2, 21,	  none, NONE, 0,	none, NONE, 0,	     DS(4, 8)),
	NPCM7XX_PINCFG(146,	 pwm6, MFSEL2, 22,	  none, NONE, 0,	none, NONE, 0,	     DS(4, 8)),
	NPCM7XX_PINCFG(147,	 pwm7, MFSEL2, 23,	  none, NONE, 0,	none, NONE, 0,	     DS(4, 8)),
	NPCM7XX_PINCFG(148,	 mmc8, MFSEL3, 11,	  none, NONE, 0,	none, NONE, 0,	     DS(8, 12) | SLEW),
	NPCM7XX_PINCFG(149,	 mmc8, MFSEL3, 11,	  none, NONE, 0,	none, NONE, 0,	     DS(8, 12) | SLEW),
	NPCM7XX_PINCFG(150,	 mmc8, MFSEL3, 11,	  none, NONE, 0,	none, NONE, 0,	     DS(8, 12) | SLEW),
	NPCM7XX_PINCFG(151,	 mmc8, MFSEL3, 11,	  none, NONE, 0,	none, NONE, 0,	     DS(8, 12) | SLEW),
	NPCM7XX_PINCFG(152,	  mmc, MFSEL3, 10,	  none, NONE, 0,	none, NONE, 0,	     DS(8, 12) | SLEW),
	NPCM7XX_PINCFG(153,     mmcwp, FLOCKR1, 24,       none, NONE, 0,	none, NONE, 0,	     0),  /* Z1/A1 */
	NPCM7XX_PINCFG(154,	  mmc, MFSEL3, 10,	  none, NONE, 0,	none, NONE, 0,	     DS(8, 12) | SLEW),
	NPCM7XX_PINCFG(155,     mmccd, MFSEL3, 25,      mmcrst, MFSEL4, 6,      none, NONE, 0,       0),  /* Z1/A1 */
	NPCM7XX_PINCFG(156,	  mmc, MFSEL3, 10,	  none, NONE, 0,	none, NONE, 0,	     DS(8, 12) | SLEW),
	NPCM7XX_PINCFG(157,	  mmc, MFSEL3, 10,	  none, NONE, 0,	none, NONE, 0,	     DS(8, 12) | SLEW),
	NPCM7XX_PINCFG(158,	  mmc, MFSEL3, 10,	  none, NONE, 0,	none, NONE, 0,	     DS(8, 12) | SLEW),
	NPCM7XX_PINCFG(159,	  mmc, MFSEL3, 10,	  none, NONE, 0,	none, NONE, 0,	     DS(8, 12) | SLEW),

	NPCM7XX_PINCFG(160,    clkout, MFSEL1, 21,        none, NONE, 0,        none, NONE, 0,	     DS(8, 12) | SLEW),
	NPCM7XX_PINCFG(161,	  lpc, NONE, 0,		  espi, MFSEL4, 8,      gpio, MFSEL1, 26,    DS(8, 12)),
	NPCM7XX_PINCFG(162,    serirq, NONE, 0,           gpio, MFSEL1, 31,	none, NONE, 0,	     DS(8, 12)),
	NPCM7XX_PINCFG(163,	  lpc, NONE, 0,		  espi, MFSEL4, 8,      gpio, MFSEL1, 26,    0),
	NPCM7XX_PINCFG(164,	  lpc, NONE, 0,		  espi, MFSEL4, 8,      gpio, MFSEL1, 26,    SLEWLPC),
	NPCM7XX_PINCFG(165,	  lpc, NONE, 0,		  espi, MFSEL4, 8,      gpio, MFSEL1, 26,    SLEWLPC),
	NPCM7XX_PINCFG(166,	  lpc, NONE, 0,		  espi, MFSEL4, 8,      gpio, MFSEL1, 26,    SLEWLPC),
	NPCM7XX_PINCFG(167,	  lpc, NONE, 0,		  espi, MFSEL4, 8,      gpio, MFSEL1, 26,    SLEWLPC),
	NPCM7XX_PINCFG(168,    lpcclk, NONE, 0,           espi, MFSEL4, 8,      gpio, MFSEL3, 16,    0),
	NPCM7XX_PINCFG(169,    scipme, MFSEL3, 0,         none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(170,	  sci, MFSEL1, 22,        none, NONE, 0,        none, NONE, 0,	     0),
	NPCM7XX_PINCFG(171,	 smb6, MFSEL3, 1,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(172,	 smb6, MFSEL3, 1,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(173,	 smb7, MFSEL3, 2,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(174,	 smb7, MFSEL3, 2,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(175,	pspi1, MFSEL3, 4,       faninx, MFSEL3, 3,      none, NONE, 0,	     DS(8, 12)),
	NPCM7XX_PINCFG(176,     pspi1, MFSEL3, 4,       faninx, MFSEL3, 3,      none, NONE, 0,	     DS(8, 12)),
	NPCM7XX_PINCFG(177,     pspi1, MFSEL3, 4,       faninx, MFSEL3, 3,      none, NONE, 0,	     DS(8, 12)),
	NPCM7XX_PINCFG(178,	   r1, MFSEL3, 9,	  none, NONE, 0,	none, NONE, 0,	     DS(8, 12) | SLEW),
	NPCM7XX_PINCFG(179,	   r1, MFSEL3, 9,	  none, NONE, 0,	none, NONE, 0,	     DS(8, 12) | SLEW),
	NPCM7XX_PINCFG(180,	   r1, MFSEL3, 9,	  none, NONE, 0,	none, NONE, 0,	     DS(8, 12) | SLEW),
	NPCM7XX_PINCFG(181,	   r1, MFSEL3, 9,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(182,	   r1, MFSEL3, 9,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(183,     spi3, MFSEL4, 16,	  none, NONE, 0,	none, NONE, 0,	     DS(8, 12) | SLEW),
	NPCM7XX_PINCFG(184,     spi3, MFSEL4, 16,	  none, NONE, 0,	none, NONE, 0,	     DS(8, 12) | SLEW | GPO),
	NPCM7XX_PINCFG(185,     spi3, MFSEL4, 16,	  none, NONE, 0,	none, NONE, 0,	     DS(8, 12) | SLEW | GPO),
	NPCM7XX_PINCFG(186,     spi3, MFSEL4, 16,	  none, NONE, 0,	none, NONE, 0,	     DS(8, 12)),
	NPCM7XX_PINCFG(187,   spi3cs1, MFSEL4, 17,        none, NONE, 0,	none, NONE, 0,	     DS(8, 12)),
	NPCM7XX_PINCFG(188,  spi3quad, MFSEL4, 20,     spi3cs2, MFSEL4, 18,     none, NONE, 0,    DS(8, 12) | SLEW),
	NPCM7XX_PINCFG(189,  spi3quad, MFSEL4, 20,     spi3cs3, MFSEL4, 19,     none, NONE, 0,    DS(8, 12) | SLEW),
	NPCM7XX_PINCFG(190,      gpio, FLOCKR1, 20,   nprd_smi, NONE, 0,	none, NONE, 0,	     DS(2, 4)),
	NPCM7XX_PINCFG(191,	 none, NONE, 0,		  none, NONE, 0,	none, NONE, 0,	     DS(8, 12)),  /* XX */

	NPCM7XX_PINCFG(192,	 none, NONE, 0,		  none, NONE, 0,	none, NONE, 0,	     DS(8, 12)),  /* XX */
	NPCM7XX_PINCFG(193,	   r1, MFSEL3, 9,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(194,	smb0b, I2CSEGSEL, 0,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(195,	smb0b, I2CSEGSEL, 0,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(196,	smb0c, I2CSEGSEL, 1,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(197,   smb0den, I2CSEGSEL, 22,     none, NONE, 0,	none, NONE, 0,	     SLEW),
	NPCM7XX_PINCFG(198,	smb0d, I2CSEGSEL, 2,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(199,	smb0d, I2CSEGSEL, 2,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(200,        r2, MFSEL1, 14,        none, NONE, 0,        none, NONE, 0,       0),
	NPCM7XX_PINCFG(201,	   r1, MFSEL3, 9,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(202,	smb0c, I2CSEGSEL, 1,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(203,    faninx, MFSEL3, 3,         none, NONE, 0,	none, NONE, 0,	     DS(8, 12)),
	NPCM7XX_PINCFG(204,	  ddc, NONE, 0,           gpio, MFSEL3, 22,	none, NONE, 0,	     SLEW),
	NPCM7XX_PINCFG(205,	  ddc, NONE, 0,           gpio, MFSEL3, 22,	none, NONE, 0,	     SLEW),
	NPCM7XX_PINCFG(206,	  ddc, NONE, 0,           gpio, MFSEL3, 22,	none, NONE, 0,	     DS(4, 8)),
	NPCM7XX_PINCFG(207,	  ddc, NONE, 0,           gpio, MFSEL3, 22,	none, NONE, 0,	     DS(4, 8)),
	NPCM7XX_PINCFG(208,       rg2, MFSEL4, 24,         ddr, MFSEL3, 26,     none, NONE, 0,       0),
	NPCM7XX_PINCFG(209,       rg2, MFSEL4, 24,         ddr, MFSEL3, 26,     none, NONE, 0,       0),
	NPCM7XX_PINCFG(210,       rg2, MFSEL4, 24,         ddr, MFSEL3, 26,     none, NONE, 0,       0),
	NPCM7XX_PINCFG(211,       rg2, MFSEL4, 24,         ddr, MFSEL3, 26,     none, NONE, 0,       0),
	NPCM7XX_PINCFG(212,       rg2, MFSEL4, 24,         ddr, MFSEL3, 26,     none, NONE, 0,       0),
	NPCM7XX_PINCFG(213,       rg2, MFSEL4, 24,         ddr, MFSEL3, 26,     none, NONE, 0,       0),
	NPCM7XX_PINCFG(214,       rg2, MFSEL4, 24,         ddr, MFSEL3, 26,     none, NONE, 0,       0),
	NPCM7XX_PINCFG(215,       rg2, MFSEL4, 24,         ddr, MFSEL3, 26,     none, NONE, 0,       0),
	NPCM7XX_PINCFG(216,   rg2mdio, MFSEL4, 23,         ddr, MFSEL3, 26,     none, NONE, 0,       0),
	NPCM7XX_PINCFG(217,   rg2mdio, MFSEL4, 23,         ddr, MFSEL3, 26,     none, NONE, 0,       0),
	NPCM7XX_PINCFG(218,     wdog1, MFSEL3, 19,        none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(219,     wdog2, MFSEL3, 20,        none, NONE, 0,	none, NONE, 0,	     DS(4, 8)),
	NPCM7XX_PINCFG(220,	smb12, MFSEL3, 5,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(221,	smb12, MFSEL3, 5,	  none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(222,     smb13, MFSEL3, 6,         none, NONE, 0,	none, NONE, 0,	     0),
	NPCM7XX_PINCFG(223,     smb13, MFSEL3, 6,         none, NONE, 0,	none, NONE, 0,	     0),

	NPCM7XX_PINCFG(224,	 spix, MFSEL4, 27,        none, NONE, 0,	none, NONE, 0,	     SLEW),
	NPCM7XX_PINCFG(225,	 spix, MFSEL4, 27,        none, NONE, 0,	none, NONE, 0,	     DS(8, 12) | SLEW | GPO),
	NPCM7XX_PINCFG(226,	 spix, MFSEL4, 27,        none, NONE, 0,	none, NONE, 0,	     DS(8, 12) | SLEW | GPO),
	NPCM7XX_PINCFG(227,	 spix, MFSEL4, 27,        none, NONE, 0,	none, NONE, 0,	     DS(8, 12) | SLEW),
	NPCM7XX_PINCFG(228,   spixcs1, MFSEL4, 28,        none, NONE, 0,	none, NONE, 0,	     DS(8, 12) | SLEW),
	NPCM7XX_PINCFG(229,	 spix, MFSEL4, 27,        none, NONE, 0,	none, NONE, 0,	     DS(8, 12) | SLEW),
	NPCM7XX_PINCFG(230,	 spix, MFSEL4, 27,        none, NONE, 0,	none, NONE, 0,	     DS(8, 12) | SLEW),
	NPCM7XX_PINCFG(231,    clkreq, MFSEL4, 9,         none, NONE, 0,        none, NONE, 0,	     DS(8, 12)),
	NPCM7XX_PINCFG(253,	 none, NONE, 0,		  none, NONE, 0,	none, NONE, 0,	     GPI), /* SDHC1 power */
	NPCM7XX_PINCFG(254,	 none, NONE, 0,		  none, NONE, 0,	none, NONE, 0,	     GPI), /* SDHC2 power */
	NPCM7XX_PINCFG(255,	 none, NONE, 0,		  none, NONE, 0,	none, NONE, 0,	     GPI), /* DACOSEL */
};

#define NPCM7XX_PIN(a, b) { .number = a, .name = b }
struct npcm7xx_pin_desc {
	unsigned int number;
	const char *name;
};

/* number, name, drv_data */
static const struct npcm7xx_pin_desc npcm7xx_pins[] = {
	NPCM7XX_PIN(0,	"GPIO0/IOX1DI"),
	NPCM7XX_PIN(1,	"GPIO1/IOX1LD"),
	NPCM7XX_PIN(2,	"GPIO2/IOX1CK"),
	NPCM7XX_PIN(3,	"GPIO3/IOX1D0"),
	NPCM7XX_PIN(4,	"GPIO4/IOX2DI/SMB1DSDA"),
	NPCM7XX_PIN(5,	"GPIO5/IOX2LD/SMB1DSCL"),
	NPCM7XX_PIN(6,	"GPIO6/IOX2CK/SMB2DSDA"),
	NPCM7XX_PIN(7,	"GPIO7/IOX2D0/SMB2DSCL"),
	NPCM7XX_PIN(8,	"GPIO8/LKGPO1"),
	NPCM7XX_PIN(9,	"GPIO9/LKGPO2"),
	NPCM7XX_PIN(10, "GPIO10/IOXHLD"),
	NPCM7XX_PIN(11, "GPIO11/IOXHCK"),
	NPCM7XX_PIN(12, "GPIO12/GSPICK/SMB5BSCL"),
	NPCM7XX_PIN(13, "GPIO13/GSPIDO/SMB5BSDA"),
	NPCM7XX_PIN(14, "GPIO14/GSPIDI/SMB5CSCL"),
	NPCM7XX_PIN(15, "GPIO15/GSPICS/SMB5CSDA"),
	NPCM7XX_PIN(16, "GPIO16/LKGPO0"),
	NPCM7XX_PIN(17, "GPIO17/PSPI2DI/SMB4DEN"),
	NPCM7XX_PIN(18, "GPIO18/PSPI2D0/SMB4BSDA"),
	NPCM7XX_PIN(19, "GPIO19/PSPI2CK/SMB4BSCL"),
	NPCM7XX_PIN(20, "GPIO20/SMB4CSDA/SMB15SDA"),
	NPCM7XX_PIN(21, "GPIO21/SMB4CSCL/SMB15SCL"),
	NPCM7XX_PIN(22, "GPIO22/SMB4DSDA/SMB14SDA"),
	NPCM7XX_PIN(23, "GPIO23/SMB4DSCL/SMB14SCL"),
	NPCM7XX_PIN(24, "GPIO24/IOXHDO"),
	NPCM7XX_PIN(25, "GPIO25/IOXHDI"),
	NPCM7XX_PIN(26, "GPIO26/SMB5SDA"),
	NPCM7XX_PIN(27, "GPIO27/SMB5SCL"),
	NPCM7XX_PIN(28, "GPIO28/SMB4SDA"),
	NPCM7XX_PIN(29, "GPIO29/SMB4SCL"),
	NPCM7XX_PIN(30, "GPIO30/SMB3SDA"),
	NPCM7XX_PIN(31, "GPIO31/SMB3SCL"),

	NPCM7XX_PIN(32, "GPIO32/nSPI0CS1"),
	NPCM7XX_PIN(33, "SPI0D2"),
	NPCM7XX_PIN(34, "SPI0D3"),
	NPCM7XX_PIN(35, "NA"),
	NPCM7XX_PIN(36, "NA"),
	NPCM7XX_PIN(37, "GPIO37/SMB3CSDA"),
	NPCM7XX_PIN(38, "GPIO38/SMB3CSCL"),
	NPCM7XX_PIN(39, "GPIO39/SMB3BSDA"),
	NPCM7XX_PIN(40, "GPIO40/SMB3BSCL"),
	NPCM7XX_PIN(41, "GPIO41/BSPRXD"),
	NPCM7XX_PIN(42, "GPO42/BSPTXD/STRAP11"),
	NPCM7XX_PIN(43, "GPIO43/RXD1/JTMS2/BU1RXD"),
	NPCM7XX_PIN(44, "GPIO44/nCTS1/JTDI2/BU1CTS"),
	NPCM7XX_PIN(45, "GPIO45/nDCD1/JTDO2"),
	NPCM7XX_PIN(46, "GPIO46/nDSR1/JTCK2"),
	NPCM7XX_PIN(47, "GPIO47/nRI1/JCP_RDY2"),
	NPCM7XX_PIN(48, "GPIO48/TXD2/BSPTXD"),
	NPCM7XX_PIN(49, "GPIO49/RXD2/BSPRXD"),
	NPCM7XX_PIN(50, "GPIO50/nCTS2"),
	NPCM7XX_PIN(51, "GPO51/nRTS2/STRAP2"),
	NPCM7XX_PIN(52, "GPIO52/nDCD2"),
	NPCM7XX_PIN(53, "GPO53/nDTR2_BOUT2/STRAP1"),
	NPCM7XX_PIN(54, "GPIO54/nDSR2"),
	NPCM7XX_PIN(55, "GPIO55/nRI2"),
	NPCM7XX_PIN(56, "GPIO56/R1RXERR"),
	NPCM7XX_PIN(57, "GPIO57/R1MDC"),
	NPCM7XX_PIN(58, "GPIO58/R1MDIO"),
	NPCM7XX_PIN(59, "GPIO59/SMB3DSDA"),
	NPCM7XX_PIN(60, "GPIO60/SMB3DSCL"),
	NPCM7XX_PIN(61, "GPO61/nDTR1_BOUT1/STRAP6"),
	NPCM7XX_PIN(62, "GPO62/nRTST1/STRAP5"),
	NPCM7XX_PIN(63, "GPO63/TXD1/STRAP4"),

	NPCM7XX_PIN(64, "GPIO64/FANIN0"),
	NPCM7XX_PIN(65, "GPIO65/FANIN1"),
	NPCM7XX_PIN(66, "GPIO66/FANIN2"),
	NPCM7XX_PIN(67, "GPIO67/FANIN3"),
	NPCM7XX_PIN(68, "GPIO68/FANIN4"),
	NPCM7XX_PIN(69, "GPIO69/FANIN5"),
	NPCM7XX_PIN(70, "GPIO70/FANIN6"),
	NPCM7XX_PIN(71, "GPIO71/FANIN7"),
	NPCM7XX_PIN(72, "GPIO72/FANIN8"),
	NPCM7XX_PIN(73, "GPIO73/FANIN9"),
	NPCM7XX_PIN(74, "GPIO74/FANIN10"),
	NPCM7XX_PIN(75, "GPIO75/FANIN11"),
	NPCM7XX_PIN(76, "GPIO76/FANIN12"),
	NPCM7XX_PIN(77, "GPIO77/FANIN13"),
	NPCM7XX_PIN(78, "GPIO78/FANIN14"),
	NPCM7XX_PIN(79, "GPIO79/FANIN15"),
	NPCM7XX_PIN(80, "GPIO80/PWM0"),
	NPCM7XX_PIN(81, "GPIO81/PWM1"),
	NPCM7XX_PIN(82, "GPIO82/PWM2"),
	NPCM7XX_PIN(83, "GPIO83/PWM3"),
	NPCM7XX_PIN(84, "GPIO84/R2TXD0"),
	NPCM7XX_PIN(85, "GPIO85/R2TXD1"),
	NPCM7XX_PIN(86, "GPIO86/R2TXEN"),
	NPCM7XX_PIN(87, "GPIO87/R2RXD0"),
	NPCM7XX_PIN(88, "GPIO88/R2RXD1"),
	NPCM7XX_PIN(89, "GPIO89/R2CRSDV"),
	NPCM7XX_PIN(90, "GPIO90/R2RXERR"),
	NPCM7XX_PIN(91, "GPIO91/R2MDC"),
	NPCM7XX_PIN(92, "GPIO92/R2MDIO"),
	NPCM7XX_PIN(93, "GPIO93/GA20/SMB5DSCL"),
	NPCM7XX_PIN(94, "GPIO94/nKBRST/SMB5DSDA"),
	NPCM7XX_PIN(95, "GPIO95/nLRESET/nESPIRST"),

	NPCM7XX_PIN(96, "GPIO96/RG1TXD0"),
	NPCM7XX_PIN(97, "GPIO97/RG1TXD1"),
	NPCM7XX_PIN(98, "GPIO98/RG1TXD2"),
	NPCM7XX_PIN(99, "GPIO99/RG1TXD3"),
	NPCM7XX_PIN(100, "GPIO100/RG1TXC"),
	NPCM7XX_PIN(101, "GPIO101/RG1TXCTL"),
	NPCM7XX_PIN(102, "GPIO102/RG1RXD0"),
	NPCM7XX_PIN(103, "GPIO103/RG1RXD1"),
	NPCM7XX_PIN(104, "GPIO104/RG1RXD2"),
	NPCM7XX_PIN(105, "GPIO105/RG1RXD3"),
	NPCM7XX_PIN(106, "GPIO106/RG1RXC"),
	NPCM7XX_PIN(107, "GPIO107/RG1RXCTL"),
	NPCM7XX_PIN(108, "GPIO108/RG1MDC"),
	NPCM7XX_PIN(109, "GPIO109/RG1MDIO"),
	NPCM7XX_PIN(110, "GPIO110/RG2TXD0/DDRV0"),
	NPCM7XX_PIN(111, "GPIO111/RG2TXD1/DDRV1"),
	NPCM7XX_PIN(112, "GPIO112/RG2TXD2/DDRV2"),
	NPCM7XX_PIN(113, "GPIO113/RG2TXD3/DDRV3"),
	NPCM7XX_PIN(114, "GPIO114/SMB0SCL"),
	NPCM7XX_PIN(115, "GPIO115/SMB0SDA"),
	NPCM7XX_PIN(116, "GPIO116/SMB1SCL"),
	NPCM7XX_PIN(117, "GPIO117/SMB1SDA"),
	NPCM7XX_PIN(118, "GPIO118/SMB2SCL"),
	NPCM7XX_PIN(119, "GPIO119/SMB2SDA"),
	NPCM7XX_PIN(120, "GPIO120/SMB2CSDA"),
	NPCM7XX_PIN(121, "GPIO121/SMB2CSCL"),
	NPCM7XX_PIN(122, "GPIO122/SMB2BSDA"),
	NPCM7XX_PIN(123, "GPIO123/SMB2BSCL"),
	NPCM7XX_PIN(124, "GPIO124/SMB1CSDA"),
	NPCM7XX_PIN(125, "GPIO125/SMB1CSCL"),
	NPCM7XX_PIN(126, "GPIO126/SMB1BSDA"),
	NPCM7XX_PIN(127, "GPIO127/SMB1BSCL"),

	NPCM7XX_PIN(128, "GPIO128/SMB8SCL"),
	NPCM7XX_PIN(129, "GPIO129/SMB8SDA"),
	NPCM7XX_PIN(130, "GPIO130/SMB9SCL"),
	NPCM7XX_PIN(131, "GPIO131/SMB9SDA"),
	NPCM7XX_PIN(132, "GPIO132/SMB10SCL"),
	NPCM7XX_PIN(133, "GPIO133/SMB10SDA"),
	NPCM7XX_PIN(134, "GPIO134/SMB11SCL"),
	NPCM7XX_PIN(135, "GPIO135/SMB11SDA"),
	NPCM7XX_PIN(136, "GPIO136/SD1DT0"),
	NPCM7XX_PIN(137, "GPIO137/SD1DT1"),
	NPCM7XX_PIN(138, "GPIO138/SD1DT2"),
	NPCM7XX_PIN(139, "GPIO139/SD1DT3"),
	NPCM7XX_PIN(140, "GPIO140/SD1CLK"),
	NPCM7XX_PIN(141, "GPIO141/SD1WP"),
	NPCM7XX_PIN(142, "GPIO142/SD1CMD"),
	NPCM7XX_PIN(143, "GPIO143/SD1CD/SD1PWR"),
	NPCM7XX_PIN(144, "GPIO144/PWM4"),
	NPCM7XX_PIN(145, "GPIO145/PWM5"),
	NPCM7XX_PIN(146, "GPIO146/PWM6"),
	NPCM7XX_PIN(147, "GPIO147/PWM7"),
	NPCM7XX_PIN(148, "GPIO148/MMCDT4"),
	NPCM7XX_PIN(149, "GPIO149/MMCDT5"),
	NPCM7XX_PIN(150, "GPIO150/MMCDT6"),
	NPCM7XX_PIN(151, "GPIO151/MMCDT7"),
	NPCM7XX_PIN(152, "GPIO152/MMCCLK"),
	NPCM7XX_PIN(153, "GPIO153/MMCWP"),
	NPCM7XX_PIN(154, "GPIO154/MMCCMD"),
	NPCM7XX_PIN(155, "GPIO155/nMMCCD/nMMCRST"),
	NPCM7XX_PIN(156, "GPIO156/MMCDT0"),
	NPCM7XX_PIN(157, "GPIO157/MMCDT1"),
	NPCM7XX_PIN(158, "GPIO158/MMCDT2"),
	NPCM7XX_PIN(159, "GPIO159/MMCDT3"),

	NPCM7XX_PIN(160, "GPIO160/CLKOUT/RNGOSCOUT"),
	NPCM7XX_PIN(161, "GPIO161/nLFRAME/nESPICS"),
	NPCM7XX_PIN(162, "GPIO162/SERIRQ"),
	NPCM7XX_PIN(163, "GPIO163/LCLK/ESPICLK"),
	NPCM7XX_PIN(164, "GPIO164/LAD0/ESPI_IO0"/*dscnt6*/),
	NPCM7XX_PIN(165, "GPIO165/LAD1/ESPI_IO1"/*dscnt6*/),
	NPCM7XX_PIN(166, "GPIO166/LAD2/ESPI_IO2"/*dscnt6*/),
	NPCM7XX_PIN(167, "GPIO167/LAD3/ESPI_IO3"/*dscnt6*/),
	NPCM7XX_PIN(168, "GPIO168/nCLKRUN/nESPIALERT"),
	NPCM7XX_PIN(169, "GPIO169/nSCIPME"),
	NPCM7XX_PIN(170, "GPIO170/nSMI"),
	NPCM7XX_PIN(171, "GPIO171/SMB6SCL"),
	NPCM7XX_PIN(172, "GPIO172/SMB6SDA"),
	NPCM7XX_PIN(173, "GPIO173/SMB7SCL"),
	NPCM7XX_PIN(174, "GPIO174/SMB7SDA"),
	NPCM7XX_PIN(175, "GPIO175/PSPI1CK/FANIN19"),
	NPCM7XX_PIN(176, "GPIO176/PSPI1DO/FANIN18"),
	NPCM7XX_PIN(177, "GPIO177/PSPI1DI/FANIN17"),
	NPCM7XX_PIN(178, "GPIO178/R1TXD0"),
	NPCM7XX_PIN(179, "GPIO179/R1TXD1"),
	NPCM7XX_PIN(180, "GPIO180/R1TXEN"),
	NPCM7XX_PIN(181, "GPIO181/R1RXD0"),
	NPCM7XX_PIN(182, "GPIO182/R1RXD1"),
	NPCM7XX_PIN(183, "GPIO183/SPI3CK"),
	NPCM7XX_PIN(184, "GPO184/SPI3D0/STRAP9"),
	NPCM7XX_PIN(185, "GPO185/SPI3D1/STRAP10"),
	NPCM7XX_PIN(186, "GPIO186/nSPI3CS0"),
	NPCM7XX_PIN(187, "GPIO187/nSPI3CS1"),
	NPCM7XX_PIN(188, "GPIO188/SPI3D2/nSPI3CS2"),
	NPCM7XX_PIN(189, "GPIO189/SPI3D3/nSPI3CS3"),
	NPCM7XX_PIN(190, "GPIO190/nPRD_SMI"),
	NPCM7XX_PIN(191, "GPIO191"),

	NPCM7XX_PIN(192, "GPIO192"),
	NPCM7XX_PIN(193, "GPIO193/R1CRSDV"),
	NPCM7XX_PIN(194, "GPIO194/SMB0BSCL"),
	NPCM7XX_PIN(195, "GPIO195/SMB0BSDA"),
	NPCM7XX_PIN(196, "GPIO196/SMB0CSCL"),
	NPCM7XX_PIN(197, "GPIO197/SMB0DEN"),
	NPCM7XX_PIN(198, "GPIO198/SMB0DSDA"),
	NPCM7XX_PIN(199, "GPIO199/SMB0DSCL"),
	NPCM7XX_PIN(200, "GPIO200/R2CK"),
	NPCM7XX_PIN(201, "GPIO201/R1CK"),
	NPCM7XX_PIN(202, "GPIO202/SMB0CSDA"),
	NPCM7XX_PIN(203, "GPIO203/FANIN16"),
	NPCM7XX_PIN(204, "GPIO204/DDC2SCL"),
	NPCM7XX_PIN(205, "GPIO205/DDC2SDA"),
	NPCM7XX_PIN(206, "GPIO206/HSYNC2"),
	NPCM7XX_PIN(207, "GPIO207/VSYNC2"),
	NPCM7XX_PIN(208, "GPIO208/RG2TXC/DVCK"),
	NPCM7XX_PIN(209, "GPIO209/RG2TXCTL/DDRV4"),
	NPCM7XX_PIN(210, "GPIO210/RG2RXD0/DDRV5"),
	NPCM7XX_PIN(211, "GPIO211/RG2RXD1/DDRV6"),
	NPCM7XX_PIN(212, "GPIO212/RG2RXD2/DDRV7"),
	NPCM7XX_PIN(213, "GPIO213/RG2RXD3/DDRV8"),
	NPCM7XX_PIN(214, "GPIO214/RG2RXC/DDRV9"),
	NPCM7XX_PIN(215, "GPIO215/RG2RXCTL/DDRV10"),
	NPCM7XX_PIN(216, "GPIO216/RG2MDC/DDRV11"),
	NPCM7XX_PIN(217, "GPIO217/RG2MDIO/DVHSYNC"),
	NPCM7XX_PIN(218, "GPIO218/nWDO1"),
	NPCM7XX_PIN(219, "GPIO219/nWDO2"),
	NPCM7XX_PIN(220, "GPIO220/SMB12SCL"),
	NPCM7XX_PIN(221, "GPIO221/SMB12SDA"),
	NPCM7XX_PIN(222, "GPIO222/SMB13SCL"),
	NPCM7XX_PIN(223, "GPIO223/SMB13SDA"),
	NPCM7XX_PIN(224, "GPIO224/SPIXCK"),
	NPCM7XX_PIN(225, "GPO225/SPIXD0/STRAP12"),
	NPCM7XX_PIN(226, "GPO226/SPIXD1/STRAP13"),
	NPCM7XX_PIN(227, "GPIO227/nSPIXCS0"),
	NPCM7XX_PIN(228, "GPIO228/nSPIXCS1"),
	NPCM7XX_PIN(229, "GPO229/SPIXD2/STRAP3"),
	NPCM7XX_PIN(230, "GPIO230/SPIXD3"),
	NPCM7XX_PIN(231, "GPIO231/nCLKREQ"),
	NPCM7XX_PIN(232, "NA"),
	NPCM7XX_PIN(233, "NA"),
	NPCM7XX_PIN(234, "NA"),
	NPCM7XX_PIN(235, "NA"),
	NPCM7XX_PIN(236, "NA"),
	NPCM7XX_PIN(237, "NA"),
	NPCM7XX_PIN(238, "NA"),
	NPCM7XX_PIN(239, "NA"),
	NPCM7XX_PIN(240, "NA"),
	NPCM7XX_PIN(241, "NA"),
	NPCM7XX_PIN(242, "NA"),
	NPCM7XX_PIN(243, "NA"),
	NPCM7XX_PIN(244, "NA"),
	NPCM7XX_PIN(245, "NA"),
	NPCM7XX_PIN(246, "NA"),
	NPCM7XX_PIN(247, "NA"),
	NPCM7XX_PIN(248, "NA"),
	NPCM7XX_PIN(249, "NA"),
	NPCM7XX_PIN(250, "NA"),
	NPCM7XX_PIN(251, "NA"),
	NPCM7XX_PIN(252, "NA"),
	NPCM7XX_PIN(253, "NA"),
	NPCM7XX_PIN(254, "NA"),
	NPCM7XX_PIN(255, "GPI255/DACOSEL"),
};

struct npcm7xx_pinctrl_priv {
	void __iomem *gpio_base;
	struct regmap *gcr_regmap;
	struct regmap *rst_regmap;
};

static int npcm7xx_pinctrl_probe(struct udevice *dev)
{
	struct npcm7xx_pinctrl_priv *priv = dev_get_priv(dev);

	priv->gpio_base = dev_read_addr_ptr(dev);
	if (!priv->gpio_base)
		return -EINVAL;

	priv->gcr_regmap = syscon_regmap_lookup_by_phandle(dev, "syscon-gcr");
	if (IS_ERR(priv->gcr_regmap))
		return -EINVAL;

	priv->rst_regmap = syscon_regmap_lookup_by_phandle(dev, "syscon-rst");
	if (IS_ERR(priv->rst_regmap))
		return -EINVAL;

	return 0;
}

/* Enable mode in pin group */
static void npcm7xx_setfunc(struct udevice *dev, const int *pin,
			    int pin_number, int mode)
{
	struct npcm7xx_pinctrl_priv *priv = dev_get_priv(dev);
	const struct npcm7xx_pincfg *cfg;
	int i;

	for (i = 0 ; i < pin_number ; i++) {
		cfg = &pincfgs[pin[i]];
		if (mode == fn_gpio || cfg->fn0 == mode || cfg->fn1 == mode || cfg->fn2 == mode) {
			if (cfg->reg0) {
				if (cfg->fn0 == mode)
					regmap_update_bits(priv->gcr_regmap, cfg->reg0, BIT(cfg->bit0), BIT(cfg->bit0));
				else
					regmap_update_bits(priv->gcr_regmap, cfg->reg0, BIT(cfg->bit0), 0);
			}
			if (cfg->reg1) {
				if (cfg->fn1 == mode)
					regmap_update_bits(priv->gcr_regmap, cfg->reg1, BIT(cfg->bit1), BIT(cfg->bit1));
				else
					regmap_update_bits(priv->gcr_regmap, cfg->reg1, BIT(cfg->bit1), 0);
			}
			if (cfg->reg2) {
				if (cfg->fn2 == mode)
					regmap_update_bits(priv->gcr_regmap, cfg->reg2, BIT(cfg->bit2), BIT(cfg->bit2));
				else
					regmap_update_bits(priv->gcr_regmap, cfg->reg2, BIT(cfg->bit2), 0);
			}
		}
	}
}

static int npcm7xx_get_pins_count(struct udevice *dev)
{
	return ARRAY_SIZE(npcm7xx_pins);
}

static const char *npcm7xx_get_pin_name(struct udevice *dev,
					unsigned int selector)
{
	return npcm7xx_pins[selector].name;
}

static int npcm7xx_get_groups_count(struct udevice *dev)
{
	return ARRAY_SIZE(npcm7xx_groups);
}

static const char *npcm7xx_get_group_name(struct udevice *dev,
					  unsigned int selector)
{
	return npcm7xx_groups[selector].name;
}

static int npcm7xx_get_functions_count(struct udevice *dev)
{
	return ARRAY_SIZE(npcm7xx_funcs);
}

static const char *npcm7xx_get_function_name(struct udevice *dev,
					     unsigned int selector)
{
	return npcm7xx_funcs[selector].name;
}

static int npcm7xx_pinmux_set(struct udevice *dev,
			unsigned int group,
			unsigned int function)
{
	dev_dbg(dev, "set_mux: %d, %d[%s]\n", function, group,
		npcm7xx_groups[group].name);

	npcm7xx_setfunc(dev, npcm7xx_groups[group].pins,
			npcm7xx_groups[group].npins, group);

	return 0;
}

#if CONFIG_IS_ENABLED(PINCONF)

#define PIN_CONFIG_PERSIST_STATE (PIN_CONFIG_END + 1)
#define PIN_CONFIG_POLARITY_STATE (PIN_CONFIG_END + 2)
#define PIN_CONFIG_EVENT_CLEAR (PIN_CONFIG_END + 3)

static const struct pinconf_param npcm7xx_conf_params[] = {
	{ "bias-disable", PIN_CONFIG_BIAS_DISABLE, 0 },
	{ "bias-pull-up", PIN_CONFIG_BIAS_PULL_UP, 1 },
	{ "bias-pull-down", PIN_CONFIG_BIAS_PULL_DOWN, 1 },
	{ "input-enable", PIN_CONFIG_INPUT_ENABLE, 1 },
	{ "output-enable", PIN_CONFIG_OUTPUT_ENABLE, 1 },
	{ "output-high", PIN_CONFIG_OUTPUT, 1, },
	{ "output-low", PIN_CONFIG_OUTPUT, 0, },
	{ "drive-open-drain", PIN_CONFIG_DRIVE_OPEN_DRAIN, 1 },
	{ "drive-push-pull", PIN_CONFIG_DRIVE_PUSH_PULL, 1 },
	{ "persist-enable", PIN_CONFIG_PERSIST_STATE, 1 },
	{ "persist-disable", PIN_CONFIG_PERSIST_STATE, 0 },
	{ "input-debounce", PIN_CONFIG_INPUT_DEBOUNCE, 0 },
	{ "active-high", PIN_CONFIG_POLARITY_STATE, 0 },
	{ "active-low", PIN_CONFIG_POLARITY_STATE, 1 },
	{ "drive-strength", PIN_CONFIG_DRIVE_STRENGTH, 0 },
	{ "slew-rate", PIN_CONFIG_SLEW_RATE, 0},
	{ "event-clear", PIN_CONFIG_EVENT_CLEAR, 0},
};

static bool is_gpio_persist(struct udevice *dev, u8 bank)
{
	struct npcm7xx_pinctrl_priv *priv = dev_get_priv(dev);
	u32 value, tmp;

	u8 offset = bank + GPIOX_MODULE_RESET;
	u32 mask = 1 << offset;

	regmap_read(priv->gcr_regmap, NPCM7XX_GCR_RESSR, &value);
	if (value == 0) {
		regmap_read(priv->gcr_regmap, NPCM7XX_GCR_INTCR2, &tmp);
		value = ~tmp;
	}

	dev_dbg(dev, "reboot reason: 0x%x\n", value);

	if (value & CORST)
		regmap_read(priv->rst_regmap, NPCM7XX_RST_CORSTC, &tmp);
	else if (value & WD0RST)
		regmap_read(priv->rst_regmap, NPCM7XX_RST_WD0RCR, &tmp);
	else if (value & WD1RST)
		regmap_read(priv->rst_regmap, NPCM7XX_RST_WD1RCR, &tmp);
	else if (value & WD2RST)
		regmap_read(priv->rst_regmap, NPCM7XX_RST_WD2RCR, &tmp);
	else if (value & SWRST1)
                regmap_read(priv->rst_regmap, NPCM7XX_RST_SWRSTC1, &tmp);
	else if (value & SWRST2)
                regmap_read(priv->rst_regmap, NPCM7XX_RST_SWRSTC2, &tmp);
	else if (value & SWRST3)
                regmap_read(priv->rst_regmap, NPCM7XX_RST_SWRSTC3, &tmp);
	else if (value & SW4RST)
                regmap_read(priv->rst_regmap, NPCM7XX_RST_SWRSTC4, &tmp);
	else
		return false;

	return !((tmp & mask) >> offset);
}

static int npcm7xx_gpio_reset_persist(struct udevice *dev, unsigned int banknum, int enable)
{
	struct npcm7xx_pinctrl_priv *priv = dev_get_priv(dev);
	u32 num = GPIOX_MODULE_RESET + banknum;

	dev_dbg(dev, "set gpio persist, bank %d, enable %d\n", banknum, enable);

	if (enable) {
		regmap_update_bits(priv->rst_regmap, NPCM7XX_RST_WD0RCR, BIT(num), 0);
		regmap_update_bits(priv->rst_regmap, NPCM7XX_RST_WD1RCR, BIT(num), 0);
		regmap_update_bits(priv->rst_regmap, NPCM7XX_RST_WD2RCR, BIT(num), 0);
		regmap_update_bits(priv->rst_regmap, NPCM7XX_RST_CORSTC, BIT(num), 0);
		regmap_update_bits(priv->rst_regmap, NPCM7XX_RST_SWRSTC1, BIT(num), 0);
		regmap_update_bits(priv->rst_regmap, NPCM7XX_RST_SWRSTC2, BIT(num), 0);
		regmap_update_bits(priv->rst_regmap, NPCM7XX_RST_SWRSTC3, BIT(num), 0);
		regmap_update_bits(priv->rst_regmap, NPCM7XX_RST_SWRSTC4, BIT(num), 0);
	} else {
		regmap_update_bits(priv->rst_regmap, NPCM7XX_RST_WD0RCR, BIT(num) | CA9C_RESET, BIT(num) | CA9C_RESET);
		regmap_update_bits(priv->rst_regmap, NPCM7XX_RST_WD1RCR, BIT(num) | CA9C_RESET, BIT(num) | CA9C_RESET);
		regmap_update_bits(priv->rst_regmap, NPCM7XX_RST_WD2RCR, BIT(num) | CA9C_RESET, BIT(num) | CA9C_RESET);
		regmap_update_bits(priv->rst_regmap, NPCM7XX_RST_CORSTC, BIT(num) | CA9C_RESET, BIT(num) | CA9C_RESET);
		regmap_update_bits(priv->rst_regmap, NPCM7XX_RST_SWRSTC1, BIT(num) | CA9C_RESET, BIT(num) | CA9C_RESET);
		regmap_update_bits(priv->rst_regmap, NPCM7XX_RST_SWRSTC2, BIT(num) | CA9C_RESET, BIT(num) | CA9C_RESET);
		regmap_update_bits(priv->rst_regmap, NPCM7XX_RST_SWRSTC3, BIT(num) | CA9C_RESET, BIT(num) | CA9C_RESET);
		regmap_update_bits(priv->rst_regmap, NPCM7XX_RST_SWRSTC4, BIT(num) | CA9C_RESET, BIT(num) | CA9C_RESET);
	}

	return 0;
}

/* Set drive strength for a pin, if supported */
static int npcm7xx_set_drive_strength(struct udevice *dev,
				      unsigned int pin, int nval)
{
	struct npcm7xx_pinctrl_priv *priv = dev_get_priv(dev);
	int bank = pin / NPCM7XX_GPIO_PER_BANK;
	int gpio = (pin % NPCM7XX_GPIO_PER_BITS);
	void __iomem *base = priv->gpio_base + (NPCM7XX_GPIO_BANK_OFFSET * bank);
	int v;

	v = (pincfgs[pin].flag & DRIVE_STRENGTH_MASK);
	if (!nval || !v)
		return -ENOTSUPP;

	if (DSLO(v) == nval) {
		dev_dbg(dev,
			"setting pin %d to low strength [%d]\n", pin, nval);
		clrbits_le32(base + NPCM7XX_GP_N_ODSC, BIT(gpio));
		return 0;
	} else if (DSHI(v) == nval) {
		dev_dbg(dev,
			"setting pin %d to high strength [%d]\n", pin, nval);
		setbits_le32(base + NPCM7XX_GP_N_ODSC, BIT(gpio));
		return 0;
	}

	return -ENOTSUPP;
}

/* Set slew rate of pin (high/low) */
static int npcm7xx_set_slew_rate(struct udevice *dev, unsigned int pin,
				 int arg)
{
	struct npcm7xx_pinctrl_priv *priv = dev_get_priv(dev);
	int bank = pin / NPCM7XX_GPIO_PER_BANK;
	int gpio = (pin % NPCM7XX_GPIO_PER_BITS);
	void __iomem *base = priv->gpio_base + (NPCM7XX_GPIO_BANK_OFFSET * bank);

	if (pincfgs[pin].flag & SLEW) {
		switch (arg) {
		case 0:
			dev_dbg(dev,
				"setting pin %d slew rate to low\n", pin);
			clrbits_le32(base + NPCM7XX_GP_N_OSRC, BIT(gpio));
			return 0;
		case 1:
			dev_dbg(dev,
				"setting pin %d slew rate to high\n", pin);
			setbits_le32(base + NPCM7XX_GP_N_OSRC, BIT(gpio));
			return 0;
		default:
			return -ENOTSUPP;
		}
	}

	/* LPC Slew rate in SRCNT register */
	if (pincfgs[pin].flag & SLEWLPC) {
		switch (arg) {
		case 0:
			dev_dbg(dev,
				"setting LPC/ESPI(%d) slew rate to low\n", pin);
			regmap_update_bits(priv->gcr_regmap, NPCM7XX_GCR_SRCNT, SRCNT_ESPI, 0);
			return 0;
		case 1:
			dev_dbg(dev, "setting LPC/ESPI(%d) slew rate to high\n", pin);
			regmap_update_bits(priv->gcr_regmap, NPCM7XX_GCR_SRCNT, SRCNT_ESPI, SRCNT_ESPI);
			return 0;
		default:
			return -ENOTSUPP;
		}
	}

	return -ENOTSUPP;
}

static int npcm7xx_pinconf_set(struct udevice *dev, unsigned int pin,
			       unsigned int param, unsigned int arg)
{
	struct npcm7xx_pinctrl_priv *priv = dev_get_priv(dev);
	int err = 0;
	int bank = pin / NPCM7XX_GPIO_PER_BANK;
	int gpio = (pin % NPCM7XX_GPIO_PER_BITS);
	void __iomem *base = priv->gpio_base + (0x1000 * bank);

	npcm7xx_setfunc(dev, (const int *)&pin, 1, fn_gpio);

	/* To prevent unexpected IRQ trap at verctor 00 in linux kernel */
	if (param == PIN_CONFIG_EVENT_CLEAR) {
		dev_dbg(dev, "set pin %d event clear\n", pin);
		clrbits_le32(base + NPCM7XX_GP_N_EVEN, BIT(gpio));
		setbits_le32(base + NPCM7XX_GP_N_EVST, BIT(gpio));
		return err;
	}

	// allow set persist state disable
	if (param == PIN_CONFIG_PERSIST_STATE) {
		npcm7xx_gpio_reset_persist(dev, bank, arg);
		return err;
	}

	if (is_gpio_persist(dev, bank))
		return err;

	switch (param) {
	case PIN_CONFIG_BIAS_DISABLE:
		dev_dbg(dev, "set pin %d bias dsiable\n", pin);
		clrbits_le32(base + NPCM7XX_GP_N_PU, BIT(gpio));
		clrbits_le32(base + NPCM7XX_GP_N_PD, BIT(gpio));
		break;
	case PIN_CONFIG_BIAS_PULL_DOWN:
		dev_dbg(dev, "set pin %d bias pull down\n", pin);
		clrbits_le32(base + NPCM7XX_GP_N_PU, BIT(gpio));
		setbits_le32(base + NPCM7XX_GP_N_PD, BIT(gpio));
		break;
	case PIN_CONFIG_BIAS_PULL_UP:
		dev_dbg(dev, "set pin %d bias pull up\n", pin);
		setbits_le32(base + NPCM7XX_GP_N_PU, BIT(gpio));
		clrbits_le32(base + NPCM7XX_GP_N_PD, BIT(gpio));
		break;
	case PIN_CONFIG_INPUT_ENABLE:
		dev_dbg(dev, "set pin %d input enable\n", pin);
		setbits_le32(base + NPCM7XX_GP_N_OEC, BIT(gpio));
		setbits_le32(base + NPCM7XX_GP_N_IEM, BIT(gpio));
		break;
	case PIN_CONFIG_OUTPUT_ENABLE:
		dev_dbg(dev, "set pin %d output enable\n", pin);
		clrbits_le32(base + NPCM7XX_GP_N_IEM, BIT(gpio));
		setbits_le32(base + NPCM7XX_GP_N_OES, BIT(gpio));
	case PIN_CONFIG_OUTPUT:
		dev_dbg(dev, "set pin %d output %d\n", pin, arg);
		if (arg)
			setbits_le32(base + NPCM7XX_GP_N_DOUT, BIT(gpio));
		else
			clrbits_le32(base + NPCM7XX_GP_N_DOUT, BIT(gpio));
		clrbits_le32(base + NPCM7XX_GP_N_IEM, BIT(gpio));
		setbits_le32(base + NPCM7XX_GP_N_OES, BIT(gpio));
		break;
	case PIN_CONFIG_DRIVE_PUSH_PULL:
		dev_dbg(dev, "set pin %d push pull\n", pin);
		clrbits_le32(base + NPCM7XX_GP_N_OTYP, BIT(gpio));
		break;
	case PIN_CONFIG_DRIVE_OPEN_DRAIN:
		dev_dbg(dev, "set pin %d open drain\n", pin);
		setbits_le32(base + NPCM7XX_GP_N_OTYP, BIT(gpio));
		break;
	case PIN_CONFIG_INPUT_DEBOUNCE:
		dev_dbg(dev, "set pin %d input debounce\n", pin);
		setbits_le32(base + NPCM7XX_GP_N_DBNC, BIT(gpio));
		break;
	case PIN_CONFIG_POLARITY_STATE:
		dev_dbg(dev, "set pin %d active %d\n", pin, arg);
		if (arg)
			setbits_le32(base + NPCM7XX_GP_N_POL, BIT(gpio));
		else
			clrbits_le32(base + NPCM7XX_GP_N_POL, BIT(gpio));
		break;
	case PIN_CONFIG_DRIVE_STRENGTH:
		dev_dbg(dev, "set pin %d driver strength %d\n", pin, arg);
		err = npcm7xx_set_drive_strength(dev, pin, arg);
		break;
	case PIN_CONFIG_SLEW_RATE:
		dev_dbg(dev, "set pin %d slew rate %d\n", pin, arg);
		err = npcm7xx_set_slew_rate(dev, pin, arg);
		break;
	default:
		err = -ENOTSUPP;
	}
	return err;
}

#endif

static struct pinctrl_ops npcm7xx_pinctrl_ops = {
	.set_state	= pinctrl_generic_set_state,
	.get_pins_count = npcm7xx_get_pins_count,
	.get_pin_name = npcm7xx_get_pin_name,
	.get_groups_count = npcm7xx_get_groups_count,
	.get_group_name = npcm7xx_get_group_name,
	.get_functions_count = npcm7xx_get_functions_count,
	.get_function_name = npcm7xx_get_function_name,
	.pinmux_set = npcm7xx_pinmux_set,
	.pinmux_group_set = npcm7xx_pinmux_set,
#if CONFIG_IS_ENABLED(PINCONF)
	.pinconf_num_params = ARRAY_SIZE(npcm7xx_conf_params),
	.pinconf_params = npcm7xx_conf_params,
	.pinconf_set = npcm7xx_pinconf_set,
	.pinconf_group_set = npcm7xx_pinconf_set,
#endif
};

static const struct udevice_id npcm7xx_pinctrl_ids[] = {
	{ .compatible = "nuvoton,npcm750-pinctrl" },
	{ }
};

U_BOOT_DRIVER(pinctrl_npcm7xx) = {
	.name = "nuvoton_npcm7xx_pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = npcm7xx_pinctrl_ids,
	.priv_auto = sizeof(struct npcm7xx_pinctrl_priv),
	.ops = &npcm7xx_pinctrl_ops,
	.probe = npcm7xx_pinctrl_probe,
};
