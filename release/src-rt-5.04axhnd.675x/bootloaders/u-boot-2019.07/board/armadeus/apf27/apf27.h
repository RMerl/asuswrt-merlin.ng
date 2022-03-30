/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2008-2013 Eric Jarrige <eric.jarrige@armadeus.org>
 */

#ifndef __APF27_H
#define __APF27_H

/* FPGA program pin configuration */
#define ACFG_FPGA_PWR	(GPIO_PORTF | 19)	/* FPGA prog pin  */
#define ACFG_FPGA_PRG	(GPIO_PORTF | 11)	/* FPGA prog pin  */
#define ACFG_FPGA_CLK	(GPIO_PORTF | 15)	/* FPGA clk pin   */
#define ACFG_FPGA_RDATA	0xD6000000		/* FPGA data addr */
#define ACFG_FPGA_WDATA	0xD6000000		/* FPGA data addr */
#define ACFG_FPGA_INIT	(GPIO_PORTF | 12)	/* FPGA init pin  */
#define ACFG_FPGA_DONE	(GPIO_PORTF | 9)	/* FPGA done pin  */
#define ACFG_FPGA_RW	(GPIO_PORTF | 21)	/* FPGA done pin  */
#define ACFG_FPGA_CS	(GPIO_PORTF | 22)	/* FPGA done pin  */
#define ACFG_FPGA_SUSPEND (GPIO_PORTF | 10)	/* FPGA done pin  */
#define ACFG_FPGA_RESET	(GPIO_PORTF | 7)	/* FPGA done pin  */

/* MMC pin */
#define PC_PWRON	(GPIO_PORTF | 16)

/*
 * MPU CLOCK source before PLL
 * ACFG_CLK_FREQ (2/3 MPLL clock or ext 266 MHZ)
 */
#define ACFG_MPCTL0_VAL		0x01EF15D5	/* 399.000 MHz */
#define ACFG_MPCTL1_VAL		0
#define CONFIG_MPLL_FREQ	399

#define ACFG_CLK_FREQ	(CONFIG_MPLL_FREQ*2/3) /* 266 MHz */

/* Serial clock source before PLL (should be named ACFG_SYSPLL_CLK_FREQ)*/
#define ACFG_SPCTL0_VAL		0x0475206F	/* 299.99937 MHz */
#define ACFG_SPCTL1_VAL		0
#define CONFIG_SPLL_FREQ	300		/* MHz */

/* ARM bus frequency (have to be a CONFIG_MPLL_FREQ ratio) */
#define CONFIG_ARM_FREQ		399	/* up to 400 MHz */

/* external bus frequency (have to be a ACFG_CLK_FREQ ratio) */
#define CONFIG_HCLK_FREQ	133	/* (ACFG_CLK_FREQ/2) */

#define CONFIG_PERIF1_FREQ	16	/* 16.625 MHz UART, GPT, PWM */
#define CONFIG_PERIF2_FREQ	33	/* 33.25 MHz CSPI and SDHC */
#define CONFIG_PERIF3_FREQ	33	/* 33.25 MHz LCD */
#define CONFIG_PERIF4_FREQ	33	/* 33.25 MHz CSI */
#define CONFIG_SSI1_FREQ	66	/* 66.50 MHz SSI1 */
#define CONFIG_SSI2_FREQ	66	/* 66.50 MHz SSI2 */
#define CONFIG_MSHC_FREQ	66	/* 66.50 MHz MSHC */
#define CONFIG_H264_FREQ	66	/* 66.50 MHz H264 */
#define CONFIG_CLK0_DIV		3	/* Divide CLK0 by 4 */
#define CONFIG_CLK0_EN		1	/* CLK0 enabled */

/* external bus frequency (have to be a CONFIG_HCLK_FREQ ratio) */
#define CONFIG_NFC_FREQ		44	/* NFC Clock up to 44 MHz wh 133MHz */

/* external serial bus frequency (have to be a CONFIG_SPLL_FREQ ratio) */
#define CONFIG_USB_FREQ		60	/* 60 MHz */

/*
 * SDRAM
 */
#if (ACFG_SDRAM_MBYTE_SYZE == 64) /* micron MT46H16M32LF -6 */
/* micron 64MB */
#define ACFG_SDRAM_NUM_COL		9  /* 8, 9, 10 or 11
					    * column address bits
					    */
#define ACFG_SDRAM_NUM_ROW		13 /* 11, 12 or 13
					    * row address bits
					    */
#define ACFG_SDRAM_REFRESH		3  /* 0=OFF 1=2048
					    * 2=4096 3=8192 refresh
					    */
#define ACFG_SDRAM_EXIT_PWD		25 /* ns exit power
					    * down delay
					    */
#define ACFG_SDRAM_W2R_DELAY		1  /* write to read
					    * cycle delay > 0
					    */
#define ACFG_SDRAM_ROW_PRECHARGE_DELAY	18 /* ns */
#define ACFG_SDRAM_TMRD_DELAY		2  /* Load mode register
					    * cycle delay 1..4
					    */
#define ACFG_SDRAM_TWR_DELAY		1  /* LPDDR: 0=2ck 1=3ck
					    * SDRAM: 0=1ck 1=2ck
					    */
#define ACFG_SDRAM_RAS_DELAY		42 /* ns ACTIVE-to-PRECHARGE delay */
#define ACFG_SDRAM_RRD_DELAY		12 /* ns ACTIVE-to-ACTIVE delay */
#define ACFG_SDRAM_RCD_DELAY		18 /* ns Row to Column delay */
#define ACFG_SDRAM_RC_DELAY		70 /* ns Row cycle delay (tRFC
					    * refresh to command)
					    */
#define ACFG_SDRAM_CLOCK_CYCLE_CL_1	0 /* ns clock cycle time
					   * estimated fo CL=1
					   * 0=force 3 for lpddr
					   */
#define ACFG_SDRAM_PARTIAL_ARRAY_SR	0  /* 0=full 1=half 2=quater
					    * 3=Eighth 4=Sixteenth
					    */
#define ACFG_SDRAM_DRIVE_STRENGH	0  /* 0=Full-strength 1=half
					    * 2=quater 3=Eighth
					    */
#define ACFG_SDRAM_BURST_LENGTH		3  /* 2^N BYTES (N=0..3) */
#define ACFG_SDRAM_SINGLE_ACCESS	0  /* 1= single access
					    * 0 = Burst mode
					    */
#endif

#if (ACFG_SDRAM_MBYTE_SYZE == 128)
/* micron 128MB */
#define ACFG_SDRAM_NUM_COL		9  /* 8, 9, 10 or 11
					    * column address bits
					    */
#define ACFG_SDRAM_NUM_ROW		14 /* 11, 12 or 13
					    * row address bits
					    */
#define ACFG_SDRAM_REFRESH		3  /* 0=OFF 1=2048
					    * 2=4096 3=8192 refresh
					    */
#define ACFG_SDRAM_EXIT_PWD		25 /* ns exit power
					    * down delay
					    */
#define ACFG_SDRAM_W2R_DELAY		1  /* write to read
					    * cycle delay > 0
					    */
#define ACFG_SDRAM_ROW_PRECHARGE_DELAY	18 /* ns */
#define ACFG_SDRAM_TMRD_DELAY		2  /* Load mode register
					    * cycle delay 1..4
					    */
#define ACFG_SDRAM_TWR_DELAY		1  /* LPDDR: 0=2ck 1=3ck
					    * SDRAM: 0=1ck 1=2ck
					    */
#define ACFG_SDRAM_RAS_DELAY		42 /* ns ACTIVE-to-PRECHARGE delay */
#define ACFG_SDRAM_RRD_DELAY		12 /* ns ACTIVE-to-ACTIVE delay */
#define ACFG_SDRAM_RCD_DELAY		18 /* ns Row to Column delay */
#define ACFG_SDRAM_RC_DELAY		70 /* ns Row cycle delay (tRFC
					    * refresh to command)
					    */
#define ACFG_SDRAM_CLOCK_CYCLE_CL_1	0 /* ns clock cycle time
					   * estimated fo CL=1
					   * 0=force 3 for lpddr
					   */
#define ACFG_SDRAM_PARTIAL_ARRAY_SR	0  /* 0=full 1=half 2=quater
					    * 3=Eighth 4=Sixteenth
					    */
#define ACFG_SDRAM_DRIVE_STRENGH	0  /* 0=Full-strength 1=half
					    * 2=quater 3=Eighth
					    */
#define ACFG_SDRAM_BURST_LENGTH		3  /* 2^N BYTES (N=0..3) */
#define ACFG_SDRAM_SINGLE_ACCESS	0  /* 1= single access
					    * 0 = Burst mode
					    */
#endif

#if (ACFG_SDRAM_MBYTE_SYZE == 256)
/* micron 256MB */
#define ACFG_SDRAM_NUM_COL		10  /* 8, 9, 10 or 11
					     * column address bits
					     */
#define ACFG_SDRAM_NUM_ROW		14 /* 11, 12 or 13
					    * row address bits
					    */
#define ACFG_SDRAM_REFRESH		3  /* 0=OFF 1=2048
					    * 2=4096 3=8192 refresh
					    */
#define ACFG_SDRAM_EXIT_PWD		25 /* ns exit power
					    * down delay
					    */
#define ACFG_SDRAM_W2R_DELAY		1  /* write to read cycle
					    * delay > 0
					    */
#define ACFG_SDRAM_ROW_PRECHARGE_DELAY	18 /* ns */
#define ACFG_SDRAM_TMRD_DELAY		2  /* Load mode register
					    * cycle delay 1..4
					    */
#define ACFG_SDRAM_TWR_DELAY		1  /* LPDDR: 0=2ck 1=3ck
					    * SDRAM: 0=1ck 1=2ck
					    */
#define ACFG_SDRAM_RAS_DELAY		42 /* ns ACTIVE-to-PRECHARGE delay */
#define ACFG_SDRAM_RRD_DELAY		12 /* ns ACTIVE-to-ACTIVE delay */
#define ACFG_SDRAM_RCD_DELAY		18 /* ns Row to Column delay */
#define ACFG_SDRAM_RC_DELAY		70 /* ns Row cycle delay (tRFC
					    * refresh to command)
					    */
#define ACFG_SDRAM_CLOCK_CYCLE_CL_1	0 /* ns clock cycle time
					   * estimated fo CL=1
					   * 0=force 3 for lpddr
					   */
#define ACFG_SDRAM_PARTIAL_ARRAY_SR	0  /* 0=full 1=half 2=quater
					    * 3=Eighth 4=Sixteenth
					    */
#define ACFG_SDRAM_DRIVE_STRENGH	0  /* 0=Full-strength
					    * 1=half
					    * 2=quater
					    * 3=Eighth
					    */
#define ACFG_SDRAM_BURST_LENGTH		3  /* 2^N BYTES (N=0..3) */
#define ACFG_SDRAM_SINGLE_ACCESS	0  /* 1= single access
					    * 0 = Burst mode
					    */
#endif

/*
 * External interface
 */
/*
 * CSCRxU_VAL:
 * 31| x | x | x x |x x x x| x x | x | x  |x x x x|16
 *   |SP |WP | BCD |  BCS  | PSZ |PME|SYNC|  DOL  |
 *
 * 15| x x  | x x x x x x | x | x x x x | x x x x |0
 *   | CNC  |     WSC     |EW |   WWS   |   EDC   |
 *
 * CSCRxL_VAL:
 * 31|  x x x x  | x x x x  | x x x x  | x x x x  |16
 *   |    OEA    |   OEN    |   EBWA   |   EBWN   |
 * 15|x x x x| x |x x x |x x x x| x | x | x  | x  | 0
 *   |  CSA  |EBC| DSZ  |  CSN  |PSR|CRE|WRAP|CSEN|
 *
 * CSCRxA_VAL:
 * 31|  x x x x  | x x x x  | x x x x  | x x x x  |16
 *   |   EBRA    |   EBRN   |   RWA    |   RWN    |
 * 15| x | x x |x x x|x x|x x|x x| x | x | x  | x | 0
 *   |MUM| LAH | LBN |LBA|DWW|DCT|WWU|AGE|CNC2|FCE|
 */

/* CS0 configuration for 16 bit nor flash */
#define ACFG_CS0U_VAL	0x0000CC03
#define ACFG_CS0L_VAL	0xa0330D01
#define ACFG_CS0A_VAL	0x00220800

#define ACFG_CS1U_VAL	0x00000f00
#define ACFG_CS1L_VAL	0x00000D01
#define ACFG_CS1A_VAL	0

#define ACFG_CS2U_VAL	0
#define ACFG_CS2L_VAL	0
#define ACFG_CS2A_VAL	0

#define ACFG_CS3U_VAL	0
#define ACFG_CS3L_VAL	0
#define ACFG_CS3A_VAL	0

#define ACFG_CS4U_VAL	0
#define ACFG_CS4L_VAL	0
#define ACFG_CS4A_VAL	0

/* FPGA 16 bit data bus */
#define ACFG_CS5U_VAL	0x00000600
#define ACFG_CS5L_VAL	0x00000D01
#define ACFG_CS5A_VAL	0

#define ACFG_EIM_VAL	0x00002200


/*
 * FPGA specific settings
 */

/* CLKO */
#define ACFG_CCSR_VAL 0x00000305
/* drive strength CLKO set to 2 */
#define ACFG_DSCR10_VAL 0x00020000
/* drive strength A1..A12 set to 2 */
#define ACFG_DSCR3_VAL 0x02AAAAA8
/* drive strength ctrl */
#define ACFG_DSCR7_VAL 0x00020880
/* drive strength data */
#define ACFG_DSCR2_VAL 0xAAAAAAAA


/*
 * Default configuration for GPIOs and peripherals
 */
#define ACFG_DDIR_A_VAL		0x00000000
#define ACFG_OCR1_A_VAL		0x00000000
#define ACFG_OCR2_A_VAL		0x00000000
#define ACFG_ICFA1_A_VAL	0xFFFFFFFF
#define ACFG_ICFA2_A_VAL	0xFFFFFFFF
#define ACFG_ICFB1_A_VAL	0xFFFFFFFF
#define ACFG_ICFB2_A_VAL	0xFFFFFFFF
#define ACFG_DR_A_VAL		0x00000000
#define ACFG_GIUS_A_VAL		0xFFFFFFFF
#define ACFG_ICR1_A_VAL		0x00000000
#define ACFG_ICR2_A_VAL		0x00000000
#define ACFG_IMR_A_VAL		0x00000000
#define ACFG_GPR_A_VAL		0x00000000
#define ACFG_PUEN_A_VAL		0xFFFFFFFF

#define ACFG_DDIR_B_VAL		0x00000000
#define ACFG_OCR1_B_VAL		0x00000000
#define ACFG_OCR2_B_VAL		0x00000000
#define ACFG_ICFA1_B_VAL	0xFFFFFFFF
#define ACFG_ICFA2_B_VAL	0xFFFFFFFF
#define ACFG_ICFB1_B_VAL	0xFFFFFFFF
#define ACFG_ICFB2_B_VAL	0xFFFFFFFF
#define ACFG_DR_B_VAL		0x00000000
#define ACFG_GIUS_B_VAL		0xFF3FFFF0
#define ACFG_ICR1_B_VAL		0x00000000
#define ACFG_ICR2_B_VAL		0x00000000
#define ACFG_IMR_B_VAL		0x00000000
#define ACFG_GPR_B_VAL		0x00000000
#define ACFG_PUEN_B_VAL		0xFFFFFFFF

#define ACFG_DDIR_C_VAL		0x00000000
#define ACFG_OCR1_C_VAL		0x00000000
#define ACFG_OCR2_C_VAL		0x00000000
#define ACFG_ICFA1_C_VAL	0xFFFFFFFF
#define ACFG_ICFA2_C_VAL	0xFFFFFFFF
#define ACFG_ICFB1_C_VAL	0xFFFFFFFF
#define ACFG_ICFB2_C_VAL	0xFFFFFFFF
#define ACFG_DR_C_VAL		0x00000000
#define ACFG_GIUS_C_VAL		0xFFFFC07F
#define ACFG_ICR1_C_VAL		0x00000000
#define ACFG_ICR2_C_VAL		0x00000000
#define ACFG_IMR_C_VAL		0x00000000
#define ACFG_GPR_C_VAL		0x00000000
#define ACFG_PUEN_C_VAL		0xFFFFFF87

#define ACFG_DDIR_D_VAL		0x00000000
#define ACFG_OCR1_D_VAL		0x00000000
#define ACFG_OCR2_D_VAL		0x00000000
#define ACFG_ICFA1_D_VAL	0xFFFFFFFF
#define ACFG_ICFA2_D_VAL	0xFFFFFFFF
#define ACFG_ICFB1_D_VAL	0xFFFFFFFF
#define ACFG_ICFB2_D_VAL	0xFFFFFFFF
#define ACFG_DR_D_VAL		0x00000000
#define ACFG_GIUS_D_VAL		0xFFFFFFFF
#define ACFG_ICR1_D_VAL		0x00000000
#define ACFG_ICR2_D_VAL		0x00000000
#define ACFG_IMR_D_VAL		0x00000000
#define ACFG_GPR_D_VAL		0x00000000
#define ACFG_PUEN_D_VAL		0xFFFFFFFF

#define ACFG_DDIR_E_VAL		0x00000000
#define ACFG_OCR1_E_VAL		0x00000000
#define ACFG_OCR2_E_VAL		0x00000000
#define ACFG_ICFA1_E_VAL	0xFFFFFFFF
#define ACFG_ICFA2_E_VAL	0xFFFFFFFF
#define ACFG_ICFB1_E_VAL	0xFFFFFFFF
#define ACFG_ICFB2_E_VAL	0xFFFFFFFF
#define ACFG_DR_E_VAL		0x00000000
#define ACFG_GIUS_E_VAL		0xFCFFCCF8
#define ACFG_ICR1_E_VAL		0x00000000
#define ACFG_ICR2_E_VAL		0x00000000
#define ACFG_IMR_E_VAL		0x00000000
#define ACFG_GPR_E_VAL		0x00000000
#define ACFG_PUEN_E_VAL		0xFFFFFFFF

#define ACFG_DDIR_F_VAL		0x00000000
#define ACFG_OCR1_F_VAL		0x00000000
#define ACFG_OCR2_F_VAL		0x00000000
#define ACFG_ICFA1_F_VAL	0xFFFFFFFF
#define ACFG_ICFA2_F_VAL	0xFFFFFFFF
#define ACFG_ICFB1_F_VAL	0xFFFFFFFF
#define ACFG_ICFB2_F_VAL	0xFFFFFFFF
#define ACFG_DR_F_VAL		0x00000000
#define ACFG_GIUS_F_VAL		0xFF7F8000
#define ACFG_ICR1_F_VAL		0x00000000
#define ACFG_ICR2_F_VAL		0x00000000
#define ACFG_IMR_F_VAL		0x00000000
#define ACFG_GPR_F_VAL		0x00000000
#define ACFG_PUEN_F_VAL		0xFFFFFFFF

/* Enforce DDR signal strengh & enable USB/PP/DMA burst override bits */
#define ACFG_GPCR_VAL		0x0003000F

#define ACFG_ESDMISC_VAL	ESDMISC_LHD+ESDMISC_MDDREN

/* FMCR select num LPDDR RAMs and nand 16bits, 2KB pages */
#if (CONFIG_NR_DRAM_BANKS == 1)
#define ACFG_FMCR_VAL 0xFFFFFFF9
#elif (CONFIG_NR_DRAM_BANKS == 2)
#define ACFG_FMCR_VAL 0xFFFFFFFB
#endif

#define ACFG_AIPI1_PSR0_VAL	0x20040304
#define ACFG_AIPI1_PSR1_VAL	0xDFFBFCFB
#define ACFG_AIPI2_PSR0_VAL	0x00000000
#define ACFG_AIPI2_PSR1_VAL	0xFFFFFFFF

/* PCCR enable DMA FEC I2C1 IIM SDHC1 */
#define ACFG_PCCR0_VAL		0x05070410
#define ACFG_PCCR1_VAL		0xA14A0608

/*
 * From here, there should not be any user configuration.
 * All Equations are automatic
 */

/* fixme none integer value (7.5ns) => 2*hclock = 15ns */
#define ACFG_2XHCLK_LGTH	(2000/CONFIG_HCLK_FREQ)	/* ns */

/* USB 60 MHz ; ARM up to 400; HClK up to 133MHz*/
#define CSCR_MASK 0x0300800D

#define ACFG_CSCR_VAL						\
	(CSCR_MASK						\
	|((((CONFIG_SPLL_FREQ/CONFIG_USB_FREQ)-1)&0x07) << 28)	\
	|((((CONFIG_MPLL_FREQ/CONFIG_ARM_FREQ)-1)&0x03) << 12)	\
	|((((ACFG_CLK_FREQ/CONFIG_HCLK_FREQ)-1)&0x03) << 8))

/* SSIx CLKO NFC H264 MSHC */
#define ACFG_PCDR0_VAL\
	(((((ACFG_CLK_FREQ/CONFIG_MSHC_FREQ)-1)&0x3F)<<0)	\
	|((((CONFIG_HCLK_FREQ/CONFIG_NFC_FREQ)-1)&0x0F)<<6)	\
	|(((((ACFG_CLK_FREQ/CONFIG_H264_FREQ)-2)*2)&0x3F)<<10)\
	|(((((ACFG_CLK_FREQ/CONFIG_SSI1_FREQ)-2)*2)&0x3F)<<16)\
	|(((CONFIG_CLK0_DIV)&0x07)<<22)\
	|(((CONFIG_CLK0_EN)&0x01)<<25)\
	|(((((ACFG_CLK_FREQ/CONFIG_SSI2_FREQ)-2)*2)&0x3F)<<26))

/* PERCLKx  */
#define ACFG_PCDR1_VAL\
	(((((ACFG_CLK_FREQ/CONFIG_PERIF1_FREQ)-1)&0x3F)<<0)	\
	|((((ACFG_CLK_FREQ/CONFIG_PERIF2_FREQ)-1)&0x3F)<<8)	\
	|((((ACFG_CLK_FREQ/CONFIG_PERIF3_FREQ)-1)&0x3F)<<16)	\
	|((((ACFG_CLK_FREQ/CONFIG_PERIF4_FREQ)-1)&0x3F)<<24))

/* SDRAM controller programming Values */
#if (((2*ACFG_SDRAM_CLOCK_CYCLE_CL_1) > (3*ACFG_2XHCLK_LGTH)) || \
	(ACFG_SDRAM_CLOCK_CYCLE_CL_1 < 1))
#define REG_FIELD_SCL_VAL 3
#define REG_FIELD_SCLIMX_VAL 0
#else
#define REG_FIELD_SCL_VAL\
	((2*ACFG_SDRAM_CLOCK_CYCLE_CL_1+ACFG_2XHCLK_LGTH-1)/ \
		ACFG_2XHCLK_LGTH)
#define REG_FIELD_SCLIMX_VAL REG_FIELD_SCL_VAL
#endif

#if ((2*ACFG_SDRAM_RC_DELAY) > (16*ACFG_2XHCLK_LGTH))
#define REG_FIELD_SRC_VAL 0
#else
#define REG_FIELD_SRC_VAL\
	((2*ACFG_SDRAM_RC_DELAY+ACFG_2XHCLK_LGTH-1)/ \
		ACFG_2XHCLK_LGTH)
#endif

/* TBD Power down timer ; PRCT Bit Field Encoding; burst length 8 ; FP = 0*/
#define REG_ESDCTL_BASE_CONFIG (0x80020485\
				| (((ACFG_SDRAM_NUM_ROW-11)&0x7)<<24)\
				| (((ACFG_SDRAM_NUM_COL-8)&0x3)<<20)\
				| (((ACFG_SDRAM_REFRESH)&0x7)<<13))

#define ACFG_NORMAL_RW_CMD	((0x0<<28)+REG_ESDCTL_BASE_CONFIG)
#define ACFG_PRECHARGE_CMD	((0x1<<28)+REG_ESDCTL_BASE_CONFIG)
#define ACFG_AUTOREFRESH_CMD	((0x2<<28)+REG_ESDCTL_BASE_CONFIG)
#define ACFG_SET_MODE_REG_CMD	((0x3<<28)+REG_ESDCTL_BASE_CONFIG)

/* ESDRAMC Configuration Registers : force CL=3 to lpddr */
#define ACFG_SDRAM_ESDCFG_REGISTER_VAL (0x0\
	| (((((2*ACFG_SDRAM_EXIT_PWD+ACFG_2XHCLK_LGTH-1)/ \
		ACFG_2XHCLK_LGTH)-1)&0x3)<<21)\
	| (((ACFG_SDRAM_W2R_DELAY-1)&0x1)<<20)\
	| (((((2*ACFG_SDRAM_ROW_PRECHARGE_DELAY+ \
		ACFG_2XHCLK_LGTH-1)/ACFG_2XHCLK_LGTH)-1)&0x3)<<18) \
	| (((ACFG_SDRAM_TMRD_DELAY-1)&0x3)<<16)\
	| (((ACFG_SDRAM_TWR_DELAY)&0x1)<<15)\
	| (((((2*ACFG_SDRAM_RAS_DELAY+ACFG_2XHCLK_LGTH-1)/ \
		ACFG_2XHCLK_LGTH)-1)&0x7)<<12) \
	| (((((2*ACFG_SDRAM_RRD_DELAY+ACFG_2XHCLK_LGTH-1)/ \
		ACFG_2XHCLK_LGTH)-1)&0x3)<<10) \
	| (((REG_FIELD_SCLIMX_VAL)&0x3)<<8)\
	| (((((2*ACFG_SDRAM_RCD_DELAY+ACFG_2XHCLK_LGTH-1)/ \
		ACFG_2XHCLK_LGTH)-1)&0x7)<<4) \
	| (((REG_FIELD_SRC_VAL)&0x0F)<<0))

/* Issue Mode register Command to SDRAM */
#define ACFG_SDRAM_MODE_REGISTER_VAL\
	((((ACFG_SDRAM_BURST_LENGTH)&0x7)<<(0))\
	| (((REG_FIELD_SCL_VAL)&0x7)<<(4))\
	| ((0)<<(3)) /* sequentiql access */ \
	/*| (((ACFG_SDRAM_SINGLE_ACCESS)&0x1)<<(1))*/)

/* Issue Extended Mode register Command to SDRAM */
#define ACFG_SDRAM_EXT_MODE_REGISTER_VAL\
	((ACFG_SDRAM_PARTIAL_ARRAY_SR<<0)\
	| (ACFG_SDRAM_DRIVE_STRENGH<<(5))\
	| (1<<(ACFG_SDRAM_NUM_COL+ACFG_SDRAM_NUM_ROW+1+2)))

/* Issue Precharge all Command to SDRAM */
#define ACFG_SDRAM_PRECHARGE_ALL_VAL (1<<10)

#endif /* __APF27_H */
