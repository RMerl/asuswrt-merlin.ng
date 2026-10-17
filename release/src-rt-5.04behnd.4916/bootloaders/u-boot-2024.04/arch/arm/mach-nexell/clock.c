// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Nexell
 * Hyunseok, Jung <hsjung@nexell.co.kr>
 */

#include <common.h>
#include <command.h>
#include <linux/err.h>
#include <asm/io.h>
#include <asm/arch/nexell.h>
#include <asm/arch/clk.h>

/*
 * clock generator macros
 */
#define	I_PLL0_BIT		(0)
#define	I_PLL1_BIT		(1)
#define	I_PLL2_BIT		(2)
#define	I_PLL3_BIT		(3)
#define	I_EXT1_BIT		(4)
#define	I_EXT2_BIT		(5)
#define	I_CLKn_BIT		(7)
#define	I_EXT1_BIT_FORCE	(8)
#define	I_EXT2_BIT_FORCE	(9)

#define	I_CLOCK_NUM		6 /* PLL0, PLL1, PLL2, PLL3, EXT1, EXT2 */

#define I_EXECEPT_CLK		(0)
#define	I_CLOCK_MASK		(((1 << I_CLOCK_NUM) - 1) & ~I_EXECEPT_CLK)

#define	I_PLL0			(1 << I_PLL0_BIT)
#define	I_PLL1			(1 << I_PLL1_BIT)
#define	I_PLL2			(1 << I_PLL2_BIT)
#define	I_PLL3			(1 << I_PLL3_BIT)
#define	I_EXTCLK1		(1 << I_EXT1_BIT)
#define	I_EXTCLK2		(1 << I_EXT2_BIT)
#define	I_EXTCLK1_FORCE		(1 << I_EXT1_BIT_FORCE)
#define	I_EXTCLK2_FORCE		(1 << I_EXT2_BIT_FORCE)

#define	I_PLL_0_1		(I_PLL0    | I_PLL1)
#define	I_PLL_0_2		(I_PLL_0_1 | I_PLL2)
#define	I_PLL_0_3		(I_PLL_0_2 | I_PLL3)
#define	I_CLKnOUT		(0)

#define	I_PCLK			(1 << 16)
#define	I_BCLK			(1 << 17)
#define	I_GATE_PCLK		(1 << 20)
#define	I_GATE_BCLK		(1 << 21)
#define	I_PCLK_MASK		(I_GATE_PCLK | I_PCLK)
#define	I_BCLK_MASK		(I_GATE_BCLK | I_BCLK)

struct clk_dev_peri {
	const char *dev_name;
	void __iomem *base;
	int dev_id;
	int periph_id;
	int clk_step;
	u32 in_mask;
	u32 in_mask1;
	int div_src_0;
	int div_val_0;
	int invert_0;
	int div_src_1;
	int div_val_1;
	int invert_1;
	int in_extclk_1;
	int in_extclk_2;
};

struct clk_dev {
	struct clk  clk;
	struct clk *link;
	const char *name;
	struct clk_dev_peri *peri;
};

struct clk_dev_map {
	unsigned int con_enb;
	unsigned int con_gen[4];
};

#define CLK_PERI_1S(name, devid, id, addr, mk)[id] = \
	{ .dev_name = name, .dev_id = devid, .periph_id = id, .clk_step = 1, \
	.base = (void *)addr, .in_mask = mk, }

#define CLK_PERI_2S(name, devid, id, addr, mk, mk2)[id] = \
	{ .dev_name = name, .dev_id = devid, .periph_id = id, .clk_step = 2, \
	.base = (void *)addr, .in_mask = mk, .in_mask1 = mk2, }

static const char * const clk_core[] = {
	CORECLK_NAME_PLL0, CORECLK_NAME_PLL1, CORECLK_NAME_PLL2,
	CORECLK_NAME_PLL3, CORECLK_NAME_FCLK, CORECLK_NAME_MCLK,
	CORECLK_NAME_BCLK, CORECLK_NAME_PCLK, CORECLK_NAME_HCLK,
};

/*
 * Section ".data" must be used because BSS is not available before relocation,
 * in board_init_f(), respectively! I.e. global variables can not be used!
 */
static struct clk_dev_peri clk_periphs[]
	__section(".data") = {
	CLK_PERI_1S(DEV_NAME_TIMER,	0,	CLK_ID_TIMER_0,
		    PHY_BASEADDR_CLKGEN14, (I_PLL_0_2)),
	CLK_PERI_1S(DEV_NAME_TIMER,	1,	CLK_ID_TIMER_1,
		    PHY_BASEADDR_CLKGEN0, (I_PLL_0_2)),
	CLK_PERI_1S(DEV_NAME_TIMER,	2,	CLK_ID_TIMER_2,
		    PHY_BASEADDR_CLKGEN1, (I_PLL_0_2)),
	CLK_PERI_1S(DEV_NAME_TIMER,	3,	CLK_ID_TIMER_3,
		    PHY_BASEADDR_CLKGEN2, (I_PLL_0_2)),
	CLK_PERI_1S(DEV_NAME_UART,	0,	CLK_ID_UART_0,
		    PHY_BASEADDR_CLKGEN22, (I_PLL_0_2)),
	CLK_PERI_1S(DEV_NAME_UART,	1,	CLK_ID_UART_1,
		    PHY_BASEADDR_CLKGEN24, (I_PLL_0_2)),
	CLK_PERI_1S(DEV_NAME_UART,	2,	CLK_ID_UART_2,
		    PHY_BASEADDR_CLKGEN23, (I_PLL_0_2)),
	CLK_PERI_1S(DEV_NAME_UART,	3,	CLK_ID_UART_3,
		    PHY_BASEADDR_CLKGEN25, (I_PLL_0_2)),
	CLK_PERI_1S(DEV_NAME_UART,	4,	CLK_ID_UART_4,
		    PHY_BASEADDR_CLKGEN26, (I_PLL_0_2)),
	CLK_PERI_1S(DEV_NAME_UART,	5,	CLK_ID_UART_5,
		    PHY_BASEADDR_CLKGEN27, (I_PLL_0_2)),
	CLK_PERI_1S(DEV_NAME_PWM,	0,	CLK_ID_PWM_0,
		    PHY_BASEADDR_CLKGEN13, (I_PLL_0_2)),
	CLK_PERI_1S(DEV_NAME_PWM,	1,	CLK_ID_PWM_1,
		    PHY_BASEADDR_CLKGEN3, (I_PLL_0_2)),
	CLK_PERI_1S(DEV_NAME_PWM,	2,	CLK_ID_PWM_2,
		    PHY_BASEADDR_CLKGEN4, (I_PLL_0_2)),
	CLK_PERI_1S(DEV_NAME_PWM,	3,	CLK_ID_PWM_3,
		    PHY_BASEADDR_CLKGEN5, (I_PLL_0_2)),
	CLK_PERI_1S(DEV_NAME_I2C,	0,	CLK_ID_I2C_0,
		    PHY_BASEADDR_CLKGEN6, (I_GATE_PCLK)),
	CLK_PERI_1S(DEV_NAME_I2C,	1,	CLK_ID_I2C_1,
		    PHY_BASEADDR_CLKGEN7, (I_GATE_PCLK)),
	CLK_PERI_1S(DEV_NAME_I2C,	2,	CLK_ID_I2C_2,
		    PHY_BASEADDR_CLKGEN8, (I_GATE_PCLK)),
	CLK_PERI_2S(DEV_NAME_GMAC,	0,	CLK_ID_GMAC,
		    PHY_BASEADDR_CLKGEN10,
		    (I_PLL_0_3 | I_EXTCLK1 | I_EXTCLK1_FORCE),
		    (I_CLKnOUT)),
	CLK_PERI_2S(DEV_NAME_I2S,	0,	CLK_ID_I2S_0,
		    PHY_BASEADDR_CLKGEN15, (I_PLL_0_3 | I_EXTCLK1),
		    (I_CLKnOUT)),
	CLK_PERI_2S(DEV_NAME_I2S,	1,	CLK_ID_I2S_1,
		    PHY_BASEADDR_CLKGEN16, (I_PLL_0_3 | I_EXTCLK1),
		    (I_CLKnOUT)),
	CLK_PERI_2S(DEV_NAME_I2S,	2,	CLK_ID_I2S_2,
		    PHY_BASEADDR_CLKGEN17, (I_PLL_0_3 | I_EXTCLK1),
		    (I_CLKnOUT)),
	CLK_PERI_1S(DEV_NAME_SDHC,	0,	CLK_ID_SDHC_0,
		    PHY_BASEADDR_CLKGEN18, (I_PLL_0_2 | I_GATE_PCLK)),
	CLK_PERI_1S(DEV_NAME_SDHC,	1,	CLK_ID_SDHC_1,
		    PHY_BASEADDR_CLKGEN19, (I_PLL_0_2 | I_GATE_PCLK)),
	CLK_PERI_1S(DEV_NAME_SDHC,	2,	CLK_ID_SDHC_2,
		    PHY_BASEADDR_CLKGEN20, (I_PLL_0_2 | I_GATE_PCLK)),
	CLK_PERI_1S(DEV_NAME_SPI,	0,	CLK_ID_SPI_0,
		    PHY_BASEADDR_CLKGEN37, (I_PLL_0_2)),
	CLK_PERI_1S(DEV_NAME_SPI,	1,	CLK_ID_SPI_1,
		    PHY_BASEADDR_CLKGEN38, (I_PLL_0_2)),
	CLK_PERI_1S(DEV_NAME_SPI,	2,	CLK_ID_SPI_2,
		    PHY_BASEADDR_CLKGEN39, (I_PLL_0_2)),
};

#define	CLK_PERI_NUM		((int)ARRAY_SIZE(clk_periphs))
#define	CLK_CORE_NUM		((int)ARRAY_SIZE(clk_core))
#define	CLK_DEVS_NUM		(CLK_CORE_NUM + CLK_PERI_NUM)
#define	MAX_DIVIDER		((1 << 8) - 1)	/* 256, align 2 */

static struct clk_dev		st_clk_devs[CLK_DEVS_NUM]
				__section(".data");
#define	clk_dev_get(n)		((struct clk_dev *)&st_clk_devs[n])
#define	clk_container(p)	(container_of(p, struct clk_dev, clk))

/*
 * Core frequencys
 */
struct _core_hz_ {
	unsigned long pll[4];					/* PLL */
	unsigned long cpu_fclk, cpu_bclk;			/* cpu */
	unsigned long mem_fclk, mem_dclk, mem_bclk, mem_pclk;	/* ddr */
	unsigned long bus_bclk, bus_pclk;			/* bus */
#if defined(CONFIG_ARCH_S5P6818)
	unsigned long cci4_bclk, cci4_pclk;			/* cci */
#endif
	/* ip */
	unsigned long g3d_bclk;
	unsigned long coda_bclk, coda_pclk;
#if defined(CONFIG_ARCH_S5P6818)
	unsigned long disp_bclk, disp_pclk;
	unsigned long hdmi_pclk;
#endif
};

/*
 * Section ".data" must be used because BSS is not available before relocation,
 * in board_init_f(), respectively! I.e. global variables can not be used!
 */
/* core clock */
static struct _core_hz_ core_hz __section(".data");

#define	CORE_HZ_SIZE	(sizeof(core_hz) / 4)

/*
 * CLKGEN HW
 */
static inline void clk_dev_bclk(void *base, int on)
{
	struct clk_dev_map *reg = base;
	unsigned int val = readl(&reg->con_enb) & ~(0x3);

	val |= (on ? 3 : 0) & 0x3;	/* always BCLK */
	writel(val, &reg->con_enb);
}

static inline void clk_dev_pclk(void *base, int on)
{
	struct clk_dev_map *reg = base;
	unsigned int val = 0;

	if (!on)
		return;

	val	 = readl(&reg->con_enb) & ~(1 << 3);
	val |= (1 << 3);
	writel(val, &reg->con_enb);
}

static inline void clk_dev_rate(void *base, int step, int src, int div)
{
	struct clk_dev_map *reg = base;
	unsigned int val = 0;

	val  = readl(&reg->con_gen[step << 1]);
	val &= ~(0x07   << 2);
	val |=  (src    << 2);	/* source */
	val	&= ~(0xFF   << 5);
	val	|=  (div - 1) << 5;	/* divider */
	writel(val, &reg->con_gen[step << 1]);
}

static inline void clk_dev_inv(void *base, int step, int inv)
{
	struct clk_dev_map *reg = base;
	unsigned int val = readl(&reg->con_gen[step << 1]) & ~(1 << 1);

	val	|= (inv << 1);
	writel(val, &reg->con_gen[step << 1]);
}

static inline void clk_dev_enb(void *base, int on)
{
	struct clk_dev_map *reg = base;
	unsigned int val = readl(&reg->con_enb) & ~(1 << 2);

	val	|= ((on ? 1 : 0) << 2);
	writel(val, &reg->con_enb);
}

/*
 * CORE FREQUENCY
 *
 * PLL0 [P,M,S]	-------	| | ----- [DIV0] --- CPU-G0
 *			|M| ----- [DIV1] --- BCLK/PCLK
 * PLL1 [P,M,S]	-------	| | ----- [DIV2] --- DDR
 *			|U| ----- [DIV3] --- 3D
 * PLL2 [P,M,S,K]-------| | ----- [DIV4] --- CODA
 *			|X| ----- [DIV5] --- DISPLAY
 * PLL3 [P,M,S,K]-------| | ----- [DIV6] --- HDMI
 *			| | ----- [DIV7] --- CPU-G1
 *			| | ----- [DIV8] --- CCI-400(FASTBUS)
 *
 */

struct	nx_clkpwr_registerset {
	u32 clkmodereg0;	/* 0x000 : Clock Mode Register0 */
	u32 __reserved0;	/* 0x004 */
	u32 pllsetreg[4];	/* 0x008 ~ 0x014 : PLL Setting Register */
	u32 __reserved1[2];	/* 0x018 ~ 0x01C */
	u32 dvoreg[9];		/* 0x020 ~ 0x040 : Divider Setting Register */
	u32 __Reserved2;	/* 0x044 */
	u32 pllsetreg_sscg[6];	/* 0x048 ~ 0x05C */
	u32 __reserved3[8];		/* 0x060 ~ 0x07C */
	u8 __reserved4[0x200 - 0x80];	/* padding (0x80 ~ 0x1FF) */
	u32 gpiowakeupriseenb;	/* 0x200 : GPIO Rising Edge Detect En. Reg. */
	u32 gpiowakeupfallenb;	/* 0x204 : GPIO Falling Edge Detect En. Reg. */
	u32 gpiorstenb;		/* 0x208 : GPIO Reset Enable Register */
	u32 gpiowakeupenb;	/* 0x20C : GPIO Wakeup Source Enable */
	u32 gpiointenb;		/* 0x210 : Interrupt Enable Register */
	u32 gpiointpend;	/* 0x214 : Interrupt Pend Register */
	u32 resetstatus;	/* 0x218 : Reset Status Register */
	u32 intenable;		/* 0x21C : Interrupt Enable Register */
	u32 intpend;		/* 0x220 : Interrupt Pend Register */
	u32 pwrcont;		/* 0x224 : Power Control Register */
	u32 pwrmode;		/* 0x228 : Power Mode Register */
	u32 __reserved5;	/* 0x22C : Reserved Region */
	u32 scratch[3];		/* 0x230 ~ 0x238 : Scratch Register */
	u32 sysrstconfig;	/* 0x23C : System Reset Configuration Reg. */
	u8  __reserved6[0x2A0 - 0x240];	/* padding (0x240 ~ 0x29F) */
	u32 cpupowerdownreq;	/* 0x2A0 : CPU Power Down Request Register */
	u32 cpupoweronreq;	/* 0x2A4 : CPU Power On Request Register */
	u32 cpuresetmode;	/* 0x2A8 : CPU Reset Mode Register */
	u32 cpuwarmresetreq;	/* 0x2AC : CPU Warm Reset Request Register */
	u32 __reserved7;	/* 0x2B0 */
	u32 cpustatus;		/* 0x2B4 : CPU Status Register */
	u8  __reserved8[0x400 - 0x2B8];	/* padding (0x2B8 ~ 0x33F) */
};

static struct nx_clkpwr_registerset * const clkpwr =
	(struct nx_clkpwr_registerset *)PHY_BASEADDR_CLKPWR;

#define	getquotient(v, d)	((v) / (d))

#define	DIV_CPUG0	0
#define	DIV_BUS		1
#define	DIV_MEM		2
#define	DIV_G3D		3
#define	DIV_CODA	4
#if defined(CONFIG_ARCH_S5P6818)
#define	DIV_DISP	5
#define	DIV_HDMI	6
#define	DIV_CPUG1	7
#define	DIV_CCI4	8
#endif

#define	DVO0		3
#define	DVO1		9
#define	DVO2		15
#define	DVO3		21

static unsigned int pll_rate(unsigned int plln, unsigned int xtal)
{
	unsigned int val, val1, nP, nM, nS, nK;
	unsigned int temp = 0;

	val   = clkpwr->pllsetreg[plln];
	val1  = clkpwr->pllsetreg_sscg[plln];
	xtal /= 1000;	/* Unit Khz */

	nP = (val >> 18) & 0x03F;
	nM = (val >>  8) & 0x3FF;
	nS = (val >>  0) & 0x0FF;
	nK = (val1 >> 16) & 0xFFFF;

	if (plln > 1 && nK) {
		temp = (unsigned int)(getquotient((getquotient((nK * 1000),
			65536) * xtal), nP) >> nS);
	}

	temp = (unsigned int)((getquotient((nM * xtal), nP) >> nS) * 1000)
	       + temp;
	return temp;
}

static unsigned int pll_dvo(int dvo)
{
	unsigned int val;

	val = (clkpwr->dvoreg[dvo] & 0x7);
	return val;
}

static unsigned int pll_div(int dvo)
{
	unsigned int val = clkpwr->dvoreg[dvo];

	return  ((((val >> DVO3) & 0x3F) + 1) << 24)  |
			((((val >> DVO2) & 0x3F) + 1) << 16) |
			((((val >> DVO1) & 0x3F) + 1) << 8)  |
			((((val >> DVO0) & 0x3F) + 1) << 0);
}

#define	PLLN_RATE(n)	 (pll_rate(n, CONFIG_SYS_PLLFIN))	/* 0~ 3 */
#define	CPU_FCLK_RATE(n) (pll_rate(pll_dvo(n), CONFIG_SYS_PLLFIN) / \
			 ((pll_div(n) >> 0) & 0x3F))
#define	CPU_BCLK_RATE(n) (pll_rate(pll_dvo(n), CONFIG_SYS_PLLFIN) / \
			 ((pll_div(n) >> 0) & 0x3F) /               \
			 ((pll_div(n) >> 8) & 0x3F))

#define	MEM_FCLK_RATE()	 (pll_rate(pll_dvo(DIV_MEM), CONFIG_SYS_PLLFIN) / \
			 ((pll_div(DIV_MEM) >> 0) & 0x3F) / \
			 ((pll_div(DIV_MEM) >> 8) & 0x3F))

#define	MEM_DCLK_RATE()	 (pll_rate(pll_dvo(DIV_MEM), CONFIG_SYS_PLLFIN) / \
			 ((pll_div(DIV_MEM) >> 0) & 0x3F))

#define	MEM_BCLK_RATE()	 (pll_rate(pll_dvo(DIV_MEM), CONFIG_SYS_PLLFIN) / \
			 ((pll_div(DIV_MEM) >> 0) & 0x3F) / \
			 ((pll_div(DIV_MEM) >> 8) & 0x3F) / \
			 ((pll_div(DIV_MEM) >> 16) & 0x3F))
#define	MEM_PCLK_RATE()	 (pll_rate(pll_dvo(DIV_MEM), CONFIG_SYS_PLLFIN) / \
			 ((pll_div(DIV_MEM) >> 0) & 0x3F) / \
			 ((pll_div(DIV_MEM) >> 8) & 0x3F) / \
			 ((pll_div(DIV_MEM) >> 16) & 0x3F) / \
			 ((pll_div(DIV_MEM) >> 24) & 0x3F))

#define	BUS_BCLK_RATE()	 (pll_rate(pll_dvo(DIV_BUS), CONFIG_SYS_PLLFIN) / \
			 ((pll_div(DIV_BUS) >> 0) & 0x3F))
#define	BUS_PCLK_RATE()	 (pll_rate(pll_dvo(DIV_BUS), CONFIG_SYS_PLLFIN) / \
			 ((pll_div(DIV_BUS) >> 0) & 0x3F) / \
			 ((pll_div(DIV_BUS) >> 8) & 0x3F))

#define	G3D_BCLK_RATE()	 (pll_rate(pll_dvo(DIV_G3D), CONFIG_SYS_PLLFIN) / \
			 ((pll_div(DIV_G3D) >> 0) & 0x3F))

#define	MPG_BCLK_RATE()	 (pll_rate(pll_dvo(DIV_CODA), CONFIG_SYS_PLLFIN) / \
			 ((pll_div(DIV_CODA) >> 0) & 0x3F))
#define	MPG_PCLK_RATE()	 (pll_rate(pll_dvo(DIV_CODA), CONFIG_SYS_PLLFIN) / \
			 ((pll_div(DIV_CODA) >> 0) & 0x3F)	/ \
			 ((pll_div(DIV_CODA) >> 8) & 0x3F))

#if defined(CONFIG_ARCH_S5P6818)
#define	DISP_BCLK_RATE() (pll_rate(pll_dvo(DIV_DISP), CONFIG_SYS_PLLFIN) / \
			 ((pll_div(DIV_DISP) >> 0) & 0x3F))
#define	DISP_PCLK_RATE() (pll_rate(pll_dvo(DIV_DISP), CONFIG_SYS_PLLFIN) / \
			 ((pll_div(DIV_DISP) >> 0) & 0x3F)	/ \
			 ((pll_div(DIV_DISP) >> 8) & 0x3F))

#define	HDMI_PCLK_RATE() (pll_rate(pll_dvo(DIV_HDMI), CONFIG_SYS_PLLFIN) / \
			 ((pll_div(DIV_HDMI) >> 0) & 0x3F))

#define	CCI4_BCLK_RATE() (pll_rate(pll_dvo(DIV_CCI4), CONFIG_SYS_PLLFIN) / \
			 ((pll_div(DIV_CCI4) >> 0) & 0x3F))
#define	CCI4_PCLK_RATE() (pll_rate(pll_dvo(DIV_CCI4), CONFIG_SYS_PLLFIN) / \
			 ((pll_div(DIV_CCI4) >> 0) & 0x3F)	/ \
			 ((pll_div(DIV_CCI4) >> 8) & 0x3F))
#endif

static void core_update_rate(int type)
{
	switch (type) {
	case  0:
		core_hz.pll[0] = PLLN_RATE(0); break;
	case  1:
		core_hz.pll[1] = PLLN_RATE(1); break;
	case  2:
		core_hz.pll[2] = PLLN_RATE(2); break;
	case  3:
		core_hz.pll[3] = PLLN_RATE(3); break;
	case  4:
		core_hz.cpu_fclk = CPU_FCLK_RATE(DIV_CPUG0); break;
	case  5:
		core_hz.mem_fclk = MEM_FCLK_RATE(); break;
	case  6:
		core_hz.bus_bclk = BUS_BCLK_RATE(); break;
	case  7:
		core_hz.bus_pclk = BUS_PCLK_RATE(); break;
	case  8:
		core_hz.cpu_bclk = CPU_BCLK_RATE(DIV_CPUG0); break;
	case  9:
		core_hz.mem_dclk = MEM_DCLK_RATE(); break;
	case 10:
		core_hz.mem_bclk = MEM_BCLK_RATE(); break;
	case 11:
		core_hz.mem_pclk = MEM_PCLK_RATE(); break;
	case 12:
		core_hz.g3d_bclk = G3D_BCLK_RATE(); break;
	case 13:
		core_hz.coda_bclk = MPG_BCLK_RATE(); break;
	case 14:
		core_hz.coda_pclk = MPG_PCLK_RATE(); break;
#if defined(CONFIG_ARCH_S5P6818)
	case 15:
		core_hz.disp_bclk = DISP_BCLK_RATE(); break;
	case 16:
		core_hz.disp_pclk = DISP_PCLK_RATE(); break;
	case 17:
		core_hz.hdmi_pclk = HDMI_PCLK_RATE(); break;
	case 18:
		core_hz.cci4_bclk = CCI4_BCLK_RATE(); break;
	case 19:
		core_hz.cci4_pclk = CCI4_PCLK_RATE(); break;
#endif
	};
}

static unsigned long core_get_rate(int type)
{
	unsigned long rate = 0;

	switch (type) {
	case  0:
		rate = core_hz.pll[0];		break;
	case  1:
		rate = core_hz.pll[1];		break;
	case  2:
		rate = core_hz.pll[2];		break;
	case  3:
		rate = core_hz.pll[3];		break;
	case  4:
		rate = core_hz.cpu_fclk;	break;
	case  5:
		rate = core_hz.mem_fclk;	break;
	case  6:
		rate = core_hz.bus_bclk;	break;
	case  7:
		rate = core_hz.bus_pclk;	break;
	case  8:
		rate = core_hz.cpu_bclk;	break;
	case  9:
		rate = core_hz.mem_dclk;	break;
	case 10:
		rate = core_hz.mem_bclk;	break;
	case 11:
		rate = core_hz.mem_pclk;	break;
	case 12:
		rate = core_hz.g3d_bclk;	break;
	case 13:
		rate = core_hz.coda_bclk;	break;
	case 14:
		rate = core_hz.coda_pclk;	break;
#if defined(CONFIG_ARCH_S5P6818)
	case 15:
		rate = core_hz.disp_bclk;	break;
	case 16:
		rate = core_hz.disp_pclk;	break;
	case 17:
		rate = core_hz.hdmi_pclk;	break;
	case 18:
		rate = core_hz.cci4_bclk;	break;
	case 19:
		rate = core_hz.cci4_pclk;	break;
#endif
	default:
		printf("unknown core clock type %d ...\n", type);
		break;
	};
	return rate;
}

static long core_set_rate(struct clk *clk, long rate)
{
	return clk->rate;
}

static void core_rate_init(void)
{
	int i;

	for (i = 0; i < CORE_HZ_SIZE; i++)
		core_update_rate(i);
}

/*
 * Clock Interfaces
 */
static inline long clk_divide(long rate, long request,
			      int align, int *divide)
{
	int div = (rate / request);
	int max = MAX_DIVIDER & ~(align - 1);
	int adv = (div & ~(align - 1)) + align;
	long ret;

	if (!div) {
		if (divide)
			*divide = 1;
		return rate;
	}

	if (div != 1)
		div &= ~(align - 1);

	if (div != adv && abs(request - rate / div) > abs(request - rate / adv))
		div = adv;

	div = (div > max ? max : div);
	if (divide)
		*divide = div;

	ret = rate / div;
	return ret;
}

void clk_put(struct clk *clk)
{
}

struct clk *clk_get(const char *id)
{
	struct clk_dev *cdev = clk_dev_get(0);
	struct clk *clk = NULL;
	const char *str = NULL, *c = NULL;
	int i, devid;

	if (id)
		str = id;

	for (i = 0; i < CLK_DEVS_NUM; i++, cdev++) {
		if (!cdev->name)
			continue;
		if (!strncmp(cdev->name, str, strlen(cdev->name))) {
			c = strrchr((const char *)str, (int)'.');
			if (!c || !cdev->peri)
				break;
		devid = dectoul(++c, NULL);
		if (cdev->peri->dev_id == devid)
			break;
		}
	}
	if (i < CLK_DEVS_NUM)
		clk = &cdev->clk;
	else
		clk = &(clk_dev_get(7))->clk;	/* pclk */

	return clk ? clk : ERR_PTR(-ENOENT);
}

long clk_round_rate(struct clk *clk, unsigned long rate)
{
	struct clk_dev *pll = NULL, *cdev = clk_container(clk);
	struct clk_dev_peri *peri = cdev->peri;
	unsigned long request = rate, rate_hz = 0;
	unsigned int mask;
	int step, div[2] = { 0, };
	int i, n, clk2 = 0;
	int start_src = 0, max_src = I_CLOCK_NUM;
	short s1 = 0, s2 = 0, d1 = 0, d2 = 0;

	if (!peri)
		return core_set_rate(clk, rate);

	step = peri->clk_step;
	mask = peri->in_mask;
	debug("clk: %s.%d request = %ld [input=0x%x]\n", peri->dev_name,
	      peri->dev_id, rate, mask);

	if (!(I_CLOCK_MASK & mask)) {
		if (I_PCLK_MASK & mask)
			return core_get_rate(CORECLK_ID_PCLK);
		else if (I_BCLK_MASK & mask)
			return core_get_rate(CORECLK_ID_BCLK);
		else
			return clk->rate;
	}

next:
	if (peri->in_mask &  I_EXTCLK1_FORCE) {
		start_src = 4; max_src = 5;
	}
	for (n = start_src ; max_src > n; n++) {
		if (!(((mask & I_CLOCK_MASK) >> n) & 0x1))
			continue;

		if (n == I_EXT1_BIT) {
			rate = peri->in_extclk_1;
		} else if (n == I_EXT2_BIT) {
			rate = peri->in_extclk_2;
		} else {
			pll  = clk_dev_get(n);
			rate = pll->clk.rate;
		}

		if (!rate)
			continue;

		for (i = 0; step > i ; i++)
			rate = clk_divide(rate, request, 2, &div[i]);

		if (rate_hz && (abs(rate - request) > abs(rate_hz - request)))
			continue;

		debug("clk: %s.%d, pll.%d[%lu] request[%ld] calc[%ld]\n",
		      peri->dev_name, peri->dev_id, n, pll->clk.rate,
		      request, rate);

		if (clk2) {
			s1 = -1, d1 = -1;	/* not use */
			s2 =  n, d2 = div[0];
		} else {
			s1 = n, d1 = div[0];
			s2 = I_CLKn_BIT, d2 = div[1];
		}
		rate_hz = rate;
	}

	/* search 2th clock from input */
	if (!clk2 && abs(rate_hz - request) &&
	    peri->in_mask1 & ((1 << I_CLOCK_NUM) - 1)) {
		clk2 = 1;
		mask = peri->in_mask1;
		step = 1;
		goto next;
	}
	if (peri->in_mask &  I_EXTCLK1_FORCE) {
		if (s1 == 0) {
			s1 = 4; s2 = 7;
			d1 = 1; d2 = 1;
		}
	}

	peri->div_src_0 = s1, peri->div_val_0 = d1;
	peri->div_src_1 = s2, peri->div_val_1 = d2;
	clk->rate = rate_hz;

	debug("clk: %s.%d, step[%d] src[%d,%d] %ld", peri->dev_name,
	      peri->dev_id, peri->clk_step, peri->div_src_0, peri->div_src_1,
	      rate);
	debug("/(div0: %d * div1: %d) = %ld, %ld diff (%ld)\n",
	      peri->div_val_0, peri->div_val_1, rate_hz, request,
	      abs(rate_hz - request));

	return clk->rate;
}

unsigned long clk_get_rate(struct clk *clk)
{
	struct clk_dev *cdev = clk_container(clk);

	if (cdev->link)
		clk = cdev->link;
	return clk->rate;
}

int clk_set_rate(struct clk *clk, unsigned long rate)
{
	struct clk_dev *cdev = clk_container(clk);
	struct clk_dev_peri *peri = cdev->peri;
	int i;

	if (!peri)
		return core_set_rate(clk, rate);

	clk_round_rate(clk, rate);

	for (i = 0; peri->clk_step > i ; i++)	{
		int s = (i == 0 ? peri->div_src_0 : peri->div_src_1);
		int d = (i == 0 ? peri->div_val_0 : peri->div_val_1);

		if (-1 == s)
			continue;

		clk_dev_rate(peri->base, i, s, d);

		debug("clk: %s.%d (%p) set_rate [%d] src[%d] div[%d]\n",
		      peri->dev_name, peri->dev_id, peri->base, i, s, d);
	}

	return clk->rate;
}

int clk_enable(struct clk *clk)
{
	struct clk_dev *cdev = clk_container(clk);
	struct clk_dev_peri *peri = cdev->peri;
	int i = 0, inv = 0;

	if (!peri)
		return 0;

	debug("clk: %s.%d enable (BCLK=%s, PCLK=%s)\n", peri->dev_name,
	      peri->dev_id, I_GATE_BCLK & peri->in_mask ? "ON" : "PASS",
	      I_GATE_PCLK & peri->in_mask ? "ON" : "PASS");

	if (!(I_CLOCK_MASK & peri->in_mask)) {
		/* Gated BCLK/PCLK enable */
		if (I_GATE_BCLK & peri->in_mask)
			clk_dev_bclk(peri->base, 1);

		if (I_GATE_PCLK & peri->in_mask)
			clk_dev_pclk(peri->base, 1);

		return 0;
	}

	/* invert */
	inv = peri->invert_0;
	for (; peri->clk_step > i; i++, inv = peri->invert_1)
		clk_dev_inv(peri->base, i, inv);

	/* Gated BCLK/PCLK enable */
	if (I_GATE_BCLK & peri->in_mask)
		clk_dev_bclk(peri->base, 1);

	if (I_GATE_PCLK & peri->in_mask)
		clk_dev_pclk(peri->base, 1);

	/* restore clock rate */
	for (i = 0; peri->clk_step > i ; i++)	{
		int s = (i == 0 ? peri->div_src_0 : peri->div_src_1);
		int d = (i == 0 ? peri->div_val_0 : peri->div_val_1);

		if (s == -1)
			continue;
		clk_dev_rate(peri->base, i, s, d);
	}

	clk_dev_enb(peri->base, 1);

	return 0;
}

void clk_disable(struct clk *clk)
{
	struct clk_dev *cdev = clk_container(clk);
	struct clk_dev_peri *peri = cdev->peri;

	if (!peri)
		return;

	debug("clk: %s.%d disable\n", peri->dev_name, peri->dev_id);

	if (!(I_CLOCK_MASK & peri->in_mask)) {
		/* Gated BCLK/PCLK disable */
		if (I_GATE_BCLK & peri->in_mask)
			clk_dev_bclk(peri->base, 0);

		if (I_GATE_PCLK & peri->in_mask)
			clk_dev_pclk(peri->base, 0);

		return;
	}

	clk_dev_rate(peri->base, 0, 7, 256);	/* for power save */
	clk_dev_enb(peri->base, 0);

	/* Gated BCLK/PCLK disable */
	if (I_GATE_BCLK & peri->in_mask)
		clk_dev_bclk(peri->base, 0);

	if (I_GATE_PCLK & peri->in_mask)
		clk_dev_pclk(peri->base, 0);
}

/*
 * Core clocks APIs
 */
void __init clk_init(void)
{
	struct clk_dev *cdev = st_clk_devs;
	struct clk_dev_peri *peri = clk_periphs;
	struct clk *clk = NULL;
	int i = 0;

	memset(cdev, 0, sizeof(st_clk_devs));
	core_rate_init();

	for (i = 0; (CLK_CORE_NUM + CLK_PERI_NUM) > i; i++, cdev++) {
		if (i < CLK_CORE_NUM) {
			cdev->name = clk_core[i];
			clk = &cdev->clk;
			clk->rate = core_get_rate(i);
			continue;
		}

		peri = &clk_periphs[i - CLK_CORE_NUM];
		peri->base = (void *)peri->base;

		cdev->peri = peri;
		cdev->name = peri->dev_name;

		if (!(I_CLOCK_MASK & peri->in_mask)) {
			if (I_BCLK_MASK & peri->in_mask)
				cdev->clk.rate = core_get_rate(CORECLK_ID_BCLK);
			if (I_PCLK_MASK & peri->in_mask)
				cdev->clk.rate = core_get_rate(CORECLK_ID_PCLK);
		}

		/* prevent uart clock disable for low step debug message */
		#ifndef CONFIG_DEBUG_UART
		if (peri->dev_name) {
			#ifdef CONFIG_BACKLIGHT_PWM
			if (!strcmp(peri->dev_name, DEV_NAME_PWM))
				continue;
			#endif
		}
		#endif
	}
	debug("CPU : Clock Generator= %d EA, ", CLK_DEVS_NUM);
}
