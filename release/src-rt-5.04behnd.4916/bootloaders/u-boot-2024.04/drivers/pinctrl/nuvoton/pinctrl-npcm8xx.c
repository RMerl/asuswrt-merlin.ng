// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Nuvoton Technology Corp.
 */

#include <dm.h>
#include <errno.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <dm/pinctrl.h>
#include <linux/bitfield.h>
#include <asm/arch/rst.h>

/* GCR register offsets */
#define WD0RCR		0x38
#define WD1RCR		0x3c
#define WD2RCR		0x40
#define SWRSTC1		0x44
#define SWRSTC2		0x48
#define SWRSTC3		0x4c
#define TIPRSTC		0x50
#define CORSTC		0x5c
#define FLOCKR1		0x74
#define INTCR4		0xc0
#define I2CSEGSEL	0xe0
#define MFSEL1		0x260
#define MFSEL2		0x264
#define MFSEL3		0x268
#define MFSEL4		0x26c
#define MFSEL5		0x270
#define MFSEL6		0x274
#define MFSEL7		0x278

/* GPIO register offsets */
#define GPIO_POL	0x08 /* Polarity */
#define GPIO_DOUT	0x0c /* Data OUT */
#define GPIO_OTYP	0x14 /* Output Type */
#define GPIO_PU		0x1c /* Pull-up */
#define GPIO_PD		0x20 /* Pull-down */
#define GPIO_DBNC	0x24 /* Debounce */
#define GPIO_EVEN	0x40 /* Event Enable */
#define GPIO_EVST	0x4c /* Event Status */
#define GPIO_IEM	0x58 /* Input Enable */
#define GPIO_OSRC	0x5c /* Output Slew-Rate Control */
#define GPIO_ODSC	0x60 /* Output Drive Strength Control */
#define GPIO_OES	0x70 /* Output Enable Set */
#define GPIO_OEC	0x74 /* Output Enable Clear */

#define NPCM8XX_GPIO_PER_BANK	32
#define GPIOX_OFFSET	16

struct npcm8xx_pinctrl_priv {
	void __iomem *gpio_base;
	struct regmap *gcr_regmap;
	struct regmap *rst_regmap;
};

/*
 * Function table
 * name, register, enable bit, pin list
 */
#define FUNC_LIST \
	FUNC(smb3, MFSEL1, 0, 30, 31) \
	FUNC(smb4, MFSEL1, 1, 28, 29) \
	FUNC(smb5, MFSEL1, 2, 26, 27) \
	FUNC(spi0cs1, MFSEL1, 3, 32) \
	FUNC(hsi1c, MFSEL1, 4, 45, 46, 47, 61) \
	FUNC(hsi2c, MFSEL1, 5, 52, 53, 54, 55) \
	FUNC(smb0, MFSEL1, 6, 114, 115) \
	FUNC(smb1, MFSEL1, 7, 116, 117) \
	FUNC(smb2, MFSEL1, 8, 118, 119) \
	FUNC(bmcuart0a, MFSEL1, 9, 41, 42) \
	FUNC(hsi1a, MFSEL1, 10, 43, 63) \
	FUNC(hsi2a, MFSEL1, 11, 48, 49) \
	FUNC(r1err, MFSEL1, 12, 56) \
	FUNC(r1md, MFSEL1, 13, 57, 58) \
	FUNC(r2, MFSEL1, 14, 84, 85, 86, 87, 88, 89, 200) \
	FUNC(r2err, MFSEL1, 15, 90) \
	FUNC(r2md, MFSEL1, 16, 91, 92) \
	FUNC(ga20kbc, MFSEL1, 17, 93, 94) \
	FUNC(clkout, MFSEL1, 21, 160) \
	FUNC(sci, MFSEL1, 22, 170) \
	FUNC(gspi, MFSEL1, 24, 12, 13, 14, 15) \
	FUNC(lpc, MFSEL1, 26, 95, 161, 163, 164, 165, 166, 167) \
	FUNC(hsi1b, MFSEL1, 28, 44, 62) \
	FUNC(hsi2b, MFSEL1, 29, 50, 51) \
	FUNC(iox1, MFSEL1, 30, 0, 1, 2, 3) \
	FUNC(serirq, MFSEL1, 31, 168) \
	FUNC(fanin0, MFSEL2, 0, 64) \
	FUNC(fanin1, MFSEL2, 1, 65) \
	FUNC(fanin2, MFSEL2, 2, 66) \
	FUNC(fanin3, MFSEL2, 3, 67) \
	FUNC(fanin4, MFSEL2, 4, 68) \
	FUNC(fanin5, MFSEL2, 5, 69) \
	FUNC(fanin6, MFSEL2, 6, 70) \
	FUNC(fanin7, MFSEL2, 7, 71) \
	FUNC(fanin8, MFSEL2, 8, 72) \
	FUNC(fanin9, MFSEL2, 9, 73) \
	FUNC(fanin10, MFSEL2, 10, 74) \
	FUNC(fanin11, MFSEL2, 11, 75) \
	FUNC(fanin12, MFSEL2, 12, 76) \
	FUNC(fanin13, MFSEL2, 13, 77) \
	FUNC(fanin14, MFSEL2, 14, 78) \
	FUNC(fanin15, MFSEL2, 15, 79) \
	FUNC(pwm0, MFSEL2, 16, 80) \
	FUNC(pwm1, MFSEL2, 17, 81) \
	FUNC(pwm2, MFSEL2, 18, 82) \
	FUNC(pwm3, MFSEL2, 19, 83) \
	FUNC(pwm4, MFSEL2, 20, 144) \
	FUNC(pwm5, MFSEL2, 21, 145) \
	FUNC(pwm6, MFSEL2, 22, 146) \
	FUNC(pwm7, MFSEL2, 23, 147) \
	FUNC(hgpio0, MFSEL2, 24, 20) \
	FUNC(hgpio1, MFSEL2, 25, 21) \
	FUNC(hgpio2, MFSEL2, 26, 22) \
	FUNC(hgpio3, MFSEL2, 27, 23) \
	FUNC(hgpio4, MFSEL2, 28, 24) \
	FUNC(hgpio5, MFSEL2, 29, 25) \
	FUNC(hgpio6, MFSEL2, 30, 59) \
	FUNC(hgpio7, MFSEL2, 31, 60) \
	FUNC(scipme, MFSEL3, 0, 169) \
	FUNC(smb6, MFSEL3, 1, 171, 172) \
	FUNC(smb7, MFSEL3, 2, 173, 174) \
	FUNC(faninx, MFSEL3, 3, 175, 176, 177, 203) \
	FUNC(spi1, MFSEL3, 4, 175, 176, 177, 203) \
	FUNC(smb12, MFSEL3, 5, 220, 221) \
	FUNC(smb13, MFSEL3, 6, 222, 223) \
	FUNC(smb14, MFSEL3, 7, 22, 23) \
	FUNC(smb15, MFSEL3, 8, 20, 21) \
	FUNC(r1,	MFSEL3,	9, 178, 179, 180, 181, 182, 193, 201) \
	FUNC(mmc, MFSEL3, 10, 152, 154, 156, 157, 158, 159) \
	FUNC(mmc8, MFSEL3, 11, 148, 149, 150, 151) \
	FUNC(pspi, MFSEL3, 13, 17, 18, 19) \
	FUNC(iox2, MFSEL3, 14, 4, 5, 6, 7) \
	FUNC(clkrun, MFSEL3, 16, 162) \
	FUNC(ioxh, MFSEL3, 18, 10, 11, 24, 25) \
	FUNC(wdog1, MFSEL3, 19, 218) \
	FUNC(wdog2, MFSEL3, 20, 219) \
	FUNC(i3c5, MFSEL3, 22, 106, 107) \
	FUNC(bmcuart1, MFSEL3, 24, 43, 63) \
	FUNC(mmccd, MFSEL3, 25, 155) \
	FUNC(ddr, MFSEL3, 26, 110, 111, 112, 113, 208, 209, 210, 211, 212,\
			     213, 214, 215, 216, 217, 250) \
	FUNC(jtag2, MFSEL4, 0, 43, 44, 45, 46, 47) \
	FUNC(bmcuart0b, MFSEL4, 1, 48, 49) \
	FUNC(mmcrst, MFSEL4, 6, 155) \
	FUNC(espi, MFSEL4, 8, 95, 161, 163, 164, 165, 166, 167, 168) \
	FUNC(clkreq, MFSEL4, 9, 231) \
	FUNC(smb8, MFSEL4, 11, 128, 129) \
	FUNC(smb9, MFSEL4, 12, 130, 131) \
	FUNC(smb10, MFSEL4, 13, 132, 133) \
	FUNC(smb11, MFSEL4, 14, 134, 135) \
	FUNC(spi3, MFSEL4, 16, 183, 184, 185, 186) \
	FUNC(spi3cs1, MFSEL4, 17, 187) \
	FUNC(spi3cs2, MFSEL4, 18, 188) \
	FUNC(spi3cs3, MFSEL4, 19, 189) \
	FUNC(spi3quad, MFSEL4, 20, 188, 189) \
	FUNC(rg1mdio, MFSEL4, 21, 108, 109) \
	FUNC(bu2, MFSEL4, 22, 96, 97) \
	FUNC(rg2mdio, MFSEL4, 23, 216, 217) \
	FUNC(rg2, MFSEL4, 24, 110, 111, 112, 113, 208, 209, 210, 211, 212,\
			     213, 214, 215) \
	FUNC(spix, MFSEL4, 27, 224, 225, 226, 227, 229, 230) \
	FUNC(spixcs1, MFSEL4, 28, 228) \
	FUNC(spi1cs1, MFSEL5, 0, 233) \
	FUNC(jm2, MFSEL5, 1) \
	FUNC(j2j3, MFSEL5, 2, 44, 62, 45, 46) \
	FUNC(spi1d23, MFSEL5, 3, 191, 192) \
	FUNC(spi1cs2, MFSEL5, 4, 191) \
	FUNC(spi1cs3, MFSEL5, 5, 192) \
	FUNC(bu6, MFSEL5, 6, 50, 51) \
	FUNC(bu5, MFSEL5, 7, 52, 53) \
	FUNC(bu4, MFSEL5, 8, 54, 55) \
	FUNC(r1oen, MFSEL5, 9, 56) \
	FUNC(r2oen, MFSEL5, 10, 90) \
	FUNC(rmii3, MFSEL5, 11, 110, 111, 209, 210, 211, 214, 215) \
	FUNC(bu5b, MFSEL5, 12, 100, 101) \
	FUNC(bu4b, MFSEL5, 13, 98, 99) \
	FUNC(r3oen, MFSEL5, 14, 213) \
	FUNC(jm1, MFSEL5, 15, 136, 137, 138, 139, 140) \
	FUNC(gpi35, MFSEL5, 16, 35) \
	FUNC(i3c0, MFSEL5, 17, 240, 241) \
	FUNC(gpi36, MFSEL5, 18, 36) \
	FUNC(i3c1, MFSEL5, 19, 242, 243) \
	FUNC(tp_gpio4b, MFSEL5, 20, 57) \
	FUNC(i3c2, MFSEL5, 21, 244, 245) \
	FUNC(tp_gpio5b, MFSEL5, 22, 58) \
	FUNC(i3c3, MFSEL5, 23, 246, 247) \
	FUNC(smb16, MFSEL5, 24, 10, 11) \
	FUNC(smb17, MFSEL5, 25, 2, 3) \
	FUNC(smb18, MFSEL5, 26, 0, 1) \
	FUNC(smb19, MFSEL5, 27, 59, 60) \
	FUNC(smb20, MFSEL5, 28, 234, 235) \
	FUNC(smb21, MFSEL5, 29, 169, 170) \
	FUNC(smb22, MFSEL5, 30, 39, 40) \
	FUNC(smb23, MFSEL5, 31, 37, 38) \
	FUNC(smb23b, MFSEL6, 0, 134, 135) \
	FUNC(cp1utxd, MFSEL6, 1, 42) \
	FUNC(cp1gpio0, MFSEL6, 2) \
	FUNC(cp1gpio1, MFSEL6, 3) \
	FUNC(cp1gpio2, MFSEL6, 4) \
	FUNC(cp1gpio3, MFSEL6, 5) \
	FUNC(cp1gpio4, MFSEL6, 6) \
	FUNC(cp1gpio5, MFSEL6, 7, 17) \
	FUNC(cp1gpio6, MFSEL6, 8, 91) \
	FUNC(cp1gpio7, MFSEL6, 9, 92) \
	FUNC(i3c4, MFSEL6, 10, 33, 34) \
	FUNC(pwm8, MFSEL6, 11, 220) \
	FUNC(pwm9, MFSEL6, 12, 221) \
	FUNC(pwm10, MFSEL6, 13, 234) \
	FUNC(pwm11, MFSEL6, 14, 235) \
	FUNC(nbu1crts, MFSEL6, 15, 44, 62) \
	FUNC(fm0, MFSEL6, 16, 194, 195, 196, 202, 199, 198, 197) \
	FUNC(fm1, MFSEL6, 17, 175, 176, 177, 203, 191, 192, 233) \
	FUNC(fm2, MFSEL6, 18, 224, 225, 226, 227, 228, 229, 230) \
	FUNC(gpio1836, MFSEL6, 19, 183, 184, 185, 186) \
	FUNC(cp1gpio0b, MFSEL6, 20, 127) \
	FUNC(cp1gpio1b, MFSEL6, 21, 126) \
	FUNC(cp1gpio2b, MFSEL6, 22, 125) \
	FUNC(cp1gpio3b, MFSEL6, 23, 124) \
	FUNC(cp1gpio7b, MFSEL6, 24, 96) \
	FUNC(cp1gpio6b, MFSEL6, 25, 97) \
	FUNC(cp1gpio5b, MFSEL6, 26, 98) \
	FUNC(cp1gpio4b, MFSEL6, 27, 99) \
	FUNC(cp1gpio3c, MFSEL6, 28, 100) \
	FUNC(cp1gpio2c, MFSEL6, 29, 101) \
	FUNC(r3rxer, MFSEL6, 30, 212) \
	FUNC(cp1urxd, MFSEL6, 31, 41) \
	FUNC(tp_gpio0, MFSEL7, 0, 8) \
	FUNC(tp_gpio1, MFSEL7, 1, 9) \
	FUNC(tp_gpio2, MFSEL7, 2, 16) \
	FUNC(tp_gpio3, MFSEL7, 3, 100) \
	FUNC(tp_gpio4, MFSEL7, 4, 99) \
	FUNC(tp_gpio5, MFSEL7, 5, 98) \
	FUNC(tp_gpio6, MFSEL7, 6, 97) \
	FUNC(tp_gpio7, MFSEL7, 7, 96) \
	FUNC(tp_gpio0b, MFSEL7, 8, 91) \
	FUNC(tp_gpio1b, MFSEL7, 9, 92) \
	FUNC(tp_gpio2b, MFSEL7, 10, 101) \
	FUNC(tp_smb1, MFSEL7, 11, 142, 143) \
	FUNC(tp_uart, MFSEL7, 12, 50, 51) \
	FUNC(tp_jtag3, MFSEL7, 13, 44, 45, 46, 62) \
	FUNC(gpio187, MFSEL7, 24, 187) \
	FUNC(gpio1889, MFSEL7, 25, 188, 189) \
	FUNC(smb14b, MFSEL7, 26, 32, 187) \
	FUNC(smb15b, MFSEL7, 27, 191, 192) \
	FUNC(tp_smb2, MFSEL7, 28, 24, 25) \
	FUNC(vgadig, MFSEL7, 29, 102, 103, 104, 105) \
	FUNC(smb16b, MFSEL7, 30, 218, 219) \
	FUNC(smb0b, I2CSEGSEL, 0, 194, 195) \
	FUNC(smb0c, I2CSEGSEL, 1, 196, 202) \
	FUNC(smb0d, I2CSEGSEL, 2, 198, 199) \
	FUNC(smb1b, I2CSEGSEL, 5, 126, 127) \
	FUNC(smb1c, I2CSEGSEL, 6, 124, 125) \
	FUNC(smb1d, I2CSEGSEL, 7, 4, 5) \
	FUNC(smb2b, I2CSEGSEL, 8, 122, 123) \
	FUNC(smb2c, I2CSEGSEL, 9, 120, 121) \
	FUNC(smb2d, I2CSEGSEL, 10, 6, 7) \
	FUNC(smb3b, I2CSEGSEL, 11, 39, 40) \
	FUNC(smb3c, I2CSEGSEL, 12, 37, 38) \
	FUNC(smb3d, I2CSEGSEL, 13, 59, 60) \
	FUNC(smb4b, I2CSEGSEL, 14, 18, 19) \
	FUNC(smb4c, I2CSEGSEL, 15, 20, 21) \
	FUNC(smb4d, I2CSEGSEL, 16, 22, 23) \
	FUNC(smb5b, I2CSEGSEL, 19, 12, 13) \
	FUNC(smb5c, I2CSEGSEL, 20, 14, 15) \
	FUNC(smb5d, I2CSEGSEL, 21, 93, 94) \
	FUNC(smb0den, I2CSEGSEL, 22, 197) \
	FUNC(smb6b, I2CSEGSEL, 24, 2, 3) \
	FUNC(smb6c, I2CSEGSEL, 25, 0, 1) \
	FUNC(smb6d, I2CSEGSEL, 26, 10, 11) \
	FUNC(smb7b, I2CSEGSEL, 27, 16, 141) \
	FUNC(smb7c, I2CSEGSEL, 28, 24, 25) \
	FUNC(smb7d, I2CSEGSEL, 29, 142, 143) \
	FUNC(lkgpo0, FLOCKR1, 0, 16) \
	FUNC(lkgpo1, FLOCKR1, 4, 8) \
	FUNC(lkgpo2, FLOCKR1, 8, 9) \
	FUNC(nprd_smi, FLOCKR1, 20, 190) \
	FUNC(mmcwp, FLOCKR1, 24, 153) \
	FUNC(rg2refck, INTCR4, 6) \
	FUNC(r1en, INTCR4, 12) \
	FUNC(r2en, INTCR4, 13) \
	FUNC(r3en, INTCR4, 14)

/* declare function pins */
#define FUNC(_name, _reg, _bit, ...) \
	static const u8 _name##_pins[] = { __VA_ARGS__ };
FUNC_LIST

/* enumerate function ids */
#undef FUNC
#define FUNC(_name, _reg, _bit, ...) \
	FN_##_name,
enum npcm8xx_func_selectors {
	FUNC_LIST
	FN_gpio
};

#undef FUNC
#define FUNC(_name, _reg, _bit, ...) {		\
	.id = FN_##_name,			\
	.name = #_name,				\
	.pins = _name##_pins,			\
	.npins = ARRAY_SIZE(_name##_pins),	\
	.reg = _reg,				\
	.bit = _bit,				\
	},

/**
 * struct group_info - group of pins for a function
 *
 * @id: identifier
 * @name: group & function name
 * @pins: group of pins used by this function
 * @npins: number of pins
 * @reg: register for enabling the function
 * @bit: offset of enable bit in the register
 */
struct group_info {
	u32 id;
	char *name;
	const u8 *pins;
	u32 npins;
	u32 reg;
	u32 bit;
};

static const struct group_info npcm8xx_groups[] = {
	FUNC_LIST
	{FN_gpio, "GPIO", NULL, 0, 0, 0}
};

/* Pin flags */
#define SLEW		BIT(0) /* Has Slew Control */
#define GPIO_ALT	BIT(1) /* GPIO function is enabled by setting alternate */
#define DSLO_MASK	GENMASK(11, 8)  /* Drive strength */
#define DSHI_MASK	GENMASK(15, 12)
#define GPIO_IDX_MASK	GENMASK(18, 16)
#define GPIO_IDX(x)	((x) << 16) /* index of alt_func[] for gpio function */
#define DS(lo, hi)	(((lo) << 8) | ((hi) << 12))
#define DSLO(x)		FIELD_GET(DSLO_MASK, x) /* Low DS value */
#define DSHI(x)		FIELD_GET(DSHI_MASK, x) /* High DS value */
#define GPIO_IDX_VAL(x)	FIELD_GET(GPIO_IDX_MASK, x)

#define MAX_ALT_FUNCS	5 /* Max alternate functions */
/**
 * struct pin_info
 *
 * @gpio_num: GPIO number as index
 * @name: pin name
 * @funcs: array of alternate function selectors of this pin
 * @num_funcs: number of alternate functions
 */
struct pin_info {
	u32 gpio_num;
	char *name;
	u32 funcs[MAX_ALT_FUNCS];
	u32 num_funcs;
	u32 flags;
};

/* Pin table */
static const struct pin_info npcm8xx_pins[] = {
	{0, "GPIO0/IOX1_DI/SMB6C_SDA/SMB18_SDA", {FN_iox1, FN_smb6c, FN_smb18}, 3, SLEW},
	{1, "GPIO1/IOX1_LD/SMB6C_SCL/SMB18_SCL", {FN_iox1, FN_smb6c, FN_smb18}, 3, SLEW},
	{2, "GPIO2/IOX1_CK/SMB6B_SDA/SMB17_SDA", {FN_iox1, FN_smb6b, FN_smb17}, 3, SLEW},
	{3, "GPIO3/IOX1_DO/SMB6B_SCL/SMB17_SCL", {FN_iox1, FN_smb6b, FN_smb17}, 3, SLEW},
	{4, "GPIO4/IOX2_DI/SMB1D_SDA", {FN_iox2, FN_smb1d}, 2, SLEW},
	{5, "GPIO5/IOX2_LD/SMB1D_SCL", {FN_iox2, FN_smb1d}, 2, SLEW},
	{6, "GPIO6/IOX2_CK/SMB2D_SDA", {FN_iox2, FN_smb2d}, 2, SLEW},
	{7, "GPIO7/IOX2_D0/SMB2D_SCL", {FN_iox2, FN_smb2d}, 2, SLEW},
	{8, "GPIO8/LKGPO1/TP_GPIO0", {FN_lkgpo1, FN_tp_gpio0b}, 2, DS(8, 12)},
	{9, "GPIO9/LKGPO2/TP_GPIO1", {FN_lkgpo2, FN_tp_gpio1b}, 2, DS(8, 12)},
	{10, "GPIO10/IOXH_LD/SMB6D_SCL/SMB16_SCL", {FN_ioxh, FN_smb6d, FN_smb16}, 3, SLEW},
	{11, "GPIO11/IOXH_CK/SMB6D_SDA/SMB16_SDA", {FN_ioxh, FN_smb6d, FN_smb16}, 3, SLEW},
	{12, "GPIO12/GSPI_CK/SMB5B_SCL", {FN_gspi, FN_smb5d}, 2, SLEW},
	{13, "GPIO13/GSPI_DO/SMB5B_SDA", {FN_gspi, FN_smb5d}, 2, SLEW},
	{14, "GPIO14/GSPI_DI/SMB5C_SCL", {FN_gspi, FN_smb5c}, 2, SLEW},
	{15, "GPIO15/GSPI_CS/SMB5C_SDA", {FN_gspi, FN_smb5c}, 2, SLEW},
	{16, "GPIO16/SMB7B_SDA/LKGPO0/TP_GPIO2", {FN_lkgpo0, FN_smb7b, FN_tp_gpio2b}, 3, SLEW},
	{17, "GPIO17/PSPI_DI/CP1_GPIO5", {FN_pspi, FN_cp1gpio5}, 2, SLEW},
	{18, "GPIO18/PSPI_D0/SMB4B_SDA", {FN_pspi, FN_smb4b}, 2, SLEW},
	{19, "GPIO19/PSPI_CK/SMB4B_SCL", {FN_pspi, FN_smb4b}, 2, SLEW},
	{20, "GPIO20/H_GPIO0/SMB4C_SDA/SMB15_SDA", {FN_hgpio0, FN_smb15, FN_smb4c}, 3, SLEW},
	{21, "GPIO21/H_GPIO1/SMB4C_SCL/SMB15_SCL", {FN_hgpio1, FN_smb15, FN_smb4c}, 3, SLEW},
	{22, "GPIO22/H_GPIO2/SMB4D_SDA/SMB14_SDA", {FN_hgpio2, FN_smb14, FN_smb4d}, 3, SLEW},
	{23, "GPIO23/H_GPIO3/SMB4D_SCL/SMB14_SCL", {FN_hgpio3, FN_smb14, FN_smb4d}, 3, SLEW},
	{24, "GPIO24/IOXH_DO/H_GPIO4/SMB7C_SCL/TP_SMB2_SCL",
	     {FN_hgpio4, FN_ioxh, FN_smb7c, FN_tp_smb2}, 4, SLEW},
	{25, "GPIO25/IOXH_DI/H_GPIO4/SMB7C_SDA/TP_SMB2_SDA", {FN_hgpio5, FN_ioxh, FN_smb7c},
	     3, SLEW},
	{26, "GPIO26/SMB5_SDA", {FN_smb5}, 1, 0},
	{27, "GPIO27/SMB5_SCL", {FN_smb5}, 1, 0},
	{28, "GPIO28/SMB4_SDA", {FN_smb4}, 1, 0},
	{29, "GPIO29/SMB4_SCL", {FN_smb4}, 1, 0},
	{30, "GPIO30/SMB3_SDA", {FN_smb3}, 1, 0},
	{31, "GPIO31/SMB3_SCL", {FN_smb3}, 1, 0},
	{32, "GPIO32/SMB14_SCL/SPI0_nCS1", {FN_smb14b, FN_spi0cs1}, 2, SLEW},
	{33, "I3C4_SCL", {FN_i3c4}, 1, SLEW},
	{34, "I3C4_SDA", {FN_i3c4}, 1, SLEW},
	{35, "GPI35/MCBPCK", {FN_gpi35}, 1, GPIO_ALT | GPIO_IDX(0)},
	{36, "GPI36/SYSBPCK", {FN_gpi36}, 1, GPIO_ALT | GPIO_IDX(0)},
	{37, "GPIO37/SMB3C_SDA/SMB23_SDA", {FN_smb3c, FN_smb23}, 2, SLEW},
	{38, "GPIO38/SMB3C_SCL/SMB23_SCL", {FN_smb3c, FN_smb23}, 2, SLEW},
	{39, "GPIO39/SMB3B_SDA/SMB22_SDA", {FN_smb3b, FN_smb22}, 2, SLEW},
	{40, "GPIO40/SMB3B_SCL/SMB22_SCL", {FN_smb3b, FN_smb22}, 2, SLEW},
	{41, "GPIO41/BU0_RXD/CP1U_RXD", {FN_bmcuart0a, FN_cp1urxd}, 2, 0},
	{42, "GPIO42/BU0_TXD/CP1U_TXD", {FN_bmcuart0a, FN_cp1utxd}, 2, DS(2, 4)},
	{43, "GPIO43/SI1_RXD/BU1_RXD", {FN_hsi1a, FN_bmcuart1}, 2, 0},
	{44, "GPIO44/SI1_nCTS/BU1_nCTS/CP_TDI/TP_TDI/CP_TP_TDI",
	     {FN_hsi1b, FN_nbu1crts, FN_jtag2, FN_tp_jtag3, FN_j2j3}, 5, 0},
	{45, "GPIO45/SI1_nDCD/CP_TMS_SWIO/TP_TMS_SWIO/CP_TP_TMS_SWIO",
	     {FN_hsi1c, FN_jtag2, FN_j2j3, FN_tp_jtag3}, 4, DS(2, 8)},
	{46, "GPIO46/SI1_nDSR/CP_TCK_SWCLK/TP_TCK_SWCLK/CP_TP_TCK_SWCLK",
	     {FN_hsi1c, FN_jtag2, FN_j2j3, FN_tp_jtag3}, 4, 0},
	{47, "GPIO47/SI1n_RI1", {FN_hsi1c,}, 1, DS(2, 8)},
	{48, "GPIO48/SI2_TXD/BU0_TXD/STRAP5", {FN_hsi2a, FN_bmcuart0b}, 2, 0},
	{49, "GPIO49/SI2_RXD/BU0_RXD", {FN_hsi2a, FN_bmcuart0b}, 2, 0},
	{50, "GPIO50/SI2_nCTS/BU6_TXD/TPU_TXD", {FN_hsi2b, FN_bu6, FN_tp_uart}, 3, 0},
	{51, "GPIO51/SI2_nRTS/BU6_RXD/TPU_RXD", {FN_hsi2b, FN_bu6, FN_tp_uart}, 3, 0},
	{52, "GPIO52/SI2_nDCD/BU5_RXD", {FN_hsi2c, FN_bu5}, 2, 0},
	{53, "GPIO53/SI2_nDTR_BOUT2/BU5_TXD", {FN_hsi2c, FN_bu5}, 2, 0},
	{54, "GPIO54/SI2_nDSR/BU4_TXD", {FN_hsi2c, FN_bu4}, 2, 0},
	{55, "GPIO55/SI2_RI2/BU4_RXD", {FN_hsi2c, FN_bu4}, 2, 0},
	{56, "GPIO56/R1_RXERR/R1_OEN", {FN_r1err, FN_r1oen}, 2, 0},
	{57, "GPIO57/R1_MDC/TP_GPIO4", {FN_r1md, FN_tp_gpio4b}, 2, DS(2, 4)},
	{58, "GPIO58/R1_MDIO/TP_GPIO5", {FN_r1md, FN_tp_gpio5b}, 2, DS(2, 4)},
	{59, "GPIO59/H_GPIO06/SMB3D_SDA/SMB19_SDA", {FN_hgpio6, FN_smb3d, FN_smb19}, 3, 0},
	{60, "GPIO60/H_GPIO07/SMB3D_SCL/SMB19_SCL", {FN_hgpio7, FN_smb3d, FN_smb19}, 3, 0},
	{61, "GPIO61/SI1_nDTR_BOUT", {FN_hsi1c}, 1, 0},
	{62, "GPIO62/SI1_nRTS/BU1_nRTS/CP_TDO_SWO/TP_TDO_SWO/CP_TP_TDO_SWO",
	     {FN_hsi1b, FN_jtag2, FN_j2j3, FN_nbu1crts, FN_tp_jtag3}, 5, 0},
	{63, "GPIO63/BU1_TXD1/SI1_TXD", {FN_hsi1a, FN_bmcuart1}, 2, 0},
	{64, "GPIO64/FANIN0", {FN_fanin0}, 1, 0},
	{65, "GPIO65/FANIN1", {FN_fanin1}, 1, 0},
	{66, "GPIO66/FANIN2", {FN_fanin2}, 1, 0},
	{67, "GPIO67/FANIN3", {FN_fanin3}, 1, 0},
	{68, "GPIO68/FANIN4", {FN_fanin4}, 1, 0},
	{69, "GPIO69/FANIN5", {FN_fanin5}, 1, 0},
	{70, "GPIO70/FANIN6", {FN_fanin6}, 1, 0},
	{71, "GPIO71/FANIN7", {FN_fanin7}, 1, 0},
	{72, "GPIO72/FANIN8", {FN_fanin8}, 1, 0},
	{73, "GPIO73/FANIN9", {FN_fanin9}, 1, 0},
	{74, "GPIO74/FANIN10", {FN_fanin10}, 1, 0},
	{75, "GPIO75/FANIN11", {FN_fanin11}, 1, 0},
	{76, "GPIO76/FANIN12", {FN_fanin12}, 1, 0},
	{77, "GPIO77/FANIN13", {FN_fanin13}, 1, 0},
	{78, "GPIO78/FANIN14", {FN_fanin14}, 1, 0},
	{79, "GPIO79/FANIN15", {FN_fanin15}, 1, 0},
	{80, "GPIO80/PWM0", {FN_pwm0}, 1, DS(4, 8)},
	{81, "GPIO81/PWM1", {FN_pwm1}, 1, DS(4, 8)},
	{82, "GPIO82/PWM2", {FN_pwm2}, 1, DS(4, 8)},
	{83, "GPIO83/PWM3", {FN_pwm3}, 1, DS(4, 8)},
	{84, "GPIO84/R2_TXD0", {FN_r2}, 1, DS(4, 8) | SLEW},
	{85, "GPIO85/R2_TXD1", {FN_r2}, 1, DS(4, 8) | SLEW},
	{86, "GPIO86/R2_TXEN", {FN_r2}, 1, DS(4, 8) | SLEW},
	{87, "GPIO87/R2_RXD0", {FN_r2}, 1, 0},
	{88, "GPIO88/R2_RXD1", {FN_r2}, 1, 0},
	{89, "GPIO89/R2_CRSDV", {FN_r2}, 1, 0},
	{90, "GPIO90/R2_RXERR/R2_OEN", {FN_r2err, FN_r2oen}, 2, 0},
	{91, "GPIO91/R2_MDC/CP1_GPIO6/TP_GPIO0", {FN_r2md, FN_cp1gpio6, FN_tp_gpio0}, 3, DS(2, 4)},
	{92, "GPIO92/R2_MDIO/CP1_GPIO7/TP_GPIO1", {FN_r2md, FN_cp1gpio7, FN_tp_gpio1}, 3, DS(2, 4)},
	{93, "GPIO93/GA20/SMB5D_SCL", {FN_ga20kbc, FN_smb5d}, 2, 0},
	{94, "GPIO94/nKBRST/SMB5D_SDA", {FN_ga20kbc, FN_smb5d}, 2, 0},
	{95, "GPIO95/nESPIRST/LPC_nLRESET", {FN_lpc, FN_espi}, 2, 0},
	{96, "GPIO96/CP1_GPIO7/BU2_TXD/TP_GPIO7", {FN_cp1gpio7b, FN_bu2, FN_tp_gpio7}, 3, SLEW},
	{97, "GPIO97/CP1_GPIO6/BU2_RXD/TP_GPIO6", {FN_cp1gpio6b, FN_bu2, FN_tp_gpio6}, 3, SLEW},
	{98, "GPIO98/CP1_GPIO5/BU4_TXD/TP_GPIO5", {FN_bu4b, FN_cp1gpio5b, FN_tp_gpio5}, 3, SLEW},
	{99, "GPIO99/CP1_GPIO4/BU4_RXD/TP_GPIO4", {FN_bu4b, FN_cp1gpio4b, FN_tp_gpio4}, 3, SLEW},
	{100, "GPIO100/CP1_GPIO3/BU5_TXD/TP_GPIO3", {FN_bu5b, FN_cp1gpio3c, FN_tp_gpio3}, 3, SLEW},
	{101, "GPIO101/CP1_GPIO2/BU5_RXD/TP_GPIO2", {FN_bu5b, FN_cp1gpio2c, FN_tp_gpio2}, 3, SLEW},
	{102, "GPIO102/HSYNC", {FN_vgadig}, 1, DS(4, 8)},
	{103, "GPIO103/VSYNC", {FN_vgadig}, 1, DS(4, 8)},
	{104, "GPIO104/DDC_SCL", {FN_vgadig}, 1, 0},
	{105, "GPIO105/DDC_SDA", {FN_vgadig}, 1, 0},
	{106, "GPIO106/I3C5_SCL", {FN_i3c5}, 1, SLEW},
	{107, "GPIO107/I3C5_SDA", {FN_i3c5}, 1, SLEW},
	{108, "GPIO108/SG1_MDC", {FN_rg1mdio}, 1, SLEW},
	{109, "GPIO109/SG1_MDIO", {FN_rg1mdio}, 1, SLEW},
	{110, "GPIO110/RG2_TXD0/DDRV0/R3_TXD0", {FN_rg2, FN_ddr, FN_rmii3}, 3, SLEW},
	{111, "GPIO111/RG2_TXD1/DDRV1/R3_TXD1", {FN_rg2, FN_ddr, FN_rmii3}, 3, SLEW},
	{112, "GPIO112/RG2_TXD2/DDRV2", {FN_rg2, FN_ddr}, 2, SLEW},
	{113, "GPIO113/RG2_TXD3/DDRV3", {FN_rg2, FN_ddr}, 2, SLEW},
	{114, "GPIO114/SMB0_SCL", {FN_smb0}, 1, 0},
	{115, "GPIO115/SMB0_SDA", {FN_smb0}, 1, 0},
	{116, "GPIO116/SMB1_SCL", {FN_smb1}, 1, 0},
	{117, "GPIO117/SMB1_SDA", {FN_smb1}, 1, 0},
	{118, "GPIO118/SMB2_SCL", {FN_smb2}, 1, 0},
	{119, "GPIO119/SMB2_SDA", {FN_smb2}, 1, 0},
	{120, "GPIO120/SMB2C_SDA", {FN_smb2c}, 1, SLEW},
	{121, "GPIO121/SMB2C_SCL", {FN_smb2c}, 1, SLEW},
	{122, "GPIO122/SMB2B_SDA", {FN_smb2b}, 1, SLEW},
	{123, "GPIO123/SMB2B_SCL", {FN_smb2b}, 1, SLEW},
	{124, "GPIO124/SMB1C_SDA/CP1_GPIO3", {FN_smb1c, FN_cp1gpio3b}, 2, SLEW},
	{125, "GPIO125/SMB1C_SCL/CP1_GPIO2", {FN_smb1c, FN_cp1gpio2b}, 2, SLEW},
	{126, "GPIO126/SMB1B_SDA/CP1_GPIO1", {FN_smb1b, FN_cp1gpio1b}, 2, SLEW},
	{127, "GPIO127/SMB1B_SCL/CP1_GPIO0", {FN_smb1b, FN_cp1gpio0b}, 2, SLEW},
	{128, "GPIO128/SMB824_SCL", {FN_smb8}, 1, 0},
	{129, "GPIO129/SMB824_SDA", {FN_smb8}, 1, 0},
	{130, "GPIO130/SMB925_SCL", {FN_smb9}, 1, 0},
	{131, "GPIO131/SMB925_SDA", {FN_smb9}, 1, 0},
	{132, "GPIO132/SMB1026_SCL", {FN_smb10}, 1, 0},
	{133, "GPIO133/SMB1026_SDA", {FN_smb10}, 1, 0},
	{134, "GPIO134/SMB11_SCL", {FN_smb11, FN_smb23b}, 2, 0},
	{135, "GPIO135/SMB11_SDA", {FN_smb11, FN_smb23b}, 2, 0},
	{136, "GPIO136/JM1_TCK", {FN_jm1}, 1, SLEW},
	{137, "GPIO137/JM1_TDO", {FN_jm1}, 1, SLEW},
	{138, "GPIO138/JM1_TMS", {FN_jm1}, 1, SLEW},
	{139, "GPIO139/JM1_TDI", {FN_jm1}, 1, SLEW},
	{140, "GPIO140/JM1_nTRST", {FN_jm1}, 1, SLEW},
	{141, "GPIO141/SMB7B_SCL", {FN_smb7b}, 1, 0},
	{142, "GPIO142/SMB7D_SCL/TPSMB1_SCL", {FN_smb7d, FN_tp_smb1}, 2, SLEW},
	{143, "GPIO143/SMB7D_SDA/TPSMB1_SDA", {FN_smb7d, FN_tp_smb1}, 2, SLEW},
	{144, "GPIO144/PWM4", {FN_pwm4}, 1, DS(4, 8)},
	{145, "GPIO145/PWM5", {FN_pwm5}, 1, DS(4, 8)},
	{146, "GPIO146/PWM6", {FN_pwm6}, 1, DS(4, 8)},
	{147, "GPIO147/PWM7", {FN_pwm7}, 1, DS(4, 8)},
	{148, "GPIO148/MMC_DT4", {FN_mmc8}, 1, DS(8, 12) | SLEW},
	{149, "GPIO149/MMC_DT5", {FN_mmc8}, 1, DS(8, 12) | SLEW},
	{150, "GPIO150/MMC_DT6", {FN_mmc8}, 1, DS(8, 12) | SLEW},
	{151, "GPIO151/MMC_DT7", {FN_mmc8}, 1, DS(8, 12) | SLEW},
	{152, "GPIO152/MMC_CLK", {FN_mmc}, 1, DS(8, 12) | SLEW},
	{153, "GPIO153/MMC_WP", {FN_mmcwp}, 1, 0},
	{154, "GPIO154/MMC_CMD", {FN_mmc}, 1, DS(8, 12) | SLEW},
	{155, "GPIO155/MMC_nCD/MMC_nRSTLK", {FN_mmccd, FN_mmcrst}, 2, 0},
	{156, "GPIO156/MMC_DT0", {FN_mmc}, 1, DS(8, 12) | SLEW},
	{157, "GPIO157/MMC_DT1", {FN_mmc}, 1, DS(8, 12) | SLEW},
	{158, "GPIO158/MMC_DT2", {FN_mmc}, 1, DS(8, 12) | SLEW},
	{159, "GPIO159/MMC_DT3", {FN_mmc}, 1, DS(8, 12) | SLEW},
	{160, "GPIO160/CLKOUT/RNGOSCOUT/GFXBYPCK", {FN_clkout}, 1, DS(8, 12) | SLEW},
	{161, "GPIO161/ESPI_nCS/LPC_nLFRAME", {FN_espi, FN_lpc}, 2, 0},
	{162, "GPIO162/LPC_nCLKRUN", {FN_clkrun}, 1, DS(8, 12)},
	{163, "GPIO163/ESPI_CK/LPC_LCLK", {FN_espi, FN_lpc}, 2, 0},
	{164, "GPIO164/ESPI_IO0/LPC_LAD0", {FN_espi, FN_lpc}, 2, 0},
	{165, "GPIO165/ESPI_IO1/LPC_LAD1", {FN_espi, FN_lpc}, 2, 0},
	{166, "GPIO166/ESPI_IO2/LPC_LAD2", {FN_espi, FN_lpc}, 2, 0},
	{167, "GPIO167/ESPI_IO3/LPC_LAD3", {FN_espi, FN_lpc}, 2, 0},
	{168, "GPIO168/ESPI_nALERT/SERIRQ", {FN_espi, FN_serirq}, 2, 0},
	{169, "GPIO169/nSCIPME/SMB21_SCL", {FN_scipme, FN_smb21}, 2, 0},
	{170, "GPIO170/nSMI/SMB21_SDA", {FN_sci, FN_smb21}, 2, 0},
	{171, "GPIO171/SMB6_SCL", {FN_smb6}, 1, 0},
	{172, "GPIO172/SMB6_SDA", {FN_smb6}, 1, 0},
	{173, "GPIO173/SMB7_SCL", {FN_smb7}, 1, 0},
	{174, "GPIO174/SMB7_SDA", {FN_smb7}, 1, 0},
	{175, "GPIO175/SPI1_CK/FANIN19/FM1_CK", {FN_spi1, FN_faninx, FN_fm1}, 3, DS(8, 12)},
	{176, "GPIO176/SPI1_DO/FANIN18/FM1_DO/STRAP9", {FN_spi1, FN_faninx, FN_fm1}, 3, DS(8, 12)},
	{177, "GPIO177/SPI1_DI/FANIN17/FM1_D1/STRAP10", {FN_spi1, FN_faninx, FN_fm1}, 3, DS(8, 12)},
	{178, "GPIO178/R1_TXD0", {FN_r1}, 1, DS(8, 12) | SLEW},
	{179, "GPIO179/R1_TXD1", {FN_r1}, 1, DS(8, 12) | SLEW},
	{180, "GPIO180/R1_TXEN", {FN_r1}, 1, DS(8, 12) | SLEW},
	{181, "GPIO181/R1_RXD0", {FN_r1}, 1, 0},
	{182, "GPIO182/R1_RXD1", {FN_r1}, 1, 0},
	{183, "GPIO183/SPI3_SEL", {FN_spi3, FN_gpio1836}, 2,
	      DS(8, 12) | SLEW | GPIO_ALT | GPIO_IDX(1)},
	{184, "GPIO184/SPI3_D0/STRAP13", {FN_spi3, FN_gpio1836}, 2,
	      DS(8, 12) | SLEW | GPIO_ALT | GPIO_IDX(1)},
	{185, "GPIO185/SPI3_D1", {FN_spi3, FN_gpio1836}, 2,
	      DS(8, 12) | SLEW | GPIO_ALT | GPIO_IDX(1)},
	{186, "GPIO186/SPI3_nCS0", {FN_spi3, FN_gpio1836}, 2,
	      DS(8, 12) | SLEW | GPIO_ALT | GPIO_IDX(1)},
	{187, "GPIO187/SPI3_nCS1_SMB14_SDA", {FN_spi3cs1, FN_smb14b, FN_gpio187}, 3,
	      SLEW | GPIO_ALT | GPIO_IDX(2)},
	{188, "GPIO188/SPI3_D2/SPI3_nCS2", {FN_spi3quad, FN_spi3cs2, FN_gpio1889}, 3,
	      DS(8, 12) | SLEW | GPIO_ALT | GPIO_IDX(2)},
	{189, "GPIO189/SPI3_D3/SPI3_nCS3", {FN_spi3quad, FN_spi3cs3, FN_gpio1889}, 3,
	      DS(8, 12) | SLEW | GPIO_ALT | GPIO_IDX(2)},
	{190, "GPIO190/nPRD_SMI", {FN_nprd_smi}, 1, DS(2, 4)},
	{191, "GPIO191/SPI1_D1/FANIN17/FM1_D1/STRAP10",
	      {FN_spi1d23, FN_spi1cs2, FN_fm1, FN_smb15}, 4, SLEW},
	{192, "GPIO192/SPI1_D3/SPI_nCS3/FM1_D3/SMB15_SCL",
	      {FN_spi1d23, FN_spi1cs3, FN_fm1, FN_smb15}, 4, SLEW},
	{193, "GPIO193/R1_CRSDV", {FN_r1}, 1, 0},
	{194, "GPIO194/SMB0B_SCL/FM0_CK", {FN_smb0b, FN_fm0}, 2, SLEW},
	{195, "GPIO195/SMB0B_SDA/FM0_D0", {FN_smb0b, FN_fm0}, 2, SLEW},
	{196, "GPIO196/SMB0C_SCL/FM0_D1", {FN_smb0c, FN_fm0}, 2, SLEW},
	{197, "GPIO197/SMB0DEN/FM0_D3", {FN_smb0den, FN_fm0}, 2, SLEW},
	{198, "GPIO198/SMB0D_SDA/FM0_D2", {FN_smb0d, FN_fm0}, 2, SLEW},
	{199, "GPIO199/SMB0D_SCL/FM0_CSO", {FN_smb0d, FN_fm0}, 2, SLEW},
	{200, "GPIO200/R2_CK", {FN_r2}, 1, 0},
	{201, "GPIO201/R1_CK", {FN_r1}, 1, 0},
	{202, "GPIO202/SMB0C_SDA/FM0_CSI", {FN_smb0c, FN_fm0}, 2, SLEW},
	{203, "GPIO203/SPI1_nCS0/FANIN16/FM1_CSI", {FN_faninx, FN_spi1, FN_fm1}, 3, DS(8, 12)},
	{208, "GPIO208/RG2_TXC/DVCK", {FN_rg2, FN_ddr}, 2, SLEW},
	{209, "GPIO209/RG2_TXCTL/DDRV4/R3_TXEN", {FN_rg2, FN_ddr, FN_rmii3}, 3, SLEW},
	{210, "GPIO210/RG2_RXD0/DDRV5/R3_RXD0", {FN_rg2, FN_ddr, FN_rmii3}, 3, DS(8, 12) | SLEW},
	{211, "GPIO211/RG2_RXD1/DDRV6/R3_RXD1", {FN_rg2, FN_ddr, FN_rmii3}, 3, DS(8, 12) | SLEW},
	{212, "GPIO212/RG2_RXD2/DDRV7/R3_RXD2", {FN_rg2, FN_ddr, FN_r3rxer}, 3, DS(8, 12) | SLEW},
	{213, "GPIO213/RG2_RXD3/DDRV8/R3_OEN", {FN_rg2, FN_ddr, FN_r3oen}, 3, DS(8, 12) | SLEW},
	{214, "GPIO214/RG2_RXC/DDRV9/R3_CK", {FN_rg2, FN_ddr, FN_rmii3}, 3, DS(8, 12) | SLEW},
	{215, "GPIO215/RG2_RXCTL/DDRV10/R3_CRSDV", {FN_rg2, FN_ddr, FN_rmii3}, 3, DS(8, 12) | SLEW},
	{216, "GPIO216/RG2_MDC/DDRV11", {FN_rg2mdio, FN_ddr}, 2, DS(8, 12) | SLEW},
	{217, "GPIO217/RG2_MDIO/DVHSYNC", {FN_rg2mdio, FN_ddr}, 2, DS(8, 12) | SLEW},
	{218, "GPIO218/nWDO1/SMB16_SCL", {FN_wdog1, FN_smb16}, 2, SLEW},
	{219, "GPIO219/nWDO2/SMB16_SDA", {FN_wdog2, FN_smb16}, 2, SLEW},
	{220, "GPIO220/SMB12_SCL/PWM8", {FN_smb12, FN_pwm8}, 2, SLEW},
	{221, "GPIO221/SMB12_SDA/PWM9", {FN_smb12, FN_pwm9}, 2, SLEW},
	{222, "GPIO222/SMB13_SCL", {FN_smb13}, 1, SLEW},
	{223, "GPIO223/SMB13_SDA", {FN_smb13}, 1, SLEW},
	{224, "GPIO224/SPIX_CK/FM2_CK", {FN_spix, FN_fm2}, 2, DS(8, 12) | SLEW},
	{225, "GPO225/SPIX_D0/FM2_D0/STRAP1", {FN_spix, FN_fm2}, 2, DS(8, 12) | SLEW},
	{226, "GPO226/SPIX_D1/FM2_D1/STRAP2", {FN_spix, FN_fm2}, 2, DS(8, 12) | SLEW},
	{227, "GPIO227/SPIX_nCS0/FM2_CSI", {FN_spix, FN_fm2}, 2, DS(8, 12) | SLEW},
	{228, "GPIO228/SPIX_nCS1/FM2_CSO", {FN_spixcs1, FN_fm2}, 2, DS(8, 12) | SLEW},
	{229, "GPO229/SPIX_D2/FM2_D2/STRAP3", {FN_spix, FN_fm2}, 2, DS(8, 12) | SLEW},
	{230, "GPO230/SPIX_D3/FM2_D3/STRAP6", {FN_spix, FN_fm2}, 2, DS(8, 12) | SLEW},
	{231, "GPIO231/EP_nCLKREQ", {FN_clkreq}, 1, DS(4, 12) | SLEW},
	{233, "GPIO233/SPI1_nCS1/FM1_CSO", {FN_spi1cs1, FN_fm1}, 2, 0},
	{234, "GPIO234/PWM10/SMB20_SCL", {FN_pwm10, FN_smb20}, 2, SLEW},
	{235, "GPIO235/PWM11/SMB20_SDA", {FN_pwm11, FN_smb20}, 2, SLEW},
	{240, "GPIO240/I3C0_SCL", {FN_i3c0}, 2, SLEW},
	{241, "GPIO241/I3C0_SDA", {FN_i3c0}, 2, SLEW},
	{242, "GPIO242/I3C1_SCL", {FN_i3c1}, 2, SLEW},
	{243, "GPIO243/I3C1_SDA", {FN_i3c1}, 2, SLEW},
	{244, "GPIO244/I3C2_SCL", {FN_i3c2}, 2, SLEW},
	{245, "GPIO245/I3C2_SDA", {FN_i3c2}, 2, SLEW},
	{246, "GPIO246/I3C3_SCL", {FN_i3c3}, 2, SLEW},
	{247, "GPIO247/I3C3_SDA", {FN_i3c3}, 2, SLEW},
	{250, "GPIO250/RG2_REFCK/DVVSYNC", {FN_ddr, FN_rg2refck}, 2, DS(8, 12) | SLEW},
};

static int npcm8xx_get_pin_selector(u8 gpio)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(npcm8xx_pins); i++) {
		if (npcm8xx_pins[i].gpio_num == gpio)
			return i;
	}

	return -ENOENT;
}

static int npcm8xx_group_set_func(struct udevice *dev,
				  const struct group_info *group,
				  unsigned int func_selector)
{
	struct npcm8xx_pinctrl_priv *priv = dev_get_priv(dev);

	dev_dbg(dev, "set_func [grp %s][func %s]\n", group->name,
		npcm8xx_groups[func_selector].name);
	if (group->id == func_selector)
		regmap_update_bits(priv->gcr_regmap, group->reg,
				   BIT(group->bit), BIT(group->bit));
	else
		regmap_update_bits(priv->gcr_regmap, group->reg,
				   BIT(group->bit), 0);

	return 0;
}

static int npcm8xx_pinmux_set(struct udevice *dev,
			      unsigned int pin_selector,
			      unsigned int func_selector)
{
	const struct pin_info *pin;
	const struct group_info *group;
	int i;

	pin = &npcm8xx_pins[pin_selector];
	dev_dbg(dev, "set_mux [pin %s][func %s]\n", pin->name,
		npcm8xx_groups[func_selector].name);

	for (i = 0; i < pin->num_funcs; i++) {
		group = &npcm8xx_groups[pin->funcs[i]];
		npcm8xx_group_set_func(dev, group, func_selector);
	}

	return 0;
}

static int npcm8xx_pinmux_group_set(struct udevice *dev,
				    unsigned int group_selector,
				    unsigned int func_selector)
{
	const struct group_info *group;
	int pin_selector;
	int i;

	dev_dbg(dev, "set_mux [grp %s][func %s]\n",
		npcm8xx_groups[group_selector].name,
		npcm8xx_groups[func_selector].name);
	group = &npcm8xx_groups[group_selector];

	if (!group->npins) {
		/* No other alternate pins, just set group function */
		npcm8xx_group_set_func(dev, group, func_selector);
		return 0;
	}

	for (i = 0; i < group->npins; i++) {
		pin_selector = npcm8xx_get_pin_selector(group->pins[i]);
		if (pin_selector < 0) {
			dev_dbg(dev, "invalid pin %d\n", group->pins[i]);
			return -EINVAL;
		}
		npcm8xx_pinmux_set(dev, pin_selector, func_selector);
	}

	return 0;
}

static int npcm8xx_get_pins_count(struct udevice *dev)
{
	return ARRAY_SIZE(npcm8xx_pins);
}

static const char *npcm8xx_get_pin_name(struct udevice *dev,
					unsigned int selector)
{
	return npcm8xx_pins[selector].name;
}

static int npcm8xx_get_groups_count(struct udevice *dev)
{
	return ARRAY_SIZE(npcm8xx_groups);
}

static const char *npcm8xx_get_group_name(struct udevice *dev,
					  unsigned int selector)
{
	return npcm8xx_groups[selector].name;
}

static int npcm8xx_get_functions_count(struct udevice *dev)
{
	return ARRAY_SIZE(npcm8xx_groups);
}

static const char *npcm8xx_get_function_name(struct udevice *dev,
					     unsigned int selector)
{
	return npcm8xx_groups[selector].name;
}

#if CONFIG_IS_ENABLED(PINCONF)
#define PIN_CONFIG_PERSIST_STATE (PIN_CONFIG_END + 1)
#define PIN_CONFIG_POLARITY_STATE (PIN_CONFIG_END + 2)
#define PIN_CONFIG_EVENT_CLEAR (PIN_CONFIG_END + 3)

static const struct pinconf_param npcm8xx_conf_params[] = {
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

/* Support for retaining the state after soft reset */
static int npcm8xx_gpio_reset_persist(struct udevice *dev, uint bank,
				      uint enable)
{
	struct npcm8xx_pinctrl_priv *priv = dev_get_priv(dev);
	u8 offset = bank + GPIOX_OFFSET;

	dev_dbg(dev, "set gpio persist, bank %d, enable %d\n", bank, enable);

	if (enable) {
		regmap_update_bits(priv->rst_regmap, WD0RCR, BIT(offset), 0);
		regmap_update_bits(priv->rst_regmap, WD1RCR, BIT(offset), 0);
		regmap_update_bits(priv->rst_regmap, WD2RCR, BIT(offset), 0);
		regmap_update_bits(priv->rst_regmap, CORSTC, BIT(offset), 0);
		regmap_update_bits(priv->rst_regmap, SWRSTC1, BIT(offset), 0);
		regmap_update_bits(priv->rst_regmap, SWRSTC2, BIT(offset), 0);
		regmap_update_bits(priv->rst_regmap, SWRSTC3, BIT(offset), 0);
		regmap_update_bits(priv->rst_regmap, TIPRSTC, BIT(offset), 0);
	} else {
		regmap_update_bits(priv->rst_regmap, WD0RCR, BIT(offset),
				   BIT(offset));
		regmap_update_bits(priv->rst_regmap, WD1RCR, BIT(offset),
				   BIT(offset));
		regmap_update_bits(priv->rst_regmap, WD2RCR, BIT(offset),
				   BIT(offset));
		regmap_update_bits(priv->rst_regmap, CORSTC, BIT(offset),
				   BIT(offset));
		regmap_update_bits(priv->rst_regmap, SWRSTC1, BIT(offset),
				   BIT(offset));
		regmap_update_bits(priv->rst_regmap, SWRSTC2, BIT(offset),
				   BIT(offset));
		regmap_update_bits(priv->rst_regmap, SWRSTC3, BIT(offset),
				   BIT(offset));
		regmap_update_bits(priv->rst_regmap, TIPRSTC, BIT(offset),
				   BIT(offset));
	}

	return 0;
}

static bool is_gpio_persist(struct udevice *dev, uint bank)
{
	struct npcm8xx_pinctrl_priv *priv = dev_get_priv(dev);
	u8 offset = bank + GPIOX_OFFSET;
	u32 val;
	int status;

	status = npcm_get_reset_status();
	dev_dbg(dev, "reset status: 0x%x\n", status);

	if (status & PORST)
		return false;

	if (status & CORST)
		regmap_read(priv->rst_regmap, CORSTC, &val);
	else if (status & WD0RST)
		regmap_read(priv->rst_regmap, WD0RCR, &val);
	else if (status & WD1RST)
		regmap_read(priv->rst_regmap, WD1RCR, &val);
	else if (status & WD2RST)
		regmap_read(priv->rst_regmap, WD2RCR, &val);
	else if (status & SW1RST)
		regmap_read(priv->rst_regmap, SWRSTC1, &val);
	else if (status & SW2RST)
		regmap_read(priv->rst_regmap, SWRSTC2, &val);
	else if (status & SW3RST)
		regmap_read(priv->rst_regmap, SWRSTC3, &val);
	else if (status & TIPRST)
		regmap_read(priv->rst_regmap, TIPRSTC, &val);
	else
		return false;

	return !(val & BIT(offset));
}

static void npcm8xx_set_gpio_func(struct udevice *dev, unsigned int selector)
{
	const struct pin_info *pin = &npcm8xx_pins[selector];
	const struct group_info *group;
	unsigned int func_selector;
	int i;

	/* gpio function is an alternate function */
	if (pin->flags & GPIO_ALT)
		func_selector = pin->funcs[GPIO_IDX_VAL(pin->flags)];
	else
		func_selector = FN_gpio;

	for (i = 0; i < pin->num_funcs; i++) {
		group = &npcm8xx_groups[pin->funcs[i]];
		npcm8xx_group_set_func(dev, group, func_selector);
	}
}

static int npcm8xx_pinconf_set(struct udevice *dev, unsigned int selector,
			       unsigned int param, unsigned int arg)
{
	struct npcm8xx_pinctrl_priv *priv = dev_get_priv(dev);
	uint pin = npcm8xx_pins[selector].gpio_num;
	uint bank = pin / NPCM8XX_GPIO_PER_BANK;
	uint gpio = (pin % NPCM8XX_GPIO_PER_BANK);
	void __iomem *base = priv->gpio_base + (0x1000 * bank);
	u32 flags = npcm8xx_pins[selector].flags;
	int ret = 0;

	dev_dbg(dev, "set_conf [pin %d][param 0x%x, arg 0x%x]\n",
		pin, param, arg);

	/* Configure pin as gpio function */
	if (param != PIN_CONFIG_SLEW_RATE)
		npcm8xx_set_gpio_func(dev, selector);

	if (is_gpio_persist(dev, bank) &&
	    param != PIN_CONFIG_EVENT_CLEAR) {
		dev_dbg(dev, "retain the state\n");
		return 0;
	}

	switch (param) {
	case PIN_CONFIG_BIAS_DISABLE:
		dev_dbg(dev, "set pin %d bias disable\n", pin);
		clrbits_le32(base + GPIO_PU, BIT(gpio));
		clrbits_le32(base + GPIO_PD, BIT(gpio));
		break;
	case PIN_CONFIG_BIAS_PULL_DOWN:
		dev_dbg(dev, "set pin %d bias pull down\n", pin);
		clrbits_le32(base + GPIO_PU, BIT(gpio));
		setbits_le32(base + GPIO_PD, BIT(gpio));
		break;
	case PIN_CONFIG_BIAS_PULL_UP:
		dev_dbg(dev, "set pin %d bias pull up\n", pin);
		setbits_le32(base + GPIO_PU, BIT(gpio));
		clrbits_le32(base + GPIO_PD, BIT(gpio));
		break;
	case PIN_CONFIG_INPUT_ENABLE:
		dev_dbg(dev, "set pin %d input enable\n", pin);
		setbits_le32(base + GPIO_OEC, BIT(gpio));
		setbits_le32(base + GPIO_IEM, BIT(gpio));
		break;
	case PIN_CONFIG_OUTPUT_ENABLE:
		dev_dbg(dev, "set pin %d output enable\n", pin);
		clrbits_le32(base + GPIO_IEM, BIT(gpio));
		setbits_le32(base + GPIO_OES, BIT(gpio));
	case PIN_CONFIG_OUTPUT:
		dev_dbg(dev, "set pin %d output %d\n", pin, arg);
		if (arg)
			setbits_le32(base + GPIO_DOUT, BIT(gpio));
		else
			clrbits_le32(base + GPIO_DOUT, BIT(gpio));
		clrbits_le32(base + GPIO_IEM, BIT(gpio));
		setbits_le32(base + GPIO_OES, BIT(gpio));
		break;
	case PIN_CONFIG_DRIVE_PUSH_PULL:
		dev_dbg(dev, "set pin %d push pull\n", pin);
		clrbits_le32(base + GPIO_OTYP, BIT(gpio));
		break;
	case PIN_CONFIG_DRIVE_OPEN_DRAIN:
		dev_dbg(dev, "set pin %d open drain\n", pin);
		setbits_le32(base + GPIO_OTYP, BIT(gpio));
		break;
	case PIN_CONFIG_INPUT_DEBOUNCE:
		dev_dbg(dev, "set pin %d input debounce\n", pin);
		setbits_le32(base + GPIO_DBNC, BIT(gpio));
		break;
	case PIN_CONFIG_POLARITY_STATE:
		dev_dbg(dev, "set pin %d active %d\n", pin, arg);
		if (arg)
			setbits_le32(base + GPIO_POL, BIT(gpio));
		else
			clrbits_le32(base + GPIO_POL, BIT(gpio));
		break;
	case PIN_CONFIG_DRIVE_STRENGTH:
		dev_dbg(dev, "set pin %d driver strength %d\n", pin, arg);
		if (DSLO(flags) == arg)
			clrbits_le32(base + GPIO_ODSC, BIT(gpio));
		else if (DSHI(flags) == arg)
			setbits_le32(base + GPIO_ODSC, BIT(gpio));
		else
			ret = -EOPNOTSUPP;
		break;
	case PIN_CONFIG_SLEW_RATE:
		dev_dbg(dev, "set pin %d slew rate %d\n", pin, arg);
		if (!(flags & SLEW)) {
			ret = -EOPNOTSUPP;
			break;
		}
		if (arg)
			setbits_le32(base + GPIO_OSRC, BIT(gpio));
		else
			clrbits_le32(base + GPIO_OSRC, BIT(gpio));
		break;
	case PIN_CONFIG_EVENT_CLEAR:
		dev_dbg(dev, "set pin %d event clear\n", pin);
		clrbits_le32(base + GPIO_EVEN, BIT(gpio));
		setbits_le32(base + GPIO_EVST, BIT(gpio));
		break;
	case  PIN_CONFIG_PERSIST_STATE:
		npcm8xx_gpio_reset_persist(dev, bank, arg);
		break;

	default:
		ret = -EOPNOTSUPP;
	}

	return ret;
}
#endif

static struct pinctrl_ops npcm8xx_pinctrl_ops = {
	.set_state	= pinctrl_generic_set_state,
	.get_pins_count = npcm8xx_get_pins_count,
	.get_pin_name = npcm8xx_get_pin_name,
	.get_groups_count = npcm8xx_get_groups_count,
	.get_group_name = npcm8xx_get_group_name,
	.get_functions_count = npcm8xx_get_functions_count,
	.get_function_name = npcm8xx_get_function_name,
	.pinmux_set = npcm8xx_pinmux_set,
	.pinmux_group_set = npcm8xx_pinmux_group_set,
#if CONFIG_IS_ENABLED(PINCONF)
	.pinconf_num_params = ARRAY_SIZE(npcm8xx_conf_params),
	.pinconf_params = npcm8xx_conf_params,
	.pinconf_set = npcm8xx_pinconf_set,
	.pinconf_group_set = npcm8xx_pinconf_set,
#endif
};

static int npcm8xx_pinctrl_probe(struct udevice *dev)
{
	struct npcm8xx_pinctrl_priv *priv = dev_get_priv(dev);

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

static const struct udevice_id npcm8xx_pinctrl_ids[] = {
	{ .compatible = "nuvoton,npcm845-pinctrl" },
	{ }
};

U_BOOT_DRIVER(pinctrl_npcm8xx) = {
	.name = "nuvoton_npcm8xx_pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = npcm8xx_pinctrl_ids,
	.priv_auto = sizeof(struct npcm8xx_pinctrl_priv),
	.ops = &npcm8xx_pinctrl_ops,
	.probe = npcm8xx_pinctrl_probe,
};
